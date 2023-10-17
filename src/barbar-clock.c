#include "barbar-clock.h"
#include <stdio.h>

struct _BarBarClock {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  GTimeZone *timezone;
};

enum {
  PROP_0,

  PROP_TZ,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarClock, g_barbar_clock, G_TYPE_OBJECT)

static GParamSpec *clock_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_clock_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
}

static void g_barbar_clock_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
}

static void g_barbar_clock_class_init(BarBarClockClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_clock_set_property;
  gobject_class->get_property = g_barbar_clock_get_property;
  clock_props[PROP_TZ] = g_param_spec_string(
      "path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, clock_props);
}

static void g_barbar_clock_init(BarBarClock *self) {
}

void g_barbar_clock_update(BarBarClock *clock) {
	GDateTime *time;
	if (clock->timezone) {
		time = g_date_time_new_now(clock->timezone);
	} else {
		time = g_date_time_new_now_local();
	}


	char *str = g_date_time_format (time, "%s");
	printf("%s\n", str);

	g_date_time_unref(time);
}
