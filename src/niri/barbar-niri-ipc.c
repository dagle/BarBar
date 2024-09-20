#include "niri/barbar-niri-ipc.h"
#include "barbar-error.h"
#include "gio/gio.h"

/**
 * g_barbar_niri_ipc_connect:
 * @error: (out) (optional):  a #GError, or %NULL
 *
 * Create a new connection to the sway ipc.
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

/**
 * g_barbar_niri_ipc_send_command:
 */
void g_barbar_niri_ipc_send_command(void) {}

void g_barbar_niri_ipc_change_workspace(int id) {
  const char *action_str = "{Action: {FocusWorkspace: reference: {Id: %d}}}";
  char *action = g_strdup_printf(action_str, id);
}
