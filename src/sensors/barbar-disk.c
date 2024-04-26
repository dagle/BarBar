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
  BarBarSensor parent_instance;

  char *path;

  guint64 capacity;
  guint64 usage;
  double percent;

  guint interval;
  guint source_id;
};

enum {
  PROP_0,

  // PROP_DEVICE,
  // PROP_INTERVAL,
  // PROP_PERCENT,
  // PROP_USAGE,
  // PROP_CAPACITY,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

static GParamSpec *disk_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarDisk, g_barbar_disk, BARBAR_TYPE_SENSOR)

static void g_barbar_disk_start(BarBarSensor *sensor);

static void g_barbar_disk_set_property(GObject *object, guint property_id,
                                       const GValue *value, GParamSpec *pspec) {
  BarBarDisk *disk = BARBAR_DISK(object);

  switch (property_id) {
  // case PROP_DEVICE:
  //   g_barbar_filesystem_set_path(fs, g_value_get_string(value));
  //   break;
  // case PROP_INTERVAL:
  //   g_barbar_filesystem_set_interval(fs, g_value_get_uint(value));
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_disk_get_property(GObject *object, guint property_id,
                                       GValue *value, GParamSpec *pspec) {
  BarBarDisk *disk = BARBAR_DISK(object);

  switch (property_id) {
  // case PROP_DEVICE:
  //   g_value_set_string(value, fs->path);
  //   break;
  // case PROP_INTERVAL:
  //   g_value_set_uint(value, fs->interval);
  //   break;
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
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_disk_set_property;
  gobject_class->get_property = g_barbar_disk_get_property;
  sensor_class->start = g_barbar_disk_start;
}

static void g_barbar_disk_init(BarBarDisk *self) {}

static gboolean g_barbar_disk_update(gpointer data) {
  BarBarDisk *disk = BARBAR_DISK(data);
  glibtop_disk buf;

  glibtop_get_disk(&buf);
  buf.xdisk_sectors_read[0];

  // gchar *str =
  //     g_strdup_printf("percentage_free: %lu%%", buf.bavail * 100 /
  //     buf.blocks);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_disk_start(BarBarSensor *sensor) {
  BarBarDisk *disk = BARBAR_DISK(sensor);

  if (disk->source_id > 0) {
    g_source_remove(disk->source_id);
  }

  // g_barbar_disk_update(fs);
  // fs->source_id =
  //     g_timeout_add_full(0, fs->interval, g_barbar_filesyste_update, fs,
  //     NULL);
}
