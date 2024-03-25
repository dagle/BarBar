#include "barbar-weather.h"
#include "glibconfig.h"
#include <libgeoclue-2.0/geoclue.h>
#include <libgweather/gweather.h>
#include <math.h>
#include <stdio.h>

/**
 * BarBarWeather:
 *
 * A weather sensor for barbar. It requires you to write desktop, if you don't
 * use barbar executable. See TODO:
 */

struct _BarBarWeather {
  GtkWidget parent;

  GWeatherLocation *location;
  GtkWidget *label;
  char *desktop_id;

  double temperature;

  guint source_id;
  guint interval;
};

enum {
  PROP_0,

  PROP_DESKTOP_ID,
  PROP_LOCATION,
  PROP_TEMPERATURE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarWeather, g_barbar_weather, GTK_TYPE_WIDGET)
// G_IMPLEMENT_INTERFACE

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

void g_barbar_weather_set_desktop_id(BarBarWeather *weather, const char *id) {
  g_return_if_fail(BARBAR_IS_WEATHER(weather));

  if (!id) {
    return;
  }

  g_free(weather->desktop_id);

  weather->desktop_id = strdup(id);
}

static void g_barbar_weather_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  BarBarWeather *weather = BARBAR_WEATHER(object);

  switch (property_id) {
  case PROP_LOCATION:
    g_barbar_weather_set_path(weather, g_value_get_string(value));
    break;
  case PROP_DESKTOP_ID:
    g_barbar_weather_set_desktop_id(weather, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_weather_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  BarBarWeather *weather = BARBAR_WEATHER(object);

  switch (property_id) {
  case PROP_LOCATION:
    g_value_set_string(value, weather->location);
    break;
  case PROP_TEMPERATURE:
    // g_barbar_weather_set_path(weather, g_value_get_string(value));
    g_value_set_double(value, weather->temperature);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_weather_class_init(BarBarWeatherClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_weather_set_property;
  gobject_class->get_property = g_barbar_weather_get_property;
  // gobject_class->constructed = g_barbar_weather_constructed;

  /**
   * BarBarWeather:desktop-id:
   *
   * Id of the desktop application using this service
   *
   */
  weather_props[PROP_DESKTOP_ID] =
      g_param_spec_string("desktop-id", NULL, NULL, "",
                          G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarWeather:location:
   *
   * Our location
   *
   */
  weather_props[PROP_LOCATION] = g_param_spec_string(
      "location", NULL, NULL, "",
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarWeather:temperature:
   *
   * The current temperature
   *
   */
  weather_props[PROP_TEMPERATURE] =
      g_param_spec_double("temperature", NULL, NULL, G_MINDOUBLE, G_MAXDOUBLE,
                          0.0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    weather_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "weather");
}

static void g_barbar_weather_init(BarBarWeather *weather) {

  weather->label = gtk_label_new("");
  gtk_widget_set_parent(weather->label, GTK_WIDGET(weather));
  // weather->location =
  // detect_nearest_city_finish
}

static void g_barbar_weather_constructed(GObject *self) {
  BarBarWeather *weather = BARBAR_WEATHER(self);
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

void geo_callback(GObject *source_object, GAsyncResult *res, gpointer data) {
  // GError *error = NULL;
  // printf("apa!\n");
  // GClueSimple *clue = gclue_simple_new_finish(res, &error);

  // GClueLocation *location = gclue_simple_get_location(clue);
  // double lat = gclue_location_get_latitude(location);
  // printf("latitude: %f\n", lat);
}

void updated(GWeatherInfo *self, gpointer user_data) {
  char *str = gweather_info_get_temp(self);
  printf("location: %s\n", str);
}

// 57.716667 11.966667
void g_barbar_weather_start(BarBarWeather *weather) {
  // GClueSimple *clue =
  //     gclue_simple_new_sync("barbar", GCLUE_ACCURACY_LEVEL_CITY, NULL, NULL);

  // printf("%p\n", clue);
  //
  // GClueLocation *location = gclue_simple_get_location(clue);
  // double lat = gclue_location_get_latitude(location);
  // printf("latitude: %f\n", lat);

  // GWeatherLocation *location =
  //     gweather_location_new_detached("Göteborg", NULL, 57.716667, 11.966667);

  // GWeatherInfo *info = gweather_info_new(location);
  // gclue_simple_new("barbar", GCLUE_ACCURACY_LEVEL_CITY, NULL, geo_callback,
  //                  NULL);

  // gweather_info_set_contact_info(info, "com.github.barbar");
  // g_signal_connect(G_OBJECT(info), "updated", G_CALLBACK(updated), NULL);
  // gweather_info_update(info);

  // char *str = gweather_location_get_city_name(location);
  // if (weather->source_id > 0) {
  //   g_source_remove(weather->source_id);
  // }
  // g_barbar_weather_update(weather);
  // weather->source_id = g_timeout_add_full(
  //     0, weather->interval, g_barbar_weather_update, weather, NULL);
}
