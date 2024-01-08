#include "barbar-rotary.h"
#include <gsk/gsk.h>

struct _BarBarRotary {
  GtkWidget parent_instance;

  double min_value;
  double max_value;
  double cur_value;

  gboolean inverted;

  GskPath *circle;
  GskPath *path;
  GskStroke *stroke;
};

enum {
  PROP_0,

  PROP_VALUE,
  PROP_MIN_VALUE,
  PROP_MAX_VALUE,
  PROP_INVERTED,

  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarRotary, g_barbar_rotary, GTK_TYPE_WIDGET)

static void g_barbar_rotary_set_value_internal(BarBarRotary *self,
                                               double value) {
  self->cur_value = value;

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VALUE]);

  // gtk_widget_queue_allocate(GTK_WIDGET(self->trough_widget));
}

static void g_barbar_rotary_set_value(BarBarRotary *self, double value) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  g_barbar_rotary_set_value_internal(self, value);
}

static void g_barbar_rotary_set_max_value(BarBarRotary *self, double value) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  if (value == self->max_value)
    return;

  self->max_value = value;

  if (self->max_value < self->cur_value)
    g_barbar_rotary_set_value_internal(self, self->max_value);

  // update_block_nodes(self);
  // update_level_style_classes(self);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MAX_VALUE]);
}

static void g_barbar_rotary_set_min_value(BarBarRotary *self, double value) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  if (value == self->min_value)
    return;

  self->min_value = value;

  if (self->min_value > self->cur_value)
    g_barbar_rotary_set_value_internal(self, self->min_value);

  // update_block_nodes(self);
  // update_level_style_classes(self);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIN_VALUE]);
}

static void g_barbar_rotary_set_inverted(BarBarRotary *rotary, gboolean value) {
  g_return_if_fail(BARBAR_IS_ROTARY(rotary));

  if (rotary->inverted == value) {
    return;
  }
  rotary->inverted = value;

  g_object_notify_by_pspec(G_OBJECT(rotary), properties[PROP_INVERTED]);
}

static void g_barbar_rotary_set_property(GObject *object, guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec) {
  BarBarRotary *rotary = BARBAR_ROTARY(object);

  switch (property_id) {
  case PROP_VALUE:
    g_barbar_rotary_set_value(rotary, g_value_get_double(value));
  case PROP_MIN_VALUE:
    g_barbar_rotary_set_min_value(rotary, g_value_get_double(value));
  case PROP_MAX_VALUE:
    g_barbar_rotary_set_max_value(rotary, g_value_get_double(value));
  case PROP_INVERTED:
    g_barbar_rotary_set_inverted(rotary, g_value_get_double(value));
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
    g_value_set_double(value, rotary->cur_value);
  case PROP_MIN_VALUE:
    g_value_set_double(value, rotary->min_value);
  case PROP_MAX_VALUE:
    g_value_set_double(value, rotary->max_value);
  case PROP_INVERTED:
    g_value_set_boolean(value, rotary->inverted);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

void g_barbar_rotary_measure(GtkWidget *widget, GtkOrientation orientation,
                             int for_size, int *minimum, int *natural,
                             int *minimum_baseline, int *natural_baseline) {
  *minimum = *natural = 100;
}

void g_barbar_rotary_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {

  BarBarRotary *rotary = BARBAR_ROTARY(widget);
  int x, y, width, height;

  x = 0;
  y = 0;

  builder = gsk_path_builder_new();
  gsk_path_builder_add_circle(builder, &GRAPHENE_POINT_INIT(50, 50), 40);
  self->circle = gsk_path_builder_free_to_path(builder);

  // self->stroke = gsk_stroke_new (5);

  // width = gtk_widget_get_width(widget);
  // height = gtk_widget_get_height(widget);

  // GskPathBuilder *builder;

  gtk_snapshot_append_stroke(snapshot, self->circle, self->stroke,
                             &self->circle_color);
  gtk_snapshot_append_stroke(snapshot, self->path, self->stroke, &self->color);

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

  // GskPathBuilder *builder;
  // GskPathPoint start, end;
  // graphene_point_t p0, p1;
  // float start_angle, end_angle;
  //
  // start_angle = self->angle;
  // end_angle = fmod (self->angle + 360 * self->completion / 100, 360);
  //
  // p0 = GRAPHENE_POINT_INIT (50 + 40 * cos (M_PI * start_angle / 180),
  //                           50 + 40 * sin (M_PI * start_angle / 180));
  // p1 = GRAPHENE_POINT_INIT (50 + 40 * cos (M_PI * end_angle / 180),
  //                           50 + 40 * sin (M_PI * end_angle / 180));
  //
  // g_clear_pointer (&self->path, gsk_path_unref);
  //
  // gsk_path_get_closest_point (self->circle, &p0, INFINITY, &start, NULL);
  // gsk_path_get_closest_point (self->circle, &p1, INFINITY, &end, NULL);
  //
  // builder = gsk_path_builder_new ();
  // gsk_path_builder_add_segment (builder, self->circle, &start, &end);
  // self->path = gsk_path_builder_free_to_path (builder);

#ifdef SHOW_CONTROLS
  g_clear_pointer(&self->controls, gsk_path_unref);
  builder = gsk_path_builder_new();
  gsk_path_foreach(self->path, -1, add_controls, builder);
  self->controls = gsk_path_builder_free_to_path(builder);
#endif

  // gdk_paintable_invalidate_contents (GDK_PAINTABLE (self));
}

void g_barbar_rotary_size_allocate(GtkWidget *widget, int width, int height,
                                   int baseline) {}

static GtkSizeRequestMode g_barbar_rotary_get_request_mode(GtkWidget *widget);
static GtkSizeRequestMode g_barbar_rotary_get_request_mode(GtkWidget *widget) {
  return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void g_barbar_rotary_class_init(BarBarRotaryClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_rotary_set_property;
  gobject_class->get_property = g_barbar_rotary_get_property;

  widget_class->size_allocate = g_barbar_rotary_size_allocate;
  widget_class->get_request_mode = g_barbar_rotary_get_request_mode;
  widget_class->measure = g_barbar_rotary_measure;
  widget_class->snapshot = g_barbar_rotary_snapshot;

  /**
   * BarBarRotary:value:
   *
   * Determines the currently filled value of the rotary.
   */
  properties[PROP_VALUE] = g_param_spec_double(
      "value", NULL, NULL, 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BarBarRotary:min-value:
   *
   * Determines the minimum value of the interval that can be displayed by the
   * rotary.
   */
  properties[PROP_MIN_VALUE] = g_param_spec_double(
      "min-value", NULL, NULL, 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BarBarRotary:max-value:
   *
   * Determines the maximum value of the interval that can be displayed by the
   * rotary.
   */
  properties[PROP_MAX_VALUE] = g_param_spec_double(
      "max-value", NULL, NULL, 0.0, G_MAXDOUBLE, 1.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BarBarRotary:inverted:
   *
   * Whether the `BarBarRotary` is inverted.
   *
   * Rotary normally grow from left to right.
   * Inverted rotaries grow in the opposite direction.
   */
  properties[PROP_INVERTED] = g_param_spec_boolean(
      "inverted", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
  gtk_widget_class_set_css_name(widget_class, "rotary");
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
