#include "sway/barbar-sway-window.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk4-layer-shell.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"
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
 * BarBarSwayWindow:
 *
 * A widget to display the current window on the screen.
 */
struct _BarBarSwayWindow {
  GtkWidget parent_instance;

  char *output_name;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zxdg_output_v1 *xdg_output;
  BarBarSwaySubscribe *sub;

  // struct wl_output *output;

  GtkWidget *label;
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

static void g_barbar_sway_window_start(GtkWidget *widget);
static void g_barbar_sway_set_focused_tree(BarBarSwayWindow *sway,
                                           JsonReader *reader);

static void g_barbar_sway_window_set_output(BarBarSwayWindow *sway,
                                            const gchar *output) {
  g_return_if_fail(BARBAR_IS_SWAY_WINDOW(sway));

  if (g_set_str(&sway->output_name, output)) {
    g_object_notify_by_pspec(G_OBJECT(clock), sway_window_props[PROP_OUTPUT]);
  }
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

static void g_barbar_sway_window_finalize(GObject *object) {
  BarBarSwayWindow *window = BARBAR_SWAY_WINDOW(object);

  g_clear_object(&window->sub);
  g_free(window->output_name);

  G_OBJECT_CLASS(g_barbar_sway_window_parent_class)->finalize(object);
}

static void g_barbar_sway_window_class_init(BarBarSwayWindowClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sway_window_set_property;
  gobject_class->get_property = g_barbar_sway_window_get_property;
  gobject_class->finalize = g_barbar_sway_window_finalize;

  widget_class->root = g_barbar_sway_window_start;

  /**
   * BarBarSwayWindow:output:
   *
   * What screen this is monitoring
   */
  sway_window_props[PROP_OUTPUT] =
      g_param_spec_string("output", NULL, NULL, NULL, G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    sway_window_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "sway-window");
}

static void g_barbar_sway_window_init(BarBarSwayWindow *self) {
  self->label = gtk_label_new("");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

static void g_barbar_sway_handle_window_change(uint32_t type,
                                               JsonParser *parser,
                                               gpointer data) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "current");
  g_barbar_sway_set_focused_tree(sway, reader);
  json_reader_end_member(reader);
}

static void g_barbar_sway_set_focused_con(BarBarSwayWindow *sway,
                                          JsonReader *reader) {
  json_reader_read_member(reader, "type");
  const char *type = json_reader_get_string_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "focused");
  gboolean focused = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  if (focused && !g_strcmp0(type, "con")) {
    json_reader_read_member(reader, "name");
    const char *name = json_reader_get_string_value(reader);
    json_reader_end_member(reader);

    gtk_label_set_text(GTK_LABEL(sway->label), name);
  }
}

static void g_barbar_sway_set_focused_tree(BarBarSwayWindow *sway,
                                           JsonReader *reader) {
  json_reader_read_member(reader, "type");
  const char *type = json_reader_get_string_value(reader);
  json_reader_end_member(reader);

  if (!g_strcmp0(type, "workspace")) {
    json_reader_read_member(reader, "output");
    const char *output = json_reader_get_string_value(reader);
    json_reader_end_member(reader);
    if (g_strcmp0(output, sway->output_name)) {
      return;
    }

    if (!json_reader_read_member(reader, "nodes")) {
      return;
    }
    if (json_reader_is_array(reader)) {
      int n = json_reader_count_elements(reader);
      for (int i = 0; i < n; i++) {
        json_reader_read_element(reader, i);
        g_barbar_sway_set_focused_con(sway, reader);
        json_reader_end_element(reader);
      }
    }
    json_reader_end_member(reader);
  } else {
    gboolean nodes = json_reader_read_member(reader, "nodes");
    json_reader_end_member(reader);
    if (!nodes) {
      return;
    }
    if (json_reader_is_array(reader)) {
      int n = json_reader_count_elements(reader);
      for (int i = 0; i < n; i++) {
        json_reader_read_element(reader, i);
        g_barbar_sway_set_focused_tree(sway, reader);
        json_reader_end_element(reader);
      }
    }
    json_reader_end_member(reader);
  }
}

static void g_barbar_sway_initial_window(BarBarSwayWindow *sway, gchar *payload,
                                         uint32_t len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway window: Failed to parse json: %s", err->message);
    g_error_free(err);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  g_barbar_sway_set_focused_tree(sway, reader);

  g_object_unref(reader);
}

static void event_listner(BarBarSwaySubscribe *sub, guint type,
                          JsonParser *parser, gpointer data) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);
  g_barbar_sway_handle_window_change(type, parser, sway);
}

static void tree_cb(GObject *object, GAsyncResult *res, gpointer data) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_oneshot_finish(res, NULL, &str, &len, &error);

  if (error) {
    g_printerr("Failed to get workspaces: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (ret) {
    g_barbar_sway_initial_window(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {

  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);
  if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    sway->xdg_output_manager =
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
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);
  sway->output_name = g_strdup(name);
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

static void g_barbar_sway_window_start(GtkWidget *widget) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_output *output;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_sway_window_parent_class)->root(widget);

  gdk_display = gdk_display_get_default();

  sway->sub =
      BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"workspace\"]"));

  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(sway), GTK_TYPE_WINDOW));
  // doesn't need to be a layer window
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    printf("Parent window not found!\n");
    return;
  }

  monitor = gtk_layer_get_monitor(window);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, sway);
  wl_display_roundtrip(wl_display);

  if (!sway->xdg_output_manager) {
    g_warning("Couldn't init the xdg output manager");
    return;
  }

  sway->xdg_output =
      zxdg_output_manager_v1_get_xdg_output(sway->xdg_output_manager, output);

  zxdg_output_v1_add_listener(sway->xdg_output, &xdg_output_listener, sway);
  wl_display_roundtrip(wl_display);

  g_barbar_sway_ipc_oneshot(SWAY_GET_TREE, TRUE, NULL, tree_cb, sway, "");
}

/**
 * g_barbar_sway_window_new:
 *
 * Returns: (transfer full): a new `BarBarSwayWindow`
 */
GtkWidget *g_barbar_sway_window_new(void) {
  BarBarSwayWindow *window;

  window = g_object_new(BARBAR_TYPE_SWAY_WINDOW, NULL);

  return GTK_WIDGET(window);
}
