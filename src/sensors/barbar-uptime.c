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

#define DEFAULT_INTERVAL 1000
#define DEFAULT_FORMAT "%D%( days), %h:%m" // kinda like the uptime tool works

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

enum prefix {
  NO_PREFIX,
  SHORT_PREFIX,
  LONG_PREFIX,
};

#define USEC_PER_SECOND (G_GINT64_CONSTANT(1000000))
#define USEC_PER_MINUTE (G_GINT64_CONSTANT(60000000))
#define USEC_PER_HOUR (G_GINT64_CONSTANT(3600000000))
#define USEC_PER_MILLISECOND (G_GINT64_CONSTANT(1000))
#define USEC_PER_DAY (G_GINT64_CONSTANT(86400000000))
#define SEC_PER_DAY (G_GINT64_CONSTANT(86400))

#define SECS_PER_MINUTE (60)
#define SECS_PER_HOUR (60 * SECS_PER_MINUTE)
#define SECS_PER_DAY (24 * SECS_PER_HOUR)
#define SECS_PER_YEAR (365 * SECS_PER_DAY)

static void format_number(GString *str, const gchar pad, gint width,
                          const char *prefix, guint32 number) {

  const gchar ascii_digits[10] = {'0', '1', '2', '3', '4',
                                  '5', '6', '7', '8', '9'};
  gchar tmp[10];
  gint i = 0;

  g_return_if_fail(width <= 10);

  do {
    tmp[i++] = ascii_digits[number % 10];
    number /= 10;
  } while (number);

  while (pad && i < width) {
    tmp[i++] = pad;
  }

  g_assert(i <= 10);

  while (i) {
    g_string_append_c(str, tmp[--i]);
  }
  if (prefix) {
    g_string_append(str, prefix);
  }
}

static inline const char *set_prefix(enum prefix prefix, const char *long_name,
                                     const char *short_name) {
  switch (prefix) {
  case NO_PREFIX:
    return NULL;
  case SHORT_PREFIX:
    return short_name;
  case LONG_PREFIX:
    return long_name;
  }
}

struct units {
  gint64 seconds;
  gint64 minutes;
  gint64 hours;
  gint64 days;
  gint64 weeks;
  gint64 months;
  gint64 years;
};

static gsize skip_modifier(const char *format) {
  int c;
  gsize len = 0;
  while (format) {
    c = g_utf8_get_char(format);
    switch (c) {
    case 'c':
    case 'p':
    case 'a':
      len++;
    }
  }
  return len;
}

static void get_parts(struct units *units, GTimeSpan span, const char *format) {
  guint len;
  int c;

  gboolean minutes = FALSE;
  gboolean hours = FALSE;
  gboolean weeks = FALSE;
  gboolean months = FALSE;
  gboolean days = FALSE;
  gboolean years = FALSE;

  while (*format) {
    len = strcspn(format, "%");

    format += len;
    if (!*format)
      break;

    format++;

    c = g_utf8_get_char(format);

    format += skip_modifier(format);

    switch (c) {
    case 'y':
      years = TRUE;
      break;
    case 'm':
      minutes = TRUE;
      break;
    case 'b':
      months = TRUE;
      break;
    case 'w':
      weeks = TRUE;
      break;
    case 'd':
      days = TRUE;
      break;
    case 'h':
      hours = TRUE;
      break;
    }
    format++;
  }

  // TODO: optimize
  if (years) {
    units->years = (span / (365 * 24 * 3600));
    span -= units->years * (365 * 24 * 3600);
  }

  if (months) {
    units->months = span / (30 * 24 * 3600);
    span -= units->months * (30 * 24 * 3600);
  }

  if (weeks) {
    units->weeks = (span / (7 * 24 * 3600));
    span -= units->weeks * (7 * 24 * 3600);
  }

  if (days) {
    units->days = (span / (24 * 3600));
    span -= units->days * (24 * 3600);
  }

  if (hours) {
    units->hours = (span / (3600));
    span -= units->hours * (3600);
  }

  if (minutes) {
    units->minutes = (span / 60);
    span -= units->minutes * 60;
  }

  units->seconds = span;
}

