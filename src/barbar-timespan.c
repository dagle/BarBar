#include "barbar-timespan.h"
#include <ctype.h>
#include <stdio.h>

#define USEC_PER_SECOND (G_GINT64_CONSTANT(1000000))

struct units {
  gint64 seconds;
  gint64 minutes;
  gint64 hours;
  gint64 days;
  gint64 weeks;
  gint64 months;
  gint64 years;
};

const char *pad_len(const char *str, uint *num) {
  uint number = 0;

  while (*str && isdigit(*str)) {
    number = number * 10 + (*str - '0');
    str++;
  }
  *num = number;
  return str;
}

static const char *skip_modifier(const char *format) {
  int c;
  while (format) {
    c = g_utf8_get_char(format);
    switch (c) {
    case '0': {
      while (*format && isdigit(*format)) {
        format++;
      }
      break;
    }
    case 'c':
    case 'p':
    case 'a':
      break;
    default:
      return format;
    }
    format++;
  }
  return format;
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

    format = skip_modifier(format);

    c = g_utf8_get_char(format);

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

  if (years) {
    // 365 * 24 * 60 * 60
    units->years = (span / (31536000));
    span -= units->years * (31536000);
  }

  if (months) {
    // 30 * 24 * 60 * 60
    units->months = span / (2592000);
    span -= units->months * (2592000);
  }

  if (weeks) {
    // 7 * 24 * 60 * 60
    units->weeks = (span / (604800));
    span -= units->weeks * (604800);
  }

  if (days) {
    // 24 * 60 * 60
    units->days = (span / (86400));
    span -= units->days * (86400);
  }

  if (hours) {
    // 60 * 60
    units->hours = (span / (3600));
    span -= units->hours * (3600);
  }

  if (minutes) {
    units->minutes = (span / 60);
    span -= units->minutes * 60;
  }

  units->seconds = span;
}

static void format_number(GString *str, const gchar pad, gint width,
                          guint32 number) {

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
}

#define cond(var)                                                              \
  if (conditional && var <= 0) {                                               \
    last_success = FALSE;                                                      \
    continue;                                                                  \
  }                                                                            \
  format_number(outstr, padchar, pad, var);                                    \
  last_success = TRUE;

/**
 * g_barbar_format_time_span
 * @span: time spane
 * @format: a utf8 formated string
 *
 * Creates a formated string describing a time span.
 * Tries mimic the flags of strftime, %y is years,
 * %b is months, %w weeks, %d days, %h hours, %m is minutes,
 * and %s is seconds. Values are reduced from the higher once,
 * so say that you print years and days, then days will reset every
 * 365 days (leap days are not handled).
 *
 * Additionally it supports a %c modifier, putting it in front of a value
 * will make it only print if it's not 0. You can then combine that with a
 * %() that will only print it's content if the condition from a %c was printed.
 * Example: %cd%(days ) will either print something like "5days " or an empty
 * string.
 *
 * You can also pad strings by putting a flag in front an the size of padding
 *
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

    gboolean conditional = FALSE;
    uint pad = 0;
    char padchar = '0';

  modifier:
    c = g_utf8_get_char(format);
    format = g_utf8_next_char(format);

    switch (c) {
    case '(': {
      const char *end = strchr(format, ')');
      if (!end) {
        g_printerr("( and ) are not matching in time format string.");
        g_string_free(outstr, TRUE);
        return NULL;
      }
      guint paran_len = end - format;
      if (last_success) {
        g_string_append_len(outstr, format, paran_len);
      }
      format = format + paran_len + 1;
      break;
    }
    case 'c':
      conditional = TRUE;
      goto modifier;
      break;
    case '0': {
      const char *end;
      padchar = '0';
      end = pad_len(format, &pad);
      if (end == format) {
        g_printerr("No number after pad symbol\n");
        g_string_free(outstr, TRUE);
        return NULL;
      }
      format = end;
      goto modifier;
      break;
    }
    case 'y':
      cond(units.years);
      break;
    case 'm':
      cond(units.minutes);
      break;
    case 'b':
      cond(units.months);
      break;
    case 'w':
      cond(units.weeks);
      break;
    case 'd':
      cond(units.days);
      break;
    case 'h':
      cond(units.hours);
      break;
    case 's':
      cond(units.seconds);
      break;
    default:
      g_string_free(outstr, TRUE);
      return NULL;
    }
  }

  return g_string_free_and_steal(outstr);
}
