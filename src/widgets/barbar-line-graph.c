#include "barbar-line-graph.h"
#include <stdlib.h>

// TODO: Mirror mode
// Add multiple colors.

enum {
  PROP_0,

  PROP_MIN_HEIGHT,
  PROP_MIN_WIDTH,
  PROP_MIN_VALUE,
  PROP_MAX_VALUE,
  PROP_SIZE,
  PROP_STROKE_WIDTH,
  PROP_FILL,
  PROP_DISCRETE,
  PROP_VALUE,

  LAST_PROP = PROP_VALUE,
  // NUM_PROPERTIES,
};

// a ringbuffer
typedef struct {
  double *buffer;
  size_t capacity;
  size_t size;
  size_t current;
} ring;

/**
 * BarBarLineGraph:
 *
 * Display changes over time for a value
 */
struct _BarBarLineGraph {
  GtkWidget parent_instance;

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

  ring ring;
};

static GParamSpec *properties[LAST_PROP] = {
    NULL,
};

static void
g_barbar_line_graph_graph_interface_init(BarBarGraphInterface *iface);

static void g_barbar_line_graph_set_values(BarBarGraph *graph, size_t len,
                                           double *values);
static void g_barbar_line_graph_push_value(BarBarGraph *graph, double value);
static size_t g_barbar_line_graph_get_size(BarBarGraph *graph);

G_DEFINE_FINAL_TYPE_WITH_CODE(
    BarBarLineGraph, g_barbar_line_graph, GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE(BARBAR_TYPE_GRAPH,
                          g_barbar_line_graph_graph_interface_init))

static void push_update(BarBarLineGraph *self, double value);
static void update_path_discrete(BarBarLineGraph *self);
static void update_path_continuous(BarBarLineGraph *self);
static void g_barbar_line_graph_update_path(BarBarLineGraph *self);

static void
g_barbar_line_graph_graph_interface_init(BarBarGraphInterface *iface) {

  iface->set_values = g_barbar_line_graph_set_values;
  iface->push_value = g_barbar_line_graph_push_value;
  iface->get_size = g_barbar_line_graph_get_size;
}

static size_t g_barbar_line_graph_get_size(BarBarGraph *graph) {
  BarBarLineGraph *self = BARBAR_LINE_GRAPH(graph);

  return self->ring.size;
}

/**
 * g_barbar_line_graph_set_stroke_width:
 * @self: a `BarBarGraph`
 * @stroke: Size of the stroke
 */
void g_barbar_line_graph_set_stroke_width(BarBarLineGraph *self, float stroke) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->stroke_size == stroke) {
    return;
  }

  self->stroke_size = stroke;
  self->stroke = gsk_stroke_new(stroke);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STROKE_WIDTH]);
}

/**
 * g_barbar_line_graph_set_fill:
 * @self: a `BarBarGraph`
 * @fill: i it should be filled
 *
 * Sets if the graph should be filled or not. Filling
 * means that we color the area between the line and the
 * buttom of the graph
 */
void g_barbar_line_graph_set_fill(BarBarLineGraph *self, gboolean fill) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->fill == fill) {
    return;
  }

  self->fill = fill;
  // we need to update here? Just wait for the next tick for now
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_FILL]);
}

// void print_ring(ring *ring) {
//
//   printf("ring: %p, %zu, %zu, %zu\n", ring->buffer, ring->size,
//   ring->capacity,
//          ring->current);
//   printf("values: ");
//   for (int i = 0; i < ring->size; i++) {
//     printf("%f ", ring->buffer[i]);
//   }
//
//   printf("\n");
// }

void recalloc(ring *ring, size_t length) {

  if (!ring->buffer) {
    ring->buffer = calloc(sizeof(double), length);
    ring->capacity = length;
    return;
  }

  ring->buffer = realloc(ring->buffer, length * sizeof(double));

  // clear the new memory
  double *ptr = ring->buffer + ring->capacity;
  memset(ptr, 0, MAX(0, length - ring->capacity - 1));

  ring->capacity = length;
}

/**
 * g_barbar_line_graph_set_history_entries:
 * @self: a `BarBarGraph`
 * @length: length
 *
 * Sets how many entries there should be in the graph (maximum).
 */
