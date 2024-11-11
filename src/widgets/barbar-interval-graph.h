#pragma once

#include "barbar-graph.h"

G_BEGIN_DECLS

#define BARBAR_TYPE_INTERVAL_GRAPH (g_barbar_graph_get_type())

G_DECLARE_FINAL_TYPE(BarBarIntervalGraph, g_barbar_interval_graph, BARBAR,
                     INTERVAL_GRAPH, BarBarGraph)

GtkWidget *g_barbar_interval_graph_new(void);

G_END_DECLS
