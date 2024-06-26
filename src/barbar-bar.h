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

/**
 * BarBarBarPosition:
 * @BARBAR_POS_TOP: Put the bar at top.
 * @BARBAR_POS_BOTTOM: Put the bar at bottom.
 * @BARBAR_POS_LEFT: Put the bar at left.
 * @BARBAR_POS_RIGHT: Put the bar at right.
 *
 * Describs the aviable positions to anchor the bar.
 */
typedef enum {
  BARBAR_POS_TOP,
  BARBAR_POS_BOTTOM,
  BARBAR_POS_LEFT,
  BARBAR_POS_RIGHT,
} BarBarBarPosition;

GType g_barbar_position_get_type(void);

#define BARBAR_TYPE_BAR (g_barbar_bar_get_type())

#define BARBAR_TYPE_POSITION (g_barbar_position_get_type())

G_DECLARE_FINAL_TYPE(BarBarBar, g_barbar_bar, BARBAR, BAR, GtkWindow)

GtkWidget *g_barbar_bar_new(void);

G_END_DECLS
