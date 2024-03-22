#include "barbar-rotary.h"
#include <gsk/gsk.h>

struct _BarBarRotary {
  GtkWidget parent_instance;

  double min_value;
  double max_value;
  double cur_value;

  float procentage;

  int width;
  int height;
  // int baseline;

  gboolean inverted;

  GdkRGBA color;
  GdkRGBA circle_color;
  GskPath *circle;
  GskPath *path;
  GskStroke *stroke;
};

enum {
  PROP_0,

  PROP_BACKGROUND,
  PROP_WIDTH_PROCENT,
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

static void update_rotary(BarBarRotary *self) {
  GskPathBuilder *builder;
  GskPathPoint start, end;
  graphene_point_t p0, p1;
  float start_angle, end_angle;
  int width = self->width / 2;
  int height = self->height / 2;
  int size = MIN(self->height, self->width) / 2 - self->procentage;

  double angle = 90;
  start_angle = angle;
  end_angle = fmod(angle + 360 * self->cur_value, 360);

  p0 = GRAPHENE_POINT_INIT(width + size * cos(M_PI * start_angle / 180),
                           height + size * sin(M_PI * start_angle / 180));
  p1 = GRAPHENE_POINT_INIT(width + size * cos(M_PI * end_angle / 180),
                           height + size * sin(M_PI * end_angle / 180));

  g_clear_pointer(&self->path, gsk_path_unref);

  gsk_path_get_closest_point(self->circle, &p0, INFINITY, &start, NULL);
  gsk_path_get_closest_point(self->circle, &p1, INFINITY, &end, NULL);

  builder = gsk_path_builder_new();
  gsk_path_builder_add_segment(builder, self->circle, &start, &end);
  self->path = gsk_path_builder_free_to_path(builder);

  gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void g_barbar_rotary_set_value_internal(BarBarRotary *self,
                                               double value) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  if (self->cur_value == value) {
    return;
  }

  self->cur_value = value;

  update_rotary(self);
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VALUE]);
}

static void g_barbar_rotary_set_value(BarBarRotary *self, double value) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  g_barbar_rotary_set_value_internal(self, value);
}

static void g_barbar_rotary_set_background(BarBarRotary *self,
                                           const char *background) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  gdk_rgba_parse(&self->circle_color, background);
}

static void update_stroke(BarBarRotary *self) {
  g_clear_pointer(&self->stroke, gsk_stroke_free);

  float size = MIN(self->height, self->width) / 2 * self->procentage;

  self->stroke = gsk_stroke_new(size);
}

static void g_barbar_rotary_set_procentage(BarBarRotary *self, float procent) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  self->procentage = procent;

  update_stroke(self);
}

static void g_barbar_rotary_set_max_value(BarBarRotary *self, double value) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  if (value == self->max_value)
    return;

  self->max_value = value;

  if (self->max_value < self->cur_value) {
    g_barbar_rotary_set_value_internal(self, self->max_value);
  }

  // update_block_nodes(self);
  // update_level_style_classes(self);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MAX_VALUE]);
}

static void g_barbar_rotary_set_min_value(BarBarRotary *self, double value) {
  g_return_if_fail(BARBAR_IS_ROTARY(self));

  if (value == self->min_value)
    return;

  self->min_value = value;

  if (self->min_value > self->cur_value) {
    g_barbar_rotary_set_value_internal(self, self->min_value);
  }

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
    break;
  case PROP_BACKGROUND:
    g_barbar_rotary_set_background(rotary, g_value_get_string(value));
    break;
  case PROP_WIDTH_PROCENT:
    g_barbar_rotary_set_procentage(rotary, g_value_get_float(value));
    break;
  case PROP_MIN_VALUE:
    g_barbar_rotary_set_min_value(rotary, g_value_get_double(value));
    break;
  case PROP_MAX_VALUE:
    g_barbar_rotary_set_max_value(rotary, g_value_get_double(value));
    break;
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
    break;
  case PROP_MIN_VALUE:
    g_value_set_double(value, rotary->min_value);
    break;
  case PROP_MAX_VALUE:
    g_value_set_double(value, rotary->max_value);
    break;
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

  BarBarRotary *self = BARBAR_ROTARY(widget);
  GtkWidget *parent = gtk_widget_get_parent(widget);
  // int width = gtk_widget_get_width(parent);
  // int height = gtk_widget_get_height(parent);
  // printf("measure: height:%d width:%d\n", width, height);

  // int new_minimum, new_natural, new_minimum_baseline, new_natural_baseline;
  // gtk_widget_measure(parent, orientation, for_size, &new_minimum,
  // &new_natural,
  //                    &new_minimum_baseline, &new_natural_baseline);
  //
  // printf(
  //     "new measure: orientation: %d, for_size: %d, minimum: %d, natural: %d,
  //     " "minimum_baseline: %d, natural_baseline: %d\n", orientation,
  //     for_size, new_minimum, new_natural, new_minimum_baseline,
  //     new_natural_baseline);

  // TODO: do we have a minimal/natural size?
  printf("measure: orientation: %d, for_size: %d, minimum: %d, natural: %d, "
         "minimum_baseline: %d, natural_baseline: %d\n",
         orientation, for_size, *minimum, *natural, *minimum_baseline,
         *natural_baseline);
  int size = MIN(self->height, self->width);

  *minimum = *natural = size;
}

