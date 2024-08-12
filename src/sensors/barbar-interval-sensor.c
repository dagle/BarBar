#include "sensors/barbar-interval-sensor.h"
#include "glib-object.h"
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdio.h>

/**
 * BarBarIntervalSensor:
 *
 * A sensor subclass that fires off every x seconds, or once if interval isn't
 * set.
 */
typedef struct {
  BarBarSensor parent;

  guint interval;
  guint source_id;
} BarBarIntervalSensorPrivate;

enum {
  PROP_0,

  PROP_INTERVAL,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(BarBarIntervalSensor,
                                    g_barbar_interval_sensor,
                                    BARBAR_TYPE_SENSOR)

static GParamSpec *interval_props[NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_interval_sensor_set_interval(BarBarIntervalSensor *self,
                                           uint interval) {
  g_return_if_fail(BARBAR_IS_INTERVAL_SENSOR(self));
  BarBarIntervalSensorPrivate *priv =
      g_barbar_interval_sensor_get_instance_private(self);

  priv->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(self), interval_props[PROP_INTERVAL]);
}

uint g_barbar_interval_sensor_get_interval(BarBarIntervalSensor *self) {
  BarBarIntervalSensorPrivate *priv =
      g_barbar_interval_sensor_get_instance_private(self);
  return priv->interval;
}

static void g_barbar_interval_sensor_set_property(GObject *object,
                                                  guint property_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec) {

  BarBarIntervalSensor *interval = BARBAR_INTERVAL_SENSOR(object);

  switch (property_id) {
  case PROP_INTERVAL:
    g_barbar_interval_sensor_set_interval(interval, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_interval_sensor_get_property(GObject *object,
                                                  guint property_id,
                                                  GValue *value,
                                                  GParamSpec *pspec) {
  BarBarIntervalSensor *interval = BARBAR_INTERVAL_SENSOR(object);

  BarBarIntervalSensorPrivate *priv =
      g_barbar_interval_sensor_get_instance_private(interval);

  switch (property_id) {
  case PROP_INTERVAL:
    g_value_set_uint(value, priv->interval);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_interval_sensor_start(BarBarSensor *sensor);

static void
g_barbar_interval_sensor_class_init(BarBarIntervalSensorClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_interval_sensor_set_property;
  gobject_class->get_property = g_barbar_interval_sensor_get_property;
  sensor_class->start = g_barbar_interval_sensor_start;

  /**
   * BarBarIntervalSensor:interval:
   *
   * How often the tick function should run.
   */
  interval_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    interval_props);
}

static void g_barbar_interval_sensor_init(BarBarIntervalSensor *self) {
  BarBarIntervalSensorPrivate *priv =
      g_barbar_interval_sensor_get_instance_private(self);
  priv->interval = DEFAULT_INTERVAL;
}

static gboolean g_barbar_interval_sensor_update(gpointer data) {
  BarBarIntervalSensor *sensor = BARBAR_INTERVAL_SENSOR(data);

  return BARBAR_INTERVAL_SENSOR_GET_CLASS(sensor)->tick(sensor);
}

static void g_barbar_interval_sensor_start(BarBarSensor *sensor) {
  gboolean ret;
  BarBarIntervalSensor *interval = BARBAR_INTERVAL_SENSOR(sensor);
  BarBarIntervalSensorPrivate *priv =
      g_barbar_interval_sensor_get_instance_private(interval);

  if (priv->source_id > 0) {
    g_source_remove(priv->source_id);
  }

  ret = BARBAR_INTERVAL_SENSOR_GET_CLASS(sensor)->tick(interval);

  if (!ret) {
    return;
  }

  priv->source_id = g_timeout_add_full(
      0, priv->interval, g_barbar_interval_sensor_update, sensor, NULL);
}
