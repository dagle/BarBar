#include "barbar-rotary.h"

struct _BarBarRotary {
  GtkWidget parent_instance;

  double min_value;
  double max_value;
  double cur_value;
  // GtkWidget *label;

  // char *path;
};

enum {
  PROP_0,

  PROP_VALUE,
  // PROP_INVERTED,

  NUM_PROPERTIES,
};

static GParamSpec *rotary_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarRotary, g_barbar_rotary, GTK_TYPE_WIDGET)

// static void g_barbar_rotary_constructed(GObject *object);

static void g_barbar_rotary_set_value(BarBarRotary *rotary, const char *path) {
  g_return_if_fail(BARBAR_IS_ROTARY(rotary));

  // g_free(bar->path);
  // bar->path = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(rotary), rotary_props[PROP_VALUE]);
}

static void g_barbar_rotary_set_property(GObject *object, guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec) {
  BarBarRotary *rotary = BARBAR_ROTARY(object);

  switch (property_id) {
  case PROP_VALUE:
    // g_barbar_disk_set_path(disk, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_rotary_get_property(GObject *object, guint property_id,
                                         GValue *value, GParamSpec *pspec) {
  BarBarRotary *rotary = BARBAR_ROTARY(object);

  switch (property_id) {
  case PROP_VALUE:
    // g_value_set_string(value, disk->path);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

void g_barbar_rotary_measure(GtkWidget *widget, GtkOrientation orientation,
                             int for_size, int *minimum, int *natural,
                             int *minimum_baseline, int *natural_baseline) {
  // GtkCssStyle *style;

  *minimum = *natural = 40;

  // gtk_widget_get_css_node(widget);
}

void g_barbar_rotary_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {
  int x, y, width, height;

  x = 0;
  y = 0;
  width = gtk_widget_get_width(widget);
  height = gtk_widget_get_height(widget);

  // let total_width = self.obj().allocated_width() as f64;
  // let total_height = self.obj().allocated_height() as f64;
  // let center = (total_width / 2.0, total_height / 2.0);
  //
  // let circle_width = total_width - margin.left as f64 - margin.right as f64;
  // let circle_height = total_height as f64 - margin.top as f64 - margin.bottom
  // as f64; let outer_ring = f64::min(circle_width, circle_height) / 2.0; let
  // inner_ring = (f64::min(circle_width, circle_height) / 2.0) - thickness;

  // double xCenter = q.x + q.size / 2;
  // double yCenter = q.y + q.size / 2;
  // double radius = (q.size / 2) - (m_Style.strokeWidth / 2);
  //
  // double beg = m_Style.start * (M_PI / 180);
  // double angle = m_Val * 2 * M_PI;

  // cairo_set_line_width(cr, m_Style.strokeWidth);

  cairo_t *cr = gtk_snapshot_append_cairo(
      snapshot, &GRAPHENE_RECT_INIT(x, y, width, height));

  // cairo_set_source_rgb(cr, bgCol->red, bgCol->green, bgCol->blue);
  cairo_arc(cr, xCenter, yCenter, radius, 0, 2 * M_PI);
  cairo_stroke(cr);

  // Inner
  // cairo_set_source_rgb(cr, fgCol->red, fgCol->green, fgCol->blue);
  cairo_arc(cr, xCenter, yCenter, radius, beg, beg + angle);
  cairo_stroke(cr);
}

void g_barbar_rotary_size_allocate(GtkWidget *widget, int width, int height,
                                   int baseline) {}

GtkSizeRequestMode g_barbar_rotary_get_request_mode(GtkWidget *widget);

static void g_barbar_rotary_class_init(BarBarRotaryClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_rotary_set_property;
  gobject_class->get_property = g_barbar_rotary_get_property;

  widget_class->size_allocate = g_barbar_rotary_size_allocate;
  widget_class->get_request_mode = g_barbar_rotary_get_request_mode;
  widget_class->measure = g_barbar_rotary_measure;
  widget_class->snapshot = g_barbar_rotary_snapshot;
  rotary_props[PROP_VALUE] =
      g_param_spec_string("path", NULL, NULL, NULL, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    rotary_props);
  // gtk_widget_class_set_layout_manager_type(widget_class,
  // GTK_TYPE_BOX_LAYOUT);
}

static void g_barbar_rotary_init(BarBarRotary *self) {
  self->cur_value = 0.0;
  self->min_value = 0.0;
  self->max_value = 1.0;
}

// GtkWidget *g_barbar_disk_new(char *path) {
//   GtkWidget *disk = g_object_new(BARBAR_TYPE_DISK, "path", path);
//   return disk;
// }

// static gboolean g_barbar_disk_update(gpointer data) {
//   BarBarDisk *disk = BARBAR_DISK(data);
//   glibtop_fsusage buf;
//   struct statvfs stats;
//
//   glibtop_get_fsusage(&buf, disk->path);
//
//   gchar *str =
//       g_strdup_printf("percentage_free: %lu%%", buf.bavail * 100 /
//       buf.blocks);
//
//   gtk_label_set_text(GTK_LABEL(disk->label), str);
//
//   g_free(str);
//
//   return G_SOURCE_CONTINUE;
// }
//
// void g_barbar_disk_start(BarBarDisk *disk) {
//   g_barbar_disk_update(disk);
//   g_timeout_add_full(0, 30 * 1000, g_barbar_disk_update, disk, NULL);
// }
