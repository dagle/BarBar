#include "barbar-graph.h"

enum {
  PROP_0,

  PROP_MIN_HEIGHT,
  PROP_MIN_WIDTH,
  PROP_MIN_VALUE,
  PROP_MAX_VALUE,
  PROP_HISTORY_LENGTH,
  PROP_STROKE_WIDTH,
  PROP_FILL,
  PROP_DISCRETE,

  NUM_PROPERTIES,
};

/**
 * BarBarGraph:
 *
 * Display changes over time for a value
 */
typedef struct {
  GtkWidgetClass parent_instance;

  double min_value;
  double max_value;

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
} BarBarGraphPrivate;

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE_WITH_PRIVATE(BarBarGraph, g_barbar_graph, GTK_TYPE_WIDGET)

static void push_update(BarBarGraph *self, double value);
static void update_path_discrete(BarBarGraph *self);
static void update_path_continuous(BarBarGraph *self);

/**
 * g_barbar_graph_set_stroke_width:
 * @self: a `BarBarGraph`
 * @stroke: Size of the stroke
 */
void g_barbar_graph_set_stroke_width(BarBarGraph *self, float stroke) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->stroke_size == stroke) {
    return;
  }

  private->stroke_size = stroke;
  private->stroke = gsk_stroke_new(stroke);
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
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->fill == fill) {
    return;
  }

  private->fill = fill;
  // we need to update here? Just wait for the next tick for now
}

/**
 * g_barbar_graph_set_history_entries:
 * @self: a `BarBarGraph`
 * @length: length
 *
 * Sets how many entries there should be in the graph (maximum).
 */
void g_barbar_graph_set_entry_numbers(BarBarGraph *self, guint length) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->capasity == length) {
    return;
  }

  if (private->capasity > length && private->items > length) {
    // drop values and call it full
    private->full = TRUE;
  } else {
    // if we increase the size, it can't be full anymore
    private->full = FALSE;
  }

  private->capasity = length;
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
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->min_value == min) {
    return;
  }

  private->min_value = min;
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

  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->max_value == max) {
    return;
  }

  private->max_value = max;
}

GQueue *g_barbar_graph_get_queue(BarBarGraph *self) {
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  return private->queue;
}

void g_barbar_graph_set_entries(BarBarGraph *self, GQueue *queue) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->queue != queue) {
    if (private->queue) {
      g_queue_free_full(queue, free);
    }
    private->queue = queue;
  }

  GList *link = private->queue->head;

  while (link) {
    double value = *(double *)link->data;
    if (value > private->max_value) {
      g_barbar_graph_set_max_value(self, value);
    }
    if (value < private->min_value) {
      g_barbar_graph_set_min_value(self, value);
    }
    link = link->next;
  }

  if (private->discrete) {
    update_path_discrete(self);
  } else {
    update_path_continuous(self);
  }
}
/**
 * g_barbar_graph_set_push_entry:
 * @self: a `BarBarGraph`
 * @value: a value
 *
 * Sets the current value of a graph.
 */
void g_barbar_graph_set_push_entry(BarBarGraph *self, double value) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (value > private->max_value) {
    g_barbar_graph_set_max_value(self, value);
  }
  if (value < private->min_value) {
    g_barbar_graph_set_min_value(self, value);
  }

  push_update(self, value);
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

  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->height == height) {
    return;
  }

  private->height = height;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIN_HEIGHT]);
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

  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->width == width) {
    return;
  }

  private->width = width;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIN_WIDTH]);
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

  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->discrete == discrete) {
    return;
  }
  private->discrete = discrete;
}

