#include "barbar-clock.h"
#include <stdio.h>

struct _BarBarClock {
  GObject parent;

  GtkLabel *label;
  char *format;

  GTimeZone *timezone;
};

enum {
  PROP_0,

  PROP_TZ,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarClock, g_barbar_clock, GTK_TYPE_WIDGET)

static GParamSpec *clock_props[NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_clock_set_tz(BarBarClock *clock, const char *identifier) {
  g_return_if_fail(BARBAR_IS_CLOCK(clock));

  printf("id: %s\n", identifier);
  if (clock->timezone) {
    g_time_zone_unref(clock->timezone);
  }
  clock->timezone = g_time_zone_new_identifier(identifier);

  g_object_notify_by_pspec(G_OBJECT(clock), clock_props[PROP_TZ]);
}

static void g_barbar_clock_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarClock *clock = BARBAR_CLOCK(object);

  switch (property_id) {
  case PROP_TZ:
    g_barbar_clock_set_tz(clock, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_clock_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {}

static void g_barbar_clock_class_init(BarBarClockClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_clock_set_property;
  gobject_class->get_property = g_barbar_clock_get_property;
  clock_props[PROP_TZ] =
      g_param_spec_string("tz", NULL, NULL, NULL, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, clock_props);
}

static void g_barbar_clock_init(BarBarClock *self) {
  self->label = g_object_new(GTK_TYPE_LABEL, NULL);
}

static gboolean g_barbar_clock_udate(gpointer data) {
  BarBarClock *clock = BARBAR_CLOCK(data);
  GDateTime *time;
  if (clock->timezone) {
    time = g_date_time_new_now(clock->timezone);
  } else {
    time = g_date_time_new_now_local();
  }

  char *str = g_date_time_format(time, "%F %k:%M:%S");
  // g_print("%s\n", str);
  gtk_label_set_text(clock->label, str);

  g_date_time_unref(time);
  return G_SOURCE_CONTINUE;
}

void g_barbar_clock_start(BarBarClock *clock) {
  g_timeout_add_full(0, 1000, g_barbar_clock_udate, clock, NULL);
}
