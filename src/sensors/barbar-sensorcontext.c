#include <sensors/barbar-sensorcontext.h>

G_DEFINE_INTERFACE(BarBarSensorContext, barbar_sensor_context, G_TYPE_OBJECT)

static void
barbar_sensor_context_default_init(BarBarSensorContextInterface *iface) {}

void barbar_sensor_context_start(BarBarSensorContext *self, void *ptr) {
  g_return_if_fail(BARBAR_IS_SENSOR_CONTEXT(self));

  return BARBAR_SENSOR_CONTEXT_GET_IFACE(self)->start(self, ptr);
}
