#include "barbar-hyprland-ipc.h"
#include "barbar-error.h"
#include "gio/gio.h"
#include "glib.h"
#include "json-glib/json-glib.h"
#include <stdint.h>
#include <stdio.h>

GSocketAddress *g_barbar_hyprland_ipc_address(const char *socket,
                                              GError **error) {
  GSocketAddress *address;
  const char *his = getenv("HYPRLAND_INSTANCE_SIGNATURE");
  if (!his) {
    g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_HYPRLAND_IPC,
                "HYPRLAND_INSTANCE_SIGNATURE not set");
    return NULL;
  }
  char *socket_path =
      g_strdup_printf("%s/hypr/%s/%s", g_get_user_runtime_dir(), his, socket);
  address = g_unix_socket_address_new(socket_path);
  g_free(socket_path);
  return address;
}

/**
 * g_barbar_hyprland_ipc_controller:
 * @error (out): the error, in case connection failed
 *
 * Returns: (nullable) (transfer full): the connection
 */
GSocketConnection *g_barbar_hyprland_ipc_controller(GError **error) {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  GSocketAddress *address;
  GError *err = NULL;

  address =
      g_barbar_hyprland_ipc_address(BARBAR_HYPERLAND_REQUEST_SOCKET, &err);

  if (err) {
    g_propagate_error(error, err);
    return NULL;
  }

  socket_client = g_socket_client_new();

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);
  g_object_unref(address);

  return connection;
}

typedef struct HyprIpc {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  char *response;
  char *message;
  JsonParser *parser;
} HyprIpc;

static void g_hyprland_ipc_free(HyprIpc *ipc) {
  g_clear_object(&ipc->socket_client);
  g_clear_object(&ipc->connection);
  g_free(ipc->message);
  g_free(ipc->response);
  g_free(ipc);
}

static void cmd_close(GObject *source, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  HyprIpc *ipc = data;
  g_io_stream_close_finish(G_IO_STREAM(source), res, &error);

  if (error) {
    g_warning("Couldn't close socket: %s\n", error->message);
    g_error_free(error);
  }
  g_hyprland_ipc_free(ipc);
}

static void ipc_done(HyprIpc *ipc) {
  g_io_stream_close_async(G_IO_STREAM(ipc->connection), 0, NULL, cmd_close,
                          ipc);
}

/**
 * g_barbar_hyprland_ipc_oneshot_finish:
 * @result: a #GAsyncResult
 * @error: a #GError, or %NULL
 *
 * Finishes an asynchronous sway ipc response that was started
 * with g_barbar_hyprland_ipc_oneshot().
 *
 * Returns: (transfer full) (nullable): A `JsonParser` or %NULL if an error.
 * In that case error is set.
 */
JsonParser *g_barbar_hyprland_ipc_oneshot_finish(GAsyncResult *result,
                                                 GError **error) {
  GTask *task;
  HyprIpc *ipc;
  // gboolean res;

  task = G_TASK(result);
  // res = g_task_propagate_boolean(task, error);

  ipc = g_task_get_task_data(task);

  return g_object_ref(ipc->parser);
}

static void json_parsed(GObject *source, GAsyncResult *result, gpointer data) {
  JsonParser *parser = JSON_PARSER(source);
  GTask *task = data;
  GError *err = NULL;

  json_parser_load_from_stream_finish(parser, result, &err);
  if (err) {
    g_task_return_error(task, err);
    g_object_unref(task);
    return;
  }
  g_task_return_boolean(task, TRUE);
  g_object_unref(task);
}

static void send_cb(GObject *source, GAsyncResult *res, gpointer data) {
  GOutputStream *output_stream = G_OUTPUT_STREAM(source);
  GInputStream *input_stream;
  GTask *task = data;
  GError *error = NULL;
  gsize bytes;
  HyprIpc *ipc = g_task_get_task_data(task);

  g_output_stream_write_all_finish(output_stream, res, &bytes, &error);

  if (error) {
    g_task_return_error(task, error);
    g_object_unref(task);
    return;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc->connection));
  ipc->parser = json_parser_new();

  json_parser_load_from_stream_async(ipc->parser, input_stream,
                                     g_task_get_cancellable(task), json_parsed,
                                     task);
}

