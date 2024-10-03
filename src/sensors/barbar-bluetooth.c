#include "sensors/barbar-bluetooth.h"
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

/**
 * BarBarBluetooth:
 *
 * A sensor for bluetooth
 *
 */
struct _BarBarBluetooth {
  BarBarSensor parent_instance;

  GDBusObjectManager *manager;

  gboolean enabled;
};

enum {
  PROP_0,

  PROP_DEVICES,
  PROP_ENABLED,
  PROP_CONNECTED,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarBluetooth, g_barbar_bluetooth, BARBAR_TYPE_SENSOR)

static void g_barbar_bluetooth_start(BarBarSensor *sensor);

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

static void g_barbar_bluetooth_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  BarBarBluetooth *mode = BARBAR_BLUETOOTH(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_bluetooth_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {

  BarBarBluetooth *bt = BARBAR_BLUETOOTH(object);
  switch (property_id) {
  // case PROP_GAME_COUNT:
  //   // g_value_set_uint(value, mode->game_count);
  //   break;
  case PROP_ENABLED:
    g_value_set_boolean(value, bt->enabled);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_bluetooth_class_init(BarBarBluetoothClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_bluetooth_start;

  gobject_class->set_property = g_barbar_bluetooth_set_property;
  gobject_class->get_property = g_barbar_bluetooth_get_property;

  /**
   * BarBarBluetooth:devices:
   *
   * How many games are currently running.
   *
   */
  systemd_props[PROP_DEVICES] =
      g_param_spec_boxed("devices", NULL, NULL, G_TYPE_STRV,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   *
   * BarBarBluetooth:enabled:
   *
   * If bluetooth is enabled
   */
  systemd_props[PROP_ENABLED] = g_param_spec_boolean(
      "enabled", NULL, NULL, FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   *
   * BarBarBluetooth:connected:
   *
   * If we are connected to bluetooth
   */
  systemd_props[PROP_CONNECTED] =
      g_param_spec_boolean("connected", NULL, NULL, FALSE,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    systemd_props);
}
static void on_interface_proxy_properties_changed(
    GDBusObjectManagerClient *manager, GDBusObjectProxy *object_proxy,
    GDBusProxy *interface_proxy, GVariant *changed_properties,
    const gchar *const *invalidated_properties, gpointer user_data) {}

static void on_interface_added(GDBusObjectManager *manager, GDBusObject *object,
                               GDBusInterface *interface, gpointer user_data) {

  GDBusProxy *proxy = G_DBUS_PROXY(interface);

  // add the device to list of devices

  // if this is our first device and we are connected, set connected.
}

static void on_interface_removed(GDBusObjectManager *manager,
                                 GDBusObject *object, GDBusInterface *interface,
                                 gpointer user_data) {

  GDBusProxy *proxy = G_DBUS_PROXY(interface);
  // remove the interface from the list

  // if this was our last device, set disconnected
}

static void g_barbar_bluetooth_init(BarBarBluetooth *mode) {}

static void get_initial_state(BarBarBluetooth *bt) {}

static void manager_cb(GObject *source_object, GAsyncResult *res,
                       gpointer data) {
  BarBarBluetooth *bt = BARBAR_BLUETOOTH(data);
  GError *error = NULL;

  bt->manager = g_dbus_object_manager_client_new_for_bus_finish(res, &error);

  get_initial_state(bt);

  g_signal_connect(bt->manager, "interface-proxy-properties-changed",
                   G_CALLBACK(on_interface_proxy_properties_changed), bt);
  g_signal_connect(bt->manager, "interface-added",
                   G_CALLBACK(on_interface_added), bt);
  g_signal_connect(bt->manager, "interface-removed",
                   G_CALLBACK(on_interface_removed), bt);
}

static void g_barbar_bluetooth_start(BarBarSensor *sensor) {
  BarBarBluetooth *bt = BARBAR_BLUETOOTH(sensor);
  g_dbus_object_manager_client_new_for_bus(
      G_BUS_TYPE_SESSION, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_DO_NOT_AUTO_START,
      "org.bluez", "/", NULL, NULL, NULL, NULL, manager_cb, bt);
}

/**
 * g_barbar_bluetooth_new:
 *
 * Returns: (transfer full): a `BarBarBluetooth`
 */
BarBarSensor *g_barbar_bluetooth_new(void) {
  BarBarBluetooth *bt;

  bt = g_object_new(BARBAR_TYPE_BLUETOOTH, NULL);

  return BARBAR_SENSOR(bt);
}
