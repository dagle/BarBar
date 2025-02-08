#include "barbar-graph.h"
#include <stdio.h>

/**
 * BarBarGraph:
 *
 * An abstract class to for graphs
 * Allows you to set push a value
 * or to set all values
 *
 */
G_DEFINE_INTERFACE(BarBarGraph, g_barbar_graph, GTK_TYPE_WIDGET)

static void g_barbar_graph_default_init(BarBarGraphInterface *iface) {

  iface->set_values = NULL;
  iface->push_value = NULL;
  iface->get_size = NULL;

  g_object_interface_install_property(
      iface,
      g_param_spec_double("value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY));
}

void gtk_editable_delete_text(GtkEditable *editable, int start_pos,
                              int end_pos) {
  g_return_if_fail(GTK_IS_EDITABLE(editable));
  g_return_if_fail(start_pos >= 0);
  g_return_if_fail(end_pos == -1 || end_pos >= start_pos);

  GTK_EDITABLE_GET_IFACE(editable)->do_delete_text(editable, start_pos,
                                                   end_pos);
  // BARBAR_GRAPH_GET_IFACE (editable)->do_delete_text (editable, start_pos,
  // end_pos);
}

// static void g_barbar_graph_class_init(BarBarGraphClass *class) {
// }
//
// static void g_barbar_graph_init(BarBarGraph *self) {}

void g_barbar_graph_set_values(BarBarGraph *self, size_t len, double *values) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));
  //
  // BARBAR_GRAPH_GET_CLASS(self)->set_values(self, len, values);
  //
  BARBAR_GRAPH_GET_IFACE(self)->set_values(self, len, values);
}

void g_barbar_graph_push_value(BarBarGraph *self, double value) {
  g_return_if_fail(BARBAR_IS_GRAPH(self));

  BARBAR_GRAPH_GET_IFACE(self)->push_value(self, value);
}

size_t g_barbar_graph_get_size(BarBarGraph *self) {
  g_return_val_if_fail(BARBAR_IS_GRAPH(self), -1);

  return BARBAR_GRAPH_GET_IFACE(self)->get_size(self);
}
