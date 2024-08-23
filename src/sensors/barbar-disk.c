#include "barbar-disk.h"
#include "glibconfig.h"
#include <glibtop.h>
#include <glibtop/disk.h>
#include <stdio.h>

/**
 * BarBarDisk:
 *
 * A simple disk sensor
 *
 * Read/write speeds doesn't know what the maximum is. So if you want
 * use them in a graph, it might be worth using hdparm to get the maximum
 * value.
 */
struct _BarBarDisk {
  BarBarIntervalSensor parent_instance;

  char *device;
  char *path;

  guint64 read;
  guint64 write;

  guint64 old_read;
  guint64 old_write;

  guint64 read_speed;
  guint64 write_speed;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_READ,
  PROP_READ_SPEED,
  PROP_WRITE,
  PROP_WRITE_SPEED,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

static guint disk_signals[NUM_SIGNALS];

#define DEFAULT_INTERVAL 10000

static GParamSpec *disk_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarDisk, g_barbar_disk, BARBAR_TYPE_INTERVAL_SENSOR)

static gboolean g_barbar_disk_tick(BarBarIntervalSensor *sensor);

static void g_barbar_disk_set_device(BarBarDisk *disk, const char *device) {
  g_return_if_fail(BARBAR_IS_DISK(disk));

  if (g_set_str(&disk->device, device)) {
    g_free(disk->path);
    disk->path = g_strdup_printf("/sys/block/%s/stat", device);
    g_object_notify_by_pspec(G_OBJECT(disk), disk_props[PROP_DEVICE]);
  }
}

static void g_barbar_disk_set_property(GObject *object, guint property_id,
                                       const GValue *value, GParamSpec *pspec) {
  BarBarDisk *disk = BARBAR_DISK(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_disk_set_device(disk, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_disk_get_property(GObject *object, guint property_id,
                                       GValue *value, GParamSpec *pspec) {
  BarBarDisk *disk = BARBAR_DISK(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_value_set_string(value, disk->device);
    break;
  case PROP_READ:
    g_value_set_uint64(value, disk->read);
    break;
  case PROP_WRITE:
    g_value_set_uint64(value, disk->write);
    break;
  case PROP_READ_SPEED:
    g_value_set_uint64(value, disk->read_speed);
    break;
  case PROP_WRITE_SPEED:
    g_value_set_uint64(value, disk->write_speed);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_disk_class_init(BarBarDiskClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_disk_set_property;
  gobject_class->get_property = g_barbar_disk_get_property;
  interval_class->tick = g_barbar_disk_tick;

  /**
   * BarBarDisk:device:
   *
   * The name of the device
   */
  disk_props[PROP_DEVICE] = g_param_spec_string(
      "device", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarDisk:read:
   *
   * How much data has been read from the device
   */
  disk_props[PROP_READ] = g_param_spec_uint64("read", NULL, NULL, 0,
                                              G_MAXUINT64, 0, G_PARAM_READABLE);
  /**
   * BarBarDisk:write:
   *
   * How much data has been written to the device
   */
  disk_props[PROP_WRITE] = g_param_spec_uint64(
      "write", NULL, NULL, 0, G_MAXUINT64, 0, G_PARAM_READABLE);
  /**
   * BarBarDisk:read-speed:
   *
   * How much data has been read since the last update
   */
  disk_props[PROP_READ_SPEED] = g_param_spec_uint64(
      "read-speed", NULL, NULL, 0, G_MAXUINT64, 0, G_PARAM_READABLE);
  /**
   * BarBarDisk:write-speed:
   *
   * How much data has been written since the last update
   */
  disk_props[PROP_WRITE_SPEED] = g_param_spec_uint64(
      "write-speed", NULL, NULL, 0, G_MAXUINT64, 0, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, disk_props);
  /**
   * BarBarDisk::tick:
   * @sensor: This sensor
   *
   * Emit that disk has updated
   */
  disk_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_DISK,                       /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_disk_init(BarBarDisk *self) {}

#define BUFSIZ 8192
#define format "%*llu %*llu %llu %*llu %*llu %*llu %llu";

static gboolean g_barbar_disk_tick(BarBarIntervalSensor *sensor) {
  BarBarDisk *disk = BARBAR_DISK(sensor);
  FILE *f;

  f = fopen(disk->path, "r");

  if (!f) {
    char *str = strerror(errno);
    g_printerr("Couldn't open file %s: %s\n", disk->path, str);
    g_free(str);
    return G_SOURCE_REMOVE;
  }
  disk->old_read = disk->read;
  disk->old_write = disk->write;

  if (fscanf(f, "%*llu %*llu %lu %*llu %*llu %*llu %lu", &disk->read,
             &disk->write) != 2) {
    g_printerr("%s format is bad\n", disk->path);
    return G_SOURCE_REMOVE;
  }

  guint interval = g_barbar_interval_sensor_get_interval(sensor);

  guint tick = interval / 1000;
  if (!disk->old_read) {
    disk->old_read = disk->read;
  }

  if (!disk->old_write) {
    disk->old_write = disk->write;
  }

  disk->read_speed = (disk->read - disk->old_read) / tick;
  disk->write_speed = (disk->write - disk->old_write) / tick;

  g_object_notify_by_pspec(G_OBJECT(disk), disk_props[PROP_READ]);
  g_object_notify_by_pspec(G_OBJECT(disk), disk_props[PROP_WRITE]);
  g_object_notify_by_pspec(G_OBJECT(disk), disk_props[PROP_READ_SPEED]);
  g_object_notify_by_pspec(G_OBJECT(disk), disk_props[PROP_WRITE_SPEED]);

  g_signal_emit(G_OBJECT(disk), disk_signals[TICK], 0);

  return G_SOURCE_CONTINUE;
}
