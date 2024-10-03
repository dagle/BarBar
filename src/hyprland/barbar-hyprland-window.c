#include "barbar-hyprland-window.h"
#include "barbar-hyprland-ipc.h"
#include "gtk4-layer-shell.h"
#include "hyprland/barbar-hyprland-service.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

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
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zxdg_output_v1 *xdg_output;

  BarBarHyprlandService *service;

  GtkWidget *label;
};

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

static void g_barbar_hyprland_window_finalize(GObject *object) {
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(object);

  g_free(hypr->output_name);
  G_OBJECT_CLASS(g_barbar_hyprland_window_parent_class)->finalize(object);
}

static void
g_barbar_hyprland_window_class_init(BarBarHyprlandWindowClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_hyprland_window_set_property;
  gobject_class->get_property = g_barbar_hyprland_window_get_property;
  gobject_class->finalize = g_barbar_hyprland_window_finalize;
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

static void g_barbar_hyprland_workspace_focused_monitor_callback(
    BarBarHyprlandService *service, const char *monitor, const char *workspace,
    gpointer data) {
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(data);

  if (!hypr->output_name || !g_strcmp0(hypr->output_name, monitor)) {
    hypr->focused = TRUE;
  } else {
    hypr->focused = FALSE;
  }
}

static void g_barbar_hyprland_workspace_active_window_callback(
    BarBarHyprlandService *service, const char *class, const char *title,
    gpointer data) {

  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(data);

  g_barbar_hyprland_window_set_window(hypr, title);
}

static void parse_active_workspace(BarBarHyprlandWindow *hypr,
                                   JsonParser *parser) {
  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "title");
  const char *title = json_reader_get_string_value(reader);
  gtk_label_set_text(GTK_LABEL(hypr->label), title);
  json_reader_end_member(reader);

  g_object_unref(reader);
}

static void parse_initional_monitor(BarBarHyprlandWindow *hypr,
                                    JsonParser *parser) {

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "monitor");
  const char *monitor = json_reader_get_string_value(reader);
  if (!g_strcmp0(hypr->output_name, monitor)) {
    hypr->focused = TRUE;
  }
  json_reader_end_member(reader);

  g_object_unref(reader);
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {

  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(data);
  if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    hypr->xdg_output_manager =
        wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
  }
}

static void registry_handle_global_remove(void *_data,
                                          struct wl_registry *_registry,
                                          uint32_t _name) {
  (void)_data;
  (void)_registry;
  (void)_name;
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void xdg_output_handle_logical_position(
    void *data, struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y) {}

static void xdg_output_handle_logical_size(void *data,
                                           struct zxdg_output_v1 *xdg_output,
                                           int32_t width, int32_t height) {}
static void xdg_output_handle_done(void *data,
                                   struct zxdg_output_v1 *xdg_output) {}

static void xdg_output_handle_name(void *data,
                                   struct zxdg_output_v1 *xdg_output,
                                   const char *name) {
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(data);
  g_set_str(&hypr->output_name, name);
}

static void xdg_output_handle_description(void *data,
                                          struct zxdg_output_v1 *xdg_output,
                                          const char *description) {}

static const struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_handle_logical_position,
    .logical_size = xdg_output_handle_logical_size,
    .done = xdg_output_handle_done,
    .name = xdg_output_handle_name,
    .description = xdg_output_handle_description,
};

static void window_async(GObject *source_object, GAsyncResult *res,
                         gpointer data) {
  GError *error = NULL;
  JsonParser *parser;
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(data);

  parser = g_barbar_hyprland_ipc_oneshot_finish(res, &error);

  if (error) {
    g_warning("Failed to setup hyprland active window: %s\n", error->message);
    g_clear_object(&parser);
    g_error_free(error);
    return;
  }

  parse_active_workspace(hypr, parser);
  hypr->service = g_barbar_hyprland_service_new();

  g_signal_connect(
      hypr->service, "focused-monitor",
      G_CALLBACK(g_barbar_hyprland_workspace_focused_monitor_callback), hypr);

  g_signal_connect(
      hypr->service, "active-window",
      G_CALLBACK(g_barbar_hyprland_workspace_active_window_callback), hypr);
}

static void active_async(GObject *source_object, GAsyncResult *res,
                         gpointer data) {
  GError *error = NULL;
  JsonParser *parser;
  BarBarHyprlandWindow *hypr = BARBAR_HYPRLAND_WINDOW(data);

  parser = g_barbar_hyprland_ipc_oneshot_finish(res, &error);

  if (error) {
    g_printerr("Failed to setup hyprland active workspace: %s\n",
               error->message);
    g_clear_object(&parser);
    g_error_free(error);
    return;
  }

  parse_initional_monitor(hypr, parser);
  g_object_unref(parser);
  g_barbar_hyprland_ipc_oneshot(NULL, window_async, "j/activewindow", hypr);
}

static void g_barbar_hyprland_window_map(GtkWidget *widget) {
  GTK_WIDGET_CLASS(g_barbar_hyprland_window_parent_class)->root(widget);
  GError *error = NULL;
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_output *output;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;
  GSocketConnection *ipc;
  BarBarHyprlandWindow *hypr;

  hypr = BARBAR_HYPRLAND_WINDOW(widget);

  gdk_display = gdk_display_get_default();

  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(hypr), GTK_TYPE_WINDOW));
  // doesn't need to be a layer window
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    printf("Parent window not found!\n");
    return;
  }

  monitor = gtk_layer_get_monitor(window);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, hypr);
  wl_display_roundtrip(wl_display);

  if (!hypr->xdg_output_manager) {
    g_warning("Couldn't init the xdg output manager");
    return;
  }

  hypr->xdg_output =
      zxdg_output_manager_v1_get_xdg_output(hypr->xdg_output_manager, output);

  zxdg_output_v1_add_listener(hypr->xdg_output, &xdg_output_listener, hypr);
  wl_display_roundtrip(wl_display);

  g_barbar_hyprland_ipc_oneshot(NULL, active_async, "j/activeworkspace", hypr);
}

/**
 * g_barbar_hyprland_window_new:
 *
 * Returns: (transfer none): a `BarBarHyprlandWindow`
 */
GtkWidget *g_barbar_hyprland_window_new(void) {
  BarBarHyprlandWindow *hypr;

  hypr = g_object_new(BARBAR_TYPE_HYPRLAND_WINDOW, NULL);

  return GTK_WIDGET(hypr);
}