static void connect_cb(GObject *source, GAsyncResult *res, gpointer data) {
  GTask *task = data;
  GError *error = NULL;
  GSocketClient *socket_client = G_SOCKET_CLIENT(source);
  GOutputStream *output_stream;
  HyprIpc *ipc = g_task_get_task_data(task);

  ipc->connection = g_socket_client_connect_finish(socket_client, res, &error);

  if (error) {
    g_task_return_error(task, error);
    g_object_unref(task);
    return;
  }
  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(ipc->connection));
  g_output_stream_write_all_async(output_stream, ipc->message,
                                  strlen(ipc->message), 0,
                                  g_task_get_cancellable(task), send_cb, task);
}

/**
 * g_barbar_hyprland_ipc_oneshot:
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: (scope async): a #GAsyncReadyCallback
 *     to call when the request is satisfied
 * @msg: a message to send
 * @data: the data to pass to callback function
 *
 * Opens a new connection and sends a command to hyprland async.
 * a callback is set we will try to read the output. Use
 * g_barbar_hyprland_ipc_oneshot_finish() in the callback function to get the
 * result. After the message is sent, all data is freed and connections are
 * closed.
 *
 */
void g_barbar_hyprland_ipc_oneshot(GCancellable *cancellable,
                                   GAsyncReadyCallback callback,
                                   const char *msg, gpointer data) {
  HyprIpc *ipc;
  GSocketAddress *address;
  GError *err = NULL;
  GTask *task;

  task = g_task_new(NULL, cancellable, callback, data);

  address =
      g_barbar_hyprland_ipc_address(BARBAR_HYPERLAND_REQUEST_SOCKET, &err);

  if (err) {
    g_task_return_error(task, err);
    g_object_unref(task);
    return;
  }

  ipc = g_malloc0(sizeof(HyprIpc));
  ipc->socket_client = g_socket_client_new();
  ipc->message = g_strdup(msg);

  g_task_set_task_data(task, ipc, (GDestroyNotify)ipc_done);

  g_socket_client_connect_async(ipc->socket_client,
                                G_SOCKET_CONNECTABLE(address), cancellable,
                                connect_cb, task);

  g_object_unref(address);
}

/* g_barbar_hyprland_ipc_send_command:
 * @ipc: the socket
 * @msg: message to send
 * @err (out): the error, in case sending the message failed
 *
 * Returns: True if sending the message was a success.
 */
gboolean g_barbar_hyprland_ipc_send_command(GSocketConnection *ipc, char *msg,
                                            GError **err) {
  GOutputStream *output_stream;
  gboolean ret;

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(ipc));

  ret = g_output_stream_write_all(output_stream, msg, strlen(msg), NULL, NULL,
                                  err);
  return ret;
}

/* g_barbar_hyprland_ipc_message_resp:
 * @ipc: the socket
 * @err (out): the error, in case sending the message failed
 *
 * Read the response from a command
 *
 * Returns: (transfer full): the response
 */
gchar *g_barbar_hyprland_ipc_message_resp(GSocketConnection *ipc,
                                          GError **err) {
  GInputStream *input_stream;
  GDataInputStream *data_stream;

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc));
  data_stream = g_data_input_stream_new(input_stream);

  gsize length;
  return g_data_input_stream_read_upto(data_stream, NULL, 0, &length, NULL,
                                       err);
}