// this makes %ah print 61 min instead of 1 min (because it would be 1 hour
// and 1 min) Upper case only prints if the value isn't 0 gboolean absolute;
// TODO: DOCUMENATION
/**
 * g_barbar_format_time_span
 * @span: time spane
 * @format: a utf8 formated string
 *
 * Creates a formated string describing a time span.
 * Returns: (transfer full): a new string
 */
char *g_barbar_format_time_span(GTimeSpan span, const char *format) {
  g_return_val_if_fail(format != NULL, NULL);
  gboolean last_success = TRUE;

  guint len;
  GString *outstr = g_string_sized_new(strlen(format) * 2);
  int c;

  // convert to seconds, we don't care about miliseconds
  span = span / USEC_PER_SECOND;

  struct units units;

  get_parts(&units, span, format);

  // gint64 minutes = (span / 60) % 60;
  // gint64 hours = (span / (60 * 60)) % 24;
  // gint64 month_days = (span / (24 * 60 * 60)) % 30;
  // gint64 days = (span / (24 * 60 * 60)) % 365;
  // gint64 months =
  //     (span / (30 * 24 * 3600)) % 12; // average month being 30 months
  // gint64 years = (span / (365 * 24 * 3600));
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

    enum prefix prefix = NO_PREFIX;
    gboolean absolute = FALSE;
    // gboolean conditional = FALSE;
    uint pad = 0;

  modifier:
    c = g_utf8_get_char(format);

    switch (c) {
    case '(': {
      const char *end = strchr(format, ')');
      if (!end) {
        g_printerr("( and ) are not matching in format string.");
        g_string_free(outstr, TRUE);
        return NULL;
      }
      format++;
      guint paran_len = end - format;
      if (last_success) {
        g_string_append_len(outstr, format, paran_len - 1);
      }
      format = format + paran_len;
      break;
    }
    case 'a':
      absolute = TRUE;
      format++;
      goto modifier;
      break;
    case 'p':
      prefix = SHORT_PREFIX;
      format++;
      goto modifier;
      break;
    case 'P':
      prefix = LONG_PREFIX;
      format++;
      goto modifier;
      break;
    case 'y':
      format_number(outstr, '0', 0, set_prefix(prefix, "year", "y"), years);
      last_success = TRUE;
      break;
    case 'Y':
      if (years > 0) {
        format_number(outstr, '0', 0, set_prefix(prefix, "year", "y"), years);
        last_success = TRUE;
      } else {
        last_success = FALSE;
      }
      break;
    case 'm':
      format_number(outstr, '0', 0, set_prefix(prefix, "minutes", "m"),
                    minutes);
      last_success = TRUE;
      break;
    case 'M':
      if (minutes > 0) {
        format_number(outstr, '0', 0, set_prefix(prefix, "minutes", "m"),
                      minutes);
        last_success = TRUE;
      } else {
        last_success = FALSE;
      }
      break;
    // b as in months, seems to be the norm.
    case 'b':
      break;
    case 'B':
      break;
    case 'w':
      break;
    case 'W':
      break;
    case 'd':
      format_number(outstr, '0', 0, set_prefix(prefix, "days", "d"), days);
      last_success = TRUE;
      break;
    case 'D':
      if (days > 0) {
        format_number(outstr, '0', 0, set_prefix(prefix, "days", "d"), days);
      } else {
        last_success = FALSE;
      }
      break;
    case 'h':
      format_number(outstr, '0', 0, set_prefix(prefix, "hours", "h"), hours);
      break;
    case 'H':
      if (hours > 0) {
        format_number(outstr, '0', 0, set_prefix(prefix, "hours", "h"), hours);
        last_success = TRUE;
      } else {
        printf("bug!\n");
        last_success = FALSE;
      }
      break;
    case 's':
      format_number(outstr, '0', 0, set_prefix(prefix, "seconds", "s"),
                    span % 60);
      last_success = TRUE;
      break;
    default:
      g_string_free(outstr, TRUE);
      return NULL;
    }
    format++;
  }

  return g_string_free_and_steal(outstr);
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

// #define DEFAULT_FORMAT "%F %k:%M:%S"
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
