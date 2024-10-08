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

#include <gio/gio.h>

G_BEGIN_DECLS

typedef enum {
  NIRI_Quit,
} BarBarNiriActionType;

GSocketConnection *g_barbar_niri_ipc_connect(GError **error);

void g_barbar_niri_ipc_oneshot(GCancellable *cancellable,
                               GAsyncReadyCallback callback, gpointer data,
                               const char *format, ...);

gboolean g_barbar_niri_ipc_oneshot_finish(GOutputStream *stream,
                                          GAsyncResult *result, GError **error);

G_END_DECLS
