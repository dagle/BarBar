#include "sensors/barbar-systemd-units.h"
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

/**
 * BarBarSystemdUnits:
 *
 * A sensor to display runtime info about systemd
 *
 */
struct _BarBarSystemdUnits {
  BarBarSensor parent_instance;

  GDBusProxy *proxy;

  GBusType bus_type;

  guint failed_unit;

  char *active;
};

enum {
  PROP_0,

  PROP_BUS_TYPE,
  PROP_FAILED_UNITS,
  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarSystemdUnits, g_barbar_systemd_units, BARBAR_TYPE_SENSOR)

static void g_barbar_systemd_units_start(BarBarSensor *sensor);

static GParamSpec *systemd_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_systemd_units_set_failed(BarBarSystemdUnits *units,
                                              guint failed) {
  g_return_if_fail(BARBAR_IS_SYSTEMD_UNITS(units));

  if (units->failed_unit == failed) {
    return;
  }

  units->failed_unit = failed;

  g_object_notify_by_pspec(G_OBJECT(units), systemd_props[PROP_FAILED_UNITS]);
}

GType g_barbar_bus_type_get_type(void) {

  static gsize barbar_bus_type;
  if (g_once_init_enter(&barbar_bus_type)) {

    static const GEnumValue pattern_types[] = {
        {G_BUS_TYPE_STARTER, "G_BUS_TYPE_STARTER", "starter"},
        {G_BUS_TYPE_NONE, "G_BUS_TYPE_NONE", "none"},
        {G_BUS_TYPE_SYSTEM, "G_BUS_TYPE_SYSTEM", "system"},
        {G_BUS_TYPE_SESSION, "G_BUS_TYPE_SESSION", "session"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarBusType", pattern_types);
    g_once_init_leave(&barbar_bus_type, type);
  }
  return barbar_bus_type;
}

static void g_barbar_systemd_units_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {

  BarBarSystemdUnits *units = BARBAR_SYSTEMD_UNITS(object);

  switch (property_id) {
  case PROP_BUS_TYPE:
    units->bus_type = g_value_get_enum(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_systemd_units_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {

  BarBarSystemdUnits *units = BARBAR_SYSTEMD_UNITS(object);
  switch (property_id) {
  case PROP_BUS_TYPE:
    g_value_set_enum(value, units->bus_type);
    break;
  case PROP_FAILED_UNITS:
    g_value_set_uint(value, units->failed_unit);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_systemd_units_class_init(BarBarSystemdUnitsClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_systemd_units_start;

  gobject_class->set_property = g_barbar_systemd_units_set_property;
  gobject_class->get_property = g_barbar_systemd_units_get_property;

  /**
   * BarBarSystemdUnits:bus-type:
   *
   * [enum@BarBar.BusType] What bus-type to connect to
   */
  systemd_props[PROP_BUS_TYPE] = g_param_spec_enum(
      "bus-type", NULL, NULL, BARBAR_TYPE_BUS_TYPE, G_BUS_TYPE_SYSTEM,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarSystemdUnits:failed-units:
   *
   * How many units have failed
   */
  systemd_props[PROP_FAILED_UNITS] =
      g_param_spec_uint("failed-units", NULL, NULL, 0, G_MAXUINT, 0,
                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    systemd_props);
}

static void g_barbar_systemd_units_init(BarBarSystemdUnits *units) {}

static void update_failed(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  BarBarSystemdUnits *units = BARBAR_SYSTEMD_UNITS(data);

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

    g_barbar_systemd_units_set_failed(units, failed);
  }
  g_variant_unref(container);
  g_variant_unref(ret);
}

static void get_failed_units(BarBarSystemdUnits *units) {
  g_dbus_proxy_call(
      units->proxy, "Get",
      g_variant_new("(ss)", "org.freedesktop.systemd1.Manager", "NFailedUnits"),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, update_failed, units);
}

static void get_default_value(BarBarSystemdUnits *units) {
  get_failed_units(units);
}

static void g_properties_changed(GDBusProxy *self, GVariant *changed_properties,
                                 char **invalidated_properties, gpointer data) {
  BarBarSystemdUnits *units = BARBAR_SYSTEMD_UNITS(data);
}

static void units_cb(GObject *object, GAsyncResult *res, gpointer data) {
  BarBarSystemdUnits *units = BARBAR_SYSTEMD_UNITS(data);
  GError *error = NULL;

  units->proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

  if (error) {
    g_printerr("Failed to setup systemd-units: %s\n", error->message);
    g_error_free(error);
  }
  get_default_value(units);

  g_signal_connect(units->proxy, "g-properties-changed",
                   G_CALLBACK(g_properties_changed), units);
}

static void g_barbar_systemd_units_start(BarBarSensor *sensor) {
  BarBarSystemdUnits *units = BARBAR_SYSTEMD_UNITS(sensor);

  g_dbus_proxy_new_for_bus(
      units->bus_type, G_DBUS_PROXY_FLAGS_NONE, NULL,
      "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
      "org.freedesktop.DBus.Properties", NULL, units_cb, units);
}
