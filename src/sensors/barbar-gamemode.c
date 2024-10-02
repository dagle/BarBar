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
  int name_watcher;

  gint game_count;
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

static void g_barbar_game_mode_set_count(BarBarGameMode *mode, gint games) {
  g_return_if_fail(BARBAR_IS_GAME_MODE(mode));
  gint old;

  if (mode->game_count == games) {
    return;
  }
  old = mode->game_count;

  mode->game_count = games;

  g_object_notify_by_pspec(G_OBJECT(mode), systemd_props[PROP_GAME_COUNT]);

  // if we swapped from true to false or false to true, we need to single it.
  if (old > 1 && games <= 0) {
    g_object_notify_by_pspec(G_OBJECT(mode), systemd_props[PROP_ENABLED]);
  } else if (old <= 0 && games > 0) {
    g_object_notify_by_pspec(G_OBJECT(mode), systemd_props[PROP_ENABLED]);
  }
}

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

static void g_barbar_game_mode_class_init(BarBarGameModeClass *class) {
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
  systemd_props[PROP_GAME_COUNT] =
      g_param_spec_uint("game-count", NULL, NULL, 0, G_MAXUINT, 0,
                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

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

static void get_clients(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;

  BarBarGameMode *mode = BARBAR_GAME_MODE(data);

  GVariant *ret = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);

  if (error || !ret) {
    g_printerr("Failed to get initial game-mode: %s\n", error->message);
    g_error_free(error);
  }

  gint games;
  GVariant *container;
  g_variant_get(ret, "(v)", &container);

  if (!container) {
    return;
  }
  const GVariantType *type = g_variant_get_type(container);
  if (g_variant_type_equal(type, G_VARIANT_TYPE_INT32)) {
    g_variant_get(container, "i", &games);
    g_barbar_game_mode_set_count(mode, games);
  }
  g_variant_unref(container);
  g_variant_unref(ret);
}

static void get_running_games(BarBarGameMode *mode) {
  g_dbus_proxy_call(
      mode->proxy, "Get",
      g_variant_new("(ss)", "com.feralinteractive.GameMode", "ClientCount"),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, get_clients, mode);
}

static void get_default_value(BarBarGameMode *mode) { get_running_games(mode); }

static void g_properties_changed(GDBusProxy *self, gchar *sender_name,
                                 gchar *signal_name, GVariant *parameters,
                                 gpointer data) {
  BarBarGameMode *mode = BARBAR_GAME_MODE(data);
  // Why can't we use g-property-changed? A bug in game-mode?
  if (!g_strcmp0(signal_name, "PropertiesChanged")) {
    get_default_value(mode);
  }
}

static void mode_appeared(GDBusConnection *connection, const gchar *name,
                          const gchar *name_owner, gpointer data) {
  BarBarGameMode *mode = BARBAR_GAME_MODE(data);

  get_default_value(mode);
}

static void mode_disappear(GDBusConnection *connection, const gchar *name,
                           gpointer data) {

  BarBarGameMode *mode = BARBAR_GAME_MODE(data);

  g_barbar_game_mode_set_count(mode, 0);
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

  g_signal_connect(mode->proxy, "g-signal", G_CALLBACK(g_properties_changed),
                   mode);
  mode->name_watcher = g_bus_watch_name(
      G_BUS_TYPE_SESSION, "com.feralinteractive.GameMode",
      G_BUS_NAME_WATCHER_FLAGS_NONE, mode_appeared, mode_disappear, mode, NULL);
}

static void g_barbar_game_mode_start(BarBarSensor *sensor) {
  BarBarGameMode *mode = BARBAR_GAME_MODE(sensor);

  g_dbus_proxy_new_for_bus(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL,
      "com.feralinteractive.GameMode", "/com/feralinteractive/GameMode",
      "org.freedesktop.DBus.Properties", NULL, mode_cb, mode);
}

BarBarSensor *g_barbar_game_mode_new(void) {
  BarBarGameMode *gm;

  gm = g_object_new(BARBAR_TYPE_GAME_MODE, NULL);

  return BARBAR_SENSOR(gm);
}
