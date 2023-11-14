#include "sway/barbar-sway-workspace.h"
#include "sway/barbar-sway-ipc.h"
#include <gio/gio.h>
#include <json-glib/json-glib.h>
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

  // This should be configureable, this isn't hardcoded
  GtkWidget *buttons[10];
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
  GtkWidget *btn;
  char str[2];
  for (uint32_t i = 0; i < 10; i++) {
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label("");
    // gtk_widget_set_name(btn, "tags");
    gtk_widget_set_parent(btn, GTK_WIDGET(sway));
    // g_signal_connect(btn, "clicked", G_CALLBACK(clicked), river);
    sway->buttons[i] = btn;
  }
  // sway->label = gtk_label_new("");
  // gtk_widget_set_parent(sway->label, GTK_WIDGET(sway));
  // sway->focused = FALSE;
}

static void default_clicked_handler(BarBarSwayWorkspace *sway, guint tag,
                                    gpointer user_data) {
  // send a clicky clock
}

static void g_barbar_sway_handle_workspaces(BarBarSwayWorkspace *sway,
                                            gchar *payload, gssize len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    //   printf("json error: %s\n", err->message);
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  gint i = json_reader_count_elements(reader);

  for (int j = 0; j < 10; j++) {
    gtk_widget_set_visible(sway->buttons[j], false);
  }
  for (int j = 0; j < i; j++) {
    json_reader_read_element(reader, j);
    json_reader_read_member(reader, "num");
    int num = json_reader_get_int_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "name");
    const char *name = json_reader_get_string_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "visible");
    gboolean vis = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "focused");
    gboolean focused = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "urgent");
    gboolean urgent = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "output");
    const char *output = json_reader_get_string_value(reader);
    json_reader_end_member(reader);

    gtk_button_set_label(GTK_BUTTON(sway->buttons[num - 1]), name);
    gtk_widget_set_visible(sway->buttons[num - 1], true);

    printf("workspace: %d %s %d %d %d %s\n", num, name, vis, focused, urgent,
           output);

    json_reader_end_element(reader);
  }
}

void g_barbar_sway_workspace_start(BarBarSwayWorkspace *sway) {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  GError *error = NULL;
  gchar *buf = NULL;
  int len;

  const char *intrest = "[\"workspace\"]";

  // if (!socket_path) {
  //   // TODO: Error stuff
  //   return;
  // }
  //
  // socket_client = g_socket_client_new();
  // GSocketAddress *address = g_unix_socket_address_new(socket_path);
  //
  // connection = g_socket_client_connect(
  //     socket_client, G_SOCKET_CONNECTABLE(address), NULL, &error);

  connection = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    printf("Error: %s\n", error->message);
    // TODO: Error stuff
    return;
  }
  // g_barbar_sway_ipc_subscribe(connection, payload);
  g_barbar_sway_ipc_send(connection, SWAY_GET_WORKSPACES, "");
  len = g_barbar_sway_ipc_read(connection, &buf, NULL);
  if (len > 0) {
    // printf("json: %.*s\n", len, buf);
    g_barbar_sway_ipc_send(connection, SWAY_SUBSCRIBE, intrest);
    len = g_barbar_sway_ipc_read(connection, &buf, NULL);

    g_barbar_sway_handle_workspaces(sway, buf, len);
    g_free(buf);
  }

  g_io_stream_close(G_IO_STREAM(connection), NULL, NULL);
}
