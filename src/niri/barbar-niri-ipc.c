#include "niri/barbar-niri-ipc.h"
#include "barbar-error.h"
#include "gio/gio.h"

/**
 * g_barbar_niri_ipc_connect:
 * @error: (out) (optional):  a #GError, or %NULL
 *
 * Create a new connection to the sway ipc.
 * To send data use g_output_stream_write_all_async()
 * sending a json request and g_output_stream_write_all_finish()
 * to finish the request.
 *
 * Returns: (transfer full): A #GSocketConnection or %NULL on failure
 */
GSocketConnection *g_barbar_niri_ipc_connect(GError **error) {
  GSocketClient *socket_client;
  GSocketConnection *connection;

  const char *socket_path = getenv("NIRI_SOCKET");

  if (!socket_path) {
    g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_NIRI_IPC,
                "No socket path found, is niri running?");
    return NULL;
  }

  socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);

  g_object_unref(address);
  g_object_unref(socket_client);

  return connection;
}

void g_barbar_niri_ipc_change_workspace(int id) {
  const char *action_str = "{Action: {FocusWorkspace: reference: {Id: %d}}}";
  char *action = g_strdup_printf(action_str, id);
}

typedef struct NiriIpcAction {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  char *message;
  char *response;
  gsize length;
} NiriIpcAction;

static void g_sway_niri_action_free(NiriIpcAction *ipc) {
  g_clear_object(&ipc->socket_client);
  g_clear_object(&ipc->connection);
  g_free(ipc->message);
  g_free(ipc->response);
  g_free(ipc);
}

static void cmd_close(GObject *source, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  NiriIpcAction *ipc = data;
  g_io_stream_close_finish(G_IO_STREAM(source), res, &error);

  if (error) {
    g_warning("Couldn't close socket: %s\n", error->message);
    g_error_free(error);
  }
  g_sway_niri_action_free(ipc);
}

static void ipc_done(NiriIpcAction *ipc) {
  g_io_stream_close_async(G_IO_STREAM(ipc->connection), 0, NULL, cmd_close,
                          ipc);
}

static void send_cb(GObject *source, GAsyncResult *res, gpointer data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source);
  GTask *task = data;
  GError *error = NULL;
  gsize bytes;
  // NiriIpcAction *ipc = g_task_get_task_data(task);

  g_output_stream_write_all_finish(stream, res, &bytes, &error);

  if (error) {
    g_task_return_error(task, error);
    g_object_unref(task);
    return;
  }
  g_task_return_boolean(task, TRUE);
  g_object_unref(task);

  // TODO: Maybe read the output to see result
}

static void connect_cb(GObject *source, GAsyncResult *res, gpointer data) {
  GTask *task = data;
  GError *error = NULL;
  GSocketClient *socket_client = G_SOCKET_CLIENT(source);
  GOutputStream *output_stream;
  NiriIpcAction *ipc = g_task_get_task_data(task);

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
 * g_barbar_niri_ipc_oneshot_finish:
 * @stream: a #GOutputStream
 * @result: a #GAsyncResult
 * @error: a #GError, or %NULL
 *
 * Finishes an asynchronous niri ipc response that was started
 * with g_barbar_sway_ipc_read_async(). The data is always
 * zero-terminated. The returned @contents should be freed with g_free()
 * when no longer needed.
 *
 * Returns: %TRUE if the ipc was successfully communicated (the ipc content can
 * still contain an error). If %FALSE and @error is present, it will be set
 * appropriately.
 */
gboolean g_barbar_niri_ipc_oneshot_finish(GOutputStream *stream,
                                          GAsyncResult *result,
                                          GError **error) {
  GTask *task;
  gboolean res;

  task = G_TASK(result);
  res = g_task_propagate_boolean(task, error);

  return res;
}

/**
 * g_barbar_niri_ipc_oneshot:
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: (scope async): a #GAsyncReadyCallback
 *     to call when the request is satisfied
 * @data: the data to pass to callback function
 * @format: a standard `printf()` format string, to send to sway
 * @...: the arguments to insert in the output
 *
 * Opens a new connection and sends a command to niri async. Use
 * g_barbar_niri_ipc_oneshot_finish() in the callback function to get the
 * result. After the message is sent, all data is freed and connections are
 * closed.
 *
 */
void g_barbar_niri_ipc_oneshot(GCancellable *cancellable,
                               GAsyncReadyCallback callback, gpointer data,
                               const char *format, ...) {
  va_list args;
  GTask *task;

  NiriIpcAction *ipc = g_malloc0(sizeof(NiriIpcAction));

  const char *socket_path = getenv("NIRI_SOCKET");

  task = g_task_new(NULL, cancellable, callback, data);

  if (!socket_path) {
    g_task_return_new_error(task, BARBAR_ERROR, BARBAR_ERROR_BAD_SWAY_IPC,
                            "No socket path found, is sway running?");
    g_object_unref(task);
    g_sway_niri_action_free(ipc);
    return;
  }

  ipc->socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  va_start(args, format);
  ipc->message = g_strdup_vprintf(format, args);
  va_end(args);

  g_task_set_task_data(task, ipc, (GDestroyNotify)ipc_done);

  g_socket_client_connect_async(ipc->socket_client,
                                G_SOCKET_CONNECTABLE(address), cancellable,
                                connect_cb, task);
  g_object_unref(address);
}
