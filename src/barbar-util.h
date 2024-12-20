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

GtkWindow *g_barbar_get_parent_layer_window(GtkWidget *widget);

void g_barbar_default_style_provider(const char *path);

void g_barbar_search_style_provider(const char *path);

GtkBuilder *g_barbar_default_builder(const char *path, GError **err);

/*GtkBuilder *g_barbar_default_blueprint(const char *path, GError *err);*/

char *g_barbar_print_percent(gpointer *ptr, double percent, guint decimal,
                             gboolean sign);

char *g_barbar_print_bytes(gpointer *ptr, const char *format, ...);
char *g_barbar_printf(gpointer *ptr, const char *format, ...);
char *g_barbar_print_autosized(guint64 bytes, uint decimals);

G_END_DECLS
