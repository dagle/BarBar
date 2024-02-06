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

/**
 * BarBarSensorWidget:
 *
 * A gtk sensors works like a gtkbin but also contains a sensor.
 * The reason for this is that the widget needs to be mapped before
 * we can start the sensor and the sensor and the widget is coupled.
 *
 */

#include "barbar-sensorwidget.h"
#include "sensors/barbar-sensorcontext.h"

struct _BarBarSensorWidget {
  GtkWidget parent_instance;

  GtkWidget *child;
  BarBarSensorContext *sensor;
};

G_DEFINE_TYPE(BarBarSensorWidget, g_barbar_sensor_widget, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_CHILD,
  PROP_SENSOR,

  NUM_PROPERTIES,
};

static GParamSpec *sw_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_sensor_widget_map(GtkWidget *widget);

static void g_barbar_sensor_wiget_set_child(BarBarSensorWidget *widget,
                                            GtkWidget *child) {
  g_return_if_fail(BARBAR_IS_SENSOR_WIDGET(widget));
  g_return_if_fail(GTK_IS_WIDGET(child));

  if (widget->child) {
    g_object_unref(widget->child);
  }

  widget->child = g_object_ref(child);

  g_object_notify_by_pspec(G_OBJECT(widget), sw_props[PROP_CHILD]);
}

static void g_barbar_sensor_wiget_set_sensor(BarBarSensorWidget *widget,
                                             BarBarSensorContext *sensor) {
  g_return_if_fail(BARBAR_IS_SENSOR_WIDGET(widget));
  g_return_if_fail(BARBAR_IS_SENSOR_CONTEXT(sensor));

  if (widget->sensor) {
    g_object_unref(widget->sensor);
  }

  widget->sensor = g_object_ref(sensor);

  g_object_notify_by_pspec(G_OBJECT(widget), sw_props[PROP_SENSOR]);
}

static void g_barbar_sensor_widget_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {
  BarBarSensorWidget *widget = BARBAR_SENSOR_WIDGET(object);

  switch (property_id) {
  case PROP_CHILD:
    g_barbar_sensor_wiget_set_child(widget, g_value_get_object(value));
    break;
  case PROP_SENSOR:
    g_barbar_sensor_wiget_set_sensor(widget, g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sensor_widget_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {

  BarBarSensorWidget *widget = BARBAR_SENSOR_WIDGET(object);
  switch (property_id) {
  case PROP_CHILD:
    g_value_set_object(value, widget->child);
    break;
  case PROP_SENSOR:
    g_value_set_object(value, widget->sensor);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sensor_widget_class_init(BarBarSensorWidgetClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sensor_widget_set_property;
  gobject_class->get_property = g_barbar_sensor_widget_get_property;

  widget_class->map = g_barbar_sensor_widget_map;

  /**
   * BarBarSensorWidget:sensor:
   *
   * The label string
   */
  sw_props[PROP_SENSOR] = g_param_spec_object(
      "sensor", "Sensor", "The sensor", BARBAR_TYPE_SENSOR_CONTEXT,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarSensorWidget:child:
   *
   * Child widget
   */
  sw_props[PROP_CHILD] =
      g_param_spec_object("child", "Child", "Child widget", GTK_TYPE_WIDGET,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, sw_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void g_barbar_sensor_widget_init(BarBarSensorWidget *self) {}

static void g_barbar_sensor_widget_map(GtkWidget *widget) {
  BarBarSensorWidget *sw = BARBAR_SENSOR_WIDGET(widget);
  gtk_widget_set_parent(sw->child, widget);

  GTK_WIDGET_CLASS(g_barbar_sensor_widget_parent_class)->map(widget);

  if (sw->sensor) {
    barbar_sensor_context_start(sw->sensor, widget);
  }
}
