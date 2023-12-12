#include "barbar-clock.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

struct _BarBarClock {
  GtkWidget parent_instance;

  GtkWidget *label;
  char *format;
  guint interval;

  GTimeZone *timezone;
  guint source_id;
};

#define DEFAULT_INTERVAL 60000
#define DEFAULT_FORMAT "%F %k:%M:%S"

enum {
  PROP_0,

  PROP_TZ,
  PROP_FORMAT,
  PROP_INTERVAL,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarClock, g_barbar_clock, GTK_TYPE_WIDGET)

static GParamSpec *clock_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_clock_constructed(GObject *self);

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

  g_free(clock->format);
  clock->format = g_strdup(format);

  g_object_notify_by_pspec(G_OBJECT(clock), clock_props[PROP_FORMAT]);
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
    // g_value_set_string(value, clock->timezone);
    break;
  case PROP_FORMAT:
    g_value_set_string(value, clock->format);
    break;
  case PROP_INTERVAL:
    g_value_set_uint(value, clock->interval);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_clock_class_init(BarBarClockClass *class) {
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_clock_set_property;
  gobject_class->get_property = g_barbar_clock_get_property;
  gobject_class->constructed = g_barbar_clock_constructed;
  clock_props[PROP_TZ] = g_param_spec_string("tz", "Time Zone", "Time zone",
                                             NULL, G_PARAM_READWRITE);
  clock_props[PROP_FORMAT] =
      g_param_spec_string("format", "Format", "date time format string",
                          DEFAULT_FORMAT, G_PARAM_READWRITE);
  clock_props[PROP_INTERVAL] =
      g_param_spec_uint("interval", "Interval", "Interval in milli seconds", 0,
                        G_MAXUINT32, DEFAULT_INTERVAL, G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, clock_props);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}

static gboolean g_barbar_clock_update(gpointer data);

static void g_barbar_clock_init(BarBarClock *clock) {}
void g_barbar_clock_start(BarBarClock *clock, gpointer data);

static void g_barbar_clock_constructed(GObject *object) {
  BarBarClock *clock = BARBAR_CLOCK(object);

  G_OBJECT_CLASS(g_barbar_clock_parent_class)->constructed(object);

  clock->format = g_strdup(DEFAULT_FORMAT);
  clock->interval = DEFAULT_INTERVAL;

  clock->label = gtk_label_new("");
  gtk_widget_set_parent(clock->label, GTK_WIDGET(clock));
  g_signal_connect(clock, "map", G_CALLBACK(g_barbar_clock_start), NULL);
}

static gboolean g_barbar_clock_update(gpointer data) {
  BarBarClock *clock = BARBAR_CLOCK(data);
  GDateTime *time;
  if (clock->timezone) {
    time = g_date_time_new_now(clock->timezone);
  } else {
    time = g_date_time_new_now_local();
  }

  char *str = g_date_time_format(time, clock->format);
  gtk_label_set_text(GTK_LABEL(clock->label), str);

  g_date_time_unref(time);
  return G_SOURCE_CONTINUE;
}

void g_barbar_clock_start(BarBarClock *clock, gpointer data) {
  if (clock->source_id > 0) {
    g_source_remove(clock->source_id);
  }
  g_barbar_clock_update(clock);
  clock->source_id = g_timeout_add_full(0, clock->interval,
                                        g_barbar_clock_update, clock, NULL);
}
