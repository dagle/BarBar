#include "barbar-activity-graph.h"
#include "glib.h"
#include "widgets/barbar-box.h"

/**
 * BarBarActivityGraph:
 *
 * An activity graph, often used to display github activity
 */
struct _BarBarActivityGraph {
  GtkWidget parent_instance;

  uint rows;
  uint cols;
  uint dotsize;
  uint row_spacing;
  uint col_spacing;

  GtkWidget *grid;
};

enum {
  PROP_0,

  PROP_ROWS,
  PROP_COLS,
  PROP_DOTSIZE,
  PROP_ROWS_SPACING,
  PROP_COLS_SPACING,

  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarActivityGraph, g_barbar_activity_graph, GTK_TYPE_WIDGET)

/**
 * g_barbar_activity_graph_set_rows:
 * @graph: a `BarBarActivityGraph`
 * @rows: number of rows
 *
 */
void g_barbar_activity_graph_set_rows(BarBarActivityGraph *graph, guint rows) {
  g_return_if_fail(BARBAR_IS_ACTIVITY_GRAPH(graph));

  if (graph->rows == rows) {
    return;
  }

  graph->rows = rows;

  g_object_notify_by_pspec(G_OBJECT(graph), properties[PROP_ROWS]);
}

/**
 * g_barbar_activity_graph_set_cols:
 * @graph: a `BarBarActivityGraph`
 * @rows: number of rows
 *
 */
void g_barbar_activity_graph_set_cols(BarBarActivityGraph *graph, guint cols) {
  g_return_if_fail(BARBAR_IS_ACTIVITY_GRAPH(graph));

  if (graph->cols == cols) {
    return;
  }

  graph->cols = cols;

  g_object_notify_by_pspec(G_OBJECT(graph), properties[PROP_COLS]);
}

/**
 * g_barbar_activity_graph_set_dot_size:
 * @graph: a `BarBarActivityGraph`
 * @dotsize: size
 *
 * Sets the size of how large the dots should be in the graph.
 *
 */
void g_barbar_activity_graph_set_dot_size(BarBarActivityGraph *graph,
                                          guint dotsize) {
  g_return_if_fail(BARBAR_IS_ACTIVITY_GRAPH(graph));

  if (graph->dotsize == dotsize) {
    return;
  }

  // TODO: Update all boxes
  graph->dotsize = dotsize;

  g_object_notify_by_pspec(G_OBJECT(graph), properties[PROP_DOTSIZE]);
}

/**
 * g_barbar_activity_graph_set_rows_spacing:
 * @graph: a `BarBarActivityGraph`
 * @spacing: sets the vertical gaps between the squares in the graph
 *
 */
void g_barbar_activity_graph_set_rows_spacing(BarBarActivityGraph *graph,
                                              guint spacing) {
  g_return_if_fail(BARBAR_IS_ACTIVITY_GRAPH(graph));

  if (graph->row_spacing == spacing) {
    return;
  }

  graph->row_spacing = spacing;
  gtk_grid_set_row_spacing(GTK_GRID(graph->grid), graph->row_spacing);

  g_object_notify_by_pspec(G_OBJECT(graph), properties[PROP_ROWS_SPACING]);
}

/**
 * g_barbar_activity_graph_set_cols_spacing:
 * @graph: a `BarBarActivityGraph`
 * @spacing: sets the horizontal gaps between the squares in the graph
 *
 */
static void g_barbar_activity_graph_set_cols_spacing(BarBarActivityGraph *graph,
                                                     guint spacing) {
  g_return_if_fail(BARBAR_IS_ACTIVITY_GRAPH(graph));

  if (graph->col_spacing == spacing) {
    return;
  }

  graph->col_spacing = spacing;
  gtk_grid_set_column_spacing(GTK_GRID(graph->grid), graph->col_spacing);

  g_object_notify_by_pspec(G_OBJECT(graph), properties[PROP_COLS_SPACING]);
}

