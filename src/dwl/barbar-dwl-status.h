/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Per Odlund
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

#define BARBAR_TYPE_DWL_STATUS (g_barbar_dwl_status_get_type())

typedef struct {
  char *output_name;

  char *title;
  char *appid;
  char *layout;
  guint32 occupied;
  guint32 selected;
  guint32 client_tags;
  guint32 urgent;
  gboolean fullscreen;
  gboolean floating;
  gboolean selmon; // this should be a guint32?

  gatomicrefcount ref_count;
} BarBarDwlStatus;

GType g_barbar_dwl_status_get_type(void);

BarBarDwlStatus *g_barbar_dwl_status_new(void);

BarBarDwlStatus *g_barbar_dwl_status_ref(BarBarDwlStatus *status);

void g_barbar_dwl_status_unref(BarBarDwlStatus *status);

const char *g_barbar_dwl_status_get_title(BarBarDwlStatus *status);
const char *g_barbar_dwl_status_get_appid(BarBarDwlStatus *status);
const char *g_barbar_dwl_status_get_layout(BarBarDwlStatus *status);

guint32 g_barbar_dwl_status_get_occupide(BarBarDwlStatus *status);
guint32 g_barbar_dwl_status_get_selected(BarBarDwlStatus *status);
guint32 g_barbar_dwl_status_get_client_tags(BarBarDwlStatus *status);
guint32 g_barbar_dwl_status_get_urgent(BarBarDwlStatus *status);

gboolean g_barbar_dwl_status_get_fullscreen(BarBarDwlStatus *status);
gboolean g_barbar_dwl_status_get_float(BarBarDwlStatus *status);
gboolean g_barbar_dwl_status_get_selmon(BarBarDwlStatus *status);
