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

#define BARBAR_TYPE_CAVA (g_barbar_cava_get_type())

G_DECLARE_FINAL_TYPE(BarBarCava, g_barbar_cava, BARBAR, CAVA, BarBarGraph)

/*GArray *g_barbar_cava_cava_get_values(BarBarCava *self);*/

/*gint g_barbar_cava_cava_get_bars(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_bars(BarBarCava *self, gint bars);*/
/**/
/*gboolean g_barbar_cava_cava_get_autosens(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_autosens(BarBarCava *self, gboolean autosens);*/
/**/
/*gboolean g_barbar_cava_cava_get_stereo(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_stereo(BarBarCava *self, gboolean stereo);*/
/**/
/*gdouble g_barbar_cava_cava_get_noise_reduction(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_noise_reduction(BarBarCava *self, gdouble
 * noise);*/
/**/
/*gint g_barbar_cava_cava_get_framerate(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_framerate(BarBarCava *self, gint framerate);*/
/**/
/*gchar *g_barbar_cava_cava_get_source(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_source(BarBarCava *self, const gchar *source);*/
/**/
/*gint g_barbar_cava_cava_get_channels(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_channels(BarBarCava *self, gint channels);*/
/**/
/*gint g_barbar_cava_cava_get_low_cutoff(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_low_cutoff(BarBarCava *self, gint low_cutoff);*/
/**/
/*gint g_barbar_cava_cava_get_high_cutoff(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_high_cutoff(BarBarCava *self, gint high_cutoff);*/
/**/
/*gint g_barbar_cava_cava_get_samplerate(BarBarCava *self);*/
/*void g_barbar_cava_cava_set_samplerate(BarBarCava *self, gint samplerate);*/

GtkWidget *g_barbar_cava_new(void);

G_END_DECLS
