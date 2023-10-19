#include "barbar-river.h"
#include "river-status-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarRiver {
  GObject parent;

  struct zriver_status_manager_v1 *status_manager;
  struct zriver_output_status_v1 *output_status;

  // struct zriver_control_v1 *control_;
  // struct wl_seat *seat_;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

// static struct wl_registry *wl_registry_global = NULL;

G_DEFINE_TYPE(BarBarRiver, g_barbar_river, G_TYPE_OBJECT)

static GParamSpec *river_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_river_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {}

static void g_barbar_river_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {}

static guint click_signal;

static void g_barbar_river_class_init(BarBarRiverClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_river_set_property;
  gobject_class->get_property = g_barbar_river_get_property;
  river_props[PROP_DEVICE] =
      g_param_spec_string("path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, river_props);

  /* TODO: */
  click_signal = g_signal_new(
      "click-signal", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      0 /* class offset.Subclass cannot override the class handler (default
           handler). */
      ,
      NULL /* accumulator */, NULL /* accumulator data */,
      NULL /* C marshaller. g_cclosure_marshal_generic() will be used */,
      G_TYPE_NONE /* return_type */, 0 /* n_params */
  );
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
								   uint32_t version) {
	BarBarRiver *river = BARBAR_RIVER(data);
	if (strcmp(interface, zriver_status_manager_v1_interface.name) == 0) {
		if (version < ZRIVER_OUTPUT_STATUS_V1_LAYOUT_NAME_CLEAR_SINCE_VERSION) {
			river->status_manager = wl_registry_bind(registry, name, 
					&zriver_status_manager_v1_interface, version);
		}
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

static void listen_focused_tags(void *data, struct zriver_output_status_v1 *zriver_output_status_v1,
                                uint32_t tags) {
	g_print("focused\n");
}

static void listen_view_tags(void *data, struct zriver_output_status_v1 *zriver_output_status_v1,
                             struct wl_array *tags) {
	g_print("view\n");
}

static void listen_urgent_tags(void *data, struct zriver_output_status_v1 *zriver_output_status_v1,
                               uint32_t tags) {
	g_print("urgent\n");
}

static const struct zriver_output_status_v1_listener output_status_listener = {
    .focused_tags = listen_focused_tags,
    .view_tags = listen_view_tags,
    .urgent_tags = listen_urgent_tags,
};

static void g_barbar_river_init(BarBarRiver *self) {
  GdkDisplay *gdk_display = gdk_display_get_default ();
  g_return_if_fail (gdk_display);
  g_return_if_fail (GDK_IS_WAYLAND_DISPLAY (gdk_display));

  GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(self));
  GtkWindow *window = GTK_WINDOW(root);
  GdkMonitor *monitor = gtk_layer_get_monitor(window);
  struct wl_output *output = gdk_wayland_monitor_get_wl_output(monitor);
  
  struct wl_display *wl_display = gdk_wayland_display_get_wl_display (gdk_display);
  struct wl_registry *wl_registry_global = wl_display_get_registry (wl_display);

  wl_registry_add_listener (wl_registry_global, &wl_registry_listener, NULL);
  wl_display_roundtrip (wl_display);

  if (!self->status_manager) {
	  return;
  }

  struct zriver_output_status_v1 *output_status = zriver_status_manager_v1_get_river_output_status(self->status_manager, output);

  zriver_output_status_v1_add_listener(output_status, &output_status_listener, self);
  zriver_status_manager_v1_destroy(self->status_manager);
  self->status_manager = NULL;

  // struct wl_display *wl_display = wl_display_connect(NULL);
  // struct wl_registry *wl_registry = NULL;
  //
  // wl_registry = wl_display_get_registry(wl_display);
  // wl_registry_add_listener(wl_registry, &registry_listener, self);
  // wl_display_roundtrip(wl_display);
  //
  // if (wl_display_roundtrip(wl_display) < 0) {
  //   g_printerr("initial roundtrip failed\n");
  //   return false;
  // }
}

void g_barbar_river_update(BarBarRiver *river) {}
