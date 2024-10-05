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

#define BARBAR_TYPE_VALUE_ICON (g_barbar_value_icon_get_type())

G_DECLARE_FINAL_TYPE(BarBarValueIcon, g_barbar_value_icon, BARBAR, VALUE_ICON,
                     GtkWidget)

void g_barbar_value_icon_set_icons(BarBarValueIcon *icons, const char **names);

void g_barbar_value_icon_set_upper(BarBarValueIcon *icons, double upper);
double g_barbar_value_icon_get_upper(BarBarValueIcon *icons);

void g_barbar_value_icon_set_lower(BarBarValueIcon *icons, double lower);
double g_barbar_value_icon_get_lower(BarBarValueIcon *icons);

void g_barbar_value_icon_set_value(BarBarValueIcon *icons, double value);
double g_barbar_value_icon_get_value(BarBarValueIcon *icons);

GtkWidget *g_barbar_value_icon_new(char **icons);

G_END_DECLS
