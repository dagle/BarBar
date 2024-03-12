#include "sensors/barbar-power-profiles.h"
#include "power-profiles-daemon.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

struct _BarBarPowerProfile {
  BarBarSensor parent_instance;

  PowerProfilesPowerProfiles *proxy;

  char *active;
};

enum {
  PROP_0,

  PROP_ACTIVE_PROFILE,
  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarPowerProfile, g_barbar_power_profile, BARBAR_TYPE_SENSOR)

static void g_barbar_power_profile_start(BarBarSensor *sensor);

static GParamSpec *power_props[NUM_PROPERTIES] = {
    NULL,
};

// static guint power_signals[NUM_SIGNALS];

static void g_barbar_power_profile_set_profile(BarBarPowerProfile *profile,
                                               const char *name) {
  g_return_if_fail(BARBAR_IS_POWER_PROFILE(profile));

  if (!g_strcmp0(profile->active, name)) {
    return;
  }

  if (!g_strcmp0(name, "power-saver") || !g_strcmp0(name, "balanced") ||
      !g_strcmp0(name, "performance")) {

    profile->active = strdup(name);

    g_object_notify_by_pspec(G_OBJECT(profile),
                             power_props[PROP_ACTIVE_PROFILE]);
  }
}

static void g_barbar_power_profile_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {
  BarBarPowerProfile *profile = BARBAR_POWER_PROFILE(object);

  switch (property_id) {
  case PROP_ACTIVE_PROFILE:
    g_barbar_power_profile_set_profile(profile, g_value_get_string(value));
    break;
  // case PROP_FORMAT:
  //   g_barbar_clock_set_format(clock, g_value_get_string(value));
  //   break;
  // case PROP_INTERVAL:
  //   g_barbar_clock_set_interval(clock, g_value_get_uint(value));
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_power_profile_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {

  BarBarPowerProfile *profile = BARBAR_POWER_PROFILE(object);
  switch (property_id) {
  case PROP_ACTIVE_PROFILE:
    g_value_set_string(value, profile->active);
    break;
  // case PROP_TIME:
  //   g_barbar_get_time(clock, value);
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_power_profile_class_init(BarBarPowerProfileClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_power_profile_start;

  gobject_class->set_property = g_barbar_power_profile_set_property;
  gobject_class->get_property = g_barbar_power_profile_get_property;
}

static void g_barbar_power_profile_init(BarBarPowerProfile *profile) {}

static void
g_barbar_power_profile_starting_values(BarBarPowerProfile *profile) {

  g_barbar_power_profile_set_profile(
      profile,
      power_profiles_power_profiles_get_active_profile(profile->proxy));

  power_profiles_power_profiles_get_performance_inhibited(profile->proxy);

  power_profiles_power_profiles_get_performance_degraded(profile->proxy);

  power_profiles_power_profiles_get_profiles(profile->proxy);

  power_profiles_power_profiles_get_actions(profile->proxy);
  //
  // GVariant *power_profiles_power_profiles_get_active_profile_holds
  // (PowerProfilesPowerProfiles *object); GVariant
  // *power_profiles_power_profiles_dup_active_profile_holds
  // (PowerProfilesPowerProfiles *object);
  //
  // const gchar *power_profiles_power_profiles_get_version
  // (PowerProfilesPowerProfiles *object); gchar
  // *power_profiles_power_profiles_dup_version (PowerProfilesPowerProfiles
  // *object);
}

static void on_some_property_notify(GObject *proxy, GParamSpec *pspec,
                                    gpointer user_data) {}

static void menu_callback(GObject *object, GAsyncResult *res, gpointer data) {
  BarBarPowerProfile *profile = BARBAR_POWER_PROFILE(data);
  GError *error = NULL;

  profile->proxy =
      power_profiles_power_profiles_proxy_new_for_bus_finish(res, &error);

  if (error) {
    g_printerr("Failed to setup PowerProfile: %s\n", error->message);
    g_error_free(error);
  }

  g_signal_connect(profile->proxy, "notify::active-profile",
                   G_CALLBACK(on_some_property_notify), NULL);
}

static void g_barbar_power_profile_start(BarBarSensor *sensor) {
  BarBarPowerProfile *profile = BARBAR_POWER_PROFILE(sensor);

  power_profiles_power_profiles_proxy_new_for_bus(
      G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
      "org.freedesktop.UPower.PowerProfiles",
      "/org/freedesktop/UPower/PowerProfiles", NULL, menu_callback, profile);
}