void g_barbar_line_graph_set_size(BarBarLineGraph *self, guint length) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->ring.size == length) {
    return;
  }

  if (self->ring.capacity < length) {
    recalloc(&self->ring, length);
  }

  self->ring.size = length;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STROKE_WIDTH]);
}

/**
 * g_barbar_line_graph_set_min_value:
 * @self: a `BarBarGraph`
 * @min: min value
 *
 * Sets the lowest value of a graph. This value can also
 * be change by setting main value that is lower than the min value
 */
void g_barbar_line_graph_set_min_value(BarBarLineGraph *self, double min) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->min_value == min) {
    return;
  }

  self->min_value = min;

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIN_VALUE]);
}

/**
 * g_barbar_line_graph_set_max_value:
 * @self: a `BarBarGraph`
 * @max: max value
 *
 * Sets the maximum value of a graph. This value can also
 * be change by setting main value that is higher than the max value
 */
void g_barbar_line_graph_set_max_value(BarBarLineGraph *self, double max) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->max_value == max) {
    return;
  }

  self->max_value = max;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MAX_VALUE]);
}

// GQueue *g_barbar_line_graph_get_queue(BarBarGraph *self) {
//   BarBarGraphPrivate *private =
//   g_barbar_line_graph_get_instance_private(self);
//
//   return private->queue;
// }

// void g_barbar_line_graph_set_entries(BarBarGraph *self, GQueue *queue) {
//   g_return_if_fail(BARBAR_IS_GRAPH(self));
//
//   BarBarGraphPrivate *private =
//   g_barbar_line_graph_get_instance_private(self);
//
//   if (private->queue != queue) {
//     if (private->queue) {
//       g_queue_free_full(queue, free);
//     }
//     private->queue = queue;
//   }
//
//   GList *link = private->queue->head;
//
//   while (link) {
//     double value = *(double *)link->data;
//     if (value > private->max_value) {
//       g_barbar_line_graph_set_max_value(self, value);
//     }
//     if (value < private->min_value) {
//       g_barbar_line_graph_set_min_value(self, value);
//     }
//     link = link->next;
//   }
//
//   if (private->discrete) {
//     update_path_discrete(self);
//   } else {
//     update_path_continuous(self);
//   }
// }
//
static void g_barbar_line_graph_push_value(BarBarGraph *graph, double value) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(graph));

  BarBarLineGraph *self = BARBAR_LINE_GRAPH(graph);

  if (value > self->max_value) {
    g_barbar_line_graph_set_max_value(self, value);
  }
  if (value < self->min_value) {
    g_barbar_line_graph_set_min_value(self, value);
  }

  push_update(self, value);
  gtk_widget_queue_draw(GTK_WIDGET(graph));
}

static void g_barbar_line_graph_set_values(BarBarGraph *graph, size_t len,
                                           double *values) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(graph));

  BarBarLineGraph *self = BARBAR_LINE_GRAPH(graph);

  if (self->ring.size > len) {
  }
  for (size_t i = 0; i < len; i++) {
    self->ring.buffer[i] = values[i];
  }
  self->ring.current = len;
  g_barbar_line_graph_update_path(self);
}

/**
 * g_barbar_line_graph_set_min_height:
 * @self: a `BarBarGraph`
 * @height: height
 *
 * Set how small the vertical height should be, at minimum. If not
 * set it will be as small as required.
 */
void g_barbar_line_graph_set_min_height(BarBarLineGraph *self, guint height) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->height == height) {
    return;
  }

  self->height = height;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIN_HEIGHT]);
}

/**
 * g_barbar_line_graph_set_min_width:
 * @self: a `BarBarGraph`
 * @width: width
 *
 * Set how small the horizontal width should be, at minimum. If not
 * set it will be as small as required.
 */
void g_barbar_line_graph_set_min_width(BarBarLineGraph *self, guint width) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->width == width) {
    return;
  }

  self->width = width;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIN_WIDTH]);
}

/**
 * g_barbar_line_graph_set_discrete:
 * @self: a `BarBarGraph`
 * @discrete: if we should operate in descrite mode
 *
 * Changes between discrete and continues mode.
 * If set to discete mode the graph will draw using a series of bars.
 */
