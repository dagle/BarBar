#pragma once

#include <gtk/gtk.h>

#define BARBAR_TYPE_GRAPH (g_barbar_graph_get_type())

G_DECLARE_FINAL_TYPE(BarBarGraph, g_barbar_graph, BARBAR, GRAPH, GtkWidget)

GtkWidget *g_barbar_graph_new(void);
