#include "barbar-bar-graph.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "widgets/barbar-graph.h"

/**
 * BarBarBarGraph:
 *
 * Display changes over time for a value
 */
struct _BarBarBarGraph {
  // BarBarGraph parent_instance;
  GtkBox parent_instance;

  // GtkWidget *box;
  // GtkOrientation orientation;
  guint spacing;
  guint current;

  guint nums;

  GtkWidget *bars[32];
};

enum {
  PROP_0,

  // PROP_NUMS,

  PROP_VALUE,
  LAST_PROP = PROP_VALUE,
};

static GtkBuildableIface *parent_buildable_iface;

static void
g_barbar_bar_graph_buildable_interface_init(GtkBuildableIface *iface);
static void g_barbar_bar_graph_root(GtkWidget *widget);

static void
g_barbar_bar_graph_graph_interface_init(BarBarGraphInterface *iface);

static size_t g_barbar_bar_graph_get_size(BarBarGraph *graph);
static void g_barbar_bar_graph_set_values(BarBarGraph *graph, size_t len,
                                          double *values);
static void g_barbar_bar_graph_push_value(BarBarGraph *graph, double value);

// static void g_barbar_bar_graph_root(GtkWidget *widget);

// static GtkOrientation oppsite_orientation(GtkOrientation orientation) {
//   if (orientation == GTK_ORIENTATION_VERTICAL) {
//     return GTK_ORIENTATION_HORIZONTAL;
//   }
//   return GTK_ORIENTATION_VERTICAL;
// }

G_DEFINE_FINAL_TYPE_WITH_CODE(
    BarBarBarGraph, g_barbar_bar_graph, GTK_TYPE_BOX,
    // G_ADD_PRIVATE(BarBarRiverTag)
    G_IMPLEMENT_INTERFACE(BARBAR_TYPE_GRAPH,
                          g_barbar_bar_graph_graph_interface_init)
        G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                              g_barbar_bar_graph_buildable_interface_init))

static GParamSpec *bar_graph_props[LAST_PROP] = {
    NULL,
};

/**
 * g_barbar_bar_graph_add_bar:
 *
 * Adds a bar to the graph.
 * The child has to be a bar, currently only gtklevelbar is supported
 *
 */
// void g_barbar_bar_graph_add_bar(BarBarBarGraph *self, GtkWidget *child) {
//   gtk_box_append(GTK_BOX(self), child);
//
//   update_capacity(self);
//
//   self->bars[self->nums] = child;
//   self->nums++;
// }

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
g_barbar_bar_graph_graph_interface_init(BarBarGraphInterface *iface) {

  iface->set_values = g_barbar_bar_graph_set_values;
  iface->push_value = g_barbar_bar_graph_push_value;
  iface->get_size = g_barbar_bar_graph_get_size;
}

static void
g_barbar_bar_graph_buildable_interface_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_bar_graph_add_child;
}

// void g_barbar_bar_graph_set_nums(BarBarBarGraph *graph, guint nums) {
//   g_return_if_fail(BARBAR_IS_BAR_GRAPH(graph));
//
//   if (graph->nums == nums) {
//     return;
//   }
//
//   graph->nums = nums;
//
//   g_object_notify_by_pspec(G_OBJECT(graph), bar_graph_props[PROP_NUMS]);
// }

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

static void g_barbar_bar_graph_set_orientation(BarBarBarGraph *graph,
                                               GtkOrientation orientation) {
  // GtkBoxLayout *layout;
  g_return_if_fail(BARBAR_IS_BAR_GRAPH(graph));

  // if (graph->orientation == orientation) {
  //   return;
  // }
  //
  // graph->orientation = orientation;

  // layout =
  // GTK_BOX_LAYOUT(gtk_widget_get_layout_manager(GTK_WIDGET(graph)));

  // if (gtk_orientable_get_orientation(GTK_ORIENTABLE(layout)) !=
  // orientation)
  // {
  //   printf("LEVELBAR :%p!\n",
  // gtk_widget_get_first_child(GTK_WIDGET(graph)));
  //   gtk_orientable_set_orientation(GTK_ORIENTABLE(layout), orientation);
  //
  //   GtkWidget *child;
  //   for (child = gtk_widget_get_first_child(GTK_WIDGET(graph)); child !=
  //   NULL;
  //        child = gtk_widget_get_next_sibling(child)) {
  //     if (GTK_IS_LEVEL_BAR(child)) {
  //       GtkLevelBar *bar = GTK_LEVEL_BAR(child);
  //       gtk_orientable_set_orientation(GTK_ORIENTABLE(bar), orientation);
  //       // TODO: Maybe don't do this.
  //       // gtk_level_bar_set_inverted(GTK_LEVEL_BAR(bar), TRUE);
  //     }
  //   }
  // }
}

// /**
//  * g_barbar_bar_graph_add_bar:
//  *
//  * Adds a bar to the graph.
//  * The child has to be a bar, currently only gtklevelbar is supported
//  *
//  */
void g_barbar_bar_graph_add_bar(BarBarBarGraph *self, GtkWidget *child) {
  gtk_box_append(GTK_BOX(self), child);

  if (GTK_IS_LEVEL_BAR(child)) {
    for (int i = 0; i < 32; i++) {
      if (self->bars[i] == NULL) {
        self->bars[i] = g_object_ref(child);
        self->nums++;
        return;
      }
    }
  }
}

