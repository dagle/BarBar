#include "barbar-uptime.h"
#include "barbar-timespan.h"
#include "sensors/barbar-interval-sensor.h"
#include <glib.h>
#include <glibtop.h>
#include <glibtop/uptime.h>
#include <stdio.h>
#include <string.h>

/**
 * BarBarUptime:
 *
 * A simple uptime sensor that updates every interval
 */
struct _BarBarUptime {
  BarBarIntervalSensor parent_instance;

  char *format;

  GDateTime *boot;
  char *time;
};

#define DEFAULT_FORMAT "%D%( days, )%h:%m" // kinda like the uptime tool works

enum {
  PROP_0,

  PROP_FORMAT,
  PROP_TIME,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarUptime, g_barbar_uptime, BARBAR_TYPE_INTERVAL_SENSOR)

static gboolean g_barbar_uptime_tick(BarBarIntervalSensor *sensor);

static GParamSpec *uptime_props[NUM_PROPERTIES] = {
    NULL,
};

static guint signals[NUM_SIGNALS];

static void g_barbar_uptime_set_format(BarBarUptime *uptime,
                                       const char *format) {
  g_return_if_fail(BARBAR_IS_UPTIME(uptime));

  if (!g_strcmp0(uptime->format, format)) {
    return;
  }

  g_free(uptime->format);
  uptime->format = g_strdup(format);

  g_object_notify_by_pspec(G_OBJECT(uptime), uptime_props[PROP_FORMAT]);
}

static void g_barbar_uptime_set_property(GObject *object, guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec) {
  BarBarUptime *uptime = BARBAR_UPTIME(object);

  switch (property_id) {
  case PROP_FORMAT:
    g_barbar_uptime_set_format(uptime, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_uptime_get_property(GObject *object, guint property_id,
                                         GValue *value, GParamSpec *pspec) {

  BarBarUptime *uptime = BARBAR_UPTIME(object);
  switch (property_id) {
  case PROP_FORMAT:
    g_value_set_string(value, uptime->format);
    break;
  case PROP_TIME:
    g_value_set_string(value, uptime->time);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

void g_barbar_uptime_dispose(GObject *object) {
  BarBarUptime *uptime = BARBAR_UPTIME(object);

  g_clear_pointer(&uptime->boot, g_date_time_unref);
  G_OBJECT_CLASS(g_barbar_uptime_parent_class)->dispose(object);
}

static void g_barbar_uptime_class_init(BarBarUptimeClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  interval_class->tick = g_barbar_uptime_tick;

  gobject_class->set_property = g_barbar_uptime_set_property;
  gobject_class->get_property = g_barbar_uptime_get_property;

  gobject_class->dispose = g_barbar_uptime_dispose;

  /**
   * BarBarUptime:format:
   *
   * The format the uptime should use, uses g_date_time_format.
   */
  uptime_props[PROP_FORMAT] = g_param_spec_string(
      "format", "Format", "uptime format string", DEFAULT_FORMAT,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarUptime:time:
   *
   * Formated string of the uptime
   */
  uptime_props[PROP_TIME] =
      g_param_spec_string("time", NULL, NULL, NULL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    uptime_props);

  /**
   * BarBarUptime::tick:
   * @sensor: This sensor
   *
   * Emit that the uptime has ticked.
   */
  signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_UPTIME,                     /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_uptime_init(BarBarUptime *uptime) {
  glibtop_uptime buf;
  uptime->format = g_strdup(DEFAULT_FORMAT);
  glibtop_init();

  glibtop_get_uptime(&buf);

  uptime->boot = g_date_time_new_from_unix_local(buf.boot_time);
}

static gboolean g_barbar_uptime_tick(BarBarIntervalSensor *sensor) {
  BarBarUptime *uptime = BARBAR_UPTIME(sensor);

  GDateTime *local = g_date_time_new_now_local();
  GTimeSpan span = g_date_time_difference(local, uptime->boot);

  g_clear_pointer(&uptime->time, g_free);

  uptime->time = g_barbar_format_time_span(span, uptime->format);

  g_object_notify_by_pspec(G_OBJECT(uptime), uptime_props[PROP_TIME]);

  g_signal_emit(uptime, signals[TICK], 0);
  g_date_time_unref(local);

  return G_SOURCE_CONTINUE;
}
