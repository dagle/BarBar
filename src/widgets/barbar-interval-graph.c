#include "barbar-interval-graph.h"
#include "barbar-graph.h"
#include "glib-object.h"
#include "gtk/gtk.h"

/**
 * BarBarIntervalGraph:
 *
 * Display changes over time for a value
 */
struct _BarBarIntervalGraph {
  GtkWidget parent_instance;
  BarBarGraph *graph;

  double current;

  guint tick_cb;
  guint interval;
};

enum {
  PROP_0,

  PROP_GRAPH,
  PROP_VALUE,
  PROP_INTERVAL,

  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarIntervalGraph, g_barbar_interval_graph, GTK_TYPE_WIDGET)

static void g_barbar_interval_graph_start(GtkWidget *widget);

/**
 * g_barbar_interval_graph_set_interval:
 * @self: a `BarBarIntervalGraph`
 * @interval: interval
 *
 * How often the graph should tick and move a value to it's history
 */
void g_barbar_interval_graph_set_interval(BarBarIntervalGraph *self,
                                          guint interval) {
  g_return_if_fail(BARBAR_IS_INTERVAL_GRAPH(self));

  if (self->interval == interval) {
    return;
  }
  self->interval = interval;
  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_INTERVAL]);
}

/**
 * g_barbar_interval_graph_set_value:
 * @self: a `BarBarIntervalGraph`
 * @value: a value
 *
 * Sets the current value of a graph.
 */
void g_barbar_interval_graph_set_value(BarBarIntervalGraph *self,
                                       double value) {
  g_return_if_fail(BARBAR_IS_INTERVAL_GRAPH(self));

  self->current = value;

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VALUE]);
}

/**
 * g_barbar_interval_graph_set_graph:
 * @self: a `BarBarIntervalGraph`
 * @graph: a `BarBarGraph`
 *
 * Sets the graph to run over the interval
 */
void g_barbar_interval_graph_set_graph(BarBarIntervalGraph *self,
                                       BarBarGraph *graph) {
  g_return_if_fail(BARBAR_IS_INTERVAL_GRAPH(self));

  if (self->graph == graph) {
    return;
  }

  gtk_widget_set_parent(GTK_WIDGET(graph), GTK_WIDGET(self));
  // self->graph = graph;
  self->graph = g_object_ref(graph);

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_GRAPH]);
}

static void g_barbar_interval_graph_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  BarBarIntervalGraph *graph = BARBAR_INTERVAL_GRAPH(object);

  switch (property_id) {
  case PROP_VALUE:
    g_barbar_interval_graph_set_value(graph, g_value_get_double(value));
    break;
  case PROP_GRAPH:
    g_barbar_interval_graph_set_graph(graph, g_value_get_object(value));
    break;
  case PROP_INTERVAL:
    g_barbar_interval_graph_set_interval(graph, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_interval_graph_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {
  BarBarIntervalGraph *graph = BARBAR_INTERVAL_GRAPH(object);

  switch (property_id) {
  case PROP_VALUE:
    g_value_set_double(value, graph->current);
    break;
  case PROP_INTERVAL:
    g_value_set_uint(value, graph->interval);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
g_barbar_interval_graph_class_init(BarBarIntervalGraphClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_interval_graph_set_property;
  gobject_class->get_property = g_barbar_interval_graph_get_property;

  widget_class->root = g_barbar_interval_graph_start;

  /**
   * BarBarIntervalGraph:Graph:
   *
   * The actual graph
   */
  properties[PROP_GRAPH] = g_param_spec_object(
      "graph", NULL, NULL, BARBAR_TYPE_GRAPH,
      G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarIntervalGraph:interval:
   *
   * How often the graph should update
   */
  properties[PROP_INTERVAL] = g_param_spec_uint(
      "interval", NULL, NULL, 0, G_MAXUINT, 1000,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarIntervalGraph:value:
   *
   * The current value
   */
  properties[PROP_VALUE] =
      g_param_spec_double("value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
}

static void g_barbar_interval_graph_init(BarBarIntervalGraph *self) {}

static gboolean g_barbar_interval_graph_update(gpointer data) {

  BarBarIntervalGraph *self = BARBAR_INTERVAL_GRAPH(data);

  g_barbar_graph_push_value(BARBAR_GRAPH(self->graph), self->current);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_interval_graph_start(GtkWidget *widget) {
  GTK_WIDGET_CLASS(g_barbar_interval_graph_parent_class)->root(widget);

  BarBarIntervalGraph *self = BARBAR_INTERVAL_GRAPH(widget);

  if (gtk_widget_get_vexpand(GTK_WIDGET(self->graph))) {
    gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);
  }

  if (gtk_widget_get_hexpand(GTK_WIDGET(self->graph))) {
    gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);
  }

  self->tick_cb = g_timeout_add_full(
      0, self->interval, g_barbar_interval_graph_update, self, NULL);
}

/**
 * g_barbar_interval_graph_new:
 *
 * Returns: (transfer full): a `BarBarGraph`
 */
GtkWidget *g_barbar_interval_graph_new(BarBarGraph *graph) {
  return g_object_new(BARBAR_TYPE_INTERVAL_GRAPH, "graph", graph, NULL);
}
