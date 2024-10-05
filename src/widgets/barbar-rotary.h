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

#define BARBAR_TYPE_ROTARY (g_barbar_rotary_get_type())

G_DECLARE_FINAL_TYPE(BarBarRotary, g_barbar_rotary, BARBAR, ROTARY, GtkWidget)

void g_barbar_rotary_set_mode(BarBarRotary *rotary, guint mode);

void g_barbar_rotary_set_value(BarBarRotary *self, double value);

void g_barbar_rotary_set_background(BarBarRotary *self, const char *background);

void g_barbar_rotary_set_width_procentage(BarBarRotary *self, float procent);

void g_barbar_rotary_set_min_value(BarBarRotary *self, double value);

void g_barbar_rotary_set_max_value(BarBarRotary *self, double value);

void g_barbar_rotary_set_inverted(BarBarRotary *self, gboolean inverted);

GtkWidget *g_barbar_rotary_new(void);

G_END_DECLS
