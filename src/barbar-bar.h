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

#include "barbar-enum.h"
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_BAR (g_barbar_bar_get_type())

G_DECLARE_FINAL_TYPE(BarBarBar, g_barbar_bar, BARBAR, BAR, GtkWindow)

GtkWidget *g_barbar_bar_new(void);

void g_barbar_bar_set_pos(BarBarBar *bar, BarBarBarPosition pos);

void g_barbar_bar_set_screen_num(BarBarBar *bar, uint num);

void g_barbar_bar_set_height(BarBarBar *bar, uint height);

void g_barbar_bar_set_margin(BarBarBar *bar, GtkLayerShellEdge edge,
                             uint margin);

G_END_DECLS
