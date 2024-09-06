#include "barbar-weather.h"
#include <libgeoclue-2.0/geoclue.h>
#include <libgweather/gweather.h>
#include <math.h>
#include <stdio.h>

/**
 * BarBarWeather:
 *
 * A weather sensor for barbar.
 * By default it also acts as a weather
 *
 * It requires you to write desktop, if you don't
 * use barbar executable.
 */

struct _BarBarWeather {
  BarBarSensor parent_instance;

  GClueSimple *clue;
  GWeatherLocation *location;
  GWeatherInfo *info;
  gboolean metar;
  char *contact_info;

  double temperature;

  guint source_id;
  guint interval;

  gboolean agent;
};

enum {
  PROP_0,

  PROP_AGENT,

  PROP_LOCATION,
  PROP_METAR,
  PROP_CONTACT_INFO,
  PROP_TEMPERATURE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarWeather, g_barbar_weather, BARBAR_TYPE_SENSOR)

static GParamSpec *weather_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_weather_constructed(GObject *self);
static void g_barbar_weather_start(BarBarSensor *sensor);

static void g_barbar_weather_set_metar(BarBarWeather *self, gboolean metar) {
  g_return_if_fail(BARBAR_IS_WEATHER(self));

  if (self->metar == metar) {
    return;
  }

  self->metar = metar;
}

static void g_barbar_weather_set_agent(BarBarWeather *self, gboolean agent) {
  g_return_if_fail(BARBAR_IS_WEATHER(self));

  if (self->agent == agent) {
    return;
  }

  self->metar = agent;
}

static void g_barbar_weather_set_contact_info(BarBarWeather *self,
                                              const gchar *contact_info) {
  g_return_if_fail(BARBAR_IS_WEATHER(self));

  if (g_set_str(&self->contact_info, contact_info)) {
    g_object_notify_by_pspec(G_OBJECT(self), weather_props[PROP_CONTACT_INFO]);
  }
}

static void g_barbar_weather_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  BarBarWeather *weather = BARBAR_WEATHER(object);

  switch (property_id) {
  case PROP_METAR:
    g_barbar_weather_set_metar(weather, g_value_get_boolean(value));
    break;
  case PROP_AGENT:
    g_barbar_weather_set_agent(weather, g_value_get_boolean(value));
    break;
  case PROP_CONTACT_INFO:
    g_barbar_weather_set_contact_info(weather, g_value_get_string(value));
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
    g_value_set_string(value, "here");
    break;
  case PROP_METAR:
    g_value_set_boolean(value, weather->metar);
    break;
  case PROP_AGENT:
    g_value_set_boolean(value, weather->agent);
    break;
  case PROP_TEMPERATURE:
    g_value_set_double(value, weather->temperature);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_weather_class_init(BarBarWeatherClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_weather_set_property;
  gobject_class->get_property = g_barbar_weather_get_property;
  sensor_class->start = g_barbar_weather_start;

  /**
   * BarBarWeather:contact-info:
   *
   * Your contact info, this is required for most providers, to inform
   * missbehaving clients.
   *
   */
  weather_props[PROP_CONTACT_INFO] = g_param_spec_string(
      "contact-info", NULL, NULL, "per.odlund@gmail.com",
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarWeather:metar:
   *
   * Should we try to find the nearest airport for our location
   *
   */
  weather_props[PROP_METAR] = g_param_spec_boolean(
      "metar", NULL, NULL, TRUE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarWeather:agent:
   *
   * If this sensor should act as a geoclue agent
   *
   */
  weather_props[PROP_AGENT] = g_param_spec_boolean(
      "agent", NULL, NULL, TRUE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarWeather:location:
   *
   * Our location
   *
   */
  weather_props[PROP_LOCATION] = g_param_spec_string(
      "location", NULL, NULL, NULL, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarWeather:temperature:
   *
   * The current temperature
   *
   */
  weather_props[PROP_TEMPERATURE] =
      g_param_spec_double("temperature", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE,
                          0.0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    weather_props);
}

static void g_barbar_weather_init(BarBarWeather *weather) {}

static void g_barbar_weather_constructed(GObject *object) {
  BarBarWeather *self = BARBAR_WEATHER(object);

  G_OBJECT_CLASS(g_barbar_weather_parent_class)->constructed(object);
}

// TODO: Farenheit etc
static double read_temp(const char *str) {
  return strtol(str, NULL, 10) / 1000.0;
}

static void print_info(GWeatherInfo *info, gpointer data) {
  GWeatherSky sky;
  gboolean r = gweather_info_get_value_sky(info, &sky);
  double temp;

  gweather_info_get_value_temp(info, GWEATHER_TEMP_UNIT_CENTIGRADE, &temp);
  const gchar *s = gweather_sky_to_string(sky);
  char *temp2 = gweather_info_get_temp(info);

  printf("sky: %d, %s\n", r, s);
  printf("temp: %f\n", temp);
  printf("temp2: %s\n", temp2);
}

GWeatherLocation *find_station(GWeatherLocation *location) {
  GWeatherLocation *child = NULL;
  while ((child = gweather_location_next_child(location, child)) != NULL) {
    if (gweather_location_get_level(child) ==
        GWEATHER_LOCATION_WEATHER_STATION) {
      return child;
    }
    g_object_unref(child);
  }

  return NULL;
}

static void update_location(gpointer data) {
  BarBarWeather *self = BARBAR_WEATHER(data);

  GClueLocation *location = gclue_simple_get_location(self->clue);

  double latitude = gclue_location_get_latitude(location);
  double longitude = gclue_location_get_longitude(location);

  GWeatherLocation *world = gweather_location_get_world();

  g_clear_object(&self->location);
  self->location =
      gweather_location_find_nearest_city(world, latitude, longitude);

  g_object_unref(world);

  g_clear_object(&self->info);
  self->info = gweather_info_new(self->location);
  gweather_info_set_contact_info(self->info, self->contact_info);

  g_signal_connect(self->info, "updated", G_CALLBACK(print_info), self);
  gweather_info_update(self->info);
}

void geo_callback(GObject *source_object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  BarBarWeather *self = BARBAR_WEATHER(data);
  self->clue = gclue_simple_new_finish(res, &error);

  if (error) {
    g_printerr("Couldn't find location: %s\n", error->message);
    return;
  }

  update_location(self);

  g_signal_connect_swapped(self->clue, "notify::location",
                           G_CALLBACK(update_location), self);
}

static void g_barbar_weather_start(BarBarSensor *sensor) {
  BarBarWeather *weather = BARBAR_WEATHER(sensor);

  GApplication *app = g_application_get_default();

  if (app != NULL) {

    gclue_simple_new(g_application_get_application_id(app),
                     GCLUE_ACCURACY_LEVEL_CITY, NULL, geo_callback, weather);
  }
}
