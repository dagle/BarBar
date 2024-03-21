#include "barbar-graph.h"

struct _BarBarGraph {
  GtkWidget parent_instance;

  double min_value;
  double max_value;
  double current;

  GskPath *path;
  GskStroke *stroke;
  float stroke_size;
  GdkRGBA color;

  // Values to plot
  GQueue *queue;
  gsize items;
  gsize capasity;
  gboolean full;

  guint tick_cb;
  // guint64 start_time;

  double period;
  // double amplitude;
};

enum {
  PROP_0,

  PROP_VALUE,
  PROP_MIN_VALUE,
  PROP_MAX_VALUE,
  PROP_STROKE_WIDTH,

  NUM_PROPERTIES,
};

struct _GraphWidgetClass {
  GtkWidgetClass parent_class;
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarGraph, g_barbar_graph, GTK_TYPE_WIDGET)

static void g_barbar_graph_set_stroke_width(BarBarGraph *self, float stroke) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->stroke_size == stroke) {
    return;
  }

  self->stroke_size = stroke;
  self->stroke = gsk_stroke_new(stroke);
}

static void g_barbar_graph_set_min_value(BarBarGraph *self, double min) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->min_value == min) {
    return;
  }

  self->min_value = min;
}

static void g_barbar_graph_set_max_value(BarBarGraph *self, double max) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->max_value == max) {
    return;
  }

  self->max_value = max;
}

static void g_barbar_graph_set_value(BarBarGraph *self, double value) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->current == value) {
    return;
  }

  if (value > self->max_value) {
    g_barbar_graph_set_max_value(self, value);
  }
  if (value < self->min_value) {
    g_barbar_graph_set_min_value(self, value);
  }
  self->current = value;
}

static void g_barbar_graph_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarGraph *graph = BARBAR_GRAPH(object);

  switch (property_id) {
  case PROP_VALUE:
    g_barbar_graph_set_value(graph, g_value_get_double(value));
    break;
  case PROP_STROKE_WIDTH:
    g_barbar_graph_set_stroke_width(graph, g_value_get_float(value));
    break;
  case PROP_MIN_VALUE:
    g_barbar_graph_set_min_value(graph, g_value_get_double(value));
    break;
  case PROP_MAX_VALUE:
    g_barbar_graph_set_max_value(graph, g_value_get_double(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_rotary_get_property(GObject *object, guint property_id,
                                         GValue *value, GParamSpec *pspec) {
  BarBarGraph *graph = BARBAR_GRAPH(object);

  switch (property_id) {
  case PROP_VALUE:
    g_value_set_double(value, graph->current);
    break;
  case PROP_MIN_VALUE:
    g_value_set_double(value, graph->min_value);
    break;
  case PROP_MAX_VALUE:
    g_value_set_double(value, graph->max_value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void update_path(BarBarGraph *self) {
  GskPathBuilder *builder;
  builder = gsk_path_builder_new();
  GList *link = self->queue->head;

  double x = 0;

  gsk_path_builder_move_to(builder, x, 0);
  while (link) {
    double *v = link->data;

    // maybe this should be a softer curve
    // but this should work for now.
    gsk_path_builder_line_to(builder, x, *v);
    link = link->next;
  }
  self->path = gsk_path_builder_free_to_path(builder);
}

static void rotate(GQueue *queue, double value) {
  GList *link = g_queue_pop_head_link(queue);
  // is this needed?
  link->next = NULL;
  link->prev = NULL;
  double *v = link->data;
  *v = value;

  g_queue_push_tail_link(queue, link);
}

static void push_update(BarBarGraph *self, double value) {
  if (self->full) {
    rotate(self->queue, value);
  } else {
    double *v = g_malloc(sizeof(double));
    *v = value;
    g_queue_push_tail(self->queue, v);

    self->items++;
    self->full = self->items >= self->capasity;
  }
  update_path(self);
}

static gboolean tick_cb(GtkWidget *widget, GdkFrameClock *frame_clock,
                        gpointer user_data) {
  BarBarGraph *self = BARBAR_GRAPH(widget);

  push_update(self, self->current);

  gtk_widget_queue_draw(widget);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_graph_dispose(GObject *object) {
  BarBarGraph *self = BARBAR_GRAPH(object);

  g_clear_pointer(&self->path, gsk_path_unref);
  gsk_stroke_free(self->stroke);

  G_OBJECT_CLASS(g_barbar_graph_parent_class)->dispose(object);
}

static void g_barbar_graph_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {
  BarBarGraph *self = BARBAR_GRAPH(widget);

  gtk_snapshot_append_stroke(snapshot, self->path, self->stroke, &self->color);
}

static void g_barbar_graph_measure(GtkWidget *widget,
                                   GtkOrientation orientation, int for_size,
                                   int *minimum, int *natural,
                                   int *minimum_baseline,
                                   int *natural_baseline) {
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    *minimum = *natural = 200;
  else
    *minimum = *natural = 100;
}

void g_barbar_graph_size_allocate(GtkWidget *widget, int width, int height,
                                  int baseline) {}

static void g_barbar_graph_class_init(BarBarGraphClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_graph_set_property;
  gobject_class->get_property = g_barbar_graph_get_property;

  gobject_class->dispose = g_barbar_graph_dispose;

  widget_class->snapshot = g_barbar_graph_snapshot;
  widget_class->measure = g_barbar_graph_measure;
  widget_class->size_allocate = g_barbar_graph_size_allocate;

  /**
   * BarBarGraph:stroke-width:
   *
   * How big pen we use for our graph
   */
  properties[PROP_STROKE_WIDTH] = g_param_spec_float(
      "stroke-width", NULL, NULL, 0.0, G_MAXFLOAT, 2.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  /**
   * BarBarGraph:value:
   *
   * Determines the currently filled value of the rotary.
   */
  properties[PROP_VALUE] = g_param_spec_double(
      "value", NULL, NULL, 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BarBarGraph:min-value:
   *
   * min-value is a soft value to aid with render, if a value lower
   * is produced, the value is adjusted.
   */
  properties[PROP_MIN_VALUE] = g_param_spec_double(
      "min-value", NULL, NULL, 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarGraph:max-value:
   *
   * max-value is a soft value to aid with render, if a value higher
   * value is produced, the value is adjusted.
   */
  properties[PROP_MAX_VALUE] = g_param_spec_double(
      "max-value", NULL, NULL, 0.0, G_MAXDOUBLE, 5.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
}

static void g_barbar_graph_init(BarBarGraph *self) {
  gtk_widget_get_color(GTK_WIDGET(self), &self->color);

  self->current = 0.0;
  self->min_value = 0.0;
  self->max_value = 1.0;

  push_update(self, self->current);

  // self->start_time = 0;

  g_queue_init(self->queue);
  // do we really need to render this often?
  self->tick_cb =
      gtk_widget_add_tick_callback(GTK_WIDGET(self), tick_cb, NULL, NULL);
}

GtkWidget *g_barbar_graph_new(void) {
  return g_object_new(BARBAR_TYPE_GRAPH, NULL);
}
