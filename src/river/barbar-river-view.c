#include "river/barbar-river-view.h"
#include "river-control-unstable-v1-client-protocol.h"
#include "river-status-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarRiverView:
 *
 * A widget to display the river the view, the current window.
 *
 */
struct _BarBarRiverView {
  GtkWidget parent_instance;

  struct zriver_status_manager_v1 *status_manager;
  struct wl_seat *seat;
  struct wl_output *output;
  struct zriver_seat_status_v1 *seat_listener;
  gboolean focused;

  GtkWidget *label;
};

enum {
  PROP_0,

  PROP_LABEL,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarRiverView, g_barbar_river_view, GTK_TYPE_WIDGET)

static GParamSpec *river_view_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_river_view_start(GtkWidget *widget);

static void g_barbar_river_view_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {

  // BarBarRiverView *rv = BARBAR_RIVER_VIEW(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_river_view_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec) {

  BarBarRiverView *rv = BARBAR_RIVER_VIEW(object);

  switch (property_id) {
  case PROP_LABEL:
    g_value_set_object(value, rv->label);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_river_view_finalize(GObject *object) {
  BarBarRiverView *river = BARBAR_RIVER_VIEW(object);

  zriver_seat_status_v1_destroy(river->seat_listener);

  G_OBJECT_CLASS(g_barbar_river_view_parent_class)->finalize(object);
}

static void g_barbar_river_view_class_init(BarBarRiverViewClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_river_view_set_property;
  gobject_class->get_property = g_barbar_river_view_get_property;
  gobject_class->finalize = g_barbar_river_view_finalize;

  widget_class->root = g_barbar_river_view_start;

  /**
   * BarBarRiverView:label:
   *
   * The gtk label widget
   */
  river_view_props[PROP_LABEL] = g_param_spec_object(
      "label", NULL, NULL, GTK_TYPE_LABEL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    river_view_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "river-view");
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarRiverView *river = BARBAR_RIVER_VIEW(data);
  if (strcmp(interface, zriver_status_manager_v1_interface.name) == 0) {
    if (version >= ZRIVER_OUTPUT_STATUS_V1_LAYOUT_NAME_CLEAR_SINCE_VERSION) {
      river->status_manager = wl_registry_bind(
          registry, name, &zriver_status_manager_v1_interface, version);
    }
  }
  if (strcmp(interface, wl_seat_interface.name) == 0) {
    river->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
  }
}

static void registry_handle_global_remove(void *_data,
                                          struct wl_registry *_registry,
                                          uint32_t _name) {
  (void)_data;
  (void)_registry;
  (void)_name;
}

static void
listen_focused_output(void *data,
                      struct zriver_seat_status_v1 *zriver_seat_status_v1,
                      struct wl_output *output) {
  BarBarRiverView *river = BARBAR_RIVER_VIEW(data);
  if (output == river->output) {
    river->focused = TRUE;
  }
}

static void
listen_unfocused_output(void *data,
                        struct zriver_seat_status_v1 *zriver_seat_status_v1,
                        struct wl_output *output) {
  BarBarRiverView *river = BARBAR_RIVER_VIEW(data);
  if (output == river->output) {
    river->focused = FALSE;
  }
}

static void
listen_focused_view(void *data,
                    struct zriver_seat_status_v1 *zriver_seat_status_v1,
                    const char *title) {
  BarBarRiverView *river = BARBAR_RIVER_VIEW(data);
  if (river->focused) {
    gtk_label_set_text(GTK_LABEL(river->label), title);
  }
}

static void listen_mode(void *data,
                        struct zriver_seat_status_v1 *zriver_seat_status_v1,
                        const char *name) {}

static const struct zriver_seat_status_v1_listener seat_status_listener = {
    .focused_output = listen_focused_output,
    .unfocused_output = listen_unfocused_output,
    .focused_view = listen_focused_view,
    .mode = listen_mode,
};

static void g_barbar_river_view_init(BarBarRiverView *self) {
  self->label = gtk_label_new("");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
  self->focused = FALSE;
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void g_barbar_river_view_start(GtkWidget *widget) {
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_river_view_parent_class)->root(widget);

  BarBarRiverView *river = BARBAR_RIVER_VIEW(widget);

  gdk_display = gdk_display_get_default();

  // This shouldn't need to be done because layered shell requires wayland
  g_return_if_fail(gdk_display);
  g_return_if_fail(GDK_IS_WAYLAND_DISPLAY(gdk_display));

  // We try to find the main screen for this widget, this should only
  // be done if no screen is specified
  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(river), GTK_TYPE_WINDOW));
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    g_warning("Parent window not found!\n");
    return;
  }
  monitor = gtk_layer_get_monitor(window);
  river->output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, river);
  wl_display_roundtrip(wl_display);

  if (!river->status_manager) {
    return;
  }

  river->seat_listener = zriver_status_manager_v1_get_river_seat_status(
      river->status_manager, river->seat);

  zriver_seat_status_v1_add_listener(river->seat_listener,
                                     &seat_status_listener, river);
  wl_display_roundtrip(wl_display);

  zriver_status_manager_v1_destroy(river->status_manager);

  river->status_manager = NULL;
}

/**
 * g_barbar_river_view_new:
 *
 * Returns: (transfer full): A `GtkWidget`
 */
GtkWidget *g_barbar_river_view_new(void) {
  BarBarRiverView *self;

  self = g_object_new(BARBAR_TYPE_RIVER_VIEW, NULL);

  return GTK_WIDGET(self);
}
