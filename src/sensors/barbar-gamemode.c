#include "sensors/barbar-gamemode.h"
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

/**
 * BarBarGameMode:
 *
 * A sensor to display gamer mode status
 *
 */
struct _BarBarGameMode {
  BarBarSensor parent_instance;

  GDBusProxy *proxy;

  guint game_count;
};

enum {
  PROP_0,

  PROP_GAME_COUNT,
  PROP_ENABLED,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarGameMode, g_barbar_game_mode, BARBAR_TYPE_SENSOR)

static void g_barbar_game_mode_start(BarBarSensor *sensor);

static GParamSpec *systemd_props[NUM_PROPERTIES] = {
    NULL,
};

// static void g_barbar_systemd_units_set_profile(BarBarSystemdUnits *units,
//                                                guint failed) {
//   g_return_if_fail(BARBAR_IS_SYSTEMD_UNITS(units));
//
//   if (units->failed_unit == failed) {
//     return;
//   }
//
//   units->failed_unit = failed;
//
//   g_object_notify_by_pspec(G_OBJECT(units),
//   systemd_props[PROP_FAILED_UNITS]);
// }

static void g_barbar_game_mode_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  BarBarGameMode *mode = BARBAR_GAME_MODE(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_game_mode_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {

  BarBarGameMode *mode = BARBAR_GAME_MODE(object);
  switch (property_id) {
  case PROP_GAME_COUNT:
    g_value_set_uint(value, mode->game_count);
    break;
  case PROP_ENABLED:
    g_value_set_boolean(value, mode->game_count > 0);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_systemd_units_class_init(BarBarGameModeClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_game_mode_start;

  gobject_class->set_property = g_barbar_game_mode_set_property;
  gobject_class->get_property = g_barbar_game_mode_get_property;

  /**
   * BarBarGameMode:count:
   *
   * How many games are currently running.
   *
   */
  systemd_props[PROP_GAME_COUNT] = g_param_spec_uint(
      "game-count", NULL, NULL, 0, G_MAXUINT, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarGameMode:enabled:
   *
   * If we are running in game mode
   */
  systemd_props[PROP_ENABLED] = g_param_spec_boolean(
      "enabled", NULL, NULL, FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    systemd_props);
}

static void g_barbar_game_mode_init(BarBarGameMode *mode) {}

static void update_failed(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;

  BarBarGameMode *mode = BARBAR_GAME_MODE(object);

  GVariant *ret = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);

  if (error || !ret) {
    g_printerr("Failed to get failed systemd-units: %s\n", error->message);
    g_error_free(error);
  }
  guint failed;
  GVariant *container;
  g_variant_get(ret, "(v)", &container);

  if (!container) {
    return;
  }
  const GVariantType *type = g_variant_get_type(container);
  if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT32)) {
    g_variant_get(container, "u", &failed);

    // g_barbar_systemd_units_set_profile(units, failed);
  }
  g_variant_unref(container);
  g_variant_unref(ret);
}

static void get_failed_units(BarBarGameMode *units) {
  g_dbus_proxy_call(
      units->proxy, "Get",
      g_variant_new("(ss)", "com.feralinteractive.GameMode", "ClientCount"),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, update_failed, units);
}

static void get_default_value(BarBarGameMode *units) {
  get_failed_units(units);
}

static void g_properties_changed(GDBusProxy *self, GVariant *changed_properties,
                                 char **invalidated_properties, gpointer data) {
  BarBarGameMode *mode = BARBAR_GAME_MODE(data);
}

static void mode_cb(GObject *object, GAsyncResult *res, gpointer data) {
  BarBarGameMode *mode = BARBAR_GAME_MODE(data);
  GError *error = NULL;

  mode->proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

  if (error) {
    g_printerr("Failed to setup game-mode: %s\n", error->message);
    g_error_free(error);
  }
  get_default_value(mode);

  g_signal_connect(mode->proxy, "g-properties-changed",
                   G_CALLBACK(g_properties_changed), mode);
}

static void g_barbar_game_mode_start(BarBarSensor *sensor) {
  BarBarGameMode *mode = BARBAR_GAME_MODE(sensor);

  g_dbus_proxy_new_for_bus(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL,
      "com.feralinteractive.GameMode", "/com/feralinteractive/GameMode",
      "org.freedesktop.DBus.Properties", NULL, mode_cb, mode);
}
