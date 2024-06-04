#include "river/barbar-river-mode.h"
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
 * BarBarRiverMode:
 *
 * A widget to display the river mode we currently are in.
 *
 */
struct _BarBarRiverMode {
  BarBarSensor parent_instance;

  char *mode;

  struct zriver_status_manager_v1 *status_manager;
  struct wl_seat *seat;
  struct zriver_seat_status_v1 *seat_listener;
};

enum {
  PROP_0,

  PROP_MODE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarRiverMode, g_barbar_river_mode, BARBAR_TYPE_SENSOR)

static void g_barbar_river_mode_start(BarBarSensor *sensor);

static GParamSpec *river_mode_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_river_mode_start(BarBarSensor *sensor);

static void g_barbar_river_mode_set_mode(BarBarRiverMode *river,
                                         const char *mode) {
  g_return_if_fail(BARBAR_IS_RIVER_MODE(river));

  if (!g_strcmp0(river->mode, mode)) {
    return;
  }

  g_free(river->mode);

  river->mode = g_strdup(mode);

  g_object_notify_by_pspec(G_OBJECT(river), river_mode_props[PROP_MODE]);
}

static void g_barbar_river_mode_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {

  // BarBarRiverMode *rm = BARBAR_RIVER_MODE(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_river_mode_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec) {
  BarBarRiverMode *rm = BARBAR_RIVER_MODE(object);

  switch (property_id) {
  case PROP_MODE:
    g_value_set_string(value, rm->mode);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_river_mode_finalize(GObject *object) {
  BarBarRiverMode *river = BARBAR_RIVER_MODE(object);

  zriver_seat_status_v1_destroy(river->seat_listener);
  g_free(river->mode);

  G_OBJECT_CLASS(g_barbar_river_mode_parent_class)->finalize(object);
}

static void g_barbar_river_mode_class_init(BarBarRiverModeClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_river_mode_set_property;
  gobject_class->get_property = g_barbar_river_mode_get_property;
  gobject_class->finalize = g_barbar_river_mode_finalize;

  sensor_class->start = g_barbar_river_mode_start;

  river_mode_props[PROP_MODE] =
      g_param_spec_string("mode", NULL, NULL, NULL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    river_mode_props);
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarRiverMode *river = BARBAR_RIVER_MODE(data);
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
                      struct wl_output *output) {}

static void
listen_unfocused_output(void *data,
                        struct zriver_seat_status_v1 *zriver_seat_status_v1,
                        struct wl_output *output) {}

static void listen_focused(void *data,
                           struct zriver_seat_status_v1 *zriver_seat_status_v1,
                           const char *title) {}

static void listen_mode(void *data,
                        struct zriver_seat_status_v1 *zriver_seat_status_v1,
                        const char *name) {
  BarBarRiverMode *mode = BARBAR_RIVER_MODE(data);
  g_barbar_river_mode_set_mode(mode, name);
}

static const struct zriver_seat_status_v1_listener seat_status_listener = {
    .focused_output = listen_focused_output,
    .unfocused_output = listen_unfocused_output,
    .focused_view = listen_focused,
    .mode = listen_mode,
};

static void g_barbar_river_mode_init(BarBarRiverMode *self) {}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void g_barbar_river_mode_start(BarBarSensor *sensor) {
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  BarBarRiverMode *river = BARBAR_RIVER_MODE(sensor);

  wl_display = wl_display_connect(NULL);
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
