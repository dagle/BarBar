#include "barbar-temperature.h"
#include <math.h>
#include <stdio.h>

struct _BarBarTemperature {
  GObject parent;

  // TODO:This should be in parent
  char *label;
  double critical;

  char *path;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_CRITICAL,
  // PROP_FORMAT_CRITICAL,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarTemperature, g_barbar_temperature, G_TYPE_OBJECT)

static GParamSpec *temperature_props[NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_temperature_set_path(BarBarTemperature *temperature, const char *path) {
  g_return_if_fail(BARBAR_IS_TEMPERATURE(temperature));

  g_free(temperature->path);
  temperature->path = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(temperature), temperature_props[PROP_DEVICE]);
}

static void g_barbar_temperature_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
  BarBarTemperature *temperature = BARBAR_TEMPERATURE(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_temperature_set_path(temperature, g_value_get_string(value));
    break;
  case PROP_CRITICAL:
	temperature->critical = g_value_get_double(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_temperature_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
}

static void g_barbar_temperature_class_init(BarBarTemperatureClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_temperature_set_property;
  gobject_class->get_property = g_barbar_temperature_get_property;
  temperature_props[PROP_DEVICE] = g_param_spec_string(
      "path", NULL, NULL, "/sys/class/thermal/thermal_zone0/temp", G_PARAM_READWRITE);
  temperature_props[PROP_CRITICAL] = g_param_spec_double(
      "critical-temp", NULL, NULL, 0.0, 300.0, 80.0, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, temperature_props);
}

static void g_barbar_temperature_init(BarBarTemperature *self) {
	self->path = strdup("/sys/class/thermal/thermal_zone0/temp");
	self->critical = 80.0;
}

// TODO: Farenheit etc
static double read_temp(const char *str, gsize length) {
	return strtol(str, NULL, 10) / 1000.0;
}

void g_barbar_temperature_update(BarBarTemperature *temperature) {
  GError *error = NULL;
  gboolean result;
  char *data;
  gsize length;
  double temp;

  if (temperature->path) {
	  result = g_file_get_contents(temperature->path, &data, &length, &error);
	  if (!result || error != NULL) { 
		  // TODO: HANDLE errors
		  return;
	  }
	  temp = read_temp(data, length);
	  free(data);
	  g_print("Temp: %f\n", round(temp));
  }
}