static void g_barbar_graph_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarGraph *graph = BARBAR_GRAPH(object);

  switch (property_id) {
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
    g_barbar_graph_set_entry_numbers(graph, g_value_get_uint(value));
    break;
  case PROP_MIN_VALUE:
    g_barbar_graph_set_min_value(graph, g_value_get_double(value));
    break;
  case PROP_MAX_VALUE:
    g_barbar_graph_set_max_value(graph, g_value_get_double(value));
    break;
  // case PROP_INTERVAL:
  //   g_barbar_graph_set_interval(graph, g_value_get_uint(value));
  //   break;
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
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(graph);

  switch (property_id) {
  case PROP_MIN_VALUE:
    g_value_set_double(value, private->min_value);
    break;
  case PROP_MAX_VALUE:
    g_value_set_double(value, private->max_value);
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
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  builder = gsk_path_builder_new();
  GList *link = private->queue->head;

  if (!link) {
    return;
  }

  double x = 0;

  double y = get_y(private->height, private->max_value, link);
  double delta = private->width / private->capasity;

  gsk_path_builder_move_to(builder, 0, y);
  double next = y;
  while (link) {

    gsk_path_builder_line_to(builder, x, next);
    next = get_y(private->height, private->max_value, link);
    gsk_path_builder_line_to(builder, x, next);
    link = link->next;
    if (link) {
      x += delta;
    }
  }

  if (private->fill) {
    gsk_path_builder_line_to(builder, x, private->height);
    gsk_path_builder_line_to(builder, 0, private->height);

    gsk_path_builder_line_to(builder, 0, y);
  }

  g_clear_pointer(&private->path, gsk_path_unref);
  private->path = gsk_path_builder_free_to_path(builder);
}

static void update_path_continuous(BarBarGraph *self) {
  GskPathBuilder *builder;
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  builder = gsk_path_builder_new();
  GList *link = private->queue->head;

  if (!link) {
    return;
  }

  double x = 0;

  double y = get_y(private->height, private->max_value, link);
  double delta = private->width / private->capasity;

  gsk_path_builder_move_to(builder, 0, y);
  while (link) {
    double y = get_y(private->height, private->max_value, link);

    gsk_path_builder_line_to(builder, x, y);
    link = link->next;
    if (link) {
      x += delta;
    }
  }

  if (private->fill) {
    gsk_path_builder_line_to(builder, x, private->height);
    gsk_path_builder_line_to(builder, 0, private->height);

    gsk_path_builder_line_to(builder, 0, y);
  }

  g_clear_pointer(&private->path, gsk_path_unref);
  private->path = gsk_path_builder_free_to_path(builder);
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
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->full) {
    rotate(private->queue, value);
  } else {
    double *v = g_malloc(sizeof(double));
    *v = value;
    g_queue_push_tail(private->queue, v);

    private->items++;
    private->full = private->items >= private->capasity;
  }
  if (private->discrete) {
    update_path_discrete(self);
  } else {
    update_path_continuous(self);
  }
}

static void g_barbar_graph_dispose(GObject *object) {
  BarBarGraph *self = BARBAR_GRAPH(object);
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  g_clear_pointer(&private->path, gsk_path_unref);
  gsk_stroke_free(private->stroke);

  G_OBJECT_CLASS(g_barbar_graph_parent_class)->dispose(object);
}

static void g_barbar_graph_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {
  BarBarGraph *self = BARBAR_GRAPH(widget);
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (!private->path) {
    return;
  }

  if (private->fill) {
    gtk_snapshot_append_fill(snapshot, private->path, GSK_FILL_RULE_WINDING,
                             &private->color);
  } else {
    gtk_snapshot_append_stroke(snapshot, private->path, private->stroke,
                               &private->color);
  }
}

static void g_barbar_graph_measure(GtkWidget *widget,
                                   GtkOrientation orientation, int for_size,
                                   int *minimum, int *natural,
                                   int *minimum_baseline,
                                   int *natural_baseline) {
  BarBarGraph *self = BARBAR_GRAPH(widget);
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);
  printf("measure: %f\n", private->width);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    *minimum = *natural = private->width;
  else
    *minimum = *natural = private->height;
}

void g_barbar_graph_size_allocate(GtkWidget *widget, int width, int height,
                                  int baseline) {
  BarBarGraph *self = BARBAR_GRAPH(widget);
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  printf("allocate: %f\n", private->width);
  private->width = width;
  private->height = height;
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

static void g_barbar_graph_init(BarBarGraph *self) {
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);
  gtk_widget_get_color(GTK_WIDGET(self), &private->color);

  private->queue = g_queue_new();
  g_queue_init(private->queue);

  private->min_value = 0.0;
  private->max_value = 1.0;
}

/**
 * g_barbar_graph_new:
 *
 * Returns: (transfer full): a `BarBarGraph`
 */
GtkWidget *g_barbar_graph_new(void) {
  return g_object_new(BARBAR_TYPE_GRAPH, NULL);
}