void g_barbar_line_graph_set_discrete(BarBarLineGraph *self,
                                      gboolean discrete) {
  g_return_if_fail(BARBAR_IS_LINE_GRAPH(self));

  if (self->discrete == discrete) {
    return;
  }
  self->discrete = discrete;

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_DISCRETE]);
}

static void g_barbar_line_graph_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {
  BarBarLineGraph *graph = BARBAR_LINE_GRAPH(object);

  switch (property_id) {
  case PROP_VALUE:
    g_barbar_line_graph_push_value(BARBAR_GRAPH(graph),
                                   g_value_get_uint(value));
    break;
  case PROP_MIN_WIDTH:
    g_barbar_line_graph_set_min_width(graph, g_value_get_uint(value));
    break;
  case PROP_MIN_HEIGHT:
    g_barbar_line_graph_set_min_height(graph, g_value_get_uint(value));
    break;
  case PROP_STROKE_WIDTH:
    g_barbar_line_graph_set_stroke_width(graph, g_value_get_float(value));
    break;
  case PROP_FILL:
    g_barbar_line_graph_set_fill(graph, g_value_get_boolean(value));
    break;
  case PROP_SIZE:
    g_barbar_line_graph_set_size(graph, g_value_get_uint(value));
    break;
  case PROP_MIN_VALUE:
    g_barbar_line_graph_set_min_value(graph, g_value_get_double(value));
    break;
  case PROP_MAX_VALUE:
    g_barbar_line_graph_set_max_value(graph, g_value_get_double(value));
    break;
  case PROP_DISCRETE:
    g_barbar_line_graph_set_discrete(graph, g_value_get_boolean(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_line_graph_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec) {
  BarBarLineGraph *graph = BARBAR_LINE_GRAPH(object);

  switch (property_id) {
  case PROP_VALUE: {
    double v;
    v = graph->ring.buffer[graph->ring.current];
    g_value_set_double(value, v);
    break;
  }
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

static inline double get_y(double height, double max_value, double v) {
  return height - (height * (v / max_value));
}
static inline size_t get_current(ring *ring) {
  if (ring->current == 0) {
    return ring->size - 1;
  }
  return ring->current - 1;
}

// gsk_path_builder_move_to(builder, 10, 10);
// gsk_path_builder_line_to(builder, 10, 20);
//
// gsk_path_builder_line_to(builder, 20, 20);
//
// gsk_path_builder_line_to(builder, 20, 10);
// gsk_path_builder_line_to(builder, 10, 10);
//
// gsk_path_builder_move_to(builder, 30, 30);
// gsk_path_builder_line_to(builder, 30, 40);
//
// gsk_path_builder_line_to(builder, 40, 40);
//
// gsk_path_builder_line_to(builder, 40, 30);
// gsk_path_builder_line_to(builder, 30, 30);

// static void draw_block(GskPathBuilder *builder, double x, double y,
//                        double size) {
//
//   gsk_path_builder_move_to(builder, x, y);
//   gsk_path_builder_line_to(builder, x, y + size);
//
//   gsk_path_builder_line_to(builder, x + size, y + size);
//
//   gsk_path_builder_line_to(builder, x + size, y);
//   gsk_path_builder_line_to(builder, x, y);
// }

// static void update_path_blocks(BarBarGraph *self) {
//   GskPathBuilder *builder;
//   BarBarGraphPrivate *private =
//   g_barbar_line_graph_get_instance_private(self);
//
//   builder = gsk_path_builder_new();
//
//   double x = private->width;
//   size_t first = get_current(&private->ring);
//
//   double delta = private->width / private->ring.size;
//
//   do {
//     double v = private->ring.buffer[first];
//
//     double next = get_y(private->height, private->max_value, v);
//
//     // draw_bar();
//
//     x -= delta;
//     // gsk_path_builder_line_to(builder, x, next);
//     //
//     // gsk_path_builder_line_to(builder, x, next);
//
//     if (first == 0) {
//       first = private->ring.size - 1;
//     } else {
//       first = first - 1;
//     }
//   } while (first != private->ring.current);
//
//   g_clear_pointer(&private->path, gsk_path_unref);
//   if (!builder) {
//     return;
//   }
//   private->path = gsk_path_builder_free_to_path(builder);
// }

static void update_path_discrete(BarBarLineGraph *self) {
  GskPathBuilder *builder;

  builder = gsk_path_builder_new();

  double x = self->width;
  size_t first = get_current(&self->ring);
  double v = self->ring.buffer[first];

  double y = get_y(self->height, self->max_value, v);
  double delta = self->width / self->ring.size;

  gsk_path_builder_move_to(builder, self->width, y);
  double next = y;

  // TODO: add a space between if wanted
  do {
    v = self->ring.buffer[first];

    x -= delta;
    gsk_path_builder_line_to(builder, x, next);

    next = get_y(self->height, self->max_value, v);
    gsk_path_builder_line_to(builder, x, next);

    if (first == 0) {
      first = self->ring.size - 1;
    } else {
      first = first - 1;
    }
  } while (first != self->ring.current);

  if (self->fill) {
    gsk_path_builder_line_to(builder, 0, next);

    gsk_path_builder_line_to(builder, 0, self->height);

    gsk_path_builder_line_to(builder, self->width, self->height);
    gsk_path_builder_line_to(builder, self->width, y);
  } else {
    gsk_path_builder_line_to(builder, 0, next);
  }

  g_clear_pointer(&self->path, gsk_path_unref);
  if (!builder) {
    return;
  }
  self->path = gsk_path_builder_free_to_path(builder);
}

static void update_path_continuous(BarBarLineGraph *self) {
  GskPathBuilder *builder;

  builder = gsk_path_builder_new();

  double x = self->width;
  size_t first = get_current(&self->ring);
  double v = self->ring.buffer[first];

  double y = get_y(self->height, self->max_value, v);
  double delta = self->width / self->ring.size;

  gsk_path_builder_move_to(builder, self->width, y);
  double next = y;

  do {
    v = self->ring.buffer[first];
    next = get_y(self->height, self->max_value, v);

    gsk_path_builder_line_to(builder, x, next);

    x -= delta;

    if (first == 0) {
      first = self->ring.size - 1;
    } else {
      first = first - 1;
    }
  } while (first != self->ring.current);

  // for (size_t idx = 0; idx < private->ring.size; idx++) {
  //   v = private->ring.buffer[idx];
  //   double y = get_y(private->height, private->max_value, v);
  //
  //   gsk_path_builder_line_to(builder, x, y);
  //   x += delta;
  // }

  if (self->fill) {
    gsk_path_builder_line_to(builder, 0, next);

    gsk_path_builder_line_to(builder, 0, self->height);

    gsk_path_builder_line_to(builder, self->width, self->height);
    gsk_path_builder_line_to(builder, self->width, y);
  } else {
    gsk_path_builder_line_to(builder, 0, next);
  }

  // g_clear_pointer(&private->path, gsk_path_unref);
  if (self->path) {
    gsk_path_unref(self->path);
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

static void g_barbar_line_graph_update_path(BarBarLineGraph *self) {
  if (self->discrete) {
    update_path_discrete(self);
  } else {
    update_path_continuous(self);
  }
}

static void push_update(BarBarLineGraph *self, double value) {

  self->ring.buffer[self->ring.current] = value;
  self->ring.current = ((self->ring.current + 1) % self->ring.size);
  g_barbar_line_graph_update_path(self);
}

static void g_barbar_line_graph_dispose(GObject *object) {
  BarBarLineGraph *self = BARBAR_LINE_GRAPH(object);

  g_clear_pointer(&self->path, gsk_path_unref);
  g_clear_pointer(&self->stroke, gsk_stroke_free);

  G_OBJECT_CLASS(g_barbar_line_graph_parent_class)->dispose(object);
}

static void g_barbar_line_graph_snapshot(GtkWidget *widget,
                                         GtkSnapshot *snapshot) {
  BarBarLineGraph *self = BARBAR_LINE_GRAPH(widget);

  if (!self->path) {
    return;
  }

  if (self->fill) {
    gtk_snapshot_append_fill(snapshot, self->path, GSK_FILL_RULE_WINDING,
                             &self->color);
  } else {
    gtk_snapshot_append_stroke(snapshot, self->path, self->stroke,
                               &self->color);
  }
}

static void g_barbar_line_graph_measure(GtkWidget *widget,
                                        GtkOrientation orientation,
                                        int for_size, int *minimum,
                                        int *natural, int *minimum_baseline,
                                        int *natural_baseline) {
  BarBarLineGraph *self = BARBAR_LINE_GRAPH(widget);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    *minimum = *natural = self->width;
  else
    *minimum = *natural = self->height;
}

void g_barbar_line_graph_size_allocate(GtkWidget *widget, int width, int height,
                                       int baseline) {
  BarBarLineGraph *self = BARBAR_LINE_GRAPH(widget);

  self->width = width;
  self->height = height;
}

// static void g_barbar_line_graph_constructed(GObject *object) {
//   BarBarGraph *self = barbar_line_graph(object);
//   BarBarGraphPrivate *private =
//   g_barbar_line_graph_get_instance_private(self);
//
//   private->ring.current = private->ring.length - 1;
//
//   G_OBJECT_CLASS(g_barbar_line_graph_parent_class)->constructed(object);
// }

static void g_barbar_line_graph_class_init(BarBarLineGraphClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
  // BarBarGraphClass *graph_class = BARBAR_GRAPH_CLASS(class);

  gobject_class->set_property = g_barbar_line_graph_set_property;
  gobject_class->get_property = g_barbar_line_graph_get_property;

  gobject_class->dispose = g_barbar_line_graph_dispose;
  // gobject_class->constructed = g_barbar_line_graph_constructed;

  widget_class->snapshot = g_barbar_line_graph_snapshot;
  widget_class->measure = g_barbar_line_graph_measure;
  widget_class->size_allocate = g_barbar_line_graph_size_allocate;

  /**
   * BarBarLineGraph:value:
   *
   * Setting this value pushes a value to the graph
   *
   */
  g_object_class_override_property(gobject_class, PROP_VALUE, "value");

  /**
   * BarBarLineGraph:min-width:
   *
   * Minimum width of this widget
   */
  properties[PROP_MIN_WIDTH] = g_param_spec_uint(
      "min-width", NULL, NULL, 0, G_MAXUINT, 30,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarLineGraph:min-height:
   *
   * Minimum height of this widget
   */
  properties[PROP_MIN_HEIGHT] = g_param_spec_uint(
      "min-height", NULL, NULL, 0, G_MAXUINT, 20,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarLineGraph:stroke-width:
   *
   * How big pen we use for our graph
   */
  properties[PROP_STROKE_WIDTH] = g_param_spec_float(
      "stroke-width", NULL, NULL, 0.0, G_MAXFLOAT, 2.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  /**
   * BarBarLineGraph:min-value:
   *
   * min-value is a soft value to aid with render, if a value lower
   * is produced, the value is adjusted.
   */
  properties[PROP_MIN_VALUE] = g_param_spec_double(
      "min-value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarLineGraph:max-value:
   *
   * max-value is a soft value to aid with render, if a value higher
   * value is produced, the value is adjusted.
   */
  properties[PROP_MAX_VALUE] = g_param_spec_double(
      "max-value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarLineGraph:history-length:
   *
   * How many points of history we should save.
   */
  properties[PROP_SIZE] = g_param_spec_uint(
      "history-length", NULL, NULL, 0, G_MAXUINT, 100,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarLineGraph:fill:
   *
   * If the graph should be filled
   */
  properties[PROP_FILL] = g_param_spec_boolean(
      "fill", NULL, NULL, TRUE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarLineGraph:discrete:
   *
   * If a discrete graph should be drawn, produces a bar like graph.
   */
  properties[PROP_DISCRETE] = g_param_spec_boolean(
      "discrete", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, LAST_PROP, properties);
  gtk_widget_class_set_css_name(widget_class, "graph");
}

static void g_barbar_line_graph_init(BarBarLineGraph *self) {
  gtk_widget_get_color(GTK_WIDGET(self), &self->color);

  // printf("GdkRGBA(red=%.2f, green=%.2f, blue=%.2f, alpha=%.2f)\n",
  //        private->color.red, private->color.green, private->color.blue,
  //        private->color.alpha);

  // private->queue = g_queue_new();
  // g_queue_init(private->queue);

  self->min_value = 0.0;
  self->max_value = 1.0;
}

/**
 * g_barbar_line_graph_new:
 *
 * Returns: (transfer full): a `BarBarLineGraph`
 */
GtkWidget *g_barbar_line_graph_new(void) {
  return g_object_new(BARBAR_TYPE_GRAPH, NULL);
}