void g_barbar_rotary_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {

  BarBarRotary *self = BARBAR_ROTARY(widget);

  gtk_snapshot_append_stroke(snapshot, self->circle, self->stroke,
                             &self->circle_color);
  gtk_snapshot_append_stroke(snapshot, self->path, self->stroke, &self->color);
}

static void update_circle(BarBarRotary *self) {
  GskPathBuilder *builder;

  int width = self->width / 2;
  int height = self->height / 2;

  update_stroke(self);

  float stroke_size = MIN(self->height, self->width) / 2 * self->procentage;
  int size = MIN(self->height, self->width) / 2 - (stroke_size / 2);

  builder = gsk_path_builder_new();
  gsk_path_builder_add_circle(builder, &GRAPHENE_POINT_INIT(width, height),
                              size);

  g_clear_pointer(&self->circle, gsk_path_unref);

  self->circle = gsk_path_builder_free_to_path(builder);
}

void g_barbar_rotary_size_allocate(GtkWidget *widget, int width, int height,
                                   int baseline) {
  BarBarRotary *self = BARBAR_ROTARY(widget);

  printf("allocate: width: %d, height: %d\n", width, height);

  self->width = width;
  self->height = height;
  update_circle(self);
}

static void g_barbar_rotary_class_init(BarBarRotaryClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_rotary_set_property;
  gobject_class->get_property = g_barbar_rotary_get_property;

  widget_class->size_allocate = g_barbar_rotary_size_allocate;
  // widget_class->get_request_mode = g_barbar_rotary_get_request_mode;
  widget_class->measure = g_barbar_rotary_measure;
  widget_class->snapshot = g_barbar_rotary_snapshot;

  /**
   * BarBarRotary:background:
   *
   * The background color to use
   */
  properties[PROP_BACKGROUND] = g_param_spec_string(
      "background", NULL, NULL, "lightgray",
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  /**
   * BarBarRotary:width-procent:
   *
   * How much of the widget should be filled
   */
  properties[PROP_WIDTH_PROCENT] = g_param_spec_float(
      "width-procent", NULL, NULL, 0.0, 1.0, 0.40,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

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
  properties[PROP_MIN_VALUE] =
      g_param_spec_double("min-value", NULL, NULL, 0.0, G_MAXDOUBLE, 0.0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY | G_PARAM_CONSTRUCT);

  /**
   * BarBarRotary:max-value:
   *
   * Determines the maximum value of the interval that can be displayed by the
   * rotary.
   */
  properties[PROP_MAX_VALUE] =
      g_param_spec_double("max-value", NULL, NULL, 0.0, G_MAXDOUBLE, 1.0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY | G_PARAM_CONSTRUCT);

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
}

static void g_barbar_rotary_init(BarBarRotary *self) {
  GskPathBuilder *builder;

  self->cur_value = 0.5;
  self->min_value = 0.0;
  self->max_value = 1.0;

  self->inverted = FALSE;
  self->width = 23;
  self->height = 23;

  update_circle(self);

  gtk_widget_get_color(GTK_WIDGET(self), &self->color);

  update_rotary(self);
}

GtkWidget *g_barbar_rotary_new(void) {
  BarBarRotary *self;

  self = g_object_new(BARBAR_TYPE_ROTARY, NULL);

  return GTK_WIDGET(self);
}
