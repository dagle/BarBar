#include "barbar-disk.h"
#include <glibtop.h>
#include <glibtop/fsusage.h>
#include <stdio.h>
#include <sys/statvfs.h>

struct _BarBarDisk {
  GtkWidget parent_instance;

  GtkWidget *label;

  char *path;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

static GParamSpec *disk_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarDisk, g_barbar_disk, GTK_TYPE_WIDGET)

static void g_barbar_disk_constructed(GObject *object);

static void g_barbar_disk_set_path(BarBarDisk *bar, const char *path) {
  g_return_if_fail(BARBAR_IS_DISK(bar));

  g_free(bar->path);
  bar->path = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(bar), disk_props[PROP_DEVICE]);
}

static void g_barbar_disk_set_property(GObject *object, guint property_id,
                                       const GValue *value, GParamSpec *pspec) {
  BarBarDisk *disk = BARBAR_DISK(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_disk_set_path(disk, g_value_get_string(value));
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
    g_value_set_string(value, disk->path);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_disk_class_init(BarBarDiskClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_disk_set_property;
  gobject_class->get_property = g_barbar_disk_get_property;
  gobject_class->constructed = g_barbar_disk_constructed;
  disk_props[PROP_DEVICE] =
      g_param_spec_string("path", NULL, NULL, NULL, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, disk_props);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}

static void g_barbar_disk_init(BarBarDisk *self) {}

static void g_barbar_disk_constructed(GObject *object) {
  BarBarDisk *disk = BARBAR_DISK(object);

  G_OBJECT_CLASS(g_barbar_disk_parent_class)->constructed(object);

  disk->label = gtk_label_new("");
  gtk_widget_set_parent(disk->label, GTK_WIDGET(disk));
}

GtkWidget *g_barbar_disk_new(char *path) {
  GtkWidget *disk = g_object_new(BARBAR_TYPE_DISK, "path", path);
  return disk;
}

static gboolean g_barbar_disk_update(gpointer data) {
  BarBarDisk *disk = BARBAR_DISK(data);
  glibtop_fsusage buf;
  struct statvfs stats;

  glibtop_get_fsusage(&buf, disk->path);

  gchar *str =
      g_strdup_printf("percentage_free: %lu%%", buf.bavail * 100 / buf.blocks);

  gtk_label_set_text(GTK_LABEL(disk->label), str);

  g_free(str);

  return G_SOURCE_CONTINUE;
}

void g_barbar_disk_start(BarBarDisk *disk) {
  g_barbar_disk_update(disk);
  g_timeout_add_full(0, 30 * 1000, g_barbar_disk_update, disk, NULL);
}
