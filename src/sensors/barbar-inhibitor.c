#include "barbar-inhibitor.h"
#include "idle-inhibit-unstable-v1-client-protocol.h"
#include "sensors/barbar-sensorcontext.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk/gtk.h>
#include <stdio.h>

// TODO: should be able specify screen

/**
 * BarBarInhibitor:
 * A sensor to toggle the zwp_idle_inhibit of a screen.
 */

struct _BarBarInhibitor {
  GObject parent_instance;

  GtkWidget *widget;
  struct wl_surface *wl_surface;

  struct zwp_idle_inhibit_manager_v1 *manager;
  struct zwp_idle_inhibitor_v1 *inhibitor;

  gboolean enabled;
};

enum {
  PROP_0,

  PROP_ENABLED,

  NUM_PROPERTIES,
};

static GParamSpec *inhibitor_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_inhibitor_start(BarBarSensorContext *self, void *ptr);

static void
barbar_sensor_context_iface_init(BarBarSensorContextInterface *iface);

G_DEFINE_TYPE_WITH_CODE(
    BarBarInhibitor, g_barbar_inhibitor, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(BARBAR_TYPE_SENSOR_CONTEXT,
                          barbar_sensor_context_iface_init));

static void
barbar_sensor_context_iface_init(BarBarSensorContextInterface *iface) {
  iface->start = g_barbar_inhibitor_start;
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarInhibitor *inhibitor = BARBAR_INHIBITOR(data);
  if (strcmp(interface, zwp_idle_inhibit_manager_v1_interface.name) == 0) {
    inhibitor->manager = wl_registry_bind(
        registry, name, &zwp_idle_inhibit_manager_v1_interface, 1);
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

static void g_barbar_inhibitor_dispose(GObject *object) {
  BarBarInhibitor *inhibitor = BARBAR_INHIBITOR(object);

  g_clear_pointer(&inhibitor->inhibitor, zwp_idle_inhibitor_v1_destroy);
  g_clear_pointer(&inhibitor->manager, zwp_idle_inhibit_manager_v1_destroy);
}

static void g_barbar_inhibitor_toggle(BarBarInhibitor *inhibitor) {
  inhibitor->enabled = !inhibitor->enabled;

  if (inhibitor->enabled) {
    if (!inhibitor->inhibitor) {
      inhibitor->inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(
          inhibitor->manager, inhibitor->wl_surface);
    }
  } else {
    g_clear_pointer(&inhibitor->inhibitor, zwp_idle_inhibitor_v1_destroy);
  }

  g_object_notify_by_pspec(G_OBJECT(inhibitor), inhibitor_props[PROP_ENABLED]);
}

static void g_barbar_inhibitor_set_enabled(BarBarInhibitor *inhibitor,
                                           gboolean enabled) {
  g_return_if_fail(BARBAR_IS_INHIBITOR(inhibitor));

  if (inhibitor->enabled != enabled) {
    g_barbar_inhibitor_toggle(inhibitor);
  }
}

static void g_barbar_inhibitor_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarInhibitor *inhibitor = BARBAR_INHIBITOR(object);

  switch (property_id) {
  case PROP_ENABLED:
    g_barbar_inhibitor_set_enabled(inhibitor, g_value_get_boolean(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_inhibitor_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarInhibitor *inhibitor = BARBAR_INHIBITOR(object);

  switch (property_id) {
  case PROP_ENABLED:
    g_value_set_boolean(value, inhibitor->enabled);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_inhibitor_class_init(BarBarInhibitorClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_inhibitor_set_property;
  gobject_class->get_property = g_barbar_inhibitor_get_property;
  gobject_class->dispose = g_barbar_inhibitor_dispose;

  /**
   * BarBarInhibitor:enabled:
   *
   * The state of the inhibitor
   */
  inhibitor_props[PROP_ENABLED] =
      g_param_spec_boolean("enabled", NULL, NULL, FALSE, G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    inhibitor_props);
}

static void g_barbar_inhibitor_init(BarBarInhibitor *self) {
  self->enabled = FALSE;
}

static void g_barbar_inhibitor_start(BarBarSensorContext *self, void *ptr) {
  GdkDisplay *gdk_display;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  g_return_if_fail(self);
  g_return_if_fail(ptr);

  BarBarInhibitor *inhibitor = BARBAR_INHIBITOR(self);
  GtkWidget *widget = GTK_WIDGET(ptr);

  gdk_display = gdk_display_get_default();

  GtkNative *native = gtk_widget_get_native(GTK_WIDGET(widget));
  GdkSurface *surface = gtk_native_get_surface(native);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);
  inhibitor->wl_surface = gdk_wayland_surface_get_wl_surface(surface);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, inhibitor);
  wl_display_roundtrip(wl_display);
}
