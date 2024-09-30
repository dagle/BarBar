#include "barbar-tick.h"
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdio.h>

/**
 * BarBarTick:
 *
 * Emits a signal at every interval, useful for when you want
 * run a command every x seconds.
 */
struct _BarBarTick {
  BarBarSensor parent;

  char *value;

  guint interval;
  guint source_id;
};

enum {
  PROP_0,

  PROP_VALUE,
  PROP_INTERVAL,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

G_DEFINE_TYPE(BarBarTick, g_barbar_tick, BARBAR_TYPE_SENSOR)

static GParamSpec *tick_props[NUM_PROPERTIES] = {
    NULL,
};

static guint tick_signal;

void g_barbar_tick_set_value(BarBarTick *self, char *value) {
  g_return_if_fail(BARBAR_IS_TICK(self));

  g_free(self->value);
  self->value = value;

  g_object_notify_by_pspec(G_OBJECT(self), tick_props[PROP_VALUE]);
}

void g_barbar_tick_set_interval(BarBarTick *self, uint interval) {
  g_return_if_fail(BARBAR_IS_TICK(self));

  self->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(self), tick_props[PROP_INTERVAL]);
}

static void g_barbar_tick_set_property(GObject *object, guint property_id,
                                       const GValue *value, GParamSpec *pspec) {

  BarBarTick *tick = BARBAR_TICK(object);

  switch (property_id) {
  case PROP_INTERVAL:
    g_barbar_tick_set_interval(tick, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_tick_get_property(GObject *object, guint property_id,
                                       GValue *value, GParamSpec *pspec) {
  BarBarTick *tick = BARBAR_TICK(object);

  switch (property_id) {
  case PROP_INTERVAL:
    g_value_set_uint(value, tick->interval);
    break;
  case PROP_VALUE:
    g_value_set_string(value, tick->value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_tick_start(BarBarSensor *sensor);

static void g_barbar_tick_class_init(BarBarTickClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_tick_set_property;
  gobject_class->get_property = g_barbar_tick_get_property;
  sensor_class->start = g_barbar_tick_start;

  /**
   * BarBarTick:value:
   *
   * Return value from the command
   */
  tick_props[PROP_VALUE] =
      g_param_spec_string("value", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
   * BarBarTick:interval:
   *
   * How often the command should be executed, in ms.
   */
  tick_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, tick_props);

  /**
   * BarBarClock::tick:
   * @sensor: This sensor
   *
   * Emit that the clock has ticked. This means that we want to refetch
   * the clock.
   */
  tick_signal =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_TICK,                       /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_STRING,                          /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_tick_init(BarBarTick *self) {}

static gboolean g_barbar_tick_update(gpointer data) {
  BarBarTick *tick = BARBAR_TICK(data);

  char *ret;
  g_signal_emit(tick, tick_signal, 0, &ret);
  g_barbar_tick_set_value(tick, ret);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_tick_start(BarBarSensor *sensor) {
  BarBarTick *tick = BARBAR_TICK(sensor);
  if (tick->source_id > 0) {
    g_source_remove(tick->source_id);
  }

  char *ret;
  g_signal_emit(tick, tick_signal, 0, &ret);
  g_barbar_tick_set_value(tick, ret);

  tick->source_id =
      g_timeout_add_full(0, tick->interval, g_barbar_tick_update, tick, NULL);
}

BarBarSensor *g_barbar_tick_new(void) {
  BarBarTick *tick;

  tick = g_object_new(BARBAR_TYPE_TICK, NULL);

  return BARBAR_SENSOR(tick);
}