#define EVENT_TYPE(cs, input, name, upper)                                     \
  if (!strncmp(input, #name ">>", sizeof(#name) + 1)) {                        \
    cs->cb(upper, line + sizeof(#name) + 1, cs->data);                         \
  }

static void g_barbar_hyprland_line_reader(GObject *object, GAsyncResult *res,
                                          gpointer data) {
  GDataInputStream *data_stream = G_DATA_INPUT_STREAM(object);
  struct BarBarHyprlandListner *ld = data;
  GError *error = NULL;
  gsize length;
  gchar *line;

  line =
      g_data_input_stream_read_line_finish(data_stream, res, &length, &error);

  EVENT_TYPE(ld, line, workspace, HYPRLAND_WORKSPACE);
  EVENT_TYPE(ld, line, workspacev2, HYPRLAND_WORKSPACEV2);
  EVENT_TYPE(ld, line, focusedmon, HYPRLAND_FOCUSEDMON)
  EVENT_TYPE(ld, line, activewindow, HYPRLAND_ACTIVEWINDOW)
  EVENT_TYPE(ld, line, activewindowv2, HYPRLAND_ACTIVEWINDOWV2)
  EVENT_TYPE(ld, line, fullscreen, HYPRLAND_FULLSCREEN)
  EVENT_TYPE(ld, line, monitorremoved, HYPRLAND_MONITORREMOVED)
  EVENT_TYPE(ld, line, monitoradded, HYPRLAND_MONITORADDED)
  EVENT_TYPE(ld, line, monitoraddedv2, HYPRLAND_MONITORADDEDV2)
  EVENT_TYPE(ld, line, createworkspace, HYPRLAND_CREATEWORKSPACE)
  EVENT_TYPE(ld, line, createworkspacev2, HYPRLAND_CREATEWORKSPACEV2)
  EVENT_TYPE(ld, line, destroyworkspace, HYPRLAND_DESTROYWORKSPACE)
  EVENT_TYPE(ld, line, destroyworkspacev2, HYPRLAND_DESTROYWORKSPACEV2)
  EVENT_TYPE(ld, line, moveworkspace, HYPRLAND_MOVEWORKSPACE)
  EVENT_TYPE(ld, line, moveworkspacev2, HYPRLAND_MOVEWORKSPACEV2)
  EVENT_TYPE(ld, line, renameworkspace, HYPRLAND_RENAMEWORKSPACE)
  EVENT_TYPE(ld, line, activespecial, HYPRLAND_ACTIVESPECIAL)
  EVENT_TYPE(ld, line, activelayout, HYPRLAND_ACTIVELAYOUT)
  EVENT_TYPE(ld, line, openwindow, HYPRLAND_OPENWINDOW)
  EVENT_TYPE(ld, line, closewindow, HYPRLAND_CLOSEWINDOW)
  EVENT_TYPE(ld, line, movewindow, HYPRLAND_MOVEWINDOW)
  EVENT_TYPE(ld, line, movewindowv2, HYPRLAND_MOVEWINDOWV2)
  EVENT_TYPE(ld, line, openlayer, HYPRLAND_OPENLAYER)
  EVENT_TYPE(ld, line, closelayer, HYPRLAND_CLOSELAYER)
  EVENT_TYPE(ld, line, submap, HYPRLAND_SUBMAP)
  EVENT_TYPE(ld, line, changefloatingmode, HYPRLAND_CHANGEFLOATINGMODE)
  EVENT_TYPE(ld, line, urgent, HYPRLAND_URGENT)
  EVENT_TYPE(ld, line, minimize, HYPRLAND_MINIMIZE)
  EVENT_TYPE(ld, line, screencast, HYPRLAND_SCREENCAST)
  EVENT_TYPE(ld, line, windowtitle, HYPRLAND_WINDOWTITLE)
  EVENT_TYPE(ld, line, ignoregrouplock, HYPRLAND_IGNOREGROUPLOCK)
  EVENT_TYPE(ld, line, lockgroups, HYPRLAND_LOCKGROUPS)

  g_data_input_stream_read_line_async(data_stream, G_PRIORITY_DEFAULT, NULL,
                                      g_barbar_hyprland_line_reader, data);
}

/**
 * g_barbar_hyprland_ipc_listner:
 * @cb: function to run on each event
 * @data: (closure): data passed to the function
 * @destroy: function to destroy data
 * @error: (out): error setting up the listner
 *
 * Returns: (transfer full): the connection
 */
GSocketConnection *
g_barbar_hyprland_ipc_listner(BarBarHyprlandSubscribeCallback cb, gpointer data,
                              GDestroyNotify destroy, GError **error) {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  GInputStream *input_stream;
  GDataInputStream *data_stream;
  GSocketAddress *address;
  GError *err = NULL;

  address = g_barbar_hyprland_ipc_address(BARBAR_HYPERLAND_EVENT_SOCKET, &err);

  if (err) {
    g_propagate_error(error, err);
    return NULL;
  }

  socket_client = g_socket_client_new();

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);
  g_object_unref(socket_client);
  g_object_unref(address);

  if (!connection) {
    g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_HYPRLAND_IPC,
                "Couldn't connect to the hyprland socket");
    return NULL;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
  data_stream = g_data_input_stream_new(input_stream);

  struct BarBarHyprlandListner *ld =
      g_malloc0(sizeof(struct BarBarHyprlandListner));

  ld->cb = cb;
  ld->data = data;

  g_data_input_stream_read_line_async(data_stream, G_PRIORITY_DEFAULT, NULL,
                                      g_barbar_hyprland_line_reader, ld);

  return connection;
}
