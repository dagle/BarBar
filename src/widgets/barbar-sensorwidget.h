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

#include "sensors/barbar-sensor.h"
#include "sensors/barbar-sensorcontext.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_SENSOR_WIDGET (g_barbar_sensor_widget_get_type())

G_DECLARE_FINAL_TYPE(BarBarSensorWidget, g_barbar_sensor_widget, BARBAR,
                     SENSOR_WIDGET, GtkWidget)

void g_barbar_sensor_wiget_set_child(BarBarSensorWidget *widget,
                                     GtkWidget *child);

GtkWidget *g_barbar_sensor_wiget_get_child(BarBarSensorWidget *widget);

void g_barbar_sensor_wiget_set_sensor(BarBarSensorWidget *widget,
                                      BarBarSensorContext *sensor);
BarBarSensorContext *
g_barbar_sensor_wiget_get_sensor(BarBarSensorWidget *widget);

GtkWidget *g_barbar_sensor_widget_new(BarBarSensor *sensor, GtkWidget *widget);

G_END_DECLS
