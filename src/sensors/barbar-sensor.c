#include "barbar-sensor.h"
#include <stdio.h>

G_DEFINE_ABSTRACT_TYPE(BarBarSensor, g_barbar_sensor, G_TYPE_OBJECT)

static void g_barbar_sensor_class_init(BarBarSensorClass *class) {
  class->start = NULL;
}

static void g_barbar_sensor_init(BarBarSensor *sensor) {}

void g_barbar_sensor_start(BarBarSensor *self) {
  g_return_if_fail(BARBAR_IS_SENSOR(self));

  BARBAR_SENSOR_GET_CLASS(self)->start(self);
}
