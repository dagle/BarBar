#include "barbar-graph.h"

/**
 * BarBarGraph:
 *
 * Display changes over time for a value
 */
struct _BarBarGraph {
  GtkWidget parent_instance;

  double min_value;
  double max_value;
  double current;

  double width;
  double height;

  gboolean fill;
  gboolean discrete;

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
  guint interval;
  // guint64 start_time;

  double period;
  // double amplitude;
};

enum {
  PROP_0,

  PROP_MIN_HEIGHT,
  PROP_MIN_WIDTH,
  PROP_VALUE,
  PROP_MIN_VALUE,
  PROP_MAX_VALUE,
  PROP_HISTORY_LENGTH,
  PROP_STROKE_WIDTH,
  PROP_FILL,
  PROP_INTERVAL,
  PROP_DISCRETE,

  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarGraph, g_barbar_graph, GTK_TYPE_WIDGET)

static void g_barbar_graph_start(GtkWidget *widget);

/**
 * g_barbar_graph_set_stroke_width:
 * @self: a `BarBarGraph`
 * @stroke: Size of the stroke
 */
void g_barbar_graph_set_stroke_width(BarBarGraph *self, float stroke) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->stroke_size == stroke) {
    return;
  }

  self->stroke_size = stroke;
  self->stroke = gsk_stroke_new(stroke);
}

/**
 * g_barbar_graph_set_fill:
 * @self: a `BarBarGraph`
 * @fill: i it should be filled
 *
 * Sets if the graph should be filled or not. Filling
 * means that we color the area between the line and the
 * buttom of the graph
 */
void g_barbar_graph_set_fill(BarBarGraph *self, gboolean fill) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->fill == fill) {
    return;
  }

  self->fill = fill;
  // we need to update here? Just wait for the next tick for now
}

/**
 * g_barbar_graph_history_entries:
 * @self: a `BarBarGraph`
 * @length: length
 *
 * Sets how many entries there should be in the graph (maximum).
 */
void g_barbar_graph_history_entries(BarBarGraph *self, guint length) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->capasity == length) {
    return;
  }

  if (self->capasity > length && self->items > length) {
    // drop values and call it full
    self->full = TRUE;
  } else {
    // if we increase the size, it can't be full anymore
    self->full = FALSE;
  }

  self->capasity = length;
}

/**
 * g_barbar_graph_set_min_value:
 * @self: a `BarBarGraph`
 * @min: min value
 *
 * Sets the lowest value of a graph. This value can also
 * be change by setting main value that is lower than the min value
 */
void g_barbar_graph_set_min_value(BarBarGraph *self, double min) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->min_value == min) {
    return;
  }

  self->min_value = min;
}

/**
 * g_barbar_graph_set_max_value:
 * @self: a `BarBarGraph`
 * @max: max value
 *
 * Sets the maximum value of a graph. This value can also
 * be change by setting main value that is higher than the max value
 */
void g_barbar_graph_set_max_value(BarBarGraph *self, double max) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->max_value == max) {
    return;
  }

  self->max_value = max;
}

/**
 * g_barbar_graph_set_value:
 * @self: a `BarBarGraph`
 * @value: a value
 *
 * Sets the current value of a graph.
 */
void g_barbar_graph_set_value(BarBarGraph *self, double value) {
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

/**
 * g_barbar_graph_set_min_height:
 * @self: a `BarBarGraph`
 * @height: height
 *
 * Set how small the vertical height should be, at minimum. If not
 * set it will be as small as required.
 */
void g_barbar_graph_set_min_height(BarBarGraph *self, guint height) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->height == height) {
    return;
  }

  self->height = height;
}

/**
 * g_barbar_graph_set_min_width:
 * @self: a `BarBarGraph`
 * @width: width
 *
 * Set how small the horizontal width should be, at minimum. If not
 * set it will be as small as required.
 */
void g_barbar_graph_set_min_width(BarBarGraph *self, guint width) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->width == width) {
    return;
  }
  self->width = width;
}

/**
 * g_barbar_graph_set_interval:
 * @self: a `BarBarGraph`
 * @interval: interval
 *
 * How often the graph should tick and move a value to it's history
 */
void g_barbar_graph_set_interval(BarBarGraph *self, guint interval) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->interval == interval) {
    return;
  }
  self->interval = interval;
}

