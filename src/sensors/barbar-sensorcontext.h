#pragma once

#include <glib-object.h>
#include <glib.h>

#define BARBAR_TYPE_SENSOR_CONTEXT (barbar_sensor_context_get_type())

G_DECLARE_INTERFACE(BarBarSensorContext, barbar_sensor_context, BARBAR,
                    SENSOR_CONTEXT, GObject)

struct _BarBarSensorContextInterface {
  GTypeInterface parent_iface;

  void (*start)(BarBarSensorContext *self, void *ptr);
};

void barbar_sensor_context_start(BarBarSensorContext *self, void *ptr);
