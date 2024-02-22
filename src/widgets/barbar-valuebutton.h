/*
 * Copyright © 2024 Per Odlund
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
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_VALUE_BUTTON (g_barbar_label_get_type())

G_DECLARE_FINAL_TYPE(BarBarLabel, g_barbar_label, BARBAR, LABEL, GtkWidget)

void label_update(GtkWidget *object, GObject *sensor);

G_END_DECLS
