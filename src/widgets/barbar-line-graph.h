/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Per Odlund
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include "widgets/barbar-graph.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_LINE_GRAPH (g_barbar_line_graph_get_type())

G_DECLARE_FINAL_TYPE(BarBarLineGraph, g_barbar_line_graph, BARBAR, LINE_GRAPH,
                     GtkWidget)

/*struct _BarBarGraphClass {*/
/*  GtkWidgetClass parent_instance;*/
/*};*/

void g_barbar_graph_set_stroke_width(BarBarLineGraph *self, float stroke);

void g_barbar_graph_set_fill(BarBarLineGraph *self, gboolean fill);

void g_barbar_graph_set_entry_numbers(BarBarLineGraph *self, guint length);

void g_barbar_graph_set_min_value(BarBarLineGraph *self, double min);

void g_barbar_graph_set_max_value(BarBarLineGraph *self, double max);

void g_barbar_graph_set_value(BarBarLineGraph *self, double value);

void g_barbar_graph_set_min_height(BarBarLineGraph *self, guint height);

void g_barbar_graph_set_min_width(BarBarLineGraph *self, guint width);
void g_barbar_graph_set_discrete(BarBarLineGraph *self, gboolean discrete);

GtkWidget *g_barbar_line_graph_new(void);

G_END_DECLS
