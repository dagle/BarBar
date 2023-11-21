#include "barbar-weather.h"
#include <math.h>
#include <stdio.h>

struct _BarBarWeather {
  GtkWidget parent;

  GtkWidget *label;
  guint source_id;
  guint interval;

  char *location;
};

enum {
  PROP_0,

  PROP_LOCATION,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarWeather, g_barbar_weather, GTK_TYPE_WIDGET)

static GParamSpec *weather_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_weather_constructed(GObject *self);

void g_barbar_weather_set_path(BarBarWeather *weather, const char *path) {
  g_return_if_fail(BARBAR_IS_WEATHER(weather));

  // g_free(temperature->path);
  // temperature->path = g_strdup(path);
  //
  // g_object_notify_by_pspec(G_OBJECT(temperature),
  //                          temperature_props[PROP_DEVICE]);
}

static void g_barbar_weather_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  BarBarWeather *weather = BARBAR_WEATHER(object);

  switch (property_id) {
  case PROP_LOCATION:
    g_barbar_weather_set_path(weather, g_value_get_string(value));
    break;
  // case PROP_CRITICAL:
  //   weather->critical = g_value_get_double(value);
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_weather_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {}

static void g_barbar_weather_class_init(BarBarWeatherClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_weather_set_property;
  gobject_class->get_property = g_barbar_weather_get_property;
  gobject_class->constructed = g_barbar_weather_constructed;

  weather_props[PROP_LOCATION] =
      g_param_spec_string("location", NULL, NULL, "", G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    weather_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "weather");
}

static void g_barbar_weather_init(BarBarWeather *self) {}

static void g_barbar_weather_constructed(GObject *self) {
  BarBarWeather *weather = BARBAR_WEATHER(self);

  // temp->path = strdup("/sys/class/thermal/thermal_zone0/temp");
  // temp->critical = 80.0;
  // temp->interval = 1000;

  weather->label = gtk_label_new("");
  gtk_widget_set_parent(weather->label, GTK_WIDGET(weather));
}

// TODO: Farenheit etc
static double read_temp(const char *str) {
  return strtol(str, NULL, 10) / 1000.0;
}

static gboolean g_barbar_weather_update(gpointer data) {
  BarBarWeather *weather = BARBAR_WEATHER(data);
  GError *error = NULL;
  gboolean result;
  char *buf;
  gsize length;
  double temp;
  static char output[128];

  // if (temperature->path) {
  //   result = g_file_get_contents(temperature->path, &buf, &length, &error);
  //   if (!result || error != NULL) {
  //     // TODO: HANDLE errors
  //     return FALSE;
  //   }
  //   temp = read_temp(buf);
  //   snprintf(output, sizeof(output), "%.0f", temp);
  //   gtk_label_set_text(GTK_LABEL(temperature->label), output);
  //   free(buf);
  // }
  return TRUE;
}

void g_barbar_weather_start(BarBarWeather *temperature) {
  if (temperature->source_id > 0) {
    g_source_remove(temperature->source_id);
  }
  g_barbar_weather_update(temperature);
  temperature->source_id = g_timeout_add_full(
      0, temperature->interval, g_barbar_weather_update, temperature, NULL);
}
