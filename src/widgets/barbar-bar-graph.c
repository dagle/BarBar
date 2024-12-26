#include "barbar-bar-graph.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"

/**
 * BarBarBarGraph:
 *
 * Display changes over time for a value
 */
struct _BarBarBarGraph {
  GtkWidget parent_instance;
  GtkOrientation orientation;
  guint spacing;
  guint current;

  guint nums;

  GtkWidget *bars[32];
};

enum {
  PROP_0,

  PROP_NUMS,
  PROP_SPACING,
  PROP_ORIENTATION,
  PROP_VALUE,

  NUM_PROPERTIES,
};

static GtkBuildableIface *parent_buildable_iface;

static void
g_barbar_bar_graph_buildable_interface_init(GtkBuildableIface *iface);

static void g_barbar_bar_graph_root(GtkWidget *widget);

static GtkOrientation oppsite_orientation(GtkOrientation orientation) {
  if (orientation == GTK_ORIENTATION_VERTICAL) {
    return GTK_ORIENTATION_HORIZONTAL;
  }
  return GTK_ORIENTATION_VERTICAL;
}

G_DEFINE_FINAL_TYPE_WITH_CODE(
    BarBarBarGraph, g_barbar_bar_graph, GTK_TYPE_WIDGET,
    // G_ADD_PRIVATE(BarBarRiverTag)
    G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                          g_barbar_bar_graph_buildable_interface_init))

static GParamSpec *bar_graph_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_bar_graph_add_child(GtkBuildable *buildable,
                                         GtkBuilder *builder, GObject *child,
                                         const char *type) {
  g_return_if_fail(GTK_IS_WIDGET(child));

  BarBarBarGraph *self = BARBAR_BAR_GRAPH(buildable);

  if (g_strcmp0(type, "bar") == 0) {
    g_barbar_bar_graph_add_bar(self, GTK_WIDGET(child));
  } else {
    parent_buildable_iface->add_child(buildable, builder, child, type);
  }
}

static void
g_barbar_bar_graph_buildable_interface_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_bar_graph_add_child;
}

void g_barbar_bar_graph_set_nums(BarBarBarGraph *graph, guint tagnums) {
  g_return_if_fail(BARBAR_IS_BAR_GRAPH(graph));

  if (graph->nums == tagnums) {
    return;
  }

  graph->nums = tagnums;

  g_object_notify_by_pspec(G_OBJECT(graph), bar_graph_props[PROP_NUMS]);
}

guint g_barbar_bar_graph_get_nums(BarBarBarGraph *graph) { return graph->nums; }

static double set_bar(GtkWidget *widget, double value) {
  double old = 0;
  if (GTK_IS_LEVEL_BAR(widget)) {
    GtkLevelBar *bar = GTK_LEVEL_BAR(widget);
    old = gtk_level_bar_get_value(bar);
    gtk_level_bar_set_value(bar, value);
  }
  return old;
}

/**
 * g_barbar_bar_graph_set_value:
 *
 * Pushes one value to the graph
 *
 */
void g_barbar_bar_graph_set_value(BarBarBarGraph *self, double value) {
  double old;
  old = set_bar(self->bars[0], value);

  for (int i = 1; i < self->nums; i++) {
    GtkWidget *child = self->bars[i];
    old = set_bar(child, old);
  }
}

/**
 * g_barbar_bar_graph_set_orientation:
 *
 * Sets the orientation of the graph.
 * The internal bars are oriented in the oposite direction
 *
 */
void g_barbar_bar_graph_set_orientation(BarBarBarGraph *graph,
                                        GtkOrientation orientation) {
  GtkBoxLayout *layout;
  g_return_if_fail(BARBAR_IS_BAR_GRAPH(graph));

  if (graph->orientation == orientation) {
    return;
  }

  graph->orientation = orientation;

  layout = GTK_BOX_LAYOUT(gtk_widget_get_layout_manager(GTK_WIDGET(graph)));
  gtk_orientable_set_orientation(GTK_ORIENTABLE(layout), orientation);

  GtkOrientation inner = oppsite_orientation(orientation);
  GtkWidget *child;
  for (child = gtk_widget_get_first_child(GTK_WIDGET(graph)); child != NULL;
       child = gtk_widget_get_next_sibling(child)) {
    if (GTK_IS_LEVEL_BAR(child)) {
      GtkLevelBar *bar = GTK_LEVEL_BAR(child);
      gtk_orientable_set_orientation(GTK_ORIENTABLE(bar), inner);
      // TODO: Fix this
      gtk_level_bar_set_inverted(GTK_LEVEL_BAR(bar), TRUE);
    }
  }

  g_object_notify_by_pspec(G_OBJECT(graph), bar_graph_props[PROP_ORIENTATION]);
}

/**
 * g_barbar_bar_graph_set_spacing:
 *
 * Sets the orientation of the graph.
 * The internal bars are oriented in the oposite direction
 *
 */
void g_barbar_bar_graph_set_spacing(BarBarBarGraph *graph, guint spacing) {
  GtkBoxLayout *layout;
  g_return_if_fail(BARBAR_IS_BAR_GRAPH(graph));

  if (graph->spacing == spacing) {
    return;
  }
  graph->spacing = spacing;

  layout = GTK_BOX_LAYOUT(gtk_widget_get_layout_manager(GTK_WIDGET(graph)));
  gtk_box_layout_set_spacing(layout, spacing);

  g_object_notify_by_pspec(G_OBJECT(graph), bar_graph_props[PROP_SPACING]);
}

