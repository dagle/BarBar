#include "barbar-clock.h"
#include "barbar-sensor.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

/**
 * BarBarClock:
 *
 * A simple clock sensor that updates every interval
 */
struct _BarBarClock {
  BarBarSensor parent_instance;

  char *format;
  guint interval;

  GTimeZone *timezone;
  GDateTime *time;

  guint source_id;
};

#define DEFAULT_INTERVAL 1000
#define DEFAULT_FORMAT "%F %k:%M:%S"

enum {
  PROP_0,

  PROP_TZ,
  PROP_FORMAT,
  PROP_INTERVAL,
  PROP_TIME,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarClock, g_barbar_clock, BARBAR_TYPE_SENSOR)

static void g_barbar_clock_start(BarBarSensor *sensor);

static GParamSpec *clock_props[NUM_PROPERTIES] = {
    NULL,
};

static guint clock_signals[NUM_SIGNALS];

static void g_barbar_clock_set_tz(BarBarClock *clock, const char *identifier) {
  g_return_if_fail(BARBAR_IS_CLOCK(clock));

  if (clock->timezone) {
    g_time_zone_unref(clock->timezone);
  }
  clock->timezone = g_time_zone_new_identifier(identifier);

  g_object_notify_by_pspec(G_OBJECT(clock), clock_props[PROP_TZ]);
}

static void g_barbar_clock_set_format(BarBarClock *clock, const char *format) {
  g_return_if_fail(BARBAR_IS_CLOCK(clock));

  if (g_set_str(&clock->format, format)) {
    g_object_notify_by_pspec(G_OBJECT(clock), clock_props[PROP_FORMAT]);
  }
}

static void g_barbar_clock_set_interval(BarBarClock *clock, guint interval) {
  g_return_if_fail(BARBAR_IS_CLOCK(clock));

  clock->interval = interval;
  // restart the clock if started. This stops the old clock
  if (clock->source_id > 0) {
    // g_barbar_clock_start(clock);
  }

  g_object_notify_by_pspec(G_OBJECT(clock), clock_props[PROP_INTERVAL]);
}

static void g_barbar_get_time(BarBarClock *clock, GValue *value) {
  g_return_if_fail(BARBAR_IS_CLOCK(clock));

  if (!clock->time) {
    return;
  }

  char *str = g_date_time_format(clock->time, clock->format);

  g_value_set_string(value, str);

  g_free(str);
}

static void g_barbar_get_timezone(BarBarClock *clock, GValue *value) {
  g_return_if_fail(BARBAR_IS_CLOCK(clock));
  g_return_if_fail(clock->timezone);

  const char *tz = g_time_zone_get_identifier(clock->timezone);

  g_value_set_string(value, tz);
}

static void g_barbar_clock_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarClock *clock = BARBAR_CLOCK(object);

  switch (property_id) {
  case PROP_TZ:
    g_barbar_clock_set_tz(clock, g_value_get_string(value));
    break;
  case PROP_FORMAT:
    g_barbar_clock_set_format(clock, g_value_get_string(value));
    break;
  case PROP_INTERVAL:
    g_barbar_clock_set_interval(clock, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_clock_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {

  BarBarClock *clock = BARBAR_CLOCK(object);
  switch (property_id) {
  case PROP_TZ:
    g_barbar_get_timezone(clock, value);
    break;
  case PROP_FORMAT:
    g_value_set_string(value, clock->format);
    break;
  case PROP_INTERVAL:
    g_value_set_uint(value, clock->interval);
    break;
  case PROP_TIME:
    g_barbar_get_time(clock, value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

void g_barbar_clock_dispose(GObject *object) {
  // BarBarClock *self = BARBAR_CLOCK(object);

  G_OBJECT_CLASS(g_barbar_clock_parent_class)->dispose(object);
}

static void g_barbar_clock_class_init(BarBarClockClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_clock_start;

  gobject_class->set_property = g_barbar_clock_set_property;
  gobject_class->get_property = g_barbar_clock_get_property;

  gobject_class->dispose = g_barbar_clock_dispose;

  /**
   * BarBarClock:tz:
   *
   * Time Zone the clock should be located in. Uses local time by default.
   */
  clock_props[PROP_TZ] =
      g_param_spec_string("tz", "Time Zone", "Time zone", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarClock:format:
   *
   * The format the clock should use, uses g_date_time_format.
   */
  clock_props[PROP_FORMAT] = g_param_spec_string(
      "format", "Format", "date time format string", DEFAULT_FORMAT,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarClock:time:
   *
   * Formated string of the time
   */
  clock_props[PROP_TIME] =
      g_param_spec_string("time", NULL, NULL, NULL, G_PARAM_READABLE);
  /**
   * BarBarClock:interval:
   *
   * How often the clock should be updated
   */
  clock_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, clock_props);

  /**
   * BarBarClock::tick:
   * @sensor: This sensor
   *
   * Emit that the clock has ticked. This means that we want to refetch
   * the clock.
   */
  clock_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_CLOCK,                      /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static gboolean g_barbar_clock_update(gpointer data);

static void g_barbar_clock_init(BarBarClock *clock) {
  clock->format = g_strdup(DEFAULT_FORMAT);
  clock->interval = DEFAULT_INTERVAL;
}

static gboolean g_barbar_clock_update(gpointer data) {
  BarBarClock *clock = BARBAR_CLOCK(data);

  g_clear_pointer(&clock->time, g_date_time_unref);

  if (clock->timezone) {
    clock->time = g_date_time_new_now(clock->timezone);
  } else {
    clock->time = g_date_time_new_now_local();
  }

  g_object_notify_by_pspec(G_OBJECT(clock), clock_props[PROP_TIME]);

  g_signal_emit(clock, clock_signals[TICK], 0);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_clock_start(BarBarSensor *sensor) {
  BarBarClock *clock = BARBAR_CLOCK(sensor);

  if (clock->source_id > 0) {
    g_source_remove(clock->source_id);
  }
  g_barbar_clock_update(clock);
  clock->source_id = g_timeout_add_full(0, clock->interval,
                                        g_barbar_clock_update, clock, NULL);
}

BarBarSensor *g_barbar_clock_new(const char *format) {
  BarBarClock *clock;

  clock = g_object_new(BARBAR_TYPE_CLOCK, "format", format, NULL);

  return BARBAR_SENSOR(clock);
}