/**
 * g_barbar_graph_set_discrete:
 * @self: a `BarBarGraph`
 * @discrete: if we should operate in descrite mode
 *
 * Changes between discrete and continues mode.
 * If set to discete mode the graph will draw using a series of bars.
 */
void g_barbar_graph_set_discrete(BarBarGraph *self, gboolean discrete) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  if (self->discrete == discrete) {
    return;
  }
  self->discrete = discrete;
}

static void g_barbar_graph_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarGraph *graph = BARBAR_GRAPH(object);

  switch (property_id) {
  case PROP_VALUE:
    g_barbar_graph_set_value(graph, g_value_get_double(value));
    break;
  case PROP_MIN_WIDTH:
    g_barbar_graph_set_min_width(graph, g_value_get_uint(value));
    break;
  case PROP_MIN_HEIGHT:
    g_barbar_graph_set_min_height(graph, g_value_get_uint(value));
    break;
  case PROP_STROKE_WIDTH:
    g_barbar_graph_set_stroke_width(graph, g_value_get_float(value));
    break;
  case PROP_FILL:
    g_barbar_graph_set_fill(graph, g_value_get_boolean(value));
    break;
  case PROP_HISTORY_LENGTH:
    g_barbar_graph_history_entries(graph, g_value_get_uint(value));
    break;
  case PROP_MIN_VALUE:
    g_barbar_graph_set_min_value(graph, g_value_get_double(value));
    break;
  case PROP_MAX_VALUE:
    g_barbar_graph_set_max_value(graph, g_value_get_double(value));
    break;
  case PROP_INTERVAL:
    g_barbar_graph_set_interval(graph, g_value_get_uint(value));
    break;
  case PROP_DISCRETE:
    g_barbar_graph_set_discrete(graph, g_value_get_boolean(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_graph_get_property(GObject *object, guint property_id,
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

static inline double get_y(double height, double max_value, GList *link) {
  double *v = link->data;
  return height - height * (*v / max_value);
}

static void update_path_discrete(BarBarGraph *self) {
  GskPathBuilder *builder;
  builder = gsk_path_builder_new();
  GList *link = self->queue->head;

  if (!link) {
    return;
  }

  double x = 0;

  double y = get_y(self->height, self->max_value, link);
  double delta = self->width / self->capasity;

  gsk_path_builder_move_to(builder, 0, y);
  double next = y;
  while (link) {

    gsk_path_builder_line_to(builder, x, next);
    next = get_y(self->height, self->max_value, link);
    gsk_path_builder_line_to(builder, x, next);
    link = link->next;
    if (link) {
      x += delta;
    }
  }

  if (self->fill) {
    gsk_path_builder_line_to(builder, x, self->height);
    gsk_path_builder_line_to(builder, 0, self->height);

    gsk_path_builder_line_to(builder, 0, y);
  }

  g_clear_pointer(&self->path, gsk_path_unref);
  self->path = gsk_path_builder_free_to_path(builder);
}

static void update_path_continuous(BarBarGraph *self) {
  GskPathBuilder *builder;
  builder = gsk_path_builder_new();
  GList *link = self->queue->head;

  if (!link) {
    return;
  }

  double x = 0;

  double y = get_y(self->height, self->max_value, link);
  double delta = self->width / self->capasity;

  gsk_path_builder_move_to(builder, 0, y);
  while (link) {
    double y = get_y(self->height, self->max_value, link);

    gsk_path_builder_line_to(builder, x, y);
    link = link->next;
    if (link) {
      x += delta;
    }
  }

  if (self->fill) {
    gsk_path_builder_line_to(builder, x, self->height);
    gsk_path_builder_line_to(builder, 0, self->height);

    gsk_path_builder_line_to(builder, 0, y);
  }

  g_clear_pointer(&self->path, gsk_path_unref);
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
  if (self->discrete) {
    update_path_discrete(self);
  } else {
    update_path_continuous(self);
  }
}

static void g_barbar_graph_dispose(GObject *object) {
  BarBarGraph *self = BARBAR_GRAPH(object);

  g_clear_pointer(&self->path, gsk_path_unref);
  gsk_stroke_free(self->stroke);

  G_OBJECT_CLASS(g_barbar_graph_parent_class)->dispose(object);
}

static void g_barbar_graph_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {
  BarBarGraph *self = BARBAR_GRAPH(widget);

  if (self->fill) {
    gtk_snapshot_append_fill(snapshot, self->path, GSK_FILL_RULE_WINDING,
                             &self->color);
  } else {
    gtk_snapshot_append_stroke(snapshot, self->path, self->stroke,
                               &self->color);
  }
}

static void g_barbar_graph_measure(GtkWidget *widget,
                                   GtkOrientation orientation, int for_size,
                                   int *minimum, int *natural,
                                   int *minimum_baseline,
                                   int *natural_baseline) {
  BarBarGraph *self = BARBAR_GRAPH(widget);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    *minimum = *natural = self->width;
  else
    *minimum = *natural = self->height;
}

void g_barbar_graph_size_allocate(GtkWidget *widget, int width, int height,
                                  int baseline) {
  BarBarGraph *self = BARBAR_GRAPH(widget);

  self->width = width;
  self->height = height;
}

static void g_barbar_graph_class_init(BarBarGraphClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_graph_set_property;
  gobject_class->get_property = g_barbar_graph_get_property;

  gobject_class->dispose = g_barbar_graph_dispose;

  widget_class->snapshot = g_barbar_graph_snapshot;
  widget_class->measure = g_barbar_graph_measure;
  widget_class->size_allocate = g_barbar_graph_size_allocate;

  widget_class->root = g_barbar_graph_start;

  /**
   * BarBarRotary:min-width:
   *
   * Minimum width of this widget
   */
  properties[PROP_MIN_WIDTH] = g_param_spec_uint(
      "min-width", NULL, NULL, 0, G_MAXUINT, 30,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarRotary:min-height:
   *
   * Minimum height of this widget
   */
  properties[PROP_MIN_HEIGHT] = g_param_spec_uint(
      "min-height", NULL, NULL, 0, G_MAXUINT, 20,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

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
  properties[PROP_VALUE] =
      g_param_spec_double("value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarGraph:min-value:
   *
   * min-value is a soft value to aid with render, if a value lower
   * is produced, the value is adjusted.
   */
  properties[PROP_MIN_VALUE] = g_param_spec_double(
      "min-value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarGraph:max-value:
   *
   * max-value is a soft value to aid with render, if a value higher
   * value is produced, the value is adjusted.
   */
  properties[PROP_MAX_VALUE] = g_param_spec_double(
      "max-value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarGraph:history-length:
   *
   * How many points of history we should save.
   */
  properties[PROP_HISTORY_LENGTH] = g_param_spec_uint(
      "history-length", NULL, NULL, 0, G_MAXUINT, 100,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarGraph:fill:
   *
   * If the graph should be filled
   */
  properties[PROP_FILL] = g_param_spec_boolean(
      "fill", NULL, NULL, TRUE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarGraph:interval:
   *
   * How often the graph should update
   */
  properties[PROP_INTERVAL] = g_param_spec_uint(
      "interval", NULL, NULL, 0, G_MAXUINT, 1000,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarGraph:discrete:
   *
   * If a discrete graph should be drawn, produces a bar like graph.
   */
  properties[PROP_DISCRETE] = g_param_spec_boolean(
      "discrete", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
  gtk_widget_class_set_css_name(widget_class, "graph");
}

static gboolean g_barbar_graph_update(gpointer data) {

  BarBarGraph *self = BARBAR_GRAPH(data);

  push_update(self, self->current);

  gtk_widget_queue_draw(GTK_WIDGET(self));

  return G_SOURCE_CONTINUE;
}

static void g_barbar_graph_init(BarBarGraph *self) {
  gtk_widget_get_color(GTK_WIDGET(self), &self->color);
  self->queue = g_queue_new();
  g_queue_init(self->queue);

  self->current = 0.0;
  self->min_value = 0.0;
  self->max_value = 1.0;
  self->interval = 1000;

  push_update(self, self->current);
}

static void g_barbar_graph_start(GtkWidget *widget) {
  GTK_WIDGET_CLASS(g_barbar_graph_parent_class)->root(widget);

  BarBarGraph *self = BARBAR_GRAPH(widget);

  self->tick_cb =
      g_timeout_add_full(0, self->interval, g_barbar_graph_update, self, NULL);
}

/**
 * g_barbar_graph_new:
 *
 * Returns: (transfer full): a `BarBarGraph`
 */
GtkWidget *g_barbar_graph_new(void) {
  return g_object_new(BARBAR_TYPE_GRAPH, NULL);
}
