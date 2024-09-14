#include "river/barbar-river-layout.h"
#include "river-control-unstable-v1-client-protocol.h"
#include "river-status-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarRiverLayout {
  GtkWidget parent_instance;

  struct zriver_status_manager_v1 *status_manager;
  // struct zriver_control_v1 *control;
  struct zriver_output_status_v1 *output_status;
  struct wl_seat *seat;

  GtkWidget *layout;
};

enum {
  PROP_0,

  PROP_LAYOUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarRiverLayout, g_barbar_river_layout, GTK_TYPE_WIDGET)

static GParamSpec *river_layout_props[NUM_PROPERTIES] = {
    NULL,
};

static guint click_signal;

static void g_barbar_river_layout_start(GtkWidget *widget);

void g_barbar_river_layout_set_layout_internal(BarBarRiverLayout *self,
                                               const char *layout) {
  if (!g_strcmp0(gtk_button_get_label(GTK_BUTTON(self->layout)), layout)) {
    return;
  }

  gtk_button_set_label(GTK_BUTTON(self->layout), layout);
  g_object_notify_by_pspec(G_OBJECT(self), river_layout_props[PROP_LAYOUT]);
}

void g_barbar_river_layout_set_layout(BarBarRiverLayout *self,
                                      const char *layout) {
  g_return_if_fail(BARBAR_IS_RIVER_LAYOUT(self));

  // g_barbar_river_layout_set_layout_internal();
  g_object_notify_by_pspec(G_OBJECT(self), river_layout_props[PROP_LAYOUT]);
}

static void g_barbar_river_layout_set_property(GObject *object,
                                               guint property_id,
                                               const GValue *value,
                                               GParamSpec *pspec) {

  BarBarRiverLayout *rl = BARBAR_RIVER_LAYOUT(object);

  switch (property_id) {
  case PROP_LAYOUT:
    g_barbar_river_layout_set_layout(rl, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_river_layout_get_property(GObject *object,
                                               guint property_id, GValue *value,
                                               GParamSpec *pspec) {

  BarBarRiverLayout *rl = BARBAR_RIVER_LAYOUT(object);

  switch (property_id) {
  case PROP_LAYOUT:
    g_value_set_string(value, "hello");
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_river_layout_finalize(GObject *object) {
  BarBarRiverLayout *river = BARBAR_RIVER_LAYOUT(object);

  zriver_output_status_v1_destroy(river->output_status);

  G_OBJECT_CLASS(g_barbar_river_layout_parent_class)->finalize(object);
}

static void g_barbar_river_layout_class_init(BarBarRiverLayoutClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->finalize = g_barbar_river_layout_finalize;
  gobject_class->set_property = g_barbar_river_layout_set_property;
  gobject_class->get_property = g_barbar_river_layout_get_property;
  widget_class->root = g_barbar_river_layout_start;

  /**
   * BarBarRiverLayout:layout:
   *
   * Our current layout, should be writeable in the future
   */
  river_layout_props[PROP_LAYOUT] =
      g_param_spec_string("layout", NULL, NULL, NULL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    river_layout_props);

  click_signal =
      g_signal_new("clicked", BARBAR_TYPE_RIVER_LAYOUT,
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "river-layout");
}

static void clicked(GtkButton *self, gpointer user_data) {
  BarBarRiverLayout *layout = BARBAR_RIVER_LAYOUT(user_data);

  g_signal_emit(layout, click_signal, 0);
}

static void g_barbar_river_layout_init(BarBarRiverLayout *self) {
  self->layout = gtk_button_new();
  g_signal_connect(self->layout, "clicked", G_CALLBACK(clicked), self);
  gtk_widget_set_parent(self->layout, GTK_WIDGET(self));
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarRiverLayout *river = BARBAR_RIVER_LAYOUT(data);
  if (strcmp(interface, zriver_status_manager_v1_interface.name) == 0) {
    if (version >= ZRIVER_OUTPUT_STATUS_V1_LAYOUT_NAME_CLEAR_SINCE_VERSION) {
      river->status_manager = wl_registry_bind(
          registry, name, &zriver_status_manager_v1_interface, version);
    }
  }
  // if (strcmp(interface, zriver_control_v1_interface.name) == 0) {
  //   river->control =
  //       wl_registry_bind(registry, name, &zriver_control_v1_interface,
  //       version);
  // }
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

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void
listen_focused_tags(void *data,
                    struct zriver_output_status_v1 *zriver_output_status_v1,
                    uint32_t tags) {}
// called when a new tag gets occupied or vacant
static void
listen_view_tags(void *data,
                 struct zriver_output_status_v1 *zriver_output_status_v1,
                 struct wl_array *tag_array) {}

static void
listen_urgent_tags(void *data,
                   struct zriver_output_status_v1 *zriver_output_status_v1,
                   uint32_t tags) {}

static void layout_name(void *data,
                        struct zriver_output_status_v1 *zriver_output_status_v1,
                        const char *name) {
  BarBarRiverLayout *layout = BARBAR_RIVER_LAYOUT(data);
  g_barbar_river_layout_set_layout_internal(layout, name);
}

static void
layout_name_clear(void *data,
                  struct zriver_output_status_v1 *zriver_output_status_v1) {

  BarBarRiverLayout *layout = BARBAR_RIVER_LAYOUT(data);
}

static const struct zriver_output_status_v1_listener output_status_listener = {
    .focused_tags = listen_focused_tags,
    .view_tags = listen_view_tags,
    .urgent_tags = listen_urgent_tags,
    .layout_name = layout_name,
    .layout_name_clear = layout_name_clear,
};

static void g_barbar_river_layout_start(GtkWidget *widget) {
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_registry *wl_registry;
  struct wl_output *output;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_river_layout_parent_class)->root(widget);

  BarBarRiverLayout *river = BARBAR_RIVER_LAYOUT(widget);

  gdk_display = gdk_display_get_default();

  // This shouldn't need to be done because layered shell requires wayland
  g_return_if_fail(gdk_display);
  g_return_if_fail(GDK_IS_WAYLAND_DISPLAY(gdk_display));

  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(river), GTK_TYPE_WINDOW));
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    printf("Parent window not found!\n");
    return;
  }

  // We try to find the main screen for this widget, this should only
  // be done if no screen is specified
  monitor = gtk_layer_get_monitor(window);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, river);
  wl_display_roundtrip(wl_display);

  if (!river->status_manager) {
    return;
  }

  river->output_status = zriver_status_manager_v1_get_river_output_status(
      river->status_manager, output);

  zriver_output_status_v1_add_listener(river->output_status,
                                       &output_status_listener, river);
  wl_display_roundtrip(wl_display);

  zriver_status_manager_v1_destroy(river->status_manager);

  river->status_manager = NULL;
}

/**
 * g_barbar_river_layout_new:
 *
 * Returns: (transfer full): A `GtkWidget`
 */
GtkWidget *g_barbar_river_layout_new(void) {
  BarBarRiverLayout *self;

  self = g_object_new(BARBAR_TYPE_RIVER_LAYOUT, NULL);

  return GTK_WIDGET(self);
}
