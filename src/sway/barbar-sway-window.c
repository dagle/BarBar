#include "sway/barbar-sway-window.h"
#include "sway/barbar-sway-ipc.h"
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarSwayWorkspace {
  GtkWidget parent_instance;

  char *output_name;

  struct wl_output *output;

  GtkWidget *window;
};

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayWindow, g_barbar_sway_window, GTK_TYPE_WIDGET)

static GParamSpec *sway_window_props[NUM_PROPERTIES] = {
    NULL,
};

// static guint click_signal;

static void g_barbar_sway_window_constructed(GObject *object);
static void default_clicked_handler(BarBarSwayWorkspace *sway, guint workspace,
                                    gpointer user_data);

static void g_barbar_sway_window_set_output(BarBarSwayWorkspace *sway,
                                            const gchar *output) {
  g_return_if_fail(BARBAR_IS_SWAY_WORKSPACE(sway));

  g_free(sway->output_name);

  sway->output_name = strdup(output);
  g_object_notify_by_pspec(G_OBJECT(clock), sway_window_props[PROP_OUTPUT]);
}

static void g_barbar_sway_window_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_sway_window_set_output(sway, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_window_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, sway->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_workspace_class_init(BarBarSwayWindowClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sway_window_set_property;
  gobject_class->get_property = g_barbar_sway_window_get_property;
  gobject_class->constructed = g_barbar_sway_window_constructed;

  sway_window_props[PROP_OUTPUT] =
      g_param_spec_string("output", NULL, NULL, NULL, G_PARAM_READWRITE);

  // click_signal = g_signal_new_class_handler(
  //     "clicked", G_TYPE_FROM_CLASS(class),
  //     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
  //     G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
  //     G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    sway_window_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "sway-window");
}

static void g_barbar_sway_window_init(BarBarSwayWindow *self) {}
static void g_barbar_sway_window_constructed(GObject *object) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(object);
}

// static void default_clicked_handler(BarBarSwayWindow *sway, guint tag,
//                                     gpointer user_data) {
//   // send a clicky clock
// }

// void g_barbar_sway_read_workspace(JsonReader *reader,
//                                   struct workspace *workspace) {
//   json_reader_read_member(reader, "id");
//   workspace->id = json_reader_get_int_value(reader);
//   json_reader_end_member(reader);
//
//   json_reader_read_member(reader, "num");
//   workspace->num = json_reader_get_int_value(reader);
//   json_reader_end_member(reader);
//
//   json_reader_read_member(reader, "visible");
//   workspace->visible = json_reader_get_boolean_value(reader);
//   json_reader_end_member(reader);
//
//   json_reader_read_member(reader, "urgent");
//   workspace->urgent = json_reader_get_boolean_value(reader);
//   json_reader_end_member(reader);
//
//   json_reader_read_member(reader, "focused");
//   workspace->focused = json_reader_get_boolean_value(reader);
//   json_reader_end_member(reader);
//
//   json_reader_read_member(reader, "name");
//   workspace->name = json_reader_get_string_value(reader);
//   json_reader_end_member(reader);
//
//   json_reader_read_member(reader, "output");
//   workspace->output = json_reader_get_string_value(reader);
//   json_reader_end_member(reader);
// }

static void g_barbar_sway_handle_window_change(gchar *payload, uint32_t len,
                                               uint32_t type, gpointer data) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);
  //
  // parser = json_parser_new();
  // ret = json_parser_load_from_data(parser, payload, len, &err);
  //
  // if (!ret) {
  //   printf("json error: %s\n", err->message);
  // }
  //
  // JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  //
  // json_reader_read_member(reader, "change");
  // const char *change = json_reader_get_string_value(reader);
  // json_reader_end_member(reader);
  //
  // if (!strcmp(change, "init")) {
  //   g_barbar_sway_workspace_add(sway, reader);
  // } else if (!strcmp(change, "empty")) {
  //   g_barbar_sway_workspace_empty(sway, reader);
  // } else if (!strcmp(change, "focus")) {
  //   g_barbar_sway_workspace_focus(sway, reader);
  // } else if (!strcmp(change, "move")) {
  //   g_barbar_sway_workspace_move(sway, reader);
  // } else if (!strcmp(change, "rename")) {
  //   printf("%.*s\n", len, payload);
  //   g_barbar_sway_workspace_rename(sway, reader);
  // } else if (!strcmp(change, "urgent")) {
  //   g_barbar_sway_workspace_urgent(sway, reader);
  // } else if (!strcmp(change, "reload")) {
  //   g_barbar_sway_workspace_reload(sway, reader);
  // }
}

void g_barbar_sway_workspace_start(BarBarSwayWorkspace *sway) {
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;

  GError *error = NULL;
  gchar *buf = NULL;
  int len;
  BarBarSwayIpc *ipc;

  const char *intrest = "[\"workspace\"]";

  gdk_display = gdk_display_get_default();

  GtkNative *native = gtk_widget_get_native(GTK_WIDGET(sway));
  GdkSurface *surface = gtk_native_get_surface(native);
  monitor = gdk_display_get_monitor_at_surface(gdk_display, surface);
  sway->output = gdk_wayland_monitor_get_wl_output(monitor);

  // TODO: We need to get the output->name, we can't really do that atm
  // because gtk4 binds to the wl_output interface version 3, which doesn't
  // this. This will change in future. For now the user needs to specify the
  // output.

  ipc = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    printf("Error: %s\n", error->message);
    // TODO: Error stuff
    return;
  }
  // g_barbar_sway_ipc_subscribe(connection, payload);
  g_barbar_sway_ipc_send(ipc, SWAY_GET_WORKSPACES, "");
  len = g_barbar_sway_ipc_read(ipc, &buf, NULL);
  if (len > 0) {
    g_barbar_sway_handle_workspaces(sway, buf, len);

    g_free(buf);
  }

  g_barbar_sway_ipc_subscribe(ipc, intrest, sway,
                              g_barbar_sway_handle_workspaces_change);

  // g_barbar_sway_ipc_close(ipc);
}
