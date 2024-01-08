#include "barbar-backlight.h"
#include <fcntl.h>
#include <libudev.h>
#include <stdio.h>

struct _BarBarBacklight {
  GObject parent_instance;

  char *device;
  struct udev *udev;
  struct udev_monitor *mon;
  GDBusConnection *connection;
  GDBusProxy *proxy;

  long max;
  long brightness;
  long power;
  double procent;

  int current_fd;
  int id;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_PROCENT,
  PROP_BRIGHTNESS,
  PROP_MAX,
  PROP_POWER,

  NUM_PROPERTIES,
};

#define DEFAULT_DEVICE "intel_backlight"

#define LOGIN_PATH "/org/freedesktop/login1"
#define LOGIN_INTERFACE "org.freedesktop.login1.Manager"
#define LOGIN_NAME "org.freedesktop.login1"

G_DEFINE_TYPE(BarBarBacklight, g_barbar_backlight, G_TYPE_OBJECT)

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
                                gpointer data) {}

void g_barbar_backlight_set_procent(BarBarBacklight *backlight,
                                    double procent) {
  g_return_if_fail(BARBAR_IS_BACKLIGHT(backlight));
  GError *err = NULL;

  backlight->procent = procent;
  backlight->brightness = procent * backlight->max;

  g_dbus_proxy_call(backlight->proxy, "SetBrightness",
                    g_variant_new("(ssu)", "backlight", backlight->device,
                                  backlight->brightness),
                    G_DBUS_CALL_FLAGS_NONE, -1, NULL, brightness_callback,
                    &err);

  g_object_notify_by_pspec(G_OBJECT(backlight), backlight_props[PROP_PROCENT]);
}

static void g_barbar_backlight_constructed(GObject *object);

static void g_barbar_backlight_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_backlight_set_device(backlight, g_value_get_string(value));
    break;
  case PROP_PROCENT:
    g_barbar_backlight_set_procent(backlight, g_value_get_double(value));
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
  case PROP_PROCENT:
    g_value_set_double(value, backlight->procent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_backlight_class_init(BarBarBacklightClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_backlight_set_property;
  gobject_class->get_property = g_barbar_backlight_get_property;
  gobject_class->constructed = g_barbar_backlight_constructed;

  backlight_props[PROP_DEVICE] =
      g_param_spec_string("device", NULL, NULL, DEFAULT_DEVICE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    backlight_props);
}

static void g_barbar_backlight_init(BarBarBacklight *self) {}
static void g_barbar_backlight_constructed(GObject *self) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(self);
  GError *err = NULL;

  if (!backlight->device) {
    backlight->device = strdup(DEFAULT_DEVICE);
  }

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
    }
  } else {
  }
}

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

static gboolean g_barbar_backlight_callback(GIOChannel *source,
                                            GIOCondition condition,
                                            gpointer data) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(data);
  g_return_val_if_fail(backlight, G_SOURCE_REMOVE);

  struct udev_device *dev = udev_monitor_receive_device(backlight->mon);

  g_return_val_if_fail(dev, G_SOURCE_REMOVE);

  const char *name = udev_device_get_sysname(dev);
  g_return_val_if_fail(name, G_SOURCE_REMOVE);

  const char *actual_brightness_attr =
      strncmp(name, "amdgpu_bl", 9) == 0 || strcmp(name, "apple-panel-bl") == 0
          ? "brightness"
          : "actual_brightness";

  const char *brightness =
      udev_device_get_sysattr_value(dev, actual_brightness_attr);
  const char *max = udev_device_get_sysattr_value(dev, "max_brightness");
  const char *power = udev_device_get_sysattr_value(dev, "bl_power");

  backlight->max = read_long(max, 100);
  backlight->brightness = read_long(brightness, 0);
  backlight->power = read_long(power, 0);

  udev_device_unref(dev);

  backlight->procent =
      100.0 * (double)backlight->brightness / (double)backlight->max;

  g_object_notify_by_pspec(G_OBJECT(backlight), backlight_props[PROP_PROCENT]);

  return G_SOURCE_CONTINUE;
}

void g_barbar_backlight_start(BarBarBacklight *backlight) {
  backlight->udev = udev_new();
  backlight->mon = udev_monitor_new_from_netlink(backlight->udev, "udev");

  udev_monitor_filter_add_match_subsystem_devtype(backlight->mon, "backlight",
                                                  NULL);
  udev_monitor_enable_receiving(backlight->mon);

  GIOChannel *io_channel =
      g_io_channel_unix_new(udev_monitor_get_fd(backlight->mon));

  backlight->id = g_io_add_watch(io_channel, G_IO_IN,
                                 g_barbar_backlight_callback, backlight);
}