static void g_barbar_activity_graph_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  BarBarActivityGraph *graph = BARBAR_ACTIVITY_GRAPH(object);

  switch (property_id) {
  case PROP_ROWS:
    g_barbar_activity_graph_set_rows(graph, g_value_get_uint(value));
    break;
  case PROP_COLS:
    g_barbar_activity_graph_set_cols(graph, g_value_get_uint(value));
    break;
  case PROP_DOTSIZE:
    g_barbar_activity_graph_set_dot_size(graph, g_value_get_uint(value));
    break;
  case PROP_ROWS_SPACING:
    g_barbar_activity_graph_set_rows_spacing(graph, g_value_get_uint(value));
    break;
  case PROP_COLS_SPACING:
    g_barbar_activity_graph_set_cols_spacing(graph, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_activity_graph_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {
  BarBarActivityGraph *graph = BARBAR_ACTIVITY_GRAPH(object);

  switch (property_id) {
  case PROP_ROWS:
    g_value_set_uint(value, graph->rows);
    break;
  case PROP_COLS:
    g_value_set_uint(value, graph->cols);
    break;
  case PROP_DOTSIZE:
    g_value_set_uint(value, graph->dotsize);
    break;
  case PROP_ROWS_SPACING:
    g_value_set_uint(value, graph->row_spacing);
    break;
  case PROP_COLS_SPACING:
    g_value_set_uint(value, graph->col_spacing);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}
static void g_barbar_activity_graph_constructed(GObject *object);

static void
g_barbar_activity_graph_class_init(BarBarActivityGraphClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_activity_graph_set_property;
  gobject_class->get_property = g_barbar_activity_graph_get_property;
  gobject_class->constructed = g_barbar_activity_graph_constructed;

  /**
   * BarBarActivityGraph:rows:
   *
   * How many rows there should the graph. By default each row is 1 week.
   */
  properties[PROP_ROWS] = g_param_spec_uint(
      "rows", NULL, NULL, 1, 500, 7,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarActivityGraph:cols:
   *
   * How many columns the graph should have
   *
   */
  properties[PROP_COLS] = g_param_spec_uint(
      "cols", NULL, NULL, 1, 500, 4 * 6,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  // kinda a hack but I can't solve this witout it, so I give up.
  /**
   * BarBarActivityGraph:dot-size:
   *
   * How large a dot should be
   */
  properties[PROP_DOTSIZE] = g_param_spec_uint(
      "dot-size", NULL, NULL, 1, 500, 8,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarActivityGraph:row-spacing:
   *
   * Spacing between each row in the graph
   */
  properties[PROP_ROWS_SPACING] = g_param_spec_uint(
      "row-spacing", NULL, NULL, 1, 500, 2,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarActivityGraph:col-spacing:
   *
   * Spacing between each column in the graph
   */
  properties[PROP_COLS_SPACING] = g_param_spec_uint(
      "col-spacing", NULL, NULL, 1, 500, 2,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "activity-graph");
}

static void g_barbar_activity_graph_constructed(GObject *object) {
  BarBarActivityGraph *self = BARBAR_ACTIVITY_GRAPH(object);

  G_OBJECT_CLASS(g_barbar_activity_graph_parent_class)->constructed(object);

  for (int j = 0; j < self->rows; j++) {
    for (int i = 0; i < self->cols; i++) {
      GtkWidget *widget = g_barbar_box_new(self->dotsize);
      gtk_grid_attach(GTK_GRID(self->grid), widget, i, j, 1, 1);
    }
  }
}

static void g_barbar_activity_graph_init(BarBarActivityGraph *self) {
  self->grid = gtk_grid_new();
  gtk_widget_set_parent(self->grid, GTK_WIDGET(self));
}

/**
 * g_barbar_activity_graph_set_activity:
 * @graph: A `BarBarActivityGraph`
 * @col: column index
 * @row: row index
 * @level: level to set
 *
 * Sets the level of the specified box.
 */
void g_barbar_activity_graph_set_activity(BarBarActivityGraph *graph, int col,
                                          int row, int level) {
  g_return_if_fail(BARBAR_IS_ACTIVITY_GRAPH(graph));

  GtkWidget *child = gtk_grid_get_child_at(GTK_GRID(graph->grid), col, row);

  if (!child) {
    g_warning("Cannot find a child for column %d and row %d", col, row);
    return;
  }

  g_barbar_box_set_level(BARBAR_BOX(child), level);
}

/**
 * g_barbar_activity_graph_get_rows:
 * @graph: a `BarBarActivityGraph`
 *
 * Returns: number of rows.
 */
uint g_barbar_activity_graph_get_rows(BarBarActivityGraph *graph) {
  return graph->cols;
}

/**
 * g_barbar_activity_graph_get_cols:
 * @graph: a `BarBarActivityGraph`
 *
 * Returns: number of columns.
 */
uint g_barbar_activity_graph_get_cols(BarBarActivityGraph *graph) {
  return graph->cols;
}

uint g_barbar_activity_graph_get_size(BarBarActivityGraph *graph) {
  return graph->rows * graph->cols;
}

/**
 * g_barbar_activity_graph_set_activity_linear:
 */
void g_barbar_activity_graph_set_activity_linear(BarBarActivityGraph *graph,
                                                 int num, int activity) {
  g_return_if_fail(BARBAR_IS_ACTIVITY_GRAPH(graph));

  uint col = num / graph->rows;
  uint row = num % graph->rows;

  g_barbar_activity_graph_set_activity(graph, col, row, activity);
}

/**
 * g_barbar_activity_graph_new:
 * @cols: number of rows
 * @rows: number of rows
 *
 * Create a new garph with rows and cols
 *
 * Returns: (transfer full): a `BarBarActivityGraph`
 */
GtkWidget *g_barbar_activity_graph_new(int cols, int rows) {
  BarBarActivityGraph *graph = g_object_new(BARBAR_TYPE_ACTIVITY_GRAPH, "cols",
                                            cols, "rows", rows, NULL);

  return GTK_WIDGET(graph);
}
