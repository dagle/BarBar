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

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_ACTIVITY_GRAPH (g_barbar_activity_graph_get_type())

G_DECLARE_FINAL_TYPE(BarBarActivityGraph, g_barbar_activity_graph, BARBAR,
                     ACTIVITY_GRAPH, GtkWidget)

GtkWidget *g_barbar_activity_graph_new(int cols, int rows);

void g_barbar_activity_graph_set_activity(BarBarActivityGraph *graph, int col,
                                          int row, int level);

void g_barbar_activity_graph_set_activity_linear(BarBarActivityGraph *graph,
                                                 int num, int activity);

void g_barbar_activity_graph_set_color(BarBarActivityGraph *graph, int col,
                                       int row, const char *color);
void g_barbar_activity_graph_set_color_linear(BarBarActivityGraph *graph,
                                              int num, const char *color);

uint g_barbar_activity_graph_get_rows(BarBarActivityGraph *graph);

uint g_barbar_activity_graph_get_cols(BarBarActivityGraph *graph);

uint g_barbar_activity_graph_get_size(BarBarActivityGraph *graph);

G_END_DECLS
