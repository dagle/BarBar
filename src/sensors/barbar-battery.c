#include "barbar-battery.h"
#include "sensors/barbar-sensor.h"
#include <libupower-glib/upower.h>
#include <stdio.h>

struct _BarBarBattery {
  BarBarSensor parent_instance;

  guint interval;

  char *device;
  double percent;
  guint source_id;

  UpClient *client;
  UpDevice *dev;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_PERCENT,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 1000

#define LOGIN_PATH "/org/freedesktop/login1"
#define LOGIN_INTERFACE "org.freedesktop.login1.Manager"
#define LOGIN_NAME "org.freedesktop.login1"

// use upower
G_DEFINE_TYPE(BarBarBattery, g_barbar_battery, BARBAR_TYPE_SENSOR)

static GParamSpec *battery_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_battery_start(BarBarSensor *sensor);

void g_barbar_battery_set_device(BarBarBattery *battery, const char *device) {
  g_return_if_fail(BARBAR_IS_BATTERY(battery));

  if (battery->device && !strcmp(battery->device, device)) {
    return;
  }

  g_free(battery->device);
  battery->device = g_strdup(device);

  g_object_notify_by_pspec(G_OBJECT(battery), battery_props[PROP_DEVICE]);
}

void g_barbar_battery_set_percent(BarBarBattery *battery, double percent) {
  g_return_if_fail(BARBAR_IS_BATTERY(battery));

  if (battery->percent == percent) {
    return;
  }
  battery->percent = percent;

  g_object_notify_by_pspec(G_OBJECT(battery), battery_props[PROP_PERCENT]);
}

static void g_barbar_battery_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {

  BarBarBattery *battery = BARBAR_BATTERY(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_battery_set_device(battery, g_value_get_string(value));
    break;
  case PROP_PERCENT:
    g_barbar_battery_set_percent(battery, g_value_get_double(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_battery_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {

  BarBarBattery *battery = BARBAR_BATTERY(object);
  switch (property_id) {
  case PROP_DEVICE:
    g_value_set_string(value, battery->device);
    break;
  case PROP_PERCENT:
    g_value_set_double(value, battery->percent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_battery_class_init(BarBarBatteryClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_battery_set_property;
  gobject_class->get_property = g_barbar_battery_get_property;
  sensor_class->start = g_barbar_battery_start;
  /**
   * BarBarBattery:device:
   *
   * What device to listen use for battery
   */
  battery_props[PROP_DEVICE] =
      g_param_spec_string("device", "device", "A path to a device", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarBattery:percent:
   *
   * The current percent of backlight we use
   */
  battery_props[PROP_PERCENT] = g_param_spec_double(
      "percent", NULL, NULL, 0, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    battery_props);
}

static void g_barbar_battery_init(BarBarBattery *self) {
  self->interval = DEFAULT_INTERVAL;
}

static void g_barbar_battery_update(UpDevice *dev, BarBarBattery *battery) {
  double percent;
  g_object_get(dev, "percentage", &percent, NULL);
  percent = percent / 100;
  g_barbar_battery_set_percent(battery, percent);
}

static void g_barbar_battery_cb(UpDevice *dev, GParamSpec *pspec,
                                gpointer user_data) {
  BarBarBattery *battery = BARBAR_BATTERY(user_data);
  g_barbar_battery_update(dev, battery);
}

// static void g_barbar_battery_update(GObject *object, GParamSpec *pspec,
//                                     gpointer data) {
//
//   BarBarBattery *battery = BARBAR_BATTERY(data);
//   GError *err = NULL;
//
//   // GDBusConnection *login1_connection =
//   //     g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
//   //
//   // GDBusProxy *proxy = g_dbus_proxy_new_sync(
//   //     login1_connection, G_DBUS_PROXY_FLAGS_NONE, NULL, LOGIN_NAME,
//   //     LOGIN_PATH, LOGIN_INTERFACE, NULL, &err);
//
//   // if (!login1_connection) {
//   // throw std::runtime_error("Unable to connect to the SYSTEM Bus!...");
//   // } else {
//   // login1_id = g_dbus_connection_signal_subscribe(
//   // 		login1_connection, "org.freedesktop.login1",
//   // "org.freedesktop.login1.Manager", 		"PrepareForSleep",
//   // "/org/freedesktop/login1", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
//   // 		prepareForSleep_cb, this, NULL);
//   // }
//
//   // event_box_.signal_button_press_event().connect(sigc::mem_fun(*this,
//   // &UPower::handleToggle));
//
//   // g_signal_connect(client, "device-added", G_CALLBACK(deviceAdded_cb),
//   this);
//   // g_signal_connect(client, "device-removed", G_CALLBACK(deviceRemoved_cb),
//   // this);
// }

static void g_barbar_battery_setup_device(BarBarBattery *battery) {
  if (!battery->device) {
    battery->dev = up_client_get_display_device(battery->client);
  } else {
    g_autoptr(GPtrArray) array = up_client_get_devices2(battery->client);
    for (int i = 0; i < array->len; i++) {
      g_autoptr(UpDevice) device = UP_DEVICE(array->pdata[i]);
      gchar *path;
      g_object_get(device, "native-path", &path, NULL);
      if (!strcmp(path, battery->device)) {
        battery->dev = g_object_ref(device);
      }
      g_free(path);
    }
  }
}

static void g_barbar_battery_start(BarBarSensor *sensor) {
  BarBarBattery *battery = BARBAR_BATTERY(sensor);
  battery->client = up_client_new();
  g_barbar_battery_setup_device(battery);

  g_barbar_battery_update(battery->dev, battery);

  g_signal_connect(battery->dev, "notify::percentage",
                   G_CALLBACK(g_barbar_battery_cb), battery);
}
