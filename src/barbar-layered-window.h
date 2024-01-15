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

#ifndef _BARBAR_LAYERED_WINDOW_H_
#define _BARBAR_LAYERED_WINDOW_H_

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * BarBarPosition:
 * @BARBAR_POS_TOP: Put the bar at top.
 * @BARBAR_POS_BOTTOM: Put the bar at bottom.
 * @BARBAR_POS_LEFT: Put the bar at left.
 * @BARBAR_POS_RIGHT: Put the bar at right.
 *
 * Describs the aviable positions to anchor the bar.
 */
typedef enum {
  BARBAR_POS_TOP = 1 << 0,
  BARBAR_POS_BOTTOM = 1 << 1,
  BARBAR_POS_LEFT = 1 << 2,
  BARBAR_POS_RIGHT = 1 << 3,
} BarBarPosition;

#define BARBAR_TYPE_LAYERED_WINDOW (g_barbar_layered_window_get_type())

#define BARBAR_TYPE_POSITION (g_barbar_layer_position_get_type())

G_DECLARE_FINAL_TYPE(BarBarBar, g_barbar_layered_window, BARBAR, LAYERED_WINDOW,
                     GtkWindow)

GtkWidget *g_barbar_layered_windown_new(void);

/* int g_barbar_run(BarBarBar *bar, int argc, char **argv, GtkWidget *w); */
/* int g_barbar_bars_run(BarBarBar **bars, int argc, char **argv); */

G_END_DECLS

#endif /* _BARBAR_LAYERED_WINDOW_H_ */
