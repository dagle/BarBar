#include "barbar-backlight.h"
#include <fcntl.h>
#include <libudev.h>
#include <stdio.h>

struct _BarBarBacklight {
  GtkWidget parent_instance;

  char *device;
  struct udev *udev;
  struct udev_monitor *mon;

  long max_brightness;
  long current_brightness;

  int current_fd;
  int id;

  GtkWidget *label;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarBacklight, g_barbar_backlight, GTK_TYPE_WIDGET)

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

static void g_barbar_backlight_constructed(GObject *object);

static void g_barbar_backlight_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_backlight_set_device(backlight, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_backlight_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {}
static void g_barbar_backlight_class_init(BarBarBacklightClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_backlight_set_property;
  gobject_class->get_property = g_barbar_backlight_get_property;
  gobject_class->constructed = g_barbar_backlight_constructed;

  // backlight_props[PROP_DEVICE] = g_param_spec_string(
  //     "path", NULL, NULL, "/sys/class/thermal/thermal_zone0/temp",
  //     G_PARAM_READWRITE);
  // g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
  //                                   temperature_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "backlight");
}

static void g_barbar_backlight_init(BarBarBacklight *self) {}
static void g_barbar_backlight_constructed(GObject *self) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(self);

  // TODO: this is the default device, we should do it in a better way
  backlight->device = strdup("intel_backlight");

  backlight->label = gtk_label_new("");
  gtk_widget_set_parent(backlight->label, GTK_WIDGET(backlight));
}

static const char *readline_from_fd(int fd) {
  static char buf[4096];

  ssize_t sz = read(fd, buf, sizeof(buf) - 1);
  lseek(fd, 0, SEEK_SET);

  if (sz < 0) {
    // LOG_WARN("failed to read from FD=%d", fd);
    return NULL;
  }

  buf[sz] = '\0';
  for (ssize_t i = sz - 1; i >= 0 && buf[i] == '\n'; sz--)
    buf[i] = '\0';

  return buf;
}

static long readint_from_fd(int fd) {
  const char *s = readline_from_fd(fd);
  if (s == NULL)
    return 0;

  long ret;
  int r = sscanf(s, "%ld", &ret);
  if (r != 1) {
    // LOG_WARN("failed to convert \"%s\" to an integer", s);
    return 0;
  }

  return ret;
}

static void g_barbar_backlight_update(BarBarBacklight *backlight) {
  char *str =
      g_strdup_printf("%0.f%%", (100.0 * (double)backlight->current_brightness /
                                 (double)backlight->max_brightness));
  gtk_label_set_text(GTK_LABEL(backlight->label), str);
  g_free(str);
}

// TODO: Error
static int g_barbar_backlight_initialize(BarBarBacklight *backlight) {
  int backlight_fd = open("/sys/class/backlight", O_RDONLY);
  if (backlight_fd == -1) {
    // LOG_ERRNO("/sys/class/backlight");
    return -1;
  }

  int base_dir_fd = openat(backlight_fd, backlight->device, O_RDONLY);
  close(backlight_fd);

  if (base_dir_fd == -1) {
    // LOG_ERRNO("/sys/class/backlight/%s", m->device);
    return -1;
  }

  int max_fd = openat(base_dir_fd, "max_brightness", O_RDONLY);
  if (max_fd == -1) {
    // LOG_ERRNO("/sys/class/backlight/%s/max_brightness", m->device);
    close(base_dir_fd);
    return -1;
  }

  backlight->max_brightness = readint_from_fd(max_fd);
  close(max_fd);

  int current_fd = openat(base_dir_fd, "brightness", O_RDONLY);
  close(base_dir_fd);

  if (current_fd == -1) {
    // LOG_ERRNO("/sys/class/backlight/%s/brightness", m->device);
    return -1;
  }

  backlight->current_brightness = readint_from_fd(current_fd);

  return current_fd;
}

// TODO: add a way to change the value via login1

gboolean g_barbar_backlight_callback(GIOChannel *source, GIOCondition condition,
                                     gpointer data) {
  BarBarBacklight *backlight = BARBAR_BACKLIGHT(data);

  struct udev_device *dev = udev_monitor_receive_device(backlight->mon);
  udev_device_unref(dev);

  backlight->current_brightness = readint_from_fd(backlight->current_fd);
  g_barbar_backlight_update(backlight);

  return TRUE;
}

void g_barbar_backlight_start(BarBarBacklight *backlight) {
  GError *error = NULL;
  gboolean result;
  char *data;
  gsize length;
  double temp;
  int ret;

  backlight->current_fd = g_barbar_backlight_initialize(backlight);
  if (backlight->current_fd == -1) {
    // free stuff
    return;
  }

  backlight->udev = udev_new();
  backlight->mon = udev_monitor_new_from_netlink(backlight->udev, "udev");

  udev_monitor_filter_add_match_subsystem_devtype(backlight->mon, "backlight",
                                                  NULL);
  udev_monitor_enable_receiving(backlight->mon);

  GIOChannel *io_channel =
      g_io_channel_unix_new(udev_monitor_get_fd(backlight->mon));

  g_barbar_backlight_update(backlight);
  backlight->id = g_io_add_watch(io_channel, G_IO_IN,
                                 g_barbar_backlight_callback, backlight);
}