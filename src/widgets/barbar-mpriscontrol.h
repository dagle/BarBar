/*
 * Copyright © 2018 Benjamin Otte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Benjamin Otte <otte@gnome.org>
 */

#pragma once

#include "sensors/barbar-mpris2.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_MPRIS_CONTROLS (g_barbar_mpris_controls_get_type())

G_DECLARE_FINAL_TYPE(BarBarMprisControls, g_barbar_mpris_controls, BARBAR,
                     MPRIS_CONTROLS, GtkWidget)

GtkWidget *g_barbar_mpris_controls_new(GtkMediaStream *stream);

/* GtkMediaStream *gtk_media_controls_get_media_stream(GtkMediaControls
 * *controls); */

void g_barbar_mpris_controls_set_media_stream(BarBarMprisControls *controls,
                                              BarBarMpris *mpris);

G_END_DECLS
