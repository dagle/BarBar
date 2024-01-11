#include "barbar-hyprland-ipc.h"
#include <stdint.h>
#include <stdio.h>

/**
 * g_barbar_hyprland_ipc_controller:
 * @error (out): the error, in case connection failed
 *
 * Returns: (nullable) (transfer full): the connection
 */
GSocketConnection *g_barbar_hyprland_ipc_controller(GError **error) {
  GSocketClient *socket_client;
  GSocketConnection *connection;

  const char *his = getenv("HYPRLAND_INSTANCE_SIGNATURE");

  if (!his) {
    // TODO: Error stuff
    return NULL;
  }

  char *socket_path = g_strdup_printf("/tmp/hypr/%s/.socket.sock", his);

  socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);

  g_free(socket_path);

  return connection;
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

struct BarBarHyprlandListner {
  /* GSocketConnection *connection; */
  BarBarHyprlandSubscribeCallback cb;
  gpointer *data;
};

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
  EVENT_TYPE(ld, line, focusedmon, HYPRLAND_FOCUSEDMON)
  EVENT_TYPE(ld, line, activewindow, HYPRLAND_ACTIVEWINDOW)
  EVENT_TYPE(ld, line, activewindowv2, HYPRLAND_ACTIVEWINDOWV2)
  EVENT_TYPE(ld, line, fullscreen, HYPRLAND_FULLSCREEN)
  EVENT_TYPE(ld, line, monitorremoved, HYPRLAND_MONITORREMOVED)
  EVENT_TYPE(ld, line, monitoradded, HYPRLAND_MONITORADDED)
  EVENT_TYPE(ld, line, createworkspace, HYPRLAND_CREATEWORKSPACE)
  EVENT_TYPE(ld, line, destroyworkspace, HYPRLAND_DESTROYWORKSPACE)
  EVENT_TYPE(ld, line, moveworkspace, HYPRLAND_MOVEWORKSPACE)
  EVENT_TYPE(ld, line, renameworkspace, HYPRLAND_RENAMEWORKSPACE)
  EVENT_TYPE(ld, line, activespecial, HYPRLAND_ACTIVESPECIAL)
  EVENT_TYPE(ld, line, activelayout, HYPRLAND_ACTIVELAYOUT)
  EVENT_TYPE(ld, line, openwindow, HYPRLAND_OPENWINDOW)
  EVENT_TYPE(ld, line, closewindow, HYPRLAND_CLOSEWINDOW)
  EVENT_TYPE(ld, line, movewindow, HYPRLAND_MOVEWINDOW)
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

  const char *his = getenv("HYPRLAND_INSTANCE_SIGNATURE");

  if (!his) {
    // TODO: Error stuff
    return NULL;
  }
  // /tmp/hypr/[HIS]/.socket2.sock
  char *socket_path = g_strdup_printf("/tmp/hypr/%s/.socket2.sock", his);

  socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);

  g_free(socket_path);

  if (!connection) {
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
