#include "barbar-filesystem.h"
#include <glibtop.h>
#include <glibtop/fsusage.h>
#include <stdio.h>
#include <sys/statvfs.h>

/**
 * BarBarFilesystem:
 *
 * A simple filesystem sensor
 */
struct _BarBarFilesystem {
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

  PROP_DEVICE,
  PROP_INTERVAL,
  PROP_PERCENT,
  PROP_USAGE,
  PROP_CAPACITY,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

static GParamSpec *fs_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarFilesystem, g_barbar_filesystem, BARBAR_TYPE_SENSOR)

static void g_barbar_filesystem_start(BarBarSensor *sensor);

static void g_barbar_filesystem_set_path(BarBarFilesystem *bar,
                                         const char *path) {
  g_return_if_fail(BARBAR_IS_FILESYSTEM(bar));

  g_free(bar->path);
  bar->path = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(bar), fs_props[PROP_DEVICE]);
}

static void g_barbar_filesystem_set_interval(BarBarFilesystem *fs,
                                             guint inteval) {
  g_return_if_fail(BARBAR_IS_FILESYSTEM(fs));

  fs->interval = inteval;

  g_object_notify_by_pspec(G_OBJECT(fs), fs_props[PROP_INTERVAL]);
}

static void g_barbar_filesystem_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {
  BarBarFilesystem *fs = BARBAR_FILESYSTEM(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_filesystem_set_path(fs, g_value_get_string(value));
    break;
  case PROP_INTERVAL:
    g_barbar_filesystem_set_interval(fs, g_value_get_uint(value));
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
  case PROP_INTERVAL:
    g_value_set_uint(value, fs->interval);
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
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_filesystem_set_property;
  gobject_class->get_property = g_barbar_filesystem_get_property;
  sensor_class->start = g_barbar_filesystem_start;

  /**
   * BarBarFilesystem:path:
   *
   * Path to the filesystem
   */
  fs_props[PROP_DEVICE] = g_param_spec_string(
      "path", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarFilesystem:interval:
   *
   * How often the cpu should be pulled for info
   */
  fs_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

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

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, fs_props);
}

static void g_barbar_filesystem_init(BarBarFilesystem *self) {}

static gboolean g_barbar_filesyste_update(gpointer data) {
  BarBarFilesystem *fs = BARBAR_FILESYSTEM(data);
  glibtop_fsusage buf;

  glibtop_get_fsusage(&buf, fs->path);

  // gchar *str =
  //     g_strdup_printf("percentage_free: %lu%%", buf.bavail * 100 /
  //     buf.blocks);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_filesystem_start(BarBarSensor *sensor) {
  BarBarFilesystem *fs = BARBAR_FILESYSTEM(sensor);

  if (fs->source_id > 0) {
    g_source_remove(fs->source_id);
  }

  g_barbar_filesyste_update(fs);
  fs->source_id =
      g_timeout_add_full(0, fs->interval, g_barbar_filesyste_update, fs, NULL);
}
