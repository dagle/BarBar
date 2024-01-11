#include "barbar-hyprland-workspace.h"
#include "barbar-hyprland-ipc.h"
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

// TODO:  We need to match against names and not just numbers
// because if the namespace is named, it isn't a number

struct _BarBarHyprlandWorkspace {
  GtkWidget parent_instance;

  char *output_name;

  struct wl_output *output;

  // struct zriver_status_manager_v1 *status_manager;
  // struct wl_seat *seat;
  // struct wl_output *output;
  // struct zriver_seat_status_v1 *seat_listener;
  // gboolean focused;

  // This should be configureable, this isn't hardcoded
  // GtkWidget *buttons[10];
  GList *workspaces; // A list of workspaces;
};

struct workspace {
  int id;
  int num;

  gboolean visible;
  gboolean urgent;
  gboolean focused;

  char *name;
  char *output;
};

struct entry {
  int id;
  int num;

  GtkWidget *button;
};

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarHyprlandWorkspace, g_barbar_hyprland_workspace,
              GTK_TYPE_WIDGET)

static GParamSpec *hypr_workspace_props[NUM_PROPERTIES] = {
    NULL,
};

static guint click_signal;

static void g_barbar_hyprland_workspace_map(GtkWidget *widget);
static void default_clicked_handler(BarBarHyprlandWorkspace *hypr,
                                    guint workspace, gpointer user_data);

// GtkWidget *g_barbar_hyprland_add_button(BarBarHyprlandWorkspace *hypr,
//                                     struct workspace *workspace);

static void
g_barbar_hyprland_workspace_set_output(BarBarHyprlandWorkspace *hypr,
                                       const gchar *output) {
  g_return_if_fail(BARBAR_IS_HYPRLAND_WORKSPACE(hypr));

  g_free(hypr->output_name);

  if (output) {
    hypr->output_name = strdup(output);
  }
  g_object_notify_by_pspec(G_OBJECT(hypr), hypr_workspace_props[PROP_OUTPUT]);
}

static void g_barbar_hyprland_workspace_set_property(GObject *object,
                                                     guint property_id,
                                                     const GValue *value,
                                                     GParamSpec *pspec) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_hyprland_workspace_set_output(hypr, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_hyprland_workspace_get_property(GObject *object,
                                                     guint property_id,
                                                     GValue *value,
                                                     GParamSpec *pspec) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, hypr->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
g_barbar_hyprland_workspace_class_init(BarBarHyprlandWorkspaceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_hyprland_workspace_set_property;
  gobject_class->get_property = g_barbar_hyprland_workspace_get_property;
  widget_class->root = g_barbar_hyprland_workspace_map;

  hypr_workspace_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    hypr_workspace_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "hypr-workspace");
}

static void g_barbar_hyprland_workspace_init(BarBarHyprlandWorkspace *self) {}

static void default_clicked_handler(BarBarHyprlandWorkspace *sway, guint tag,
                                    gpointer user_data) {
  // send a clicky clock
}

static void g_barbar_hyprland_workspace_callback(uint32_t type, char *args,
                                                 gpointer data) {
  switch (type) {
  case HYPRLAND_WORKSPACE:
    printf("workspace\n");
    printf("%s\n", args);
    break;
  case HYPRLAND_CREATEWORKSPACE:
    printf("create\n");
    break;
  case HYPRLAND_DESTROYWORKSPACE:
    printf("delete\n");
    break;
  case HYPRLAND_MOVEWORKSPACE:
    printf("move\n");
    break;
  case HYPRLAND_RENAMEWORKSPACE:
    printf("rename\n");
    break;
  }
}
static void parse_initional_workspaces(BarBarHyprlandWorkspace *hypr,
                                       GSocketConnection *ipc) {
  JsonParser *parser;
  GInputStream *input_stream;
  GError *err = NULL;
  parser = json_parser_new();

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc));
  gboolean ret = json_parser_load_from_stream(parser, input_stream, NULL, &err);

  if (!ret) {
    printf("json error: %s\n", err->message);
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  if (json_reader_is_array(reader)) {
    int n = json_reader_count_elements(reader);
    for (int j = 0; j < n; j++) {
      json_reader_read_member(reader, "id");
      int id = json_reader_get_int_value(reader);
      json_reader_end_member(reader);

      json_reader_read_member(reader, "name");
      const char *name = json_reader_get_string_value(reader);
      json_reader_end_member(reader);

      json_reader_read_member(reader, "monitor");
      const char *monitor = json_reader_get_string_value(reader);
      json_reader_end_member(reader);

      json_reader_read_member(reader, "monitorID");
      int monitor_id = json_reader_get_int_value(reader);
      json_reader_end_member(reader);
      json_reader_end_element(reader);
    }
    g_object_unref(reader);
    g_object_unref(parser);
  }

  static void g_barbar_hyprland_workspace_map(GtkWidget * widget) {
    GdkDisplay *gdk_display;
    GdkMonitor *monitor;

    GError *error = NULL;
    // gchar *buf = NULL;
    // int len;

    BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(widget);
    // BarBarHyprlandIpc *ipc;

    gdk_display = gdk_display_get_default();

    GtkNative *native = gtk_widget_get_native(GTK_WIDGET(hypr));
    GdkSurface *surface = gtk_native_get_surface(native);
    monitor = gdk_display_get_monitor_at_surface(gdk_display, surface);
    hypr->output = gdk_wayland_monitor_get_wl_output(monitor);
    GSocketConnection *ipc = g_barbar_hyprland_ipc_controller(&error);

    if (error) {
      printf("error: %s\n", error->message);
      return;
    }

    g_barbar_hyprland_ipc_send_command(ipc, "j/workspaces", &error);
    if (error) {
      printf("error: %s\n", error->message);
      return;
    }
    gchar *str = g_barbar_hyprland_ipc_message_resp(ipc, &error);
    printf("str: %s\n", str);

    GSocketConnection *con = g_barbar_hyprland_ipc_listner(
        g_barbar_hyprland_workspace_callback, NULL, NULL, &error);

    // TODO: We need to get the output->name, we can't really do that atm
    // because gtk4 binds to the wl_output interface version 3, which doesn't
    // support this. This will change in future. For now the user needs to
    // specify
    // the output. This will be fixed in the future.
  }
