#include "barbar-temperature.h"
#include <stdio.h>

/**
 * BarBarTemperature:
 *
 * A temperature sensor
 *
 */
struct _BarBarTemperature {
  BarBarIntervalSensor parent_instance;

  double critical;

  double temp;

  char *path;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_TEMPERATURE,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

#define DEFAULT_INTERVAL 10000
#define DEFAULT_DEVICE "/sys/class/thermal/thermal_zone0/temp"

G_DEFINE_TYPE(BarBarTemperature, g_barbar_temperature,
              BARBAR_TYPE_INTERVAL_SENSOR)

static GParamSpec *temperature_props[NUM_PROPERTIES] = {
    NULL,
};

static guint temperature_signals[NUM_SIGNALS];

static gboolean g_barbar_temperature_tick(BarBarIntervalSensor *sensor);

void g_barbar_temperature_set_path(BarBarTemperature *temperature,
                                   const char *path) {
  g_return_if_fail(BARBAR_IS_TEMPERATURE(temperature));

  g_free(temperature->path);
  temperature->path = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(temperature),
                           temperature_props[PROP_DEVICE]);
}

static void g_barbar_temperature_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarTemperature *temperature = BARBAR_TEMPERATURE(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_temperature_set_path(temperature, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_temperature_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {

  BarBarTemperature *temperature = BARBAR_TEMPERATURE(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_value_set_string(value, temperature->path);
    break;
  case PROP_TEMPERATURE:
    g_value_set_double(value, temperature->temp);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_temperature_class_init(BarBarTemperatureClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  interval_class->tick = g_barbar_temperature_tick;

  gobject_class->set_property = g_barbar_temperature_set_property;
  gobject_class->get_property = g_barbar_temperature_get_property;

  /**
   * BarBarTemperature:path:
   *
   * Path to the temperature sensor
   */
  temperature_props[PROP_DEVICE] =
      g_param_spec_string("path", NULL, NULL, DEFAULT_DEVICE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarTemperature:temperature:
   *
   * The current temperature value in celcius.
   */
  temperature_props[PROP_TEMPERATURE] = g_param_spec_double(
      "temperature", "Temperature", "Current temperature on device", -500, 500,
      0.0, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    temperature_props);
  /**
   * BarBarTemperature::tick:
   * @sensor: This sensor
   *
   * Emit that temperature has updated
   */
  temperature_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_TEMPERATURE,                /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_temperature_init(BarBarTemperature *self) {
  self->critical = 80.0;
}

char *g_barbar_temperature_format(BarBarTemperature *self, const char *format,
                                  ...) {
  gchar *buffer;
  va_list args;

  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);

  return buffer;
}

static double read_temp(const char *str) {
  return strtol(str, NULL, 10) / 1000.0;
}

static gboolean g_barbar_temperature_tick(BarBarIntervalSensor *sensor) {
  BarBarTemperature *temperature = BARBAR_TEMPERATURE(sensor);
  GError *error = NULL;
  gboolean result;
  char *buf;
  gsize length;

  if (!temperature->path) {
    return FALSE;
  }
  result = g_file_get_contents(temperature->path, &buf, &length, &error);
  if (!result || error != NULL) {
    g_printerr("Temperature: Couldn't read censor file: %s", error->message);
    return FALSE;
  }

  temperature->temp = read_temp(buf);

  g_object_notify_by_pspec(G_OBJECT(temperature),
                           temperature_props[PROP_TEMPERATURE]);
  g_signal_emit(temperature, temperature_signals[TICK], 0);
  return TRUE;
}

/**
 * g_barbar_temperature_new:
 *
 * Returns: (transfer full): a `BarBarTemperature`
 */
BarBarSensor *g_barbar_temperature_new(const char *device) {
  BarBarTemperature *temp;

  temp = g_object_new(BARBAR_TYPE_TEMPERATURE, "device", device, NULL);

  return BARBAR_SENSOR(temp);
}
