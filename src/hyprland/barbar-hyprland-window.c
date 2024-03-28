#include "barbar-hyprland-window.h"
#include "barbar-hyprland-ipc.h"
#include "gtk4-layer-shell.h"
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

/**
 * BarBarHyprlandWindow:
 *
 * Display the name of the current window for the current screen or
 * or the global active window
 */
struct _BarBarHyprlandWindow {
  GtkWidget parent_instance;

  char *output_name;
  gboolean focused;
  gboolean global;

  GSocketConnection *listener;

  GtkWidget *label;
};

// struct entry {
//   int id;
//   int num;
//
//   GtkWidget *button;
// };

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarHyprlandWindow, g_barbar_hyprland_window, GTK_TYPE_WIDGET)

static GParamSpec *hypr_window_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_hyprland_window_map(GtkWidget *widget);

static void g_barbar_hyprland_window_set_output(BarBarHyprlandWindow *hypr,
                                                const gchar *output) {
  g_return_if_fail(BARBAR_IS_HYPRLAND_WINDOW(hypr));

  g_free(hypr->output_name);

  if (output) {
    hypr->output_name = strdup(output);
  }
  g_object_notify_by_pspec(G_OBJECT(hypr), hypr_window_props[PROP_OUTPUT]);
}

static void g_barbar_hyprland_window_set_property(GObject *object,
                                                  guint property_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec) {
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_hyprland_window_set_output(hypr, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_hyprland_window_get_property(GObject *object,
                                                  guint property_id,
                                                  GValue *value,
                                                  GParamSpec *pspec) {
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, hypr->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
g_barbar_hyprland_window_class_init(BarBarHyprlandWindowClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_hyprland_window_set_property;
  gobject_class->get_property = g_barbar_hyprland_window_get_property;
  widget_class->root = g_barbar_hyprland_window_map;

  hypr_window_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, "WL-1", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    hypr_window_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "hypr-window");
}

static void g_barbar_hyprland_window_init(BarBarHyprlandWindow *self) {
  self->label = gtk_label_new("");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

static void g_barbar_hyprland_window_set_window(BarBarHyprlandWindow *self,
                                                const gchar *title) {
  if (self->global || self->focused) {
    gtk_label_set_text(GTK_LABEL(self->label), title);
  }
}

static void g_barbar_hyprland_window_callback(uint32_t type, char *args,
                                              gpointer data) {
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(data);

  switch (type) {
  case HYPRLAND_ACTIVEWINDOW: {
    gchar **tokens = g_strsplit(args, ",", -1);
    g_barbar_hyprland_window_set_window(hypr, tokens[1]);
    g_strfreev(tokens);
    break;
  }
  case HYPRLAND_FOCUSEDMON: {
    gchar **tokens = g_strsplit(args, ",", -1);
    if (g_strcmp0(hypr->output_name, tokens[0])) {
      hypr->focused = TRUE;
    }
    g_strfreev(tokens);
    break;
  }
  }
}

static void parse_initional_workspaces(BarBarHyprlandWindow *hypr,
                                       GSocketConnection *ipc) {
  JsonParser *parser;
  GInputStream *input_stream;
  GError *err = NULL;
  parser = json_parser_new();

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc));
  gboolean ret = json_parser_load_from_stream(parser, input_stream, NULL, &err);

  if (!ret) {
    g_printerr("Hyprland workspace: Failed to parse json: %s", err->message);
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "title");
  const char *title = json_reader_get_string_value(reader);
  gtk_label_set_text(GTK_LABEL(hypr->label), title);
  json_reader_end_member(reader);

  g_object_unref(reader);
  g_object_unref(parser);
}

static void parse_initional_monitor(BarBarHyprlandWindow *hypr,
                                    GSocketConnection *ipc) {
  JsonParser *parser;
  GInputStream *input_stream;
  GError *err = NULL;
  parser = json_parser_new();

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc));
  gboolean ret = json_parser_load_from_stream(parser, input_stream, NULL, &err);

  if (!ret) {
    g_printerr("Hyprland workspace: Failed to parse json: %s", err->message);
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "monitor");
  const char *monitor = json_reader_get_string_value(reader);
  if (!g_strcmp0(hypr->output_name, monitor)) {
    printf("true!\n");
    hypr->focused = TRUE;
  }
  json_reader_end_member(reader);

  g_object_unref(reader);
  g_object_unref(parser);
}

static void g_barbar_hyprland_window_map(GtkWidget *widget) {
  GError *error = NULL;
  GSocketConnection *ipc;

  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(widget);

  ipc = g_barbar_hyprland_ipc_controller(&error);

  if (error) {
    g_printerr("Hyprland workspace: Error connecting to the ipc: %s",
               error->message);
    return;
  }
  g_barbar_hyprland_ipc_send_command(ipc, "j/activeworkspace", &error);
  if (error) {
    g_printerr(
        "Hyprland workspace: Error sending command for initial workspace: %s",
        error->message);
    return;
  }

  parse_initional_monitor(hypr, ipc);
  g_object_unref(ipc);

  ipc = g_barbar_hyprland_ipc_controller(&error);

  if (error) {
    g_printerr("Hyprland workspace: Error connecting to the ipc: %s",
               error->message);
    return;
  }

  g_barbar_hyprland_ipc_send_command(ipc, "j/activewindow", &error);
  if (error) {
    g_printerr(
        "Hyprland workspace: Error sending command for initial window: %s",
        error->message);
    return;
  }
  parse_initional_workspaces(hypr, ipc);
  g_object_unref(ipc);

  hypr->listener = g_barbar_hyprland_ipc_listner(
      g_barbar_hyprland_window_callback, hypr, NULL, &error);

  if (error) {
    g_printerr("error setting up listner: %s\n", error->message);
  }
}
