#include "barbar-disk.h"
#include <glibtop.h>
#include <glibtop/disk.h>
#include <stdio.h>

/**
 * BarBarDisk:
 *
 * A simple disk sensor
 */
struct _BarBarDisk {
  BarBarIntervalSensor parent_instance;

  char *device;
  glibtop_disk buf;

  guint64 capacity;
  guint64 usage;
  double percent;

  guint interval;
  guint source_id;
};

enum {
  PROP_0,

  PROP_DEVICE,
  // PROP_PERCENT,
  // PROP_USAGE,
  // PROP_CAPACITY,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

static GParamSpec *disk_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarDisk, g_barbar_disk, BARBAR_TYPE_INTERVAL_SENSOR)

static gboolean g_barbar_disk_tick(BarBarIntervalSensor *sensor);

static void g_barbar_disk_set_device(BarBarDisk *bar, const char *device) {
  g_return_if_fail(BARBAR_IS_DISK(bar));

  if (g_set_str(&bar->device, device))
    g_object_notify_by_pspec(G_OBJECT(bar), disk_props[PROP_DEVICE]);
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
  // case PROP_PERCENT:
  //   g_value_set_double(value, fs->percent);
  //   break;
  // case PROP_USAGE:
  //   g_value_set_uint64(value, fs->usage);
  //   break;
  // case PROP_CAPACITY:
  //   g_value_set_uint64(value, fs->capacity);
  //   break;
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
}

static void g_barbar_disk_init(BarBarDisk *self) {}

static gboolean g_barbar_disk_tick(BarBarIntervalSensor *sensor) {
  BarBarDisk *disk = BARBAR_DISK(sensor);

  glibtop_get_disk(&disk->buf);
  disk->buf.xdisk_sectors_read[0];

  // gchar *str =
  //     g_strdup_printf("percentage_free: %lu%%", buf.bavail * 100 /
  //     buf.blocks);

  return G_SOURCE_CONTINUE;
}
