#include "barbar-sensorwidget.h"
#include "gtk/gtkshortcut.h"

/**
 * BarBarSensorWidget:
 *
 * A `BarBarSensorWidget` works like a gtkbin but also contains a sensor.
 * The reason for this is that the widget needs to be mapped before
 * we can start the sensor and the sensor and the widget needs to
 * be coupled. An example of this is an inhibitor that is connected
 * to a screen but the widget it self could be anything. Doing it like
 * this, the sensor can then find information about the screen it's
 * connected to.
 *
 */
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

/**
 * g_barbar_sensor_wiget_set_child:
 * @widget: a `BarBarSensorWidget`
 * @child: a `GtkWidget`
 *
 * Connects a widget to the sensor.
 */
void g_barbar_sensor_wiget_set_child(BarBarSensorWidget *widget,
                                     GtkWidget *child) {
  g_return_if_fail(BARBAR_IS_SENSOR_WIDGET(widget));
  g_return_if_fail(GTK_IS_WIDGET(child));

  if (widget->child) {
    g_object_unref(widget->child);
  }

  widget->child = g_object_ref(child);

  g_object_notify_by_pspec(G_OBJECT(widget), sw_props[PROP_CHILD]);
}
/**
 * g_barbar_sensor_wiget_get_child:
 * @widget: a `BarBarSensorWidget`
 *
 * Returns: (transfer none): `GtkWidget`
 */
GtkWidget *g_barbar_sensor_wiget_get_child(BarBarSensorWidget *widget) {
  g_return_val_if_fail(BARBAR_IS_SENSOR_WIDGET(widget), NULL);

  return widget->child;
}

/**
 * g_barbar_sensor_wiget_set_sensor:
 * @widget: a `BarBarSensorWidget`
 * @sensor: a `BarBarSensorContext`
 *
 * Connects a sensors to the widget.
 */
void g_barbar_sensor_wiget_set_sensor(BarBarSensorWidget *widget,
                                      BarBarSensorContext *sensor) {
  g_return_if_fail(BARBAR_IS_SENSOR_WIDGET(widget));
  g_return_if_fail(BARBAR_IS_SENSOR_CONTEXT(sensor));

  if (widget->sensor) {
    g_object_unref(widget->sensor);
  }

  widget->sensor = g_object_ref(sensor);

  g_object_notify_by_pspec(G_OBJECT(widget), sw_props[PROP_SENSOR]);
}

/**
 * g_barbar_sensor_wiget_get_sensor:
 * @widget: a `BarBarSensorWidget`
 *
 * Returns: (transfer none): `BarBarSensorContext`
 */
BarBarSensorContext *
g_barbar_sensor_wiget_get_sensor(BarBarSensorWidget *widget) {
  g_return_val_if_fail(BARBAR_IS_SENSOR_WIDGET(widget), NULL);

  return widget->sensor;
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
    g_barbar_sensor_context_start(sw->sensor, widget);
  }
}

/**
 * g_barbar_sensor_widget_new:
 *
 * Creates a new `BarBarSensorWidget`
 *
 * Returns: (transfer full): A `GtkWidget`
 */
GtkWidget *g_barbar_sensor_widget_new(BarBarSensor *sensor, GtkWidget *widget) {
  BarBarSensorWidget *self;

  self = g_object_new(BARBAR_TYPE_SENSOR_WIDGET, "sensor", sensor, "child",
                      widget, NULL);

  return GTK_WIDGET(self);
}
