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

#include "wlr-output-management-unstable-v1-client-protocol.h"
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_OUTPUT_HEAD (g_barbar_output_head_get_type())

G_DECLARE_FINAL_TYPE(BarBarOutputHead, g_barbar_output_head, BARBAR,
                     OUTPUT_HEAD, GObject)

GObject *g_barbar_output_head_new(struct zwlr_output_head_v1 *head);

void g_barbar_output_head_run(BarBarOutputHead *self);

const char *g_barbar_output_head_get_name(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_resolution_height(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_resolution_width(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_refresh(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_height(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_width(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_pos_y(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_pos_x(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_transform(BarBarOutputHead *head);

gint32 g_barbar_output_head_get_scale(BarBarOutputHead *head);

gboolean g_barbar_output_head_get_sync(BarBarOutputHead *head);

gboolean g_barbar_output_head_get_enabled(BarBarOutputHead *head);

const char *g_barbar_output_head_get_description(BarBarOutputHead *head);

const char *g_barbar_output_head_get_make(BarBarOutputHead *head);

const char *g_barbar_output_head_get_model(BarBarOutputHead *head);

const char *g_barbar_output_head_serial_get_number(BarBarOutputHead *head);

G_END_DECLS