static void g_barbar_bar_graph_push_value(BarBarGraph *graph, double value) {
  g_return_if_fail(BARBAR_IS_BAR_GRAPH(graph));

  BarBarBarGraph *self = BARBAR_BAR_GRAPH(graph);

  double old;

  old = set_bar(self->bars[0], value);

  for (int i = 1; i < self->nums; i++) {
    GtkWidget *child = self->bars[i];
    old = set_bar(child, old);
  }
}

static void g_barbar_bar_graph_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarBarGraph *graph = BARBAR_BAR_GRAPH(object);

  switch (property_id) {
  // case PROP_NUMS:
  //   g_barbar_bar_graph_set_nums(graph, g_value_get_uint(value));
  //   break;
  case PROP_VALUE:
    g_barbar_bar_graph_push_value(BARBAR_GRAPH(graph),
                                  g_value_get_double(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_bar_graph_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {

  BarBarBarGraph *graph = BARBAR_BAR_GRAPH(object);

  switch (property_id) {
  // case PROP_NUMS:
  //   g_value_set_uint(value, graph->nums);
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static size_t g_barbar_bar_graph_get_size(BarBarGraph *graph) {
  BarBarBarGraph *self = BARBAR_BAR_GRAPH(graph);

  return self->nums;
}

static void g_barbar_bar_graph_set_values(BarBarGraph *graph, size_t len,
                                          double *values) {
  g_return_if_fail(BARBAR_IS_BAR_GRAPH(graph));

  BarBarBarGraph *self = BARBAR_BAR_GRAPH(graph);

  int min = MIN(len, self->nums);

  for (int i = 0; i < min; i++) {
    GtkWidget *child = self->bars[i];
    if (GTK_IS_LEVEL_BAR(child)) {
      GtkLevelBar *bar = GTK_LEVEL_BAR(child);
      gtk_level_bar_set_value(bar, values[i]);
    }
  }
  self->current = min;
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
   * BarBarBarGraph:value:
   *
   * Setting this value pushes a value to the graph
   *
   */
  g_object_class_override_property(gobject_class, PROP_VALUE, "value");

  // /**
  //  * BarBarBarGraph:nums:
  //  *
  //  * How many bars we should show
  //  */
  // bar_graph_props[PROP_NUMS] = g_param_spec_uint(
  //     "nums", NULL, NULL, 0, 200, 7,
  //     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  // g_object_class_install_properties(gobject_class, LAST_PROP,
  // bar_graph_props); gtk_widget_class_set_layout_manager_type(widget_class,
  // GTK_TYPE_BIN_LAYOUT);

  // gtk_widget_class_set_layout_manager_type(widget_class,
  // GTK_TYPE_BOX_LAYOUT);
  // gtk_widget_class_set_css_name(widget_class, "box");
}

static void g_barbar_bar_graph_defaults(BarBarBarGraph *self);

static void g_barbar_bar_graph_init(BarBarBarGraph *self) {

  GtkOrientation orientation =
      gtk_orientable_get_orientation(GTK_ORIENTABLE(self));
  gtk_orientable_set_orientation(GTK_ORIENTABLE(self),
                                 GTK_ORIENTATION_HORIZONTAL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);
  } else {
    gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);
  }
}

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
  GtkLevelBar *bar;

  // this should be moved
  GtkOrientation orientation =
      gtk_orientable_get_orientation(GTK_ORIENTABLE(self));
  // gtk_orientable_set_orientation(GTK_ORIENTABLE(self),
  //                                GTK_ORIENTATION_HORIZONTAL);

  // if (orientation == GTK_ORIENTATION_HORIZONTAL) {
  //   gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);
  // } else {
  //   gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);
  // }
  // self->nums = 4;
  //
  // for (int i = 0; i < self->nums; i++) {
  //   if (self->bars[i]) {
  //     continue;
  //   }
  //   bar = GTK_LEVEL_BAR(gtk_level_bar_new());
  //   gtk_orientable_set_orientation(GTK_ORIENTABLE(bar),
  //                                  GTK_ORIENTATION_VERTICAL);
  //   gtk_level_bar_set_min_value(bar, 0.0);
  //   gtk_level_bar_set_max_value(bar, 1.0);
  //   gtk_level_bar_set_value(bar, 0.5);
  //   gtk_box_append(GTK_BOX(self), GTK_WIDGET(bar));
  //
  //   self->bars[i] = GTK_WIDGET(bar);
  // }
}

static void g_barbar_bar_graph_root(GtkWidget *widget) {
  BarBarBarGraph *bar = BARBAR_BAR_GRAPH(widget);

  g_barbar_bar_graph_defaults(BARBAR_BAR_GRAPH(widget));
}

GtkWidget *g_barbar_bar_graph_new(void) {
  return g_object_new(BARBAR_TYPE_BAR_GRAPH, NULL);
}
