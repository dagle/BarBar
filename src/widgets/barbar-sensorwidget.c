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

static GParamSpec *label_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_sensor_widget_map(GtkWidget *widget);

static void g_barbar_sensor_widget_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {
  BarBarSensorWidget *widget = BARBAR_SENSOR_WIDGET(object);

  switch (property_id) {
  // case PROP_TEMPL:
  //   g_barbar_label_set_templ(label, g_value_get_string(value));
  //   break;
  // case PROP_LABEL:
  //   g_barbar_label_set_label(label, g_value_get_string(value));
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sensor_widget_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {

  BarBarSensorWidget *widget = BARBAR_SENSOR_WIDGET(object);
  // BarBarLabel *label = BARBAR_LABEL(object);
  switch (property_id) {
  // case PROP_CHILD:
  //   g_value_set_object(value, label->child);
  //   break;
  // case PROP_TEMPL:
  //   g_value_set_string(value, label->templ);
  //   break;
  // case PROP_LABEL: {
  //   const char *str = gtk_label_get_text(GTK_LABEL(label->child));
  //   g_value_set_string(value, str);
  //   break;
  // }
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
  label_props[PROP_SENSOR] =
      g_param_spec_string("sensor", "Sensor", "The sensor", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  /**
   * BarBarSensorWidget:child:
   *
   * Child widget
   */
  label_props[PROP_CHILD] =
      g_param_spec_object("child", "Child", "Child label", GTK_TYPE_LABEL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, label_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void g_barbar_sensor_widget_init(BarBarSensorWidget *self) {}

static void g_barbar_sensor_widget_map(GtkWidget *widget) {
  BarBarSensorWidget *sw = BARBAR_SENSOR_WIDGET(widget);

  GTK_WIDGET_CLASS(g_barbar_sensor_widget_parent_class)->root(widget);

  g_barbar_sensor_start(sw->sensor);
}
