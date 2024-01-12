#include "barbar-backlight.h"
#include <fcntl.h>
#include <libudev.h>
#include <signal.h>
#include <stdio.h>

struct _BarBarBacklight {
  BarBarSensor parent_instance;

  char *device;
  struct udev *udev;
  struct udev_monitor *mon;
  GDBusConnection *connection;
  GDBusProxy *proxy;

  long max;
  long brightness;
  long power;
  double percent;

  int current_fd;
  int id;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_PERCENT,
  // PROP_BRIGHTNESS,
  // PROP_MAX,
  // PROP_POWER,

  NUM_PROPERTIES,
};

#define LOGIN_PATH "/org/freedesktop/login1"
#define LOGIN_INTERFACE "org.freedesktop.login1.Manager"
#define LOGIN_NAME "org.freedesktop.login1"

G_DEFINE_TYPE(BarBarBacklight, g_barbar_backlight, BARBAR_TYPE_SENSOR)

static GParamSpec *backlight_props[NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_backlight_set_device(BarBarBacklight *backlight,
                                   const char *path) {
  g_return_if_fail(BARBAR_IS_BACKLIGHT(backlight));

  g_free(backlight->device);
  backlight->device = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(backlight), backlight_props[PROP_DEVICE]);
}

static void brightness_callback(GObject *object, GAsyncResult *res,
                                gpointer data) {
  GDBusProxy *proxy = G_DBUS_PROXY(object);
  GError *err = NULL;

  GVariant *var = g_dbus_proxy_call_finish(proxy, res, &err);

  if (err) {
    printf("error: %s\n", err->message);
  }
}

void g_barbar_backlight_set_percent(BarBarBacklight *backlight,
                                    double percent) {
  g_return_if_fail(BARBAR_IS_BACKLIGHT(backlight));

  backlight->percent = percent;
  backlight->brightness = percent * backlight->max;

  g_dbus_proxy_call(backlight->proxy, "SetBrightness",
                    g_variant_new("(ssu)", "backlight", backlight->device,
                                  backlight->brightness),
                    G_DBUS_CALL_FLAGS_NONE, -1, NULL, brightness_callback,
                    NULL);

  g_object_notify_by_pspec(G_OBJECT(backlight), backlight_props[PROP_PERCENT]);
}

static void g_barbar_backlight_start(BarBarSensor *sensor);