/**
 * g_barbar_bar_graph_add_bar:
 *
 * Adds a bar to the graph.
 * The child has to be a bar, currently only gtklevelbar is supported
 *
 */
void g_barbar_bar_graph_add_bar(BarBarBarGraph *self, GtkWidget *child) {
  if (GTK_IS_LEVEL_BAR(child)) {
    for (int i = 0; i < 32; i++) {
      if (self->bars[i] != NULL) {
        self->bars[i] = g_object_ref(child);
        if (i > self->nums) {
          self->nums++;
        }
        return;
      }
    }
  }
}

static void g_barbar_bar_graph_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarBarGraph *graph = BARBAR_BAR_GRAPH(object);

  switch (property_id) {
  case PROP_NUMS:
    g_barbar_bar_graph_set_nums(graph, g_value_get_uint(value));
    break;
  case PROP_ORIENTATION:
    g_barbar_bar_graph_set_orientation(graph, g_value_get_uint(value));
    break;
  case PROP_SPACING:
    g_barbar_bar_graph_set_spacing(graph, g_value_get_uint(value));
    break;
  case PROP_VALUE:
    g_barbar_bar_graph_set_value(graph, g_value_get_double(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_bar_graph_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {

  BarBarBarGraph *graph = BARBAR_BAR_GRAPH(object);

  switch (property_id) {
  case PROP_NUMS:
    g_value_set_uint(value, graph->nums);
    break;
  case PROP_ORIENTATION:
    g_value_set_enum(value, graph->orientation);
    break;
  case PROP_SPACING:
    g_value_set_uint(value, graph->spacing);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

// static void g_barbar_bar_graph_finalize(GObject *object) {
//   BarBarRiverTag *river = BARBAR_RIVER_TAG(object);
//
//   G_OBJECT_CLASS(g_barbar_bar_graph_parent_class)->finalize(object);
// }

static void g_barbar_bar_graph_class_init(BarBarBarGraphClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_bar_graph_set_property;
  gobject_class->get_property = g_barbar_bar_graph_get_property;
  // gobject_class->finalize = g_barbar_river_tag_finalize;

  widget_class->root = g_barbar_bar_graph_root;

  /**
   * BarBarBarGraph:nums:
   *
   * How many bars we should show
   */
  bar_graph_props[PROP_NUMS] = g_param_spec_uint(
      "nums", NULL, NULL, 0, 200, 7,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarBarGraph:value:
   *
   * The current value
   */
  bar_graph_props[PROP_VALUE] =
      g_param_spec_double("value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarBarGraph:spacing:
   *
   * The space between each bar
   */
  bar_graph_props[PROP_SPACING] = g_param_spec_uint(
      "spacing", NULL, NULL, 0, 100, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarBarGraph:orientation:
   *
   * The orientation of the widget, the internal orientation of the bars is
   * the oposite
   */
  bar_graph_props[PROP_ORIENTATION] = g_param_spec_enum(
      "orientation", "orientation", "orientation of the view",
      GTK_TYPE_ORIENTATION, GTK_ORIENTATION_VERTICAL, G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    bar_graph_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "bar-graph");
}

static void g_barbar_bar_graph_init(BarBarBarGraph *self) {}

static void get_min_max(GtkWidget *bar, double *min, double *max) {
  if (bar) {
    *min = gtk_level_bar_get_min_value(GTK_LEVEL_BAR(bar));
    *max = gtk_level_bar_get_max_value(GTK_LEVEL_BAR(bar));
  } else {
    *min = 0.0;
    *max = 1.0;
  }
}
static void g_barbar_bar_graph_defaults(BarBarBarGraph *self) {
  GtkWidget *bar;
  double min, max;
  GtkOrientation orientation;

  get_min_max(self->bars[0], &min, &max);
  orientation = oppsite_orientation(self->orientation);

  for (uint32_t i = 0; i < self->nums; i++) {
    if (self->bars[i]) {
      continue;
    }
    bar = gtk_level_bar_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(bar), orientation);
    gtk_level_bar_set_inverted(GTK_LEVEL_BAR(bar), TRUE);

    gtk_widget_set_parent(bar, GTK_WIDGET(self));
    self->bars[i] = bar;
  }
}

static void g_barbar_bar_graph_root(GtkWidget *widget) {
  BarBarBarGraph *bar = BARBAR_BAR_GRAPH(widget);

  g_barbar_bar_graph_defaults(bar);
}

/**
 * g_barbar_bar_graph_set_values:
 *
 * Pushes a batch of values to the graph
 *
 */
void g_barbar_bar_graph_set_values(BarBarBarGraph *self, guint num,
                                   double values[]) {
  int min = MIN(num, self->nums);

  for (int i = 0; i < min; i++) {
    GtkWidget *child = self->bars[i];
    if (GTK_IS_LEVEL_BAR(child)) {
      GtkLevelBar *bar = GTK_LEVEL_BAR(child);
      gtk_level_bar_set_value(bar, values[i]);
    }
  }
  self->current = min % self->nums;
}

GtkWidget *g_barbar_bar_graph_new(void) {
  return g_object_new(BARBAR_TYPE_BAR_GRAPH, NULL);
}
