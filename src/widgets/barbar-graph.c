#include "barbar-graph.h"
#include <stdlib.h>

// TODO: Mirror mode
// Add multiple colors.
// Block graph, it's like
// bar graph but with

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
  // PROP_SPACE

  NUM_PROPERTIES,
};

// a ringbuffer
typedef struct {
  double *buffer;
  size_t capacity;
  size_t size;
  size_t current;
} ring;

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

  ring ring;
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

void print_ring(ring *ring) {

  printf("ring: %p, %zu, %zu, %zu\n", ring->buffer, ring->size, ring->capacity,
         ring->current);
  printf("values: ");
  for (int i = 0; i < ring->size; i++) {
    printf("%f ", ring->buffer[i]);
  }

  printf("\n");
}

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
 * g_barbar_graph_set_history_entries:
 * @self: a `BarBarGraph`
 * @length: length
 *
 * Sets how many entries there should be in the graph (maximum).
 */
void g_barbar_graph_set_entry_numbers(BarBarGraph *self, guint length) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  if (private->ring.size == length) {
    return;
  }

  if (private->ring.capacity < length) {
    recalloc(&private->ring, length);
  }

  private->ring.size = length;
}

guint g_barbar_graph_get_entry_numbers(BarBarGraph *self) {
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  return private->ring.size;
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

// GQueue *g_barbar_graph_get_queue(BarBarGraph *self) {
//   BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);
//
//   return private->queue;
// }

// void g_barbar_graph_set_entries(BarBarGraph *self, GQueue *queue) {
//   g_return_if_fail(BARBAR_IS_GRAPH(self));
//
//   BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);
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
//       g_barbar_graph_set_max_value(self, value);
//     }
//     if (value < private->min_value) {
//       g_barbar_graph_set_min_value(self, value);
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
/**
 * g_barbar_graph_push_entry:
 * @self: a `BarBarGraph`
 * @value: a value
 *
 * Sets the current value of a graph.
 */
void g_barbar_graph_push_entry(BarBarGraph *self, double value) {
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
//   BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);
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

static void update_path_discrete(BarBarGraph *self) {
  GskPathBuilder *builder;
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  builder = gsk_path_builder_new();

  double x = private->width;
  size_t first = get_current(&private->ring);
  double v = private->ring.buffer[first];

  double y = get_y(private->height, private->max_value, v);
  double delta = private->width / private->ring.size;

  gsk_path_builder_move_to(builder, private->width, y);
  double next = y;

  // TODO: add a space between if wanted
  do {
    v = private->ring.buffer[first];

    x -= delta;
    gsk_path_builder_line_to(builder, x, next);

    next = get_y(private->height, private->max_value, v);
    gsk_path_builder_line_to(builder, x, next);

    if (first == 0) {
      first = private->ring.size - 1;
    } else {
      first = first - 1;
    }
  } while (first != private->ring.current);

  if (private->fill) {
    gsk_path_builder_line_to(builder, 0, next);

    gsk_path_builder_line_to(builder, 0, private->height);

    gsk_path_builder_line_to(builder, private->width, private->height);
    gsk_path_builder_line_to(builder, private->width, y);
  } else {
    gsk_path_builder_line_to(builder, 0, next);
  }

  g_clear_pointer(&private->path, gsk_path_unref);
  if (!builder) {
    return;
  }
  private->path = gsk_path_builder_free_to_path(builder);
}

static void update_path_continuous(BarBarGraph *self) {
  GskPathBuilder *builder;
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  builder = gsk_path_builder_new();

  double x = private->width;
  size_t first = get_current(&private->ring);
  double v = private->ring.buffer[first];

  double y = get_y(private->height, private->max_value, v);
  double delta = private->width / private->ring.size;

  gsk_path_builder_move_to(builder, private->width, y);
  double next = y;

  do {
    v = private->ring.buffer[first];
    next = get_y(private->height, private->max_value, v);

    gsk_path_builder_line_to(builder, x, next);

    x -= delta;

    if (first == 0) {
      first = private->ring.size - 1;
    } else {
      first = first - 1;
    }
  } while (first != private->ring.current);

  // for (size_t idx = 0; idx < private->ring.size; idx++) {
  //   v = private->ring.buffer[idx];
  //   double y = get_y(private->height, private->max_value, v);
  //
  //   gsk_path_builder_line_to(builder, x, y);
  //   x += delta;
  // }

  if (private->fill) {
    gsk_path_builder_line_to(builder, 0, next);

    gsk_path_builder_line_to(builder, 0, private->height);

    gsk_path_builder_line_to(builder, private->width, private->height);
    gsk_path_builder_line_to(builder, private->width, y);
  } else {
    gsk_path_builder_line_to(builder, 0, next);
  }

  // g_clear_pointer(&private->path, gsk_path_unref);
  if (private->path) {
    gsk_path_unref(private->path);
  }
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

  private->ring.buffer[private->ring.current] = value;
  private->ring.current = ((private->ring.current + 1) % private->ring.size);
}

void g_barbar_graph_update_path(BarBarGraph *self) {
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);
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
  g_clear_pointer(&private->stroke, gsk_stroke_free);

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

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    *minimum = *natural = private->width;
  else
    *minimum = *natural = private->height;
}

void g_barbar_graph_size_allocate(GtkWidget *widget, int width, int height,
                                  int baseline) {
  BarBarGraph *self = BARBAR_GRAPH(widget);
  BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);

  // printf("allocate: %f\n", private->width);
  private->width = width;
  private->height = height;
}

// static void g_barbar_graph_constructed(GObject *object) {
//   BarBarGraph *self = BARBAR_GRAPH(object);
//   BarBarGraphPrivate *private = g_barbar_graph_get_instance_private(self);
//
//   private->ring.current = private->ring.length - 1;
//
//   G_OBJECT_CLASS(g_barbar_graph_parent_class)->constructed(object);
// }

static void g_barbar_graph_class_init(BarBarGraphClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_graph_set_property;
  gobject_class->get_property = g_barbar_graph_get_property;

  gobject_class->dispose = g_barbar_graph_dispose;
  // gobject_class->constructed = g_barbar_graph_constructed;

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

  // printf("GdkRGBA(red=%.2f, green=%.2f, blue=%.2f, alpha=%.2f)\n",
  //        private->color.red, private->color.green, private->color.blue,
  //        private->color.alpha);

  // private->queue = g_queue_new();
  // g_queue_init(private->queue);

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
