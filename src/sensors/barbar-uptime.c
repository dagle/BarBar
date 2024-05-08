#include "barbar-uptime.h"
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

#define DEFAULT_INTERVAL 60000
#define DEFAULT_FORMAT "%F %k:%M:%S"

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

// static void g_barbar_get_time(BarBarUptime *uptime, GValue *value) {
//   g_return_if_fail(BARBAR_IS_UPTIME(uptime));
//
//   if (!uptime->time) {
//     return;
//   }
//
//   char *str = g_date_time_format(uptime->time, uptime->format);
//
//   g_value_set_string(value, str);
//
//   g_free(str);
// }

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
  // BarBarClock *self = BARBAR_CLOCK(object);

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
      "format", "Format", "date time format string", DEFAULT_FORMAT,
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
   * Emit that the uptime has ticked. This means that we want to refetch
   * the uptime clock.
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

/**
 * g_barbar_format_time_span
 * @span: time spane
 * @format: a utf8 formated string
 *
 * Creates a formated string describing a time span.
 * Returns: (transfer full): a new string
 */
char *g_barbar_format_time_span(GTimeSpan span, const char *format) {
  guint len;
  GString *outstr = g_string_sized_new(strlen(format) * 2);
  int c;

  // convert to seconds, we don't care about miliseconds
  span = span / 1000;
  uint minutes = (span / 60) % 60;
  uint hours = (span / (60 * 60)) % 24;
  uint month_days = (span / (24 * 60 * 60)) % 30;
  uint days = (span / (24 * 60 * 60)) % 365;
  uint months = (span / (30 * 24 * 3600)) % 12; // average month being 30 months
  uint years = (span / (365 * 24 * 3600));
  while (*format) {

    len = strcspn(format, "%");
    if (len) {
      g_string_append_len(outstr, format, len);
    }
    format += len;
    if (!*format)
      break;

    g_assert(*format == '%');

    format++;
    if (!format)
      break;

    c = g_utf8_get_char(format);

    // this makes %h print 61 min instead of 1 min (because it would be 1 hour
    // and 1 min) Upper case only prints if the value isn't 0 gboolean absolute;
    switch (c) {
    case 'y':
      g_string_printf(outstr, "%d", years);
      break;
    case 'Y':
      break;
    case 'm':
      break;
    case 'M':
      break;
    case 'd':
      g_string_printf(outstr, "%d", days);
      break;
    case 'D':
      break;
    case 'h':
      break;
    case 'H':
      break;
    case 's':
      break;
    default:
      g_string_free(outstr, TRUE);
      return NULL;
    }
  }

  return g_string_free(outstr, FALSE);
}

static gboolean g_barbar_uptime_update(gpointer data) {
  BarBarUptime *uptime = BARBAR_UPTIME(data);

  GDateTime *local = g_date_time_new_now_local();
  GTimeSpan span = g_date_time_difference(local, uptime->boot);

  g_date_time_unref(local);

  g_clear_pointer(&uptime->time, g_free);

  uptime->time = g_barbar_format_time_span(span, uptime->format);

  // g_object_notify_by_pspec(G_OBJECT(uptime), uptime_props[PROP_TIME]);
  //
  // g_signal_emit(uptime, signals[TICK], 0);

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
