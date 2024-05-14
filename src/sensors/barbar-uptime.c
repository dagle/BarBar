#include "barbar-uptime.h"
#include "barbar-timespan.h"
#include <ctype.h>
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
  BarBarSensor parent_instance;

  char *format;
  guint interval;

  GDateTime *boot;
  char *time;

  guint source_id;
};

#define DEFAULT_INTERVAL 1000
#define DEFAULT_FORMAT "%D%( days, )%h:%m" // kinda like the uptime tool works

enum {
  PROP_0,

  PROP_FORMAT,
  PROP_INTERVAL,
  PROP_TIME,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarUptime, g_barbar_uptime, BARBAR_TYPE_SENSOR)

static void g_barbar_uptime_start(BarBarSensor *sensor);

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

static void g_barbar_uptime_set_interval(BarBarUptime *uptime, guint interval) {
  g_return_if_fail(BARBAR_IS_UPTIME(uptime));

  uptime->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(uptime), uptime_props[PROP_INTERVAL]);
}

static void g_barbar_uptime_set_property(GObject *object, guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec) {
  BarBarUptime *uptime = BARBAR_UPTIME(object);

  switch (property_id) {
  case PROP_FORMAT:
    g_barbar_uptime_set_format(uptime, g_value_get_string(value));
    break;
  case PROP_INTERVAL:
    g_barbar_uptime_set_interval(uptime, g_value_get_uint(value));
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
  case PROP_INTERVAL:
    g_value_set_uint(value, uptime->interval);
    break;
  case PROP_TIME:
    g_value_set_string(value, uptime->time);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

void g_barbar_uptime_dispose(GObject *object) {

  G_OBJECT_CLASS(g_barbar_uptime_parent_class)->dispose(object);
}

static void g_barbar_uptime_class_init(BarBarUptimeClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_uptime_start;

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
  /**
   * BarBarUptime:interval:
   *
   * How often the uptime should be updated
   */
  uptime_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

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

static gboolean g_barbar_uptime_update(gpointer data);

static void g_barbar_uptime_init(BarBarUptime *uptime) {
  uptime->format = g_strdup(DEFAULT_FORMAT);
  uptime->interval = DEFAULT_INTERVAL;
}

static gboolean g_barbar_uptime_update(gpointer data) {
  BarBarUptime *uptime = BARBAR_UPTIME(data);

  GDateTime *local = g_date_time_new_now_local();
  GTimeSpan span = g_date_time_difference(local, uptime->boot);

  g_clear_pointer(&uptime->time, g_free);

  uptime->time = g_barbar_format_time_span(span, uptime->format);

  g_object_notify_by_pspec(G_OBJECT(uptime), uptime_props[PROP_TIME]);

  g_signal_emit(uptime, signals[TICK], 0);
  g_date_time_unref(local);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_uptime_start(BarBarSensor *sensor) {
  glibtop_uptime buf;
  BarBarUptime *uptime = BARBAR_UPTIME(sensor);

  glibtop_init();

  glibtop_get_uptime(&buf);

  uptime->boot = g_date_time_new_from_unix_local(buf.boot_time);

  if (uptime->source_id > 0) {
    g_source_remove(uptime->source_id);
  }
  g_barbar_uptime_update(uptime);
  uptime->source_id = g_timeout_add_full(0, uptime->interval,
                                         g_barbar_uptime_update, uptime, NULL);
}
