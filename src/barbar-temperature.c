#include "barbar-temperature.h"
#include <math.h>
#include <stdio.h>

struct _BarBarTemperature {
  GtkWidget parent;

  GtkWidget *label;
  guint source_id;
  double critical;
  guint interval;

  char *path;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_CRITICAL,
  // PROP_FORMAT_CRITICAL,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarTemperature, g_barbar_temperature, GTK_TYPE_WIDGET)

static GParamSpec *temperature_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_temperature_constructed(GObject *self);

void g_barbar_temperature_set_path(BarBarTemperature *temperature,
                                   const char *path) {
  g_return_if_fail(BARBAR_IS_TEMPERATURE(temperature));

  g_free(temperature->path);
  temperature->path = g_strdup(path);

  g_object_notify_by_pspec(G_OBJECT(temperature),
                           temperature_props[PROP_DEVICE]);
}

static void g_barbar_temperature_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarTemperature *temperature = BARBAR_TEMPERATURE(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_temperature_set_path(temperature, g_value_get_string(value));
    break;
  case PROP_CRITICAL:
    temperature->critical = g_value_get_double(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_temperature_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {}

static void g_barbar_temperature_class_init(BarBarTemperatureClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_temperature_set_property;
  gobject_class->get_property = g_barbar_temperature_get_property;
  gobject_class->constructed = g_barbar_temperature_constructed;

  temperature_props[PROP_DEVICE] = g_param_spec_string(
      "path", NULL, NULL, "/sys/class/thermal/thermal_zone0/temp",
      G_PARAM_READWRITE);
  temperature_props[PROP_CRITICAL] = g_param_spec_double(
      "critical-temp", NULL, NULL, 0.0, 300.0, 80.0, G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    temperature_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "temperature");
}

static void g_barbar_temperature_init(BarBarTemperature *self) {}

static void g_barbar_temperature_constructed(GObject *self) {
  BarBarTemperature *temp = BARBAR_TEMPERATURE(self);

  temp->path = strdup("/sys/class/thermal/thermal_zone0/temp");
  temp->critical = 80.0;
  temp->interval = 1000;

  temp->label = gtk_label_new("");
  gtk_widget_set_parent(temp->label, GTK_WIDGET(temp));
}

// TODO: Farenheit etc
static double read_temp(const char *str) {
  return strtol(str, NULL, 10) / 1000.0;
}

static gboolean g_barbar_temperature_update(gpointer data) {
  BarBarTemperature *temperature = BARBAR_TEMPERATURE(data);
  GError *error = NULL;
  gboolean result;
  char *buf;
  gsize length;
  double temp;
  static char output[128];

  if (temperature->path) {
    result = g_file_get_contents(temperature->path, &buf, &length, &error);
    if (!result || error != NULL) {
      // TODO: HANDLE errors
      return FALSE;
    }
    temp = read_temp(buf);
    snprintf(output, sizeof(output), "%.0f", temp);
    gtk_label_set_text(GTK_LABEL(temperature->label), output);
    free(buf);
  }
  return TRUE;
}

void g_barbar_temperature_start(BarBarTemperature *temperature) {
  if (temperature->source_id > 0) {
    g_source_remove(temperature->source_id);
  }
  g_barbar_temperature_update(temperature);
  temperature->source_id = g_timeout_add_full(
      0, temperature->interval, g_barbar_temperature_update, temperature, NULL);
}
