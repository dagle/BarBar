#include "niri/barbar-niri-language.h"
#include "glib.h"
#include "niri/barbar-niri-subscribe.h"
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarNiriLanguage:
 *
 * A widget to display the niri language
 */
struct _BarBarNiriLanguage {
  BarBarSensor parent_instance;

  BarBarNiriSubscribe *sub;
  char *keyboard;
  char *identifier;
  char *language;
  char *variant;
};

enum {
  PROP_0,

  PROP_IDENTIFIER,
  PROP_KEYBOARD,
  PROP_LANGUAGE,
  PROP_LAYOUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarNiriLanguage, g_barbar_niri_language, BARBAR_TYPE_SENSOR);

static GParamSpec *niri_language_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_niri_language_start(BarBarSensor *sensor);

static void g_barbar_niri_language_set_language(BarBarNiriLanguage *niri,
                                                const gchar *language) {
  if (g_set_str(&niri->language, language)) {
    g_object_notify_by_pspec(G_OBJECT(niri),
                             niri_language_props[PROP_LANGUAGE]);
  }
}

static void g_barbar_niri_language_set_variant(BarBarNiriLanguage *niri,
                                               const gchar *variant) {
  if (g_set_str(&niri->variant, variant)) {
    g_object_notify_by_pspec(G_OBJECT(niri), niri_language_props[PROP_LAYOUT]);
  }
}

static void g_barbar_niri_language_set_kbd(BarBarNiriLanguage *niri,
                                           const char *kbd) {
  if (!kbd) {
    return;
  }

  gchar **split = g_strsplit(kbd, " (", -1);

  g_barbar_niri_language_set_language(niri, split[0]);
  if (split[1]) {
    uint len = MAX(0, strlen(split[1]) - 1);
    split[1][len] = '\0';
  }
  g_barbar_niri_language_set_variant(niri, split[1]);

  g_strfreev(split);
}

static void g_barbar_niri_language_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {
  BarBarNiriLanguage *niri = BARBAR_NIRI_LANGUAGE(object);

  switch (property_id) {
  case PROP_IDENTIFIER:
    // g_barbar_niri_language_set_identifier(niri, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_niri_language_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {
  BarBarNiriLanguage *niri = BARBAR_NIRI_LANGUAGE(object);

  switch (property_id) {
  case PROP_IDENTIFIER:
    g_value_set_string(value, niri->identifier);
    break;
  case PROP_LANGUAGE:
    g_value_set_string(value, niri->language);
    break;
  case PROP_KEYBOARD:
    g_value_set_string(value, niri->keyboard);
    break;
  case PROP_LAYOUT:
    g_value_set_string(value, niri->variant);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_niri_language_finalize(GObject *object) {
  BarBarNiriLanguage *language = BARBAR_NIRI_LANGUAGE(object);

  // g_clear_object(&language->sub);
  g_free(language->keyboard);
  g_free(language->identifier);
  g_free(language->language);
  g_free(language->variant);

  // G_OBJECT_CLASS(g_barbar_niri_language_parent_class)->finalize(object);
}

static void g_barbar_niri_language_class_init(BarBarNiriLanguageClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_niri_language_start;

  gobject_class->set_property = g_barbar_niri_language_set_property;
  gobject_class->get_property = g_barbar_niri_language_get_property;
  gobject_class->finalize = g_barbar_niri_language_finalize;

  niri_language_props[PROP_IDENTIFIER] = g_param_spec_string(
      "identifer", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  niri_language_props[PROP_KEYBOARD] =
      g_param_spec_string("keyboard", NULL, NULL, NULL, G_PARAM_READABLE);
  niri_language_props[PROP_LANGUAGE] =
      g_param_spec_string("language", NULL, NULL, NULL, G_PARAM_READABLE);
  niri_language_props[PROP_LAYOUT] =
      g_param_spec_string("layout", NULL, NULL, NULL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    niri_language_props);
}

static void g_barbar_niri_language_init(BarBarNiriLanguage *self) {}

static void event_listner(BarBarNiriSubscribe *sub, JsonParser *parser,
                          gpointer data) {
  BarBarNiriLanguage *niri = BARBAR_NIRI_LANGUAGE(data);

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  if (json_reader_read_member(reader, "KeyboardLayoutsChanged")) {
    json_reader_read_member(reader, "keyboard_layouts");

    json_reader_read_member(reader, "current_idx");
    gint64 idx = json_reader_get_int_value(reader);
    json_reader_end_member(reader); // end current_idx

    json_reader_read_member(reader, "names");

    json_reader_read_element(reader, idx);
    const char *kbd = json_reader_get_string_value(reader);
    g_barbar_niri_language_set_kbd(niri, kbd);
    json_reader_end_element(reader);

    json_reader_end_member(reader); // end names

    json_reader_end_member(reader); // end layouts

    json_reader_end_member(reader); // end changed
  }
}

static void g_barbar_niri_language_start(BarBarSensor *sensor) {
  BarBarNiriLanguage *niri = BARBAR_NIRI_LANGUAGE(sensor);
  GError *error = NULL;

  niri->sub = BARBAR_NIRI_SUBSCRIBE(g_barbar_niri_subscribe_new());

  g_signal_connect(niri->sub, "event", G_CALLBACK(event_listner), niri);

  g_barbar_niri_subscribe_connect(niri->sub, &error);
  if (error) {
    g_warning("can't connect: %s\n", error->message);
    g_error_free(error);
    return;
  }
}

/**
 * g_barbar_niri_language_new:
 *
 * Returns: (transfer full): a new sensor
 */
BarBarSensor *g_barbar_niri_language_new(void) {
  BarBarNiriLanguage *sensor;

  sensor = g_object_new(BARBAR_TYPE_NIRI_LANGUAGE, NULL);
  return BARBAR_SENSOR(sensor);
}
