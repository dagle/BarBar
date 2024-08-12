#include "barbar-filesystem.h"
#include <glibtop.h>
#include <glibtop/fsusage.h>
#include <stdio.h>

/**
 * BarBarFilesystem:
 *
 * A simple filesystem sensor
 */
struct _BarBarFilesystem {
  BarBarIntervalSensor parent_instance;

  char *path;

  guint64 capacity;
  guint64 usage;
  guint32 blocksize;
  double percent;

  guint interval;
  guint source_id;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_PERCENT,
  PROP_USAGE,
  PROP_CAPACITY,
  PROP_BLOCKSIZE,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

static GParamSpec *fs_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarFilesystem, g_barbar_filesystem,
              BARBAR_TYPE_INTERVAL_SENSOR)

static gboolean g_barbar_filesystem_tick(BarBarIntervalSensor *sensor);

static void g_barbar_filesystem_set_path(BarBarFilesystem *bar,
                                         const char *path) {
  g_return_if_fail(BARBAR_IS_FILESYSTEM(bar));

  g_free(bar->path);
  bar->path = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(bar), fs_props[PROP_DEVICE]);
}

static void g_barbar_filesystem_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {
  BarBarFilesystem *fs = BARBAR_FILESYSTEM(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_filesystem_set_path(fs, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_filesystem_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec) {
  BarBarFilesystem *fs = BARBAR_FILESYSTEM(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_value_set_string(value, fs->path);
    break;
  case PROP_PERCENT:
    g_value_set_double(value, fs->percent);
    break;
  case PROP_USAGE:
    g_value_set_uint64(value, fs->usage);
    break;
  case PROP_CAPACITY:
    g_value_set_uint64(value, fs->capacity);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_filesystem_class_init(BarBarFilesystemClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_filesystem_set_property;
  gobject_class->get_property = g_barbar_filesystem_get_property;
  interval_class->tick = g_barbar_filesystem_tick;

  /**
   * BarBarFilesystem:path:
   *
   * Path to the filesystem
   */
  fs_props[PROP_DEVICE] = g_param_spec_string(
      "path", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarFilesystem:percent:
   *
   * How much of the cpu is used.
   */
  fs_props[PROP_PERCENT] =
      g_param_spec_double("percent", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);

  /**
   * BarBarFilesystem:usage:
   *
   * How much of the cpu is used.
   */
  fs_props[PROP_USAGE] =
      g_param_spec_uint64("usage", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);

  /**
   * BarBarFilesystem:capacity:
   *
   * How much of the cpu is used.
   */
  fs_props[PROP_CAPACITY] =
      g_param_spec_uint64("capacity", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);

  /**
   * BarBarFilesystem:block_size:
   *
   * block size of the filesystem
   */
  fs_props[PROP_BLOCKSIZE] = g_param_spec_uint64("block_size", NULL, NULL, 0,
                                                 100, 0, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, fs_props);
}

static void g_barbar_filesystem_init(BarBarFilesystem *self) {}

static void g_barbar_filesystem_update_percent(BarBarFilesystem *fs) {

  guint64 usage = fs->usage;
  guint64 capacity = fs->capacity;

  while (capacity > G_MAXDOUBLE) {
    capacity = capacity / 1000;
    usage = capacity / 1000;
  }

  double percent = (double)usage / capacity;

  if (fs->percent == percent) {
    return;
  }

  fs->percent = percent;

  g_object_notify_by_pspec(G_OBJECT(fs), fs_props[PROP_PERCENT]);
}

static void g_barbar_filesystem_update_aviable(BarBarFilesystem *fs,
                                               guint64 bavail) {

  g_return_if_fail(BARBAR_IS_FILESYSTEM(fs));
  guint64 usage = fs->capacity - bavail * fs->blocksize;

  if (fs->usage == usage) {
    return;
  }

  fs->usage = usage;

  g_object_notify_by_pspec(G_OBJECT(fs), fs_props[PROP_USAGE]);
}

static void g_barbar_filesystem_update_blocks(BarBarFilesystem *fs,
                                              guint64 blocks) {
  g_return_if_fail(BARBAR_IS_FILESYSTEM(fs));
  guint64 capacity = blocks * fs->blocksize;

  if (fs->capacity == capacity) {
    return;
  }

  fs->capacity = capacity;

  g_object_notify_by_pspec(G_OBJECT(fs), fs_props[PROP_CAPACITY]);
}

static void g_barbar_filesystem_update_blocksize(BarBarFilesystem *fs,
                                                 guint32 blocksize) {

  g_return_if_fail(BARBAR_IS_FILESYSTEM(fs));

  if (fs->blocksize == blocksize) {
    return;
  }

  fs->blocksize = blocksize;

  g_object_notify_by_pspec(G_OBJECT(fs), fs_props[PROP_BLOCKSIZE]);
}

static gboolean g_barbar_filesystem_tick(BarBarIntervalSensor *sensor) {
  BarBarFilesystem *fs = BARBAR_FILESYSTEM(sensor);

  glibtop_fsusage buf;

  glibtop_get_fsusage(&buf, fs->path);
  g_barbar_filesystem_update_blocksize(fs, buf.block_size);
  g_barbar_filesystem_update_blocks(fs, buf.blocks);
  g_barbar_filesystem_update_aviable(fs, buf.bavail);
  g_barbar_filesystem_update_percent(fs);

  return G_SOURCE_CONTINUE;
}
