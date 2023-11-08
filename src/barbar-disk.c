#include "barbar-disk.h"
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
  struct statvfs stats;

  int err = statvfs(disk->path, &stats);

  if (err) {
    return G_SOURCE_REMOVE;
  }

  // g_barbar_clock_update(disk);
  // printf("percentage_free: %lu%%\n", stats.f_bavail * 100 / stats.f_blocks);
  gchar *str = g_strdup_printf("percentage_free: %lu%%\n",
                               stats.f_bavail * 100 / stats.f_blocks);

  gtk_label_set_text(GTK_LABEL(disk->label), str);

  g_free(str);

  return G_SOURCE_CONTINUE;
}

void g_barbar_disk_start(BarBarDisk *disk) {
  // if (clock->source_id > 0) {
  //   g_source_remove(clock->source_id);
  // }
  g_barbar_disk_update(disk);
  g_timeout_add_full(0, 30 * 1000, g_barbar_disk_update, disk, NULL);
}

// auto waybar::modules::Disk::update() -> void {
//   struct statvfs /* {
//       unsigned long  f_bsize;    // filesystem block size
//       unsigned long  f_frsize;   // fragment size
//       fsblkcnt_t     f_blocks;   // size of fs in f_frsize units
//       fsblkcnt_t     f_bfree;    // # free blocks
//       fsblkcnt_t     f_bavail;   // # free blocks for unprivileged users
//       fsfilcnt_t     f_files;    // # inodes
//       fsfilcnt_t     f_ffree;    // # free inodes
//       fsfilcnt_t     f_favail;   // # free inodes for unprivileged users
//       unsigned long  f_fsid;     // filesystem ID
//       unsigned long  f_flag;     // mount flags
//       unsigned long  f_namemax;  // maximum filename length
//   }; */
//       stats;
//   int err = statvfs(path_.c_str(), &stats);
//
//   /* Conky options
//     fs_bar - Bar that shows how much space is used
//     fs_free - Free space on a file system
//     fs_free_perc - Free percentage of space
//     fs_size - File system size
//     fs_used - File system used space
//   */
//
//   if (err != 0) {
//     event_box_.hide();
//     return;
//   }
//
//   float specific_free, specific_used, specific_total, divisor;
//
//   divisor = calc_specific_divisor(unit_);
//   specific_free = (stats.f_bavail * stats.f_frsize) / divisor;
//   specific_used = ((stats.f_blocks - stats.f_bfree) * stats.f_frsize) /
//   divisor; specific_total = (stats.f_blocks * stats.f_frsize) / divisor;
//
//   auto free = pow_format(stats.f_bavail * stats.f_frsize, "B", true);
//   auto used = pow_format((stats.f_blocks - stats.f_bfree) * stats.f_frsize,
//   "B", true); auto total = pow_format(stats.f_blocks * stats.f_frsize, "B",
//   true); auto percentage_used = (stats.f_blocks - stats.f_bfree) * 100 /
//   stats.f_blocks;
//
//   auto format = format_;
//   auto state = getState(percentage_used);
//   if (!state.empty() && config_["format-" + state].isString()) {
//     format = config_["format-" + state].asString();
//   }
//
//   if (format.empty()) {
//     event_box_.hide();
//   } else {
//     event_box_.show();
//     label_.set_markup(fmt::format(
//         fmt::runtime(format), stats.f_bavail * 100 / stats.f_blocks,
//         fmt::arg("free", free), fmt::arg("percentage_free", stats.f_bavail *
//         100 / stats.f_blocks), fmt::arg("used", used),
//         fmt::arg("percentage_used", percentage_used), fmt::arg("total",
//         total), fmt::arg("path", path_), fmt::arg("specific_free",
//         specific_free), fmt::arg("specific_used", specific_used),
//         fmt::arg("specific_total", specific_total)));
//   }
//
//   if (tooltipEnabled()) {
//     std::string tooltip_format = "{used} used out of {total} on {path}
//     ({percentage_used}%)"; if (config_["tooltip-format"].isString()) {
//       tooltip_format = config_["tooltip-format"].asString();
//     }
//     label_.set_tooltip_text(fmt::format(
//         fmt::runtime(tooltip_format), stats.f_bavail * 100 / stats.f_blocks,
//         fmt::arg("free", free), fmt::arg("percentage_free", stats.f_bavail *
//         100 / stats.f_blocks), fmt::arg("used", used),
//         fmt::arg("percentage_used", percentage_used), fmt::arg("total",
//         total), fmt::arg("path", path_), fmt::arg("specific_free",
//         specific_free), fmt::arg("specific_used", specific_used),
//         fmt::arg("specific_total", specific_total)));
//   }
//   // Call parent update
//   ALabel::update();
// }
//
// float waybar::modules::Disk::calc_specific_divisor(std::string divisor) {
//   if (divisor == "kB") {
//     return 1000.0;
//   } else if (divisor == "kiB") {
//     return 1024.0;
//   } else if (divisor == "MB") {
//     return 1000.0 * 1000.0;
//   } else if (divisor == "MiB") {
//     return 1024.0 * 1024.0;
//   } else if (divisor == "GB") {
//     return 1000.0 * 1000.0 * 1000.0;
//   } else if (divisor == "GiB") {
//     return 1024.0 * 1024.0 * 1024.0;
//   } else if (divisor == "TB") {
//     return 1000.0 * 1000.0 * 1000.0 * 1000.0;
//   } else if (divisor == "TiB") {
//     return 1024.0 * 1024.0 * 1024.0 * 1024.0;
//   } else {  // default to Bytes if it is anything that we don't recongnise
//     return 1.0;
//   }
// }