static void g_barbar_backlight_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_backlight_set_device(backlight, g_value_get_string(value));
    break;
  case PROP_PERCENT:
    g_barbar_backlight_set_percent(backlight, g_value_get_double(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_backlight_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {

  BarBarBacklight *backlight = BARBAR_BACKLIGHT(object);
  switch (property_id) {
  case PROP_DEVICE:
    g_value_set_string(value, backlight->device);
    break;
  case PROP_PERCENT:
    g_value_set_double(value, backlight->percent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_backlight_class_init(BarBarBacklightClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_backlight_start;

  gobject_class->set_property = g_barbar_backlight_set_property;
  gobject_class->get_property = g_barbar_backlight_get_property;
  // gobject_class->constructed = g_barbar_backlight_constructed;

  /**
   * BarBarBacklight:device:
   *
   * What device to listen use for backlight
   */
  backlight_props[PROP_DEVICE] = g_param_spec_string(
      "device", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarBacklight:percent:
   *
   * The current percent of backlight we use
   */
  backlight_props[PROP_PERCENT] = g_param_spec_double(
      "percent", NULL, NULL, 0, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    backlight_props);
}

static void g_barbar_backlight_init(BarBarBacklight *self) {}

static long read_long(const char *str, long def) {
  if (!str) {
    return def;
  }

  char *endptr;
  long value = strtol(str, &endptr, 10);

  if (endptr == str) {
    return def;
  }

  return value;
}

static const char *get_attr(const char *name) {
  const char *brightness_attr = g_str_has_suffix(name, "amdgpu_bl") ||
                                        g_str_has_suffix(name, "apple-panel-bl")
                                    ? "brightness"
                                    : "actual_brightness";
  return brightness_attr;
}

// static gboolean match_name(const char *path, const char *name) {
//   return g_str_has_suffix(path, name);
// }

static void g_barbar_backlight_internal(BarBarBacklight *backlight,
                                        struct udev_device *dev,
                                        const char *name) {

  const char *brightness_attr = get_attr(name);

  const char *brightness = udev_device_get_sysattr_value(dev, brightness_attr);
  const char *max = udev_device_get_sysattr_value(dev, "max_brightness");
  const char *power = udev_device_get_sysattr_value(dev, "bl_power");

  backlight->max = read_long(max, 100);
  backlight->brightness = read_long(brightness, 0);
  backlight->power = read_long(power, 0);

  backlight->percent = (double)backlight->brightness / (double)backlight->max;

  g_object_notify_by_pspec(G_OBJECT(backlight), backlight_props[PROP_PERCENT]);
}

static gboolean g_barbar_backlight_callback(GIOChannel *source,
                                            GIOCondition condition,
                                            gpointer data) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(data);
  g_return_val_if_fail(backlight, G_SOURCE_REMOVE);

  struct udev_device *dev = udev_monitor_receive_device(backlight->mon);

  g_return_val_if_fail(dev, G_SOURCE_REMOVE);

  const char *path = udev_device_get_sysname(dev);
  g_return_val_if_fail(path, G_SOURCE_REMOVE);

  // this isn't the droid... device we are looking for
  if (!g_str_has_suffix(path, backlight->device)) {
    return G_SOURCE_CONTINUE;
  }

  g_barbar_backlight_internal(backlight, dev, path);

  udev_device_unref(dev);

  return G_SOURCE_CONTINUE;
}

/**
 * g_barbar_backlight_find_device:
 * @backlight: our backlight
 *
 * If no device is set, we try to find the best device.
 */
static void g_barbar_backlight_find_device(BarBarBacklight *backlight) {
  struct udev_list_entry *entry;
  struct udev_enumerate *en = udev_enumerate_new(backlight->udev);
  udev_enumerate_add_match_subsystem(en, "backlight");
  udev_enumerate_scan_devices(en);

  struct udev_list_entry *enum_devices = udev_enumerate_get_list_entry(en);
  udev_list_entry_foreach(entry, enum_devices) {
    const char *syspath = udev_list_entry_get_name(entry);
    // match on something, lets just take the last we find for now.
    gchar *device = g_path_get_basename(syspath);
    g_barbar_backlight_set_device(backlight, device);
    g_free(device);
  }
  udev_enumerate_unref(en);
}

/**
 * g_barbar_backlight_init_value:
 * @backlight: our backlight
 *
 * Get the initial value for our device
 */
static void g_barbar_backlight_init_value(BarBarBacklight *backlight) {
  struct udev_list_entry *entry;
  struct udev_enumerate *en = udev_enumerate_new(backlight->udev);
  udev_enumerate_add_match_subsystem(en, "backlight");
  udev_enumerate_scan_devices(en);

  struct udev_list_entry *enum_devices = udev_enumerate_get_list_entry(en);
  udev_list_entry_foreach(entry, enum_devices) {
    const char *syspath = udev_list_entry_get_name(entry);
    if (g_str_has_suffix(syspath, backlight->device)) {
      struct udev_device *device =
          udev_device_new_from_syspath(backlight->udev, syspath);
      g_barbar_backlight_internal(backlight, device, syspath);
      udev_device_unref(device);
    }
  }
  udev_enumerate_unref(en);
}

static void g_barbar_backlight_start(BarBarSensor *sensor) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(sensor);
  GError *err = NULL;

  GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);

  if (!err) {
    GDBusProxy *proxy = g_dbus_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, NULL, LOGIN_NAME, LOGIN_PATH,
        LOGIN_INTERFACE, NULL, &err);

    if (!err) {
      backlight->connection = connection;
      backlight->proxy = proxy;
    } else {
      g_object_unref(connection);
      return;
    }
  } else {
    return;
  }
  backlight->udev = udev_new();
  backlight->mon = udev_monitor_new_from_netlink(backlight->udev, "udev");

  if (!backlight->device) {
    g_barbar_backlight_find_device(backlight);
  }

  g_barbar_backlight_init_value(backlight);

  udev_monitor_filter_add_match_subsystem_devtype(backlight->mon, "backlight",
                                                  NULL);
  udev_monitor_enable_receiving(backlight->mon);

  GIOChannel *io_channel =
      g_io_channel_unix_new(udev_monitor_get_fd(backlight->mon));

  backlight->id = g_io_add_watch(io_channel, G_IO_IN,
                                 g_barbar_backlight_callback, backlight);
}
