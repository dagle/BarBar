#include "sway/barbar-sway-workspace.h"
#include <gio/gio.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarSwayWorkspace {
  GtkWidget parent_instance;

  // struct zriver_status_manager_v1 *status_manager;
  // struct wl_seat *seat;
  // struct wl_output *output;
  // struct zriver_seat_status_v1 *seat_listener;
  // gboolean focused;

  GtkWidget **buttons;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayWorkspace, g_barbar_sway_workspace, GTK_TYPE_WIDGET)

static GParamSpec *sway_workspace_props[NUM_PROPERTIES] = {
    NULL,
};

static guint click_signal;

static void g_barbar_sway_workspace_constructed(GObject *object);
static void default_clicked_handler(BarBarSwayWorkspace *sway, guint workspace,
                                    gpointer user_data);

static void g_barbar_sway_workspace_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {}

static void g_barbar_sway_workspace_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {}

static void
g_barbar_sway_workspace_class_init(BarBarSwayWorkspaceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sway_workspace_set_property;
  gobject_class->get_property = g_barbar_sway_workspace_get_property;
  gobject_class->constructed = g_barbar_sway_workspace_constructed;
  sway_workspace_props[PROP_DEVICE] =
      g_param_spec_uint("tagnums", NULL, NULL, 0, 9, 9, G_PARAM_READWRITE);

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    sway_workspace_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "sway-workspace");
}

static void g_barbar_sway_workspace_init(BarBarSwayWorkspace *self) {}
static void g_barbar_sway_workspace_constructed(GObject *object) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(object);
  // sway->label = gtk_label_new("");
  // gtk_widget_set_parent(sway->label, GTK_WIDGET(sway));
  // sway->focused = FALSE;
}

static void default_clicked_handler(BarBarSwayWorkspace *sway, guint tag,
                                    gpointer user_data) {
  // send a clicky clock
}

static void on_ready(GObject *source_object, GAsyncResult *res,
                     gpointer user_data) {
  GSocketConnection *connection = g_socket_client_connect_to_host_finish(
      G_SOCKET_CLIENT(source_object), res, NULL);
  if (connection != NULL) {
    // Connection successful, do something with the connection
    g_print("Connected to the server!\n");

    // Example: Send a message
    const gchar *message = "Hello, server!";
    GOutputStream *output_stream =
        g_io_stream_get_output_stream(G_IO_STREAM(connection));
    g_output_stream_write_all(output_stream, message, strlen(message), NULL,
                              NULL, NULL);
  } else {
    // Connection failed
    g_print("Failed to connect to the server\n");
  }
}

enum {
  SWAY_IPC_SUBSCRIBE,
};
void g_barbar_sway_ipc_send(GSocketConnection *connection, guint type,
                            const char *payload) {
  GOutputStream *output_stream =
      g_io_stream_get_output_stream(G_IO_STREAM(connection));
  guint size = strlen(payload);
  // g_output_stream_printf("")
  // g_output_stream_write_all(output_stream, message, strlen(message), NULL,
  //                             NULL, NULL);
}

void g_barbar_sway_workspace_start(BarBarSwayWorkspace *sway) {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  GError *error = NULL;

  const char *socket_path = getenv("SWAYSOCK");
  const char *payload = "[\"workspace\"]";

  if (!socket_path) {
    // TODO: Error stuff
    return;
  }

  guint length = strlen("i3-ipc") + 4 + 4;
  char *buffer = calloc(length, sizeof(char));
  memcpy(buffer, "i3-ipc", 6);
  memcpy(buffer, "i3-ipc", 6);

  printf("%s\n", buffer);

  // 00 00 00 04
  // printf("i3-ipc%08X%08X%s\n", 4, 0, "ex");
  // 00000004

  // socket_client = g_socket_client_new();

  // connection = g_socket_client_connect_to_host(socket_client, socket_path, 0,
  //                                              NULL, &error);

  // if (connection != NULL) {
  //   // TODO: Error stuff
  //   return;
  // }
  // g_barbar_sway_ipc_send(connection, SWAY_IPC_SUBSCRIBE, payload);
}

//
// #define SOCKET_PATH "/tmp/my_unix_socket"
//

// int main() {
//
//     // Create a new socket client
//
//     // Connect to the server in an asynchronous manner
//
//     // Run the main loop
//     GMainLoop *loop = g_main_loop_new(NULL, FALSE);
//     g_main_loop_run(loop);
//
//     return 0;
// }
