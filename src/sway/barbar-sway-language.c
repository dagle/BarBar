#include "sway/barbar-sway-language.h"
#include "glib-object.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"
#include <string.h>

/**
 * BarBarSwayLanguage:
 *
 * A sensor to display the keyboard and language in sway.
 */
struct _BarBarSwayLanguage {
  BarBarSensor parent_instance;

  BarBarSwaySubscribe *sub;

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

G_DEFINE_TYPE(BarBarSwayLanguage, g_barbar_sway_language, BARBAR_TYPE_SENSOR)

static GParamSpec *sway_language_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_sway_language_start(BarBarSensor *sensor);

static void g_barbar_sway_language_set_identifier(BarBarSwayLanguage *sway,
                                                  const gchar *identifier) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));

  if (g_set_str(&sway->identifier, identifier)) {
    g_object_notify_by_pspec(G_OBJECT(sway),
                             sway_language_props[PROP_IDENTIFIER]);
  }
}

static void g_barbar_sway_language_set_keyboard(BarBarSwayLanguage *sway,
                                                const gchar *keyboard) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));

  if (g_set_str(&sway->keyboard, keyboard)) {
    g_object_notify_by_pspec(G_OBJECT(sway),
                             sway_language_props[PROP_KEYBOARD]);
  }
}

static void g_barbar_sway_language_set_language(BarBarSwayLanguage *sway,
                                                const gchar *language) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));
  char *str;

  if (language == NULL && sway->language == NULL) {
    return;
  }

  if (language == NULL) {
    sway->language = NULL;

    g_object_notify_by_pspec(G_OBJECT(sway),
                             sway_language_props[PROP_LANGUAGE]);
    return;
  }

  str = g_strdup(language);
  str = g_strstrip(str);

  if (!g_strcmp0(sway->language, str)) {
    g_free(str);
    return;
  }

  g_free(sway->language);
  sway->language = str;

  g_object_notify_by_pspec(G_OBJECT(sway), sway_language_props[PROP_LANGUAGE]);
}

static void g_barbar_sway_language_set_variant(BarBarSwayLanguage *sway,
                                               const gchar *variant) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));

  char *str;

  if (variant == NULL && sway->language == NULL) {
    return;
  }

  if (variant == NULL) {
    sway->variant = NULL;

    g_object_notify_by_pspec(G_OBJECT(sway), sway_language_props[PROP_LAYOUT]);
    return;
  }

  str = g_strdup(variant);
  str = g_strstrip(str);

  if (!g_strcmp0(sway->variant, str)) {
    g_free(str);
    return;
  }

  g_free(sway->variant);
  sway->variant = str;

  g_object_notify_by_pspec(G_OBJECT(sway), sway_language_props[PROP_LAYOUT]);
}

static void g_barbar_sway_language_set_layout(BarBarSwayLanguage *sway,
                                              const gchar *layout) {
  if (!layout) {
    return;
  }

  // printf("layout: %s\n", layout);
  gchar **split = g_strsplit(layout, " (", -1);

  g_barbar_sway_language_set_language(sway, split[0]);
  if (split[1]) {
    uint len = MAX(0, strlen(split[1]) - 1);
    split[1][len] = '\0';
  }
  g_barbar_sway_language_set_variant(sway, split[1]);

  g_strfreev(split);
}

static void g_barbar_sway_language_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(object);

  switch (property_id) {
  case PROP_IDENTIFIER:
    g_barbar_sway_language_set_identifier(sway, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_language_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(object);

  switch (property_id) {
  case PROP_IDENTIFIER:
    g_value_set_string(value, sway->identifier);
    break;
  case PROP_LANGUAGE:
    g_value_set_string(value, sway->language);
    break;
  case PROP_KEYBOARD:
    g_value_set_string(value, sway->keyboard);
    break;
  case PROP_LAYOUT:
    g_value_set_string(value, sway->variant);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_language_finalize(GObject *object) {
  BarBarSwayLanguage *language = BARBAR_SWAY_LANGUAGE(object);

  g_clear_object(&language->sub);
  g_free(language->keyboard);
  g_free(language->identifier);
  g_free(language->language);
  g_free(language->variant);

  G_OBJECT_CLASS(g_barbar_sway_language_parent_class)->finalize(object);
}

static void g_barbar_sway_language_class_init(BarBarSwayLanguageClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_sway_language_start;

  gobject_class->set_property = g_barbar_sway_language_set_property;
  gobject_class->get_property = g_barbar_sway_language_get_property;
  gobject_class->finalize = g_barbar_sway_language_finalize;

  sway_language_props[PROP_IDENTIFIER] = g_param_spec_string(
      "identifer", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  sway_language_props[PROP_KEYBOARD] =
      g_param_spec_string("keyboard", NULL, NULL, NULL, G_PARAM_READABLE);
  sway_language_props[PROP_LANGUAGE] =
      g_param_spec_string("language", NULL, NULL, NULL, G_PARAM_READABLE);
  sway_language_props[PROP_LAYOUT] =
      g_param_spec_string("layout", NULL, NULL, NULL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    sway_language_props);
}

static void g_barbar_sway_language_init(BarBarSwayLanguage *self) {}

static void g_barbar_sway_handle_inputs(BarBarSwayLanguage *sway,
                                        gchar *payload, gssize len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway language: Failed to parse json: %s", err->message);
    g_error_free(err);
    g_object_unref(parser);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  gint i = json_reader_count_elements(reader);

  for (int j = 0; j < i; j++) {
    json_reader_read_element(reader, j);
    json_reader_read_member(reader, "type");
    const char *type = json_reader_get_string_value(reader);
    json_reader_end_member(reader);
    if (!g_strcmp0(type, "keyboard")) {
      json_reader_read_member(reader, "identifier");
      const char *identifier = json_reader_get_string_value(reader);
      json_reader_end_member(reader);
      if (!sway->identifier) {
        g_barbar_sway_language_set_identifier(sway, identifier);
      }
      // if (!g_strcmp0(sway->identifier, identifier)) {
      json_reader_read_member(reader, "name");
      const char *name = json_reader_get_string_value(reader);
      g_barbar_sway_language_set_keyboard(sway, name);
      json_reader_end_member(reader);

      json_reader_read_member(reader, "xkb_active_layout_name");
      const char *layout = json_reader_get_string_value(reader);
      g_barbar_sway_language_set_layout(sway, layout);
      json_reader_end_member(reader);
      json_reader_end_element(reader);
      break;
      // }
    }
    json_reader_end_element(reader);
  }
  g_object_unref(reader);
  g_object_unref(parser);
}

// works like g_strcmp0 but if either dev or identifier is null, returns success
static gint g_identifier_cmp(const char *identifier, const char *dev) {
  if (!identifier || !dev) {
    return 0;
  }
  return g_strcmp0(identifier, dev);
}

static void event_listner(BarBarSwaySubscribe *sub, guint type,
                          const char *payload, guint len, gpointer data) {
  JsonParser *parser;
  GError *err = NULL;
  gboolean ret;
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(data);

  if (type != SWAY_INPUT_EVENT) {
    return;
  }

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway language: Failed to parse json: %s", err->message);
    g_error_free(err);
    g_object_unref(parser);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "type");
  const char *jtype = json_reader_get_string_value(reader);
  json_reader_end_element(reader);
  if (!g_strcmp0(jtype, "keyboard")) {
    json_reader_read_member(reader, "identifier");
    const char *identifier = json_reader_get_string_value(reader);
    json_reader_end_member(reader);
    if (!g_identifier_cmp(sway->identifier, identifier)) {
      json_reader_read_member(reader, "name");
      const char *name = json_reader_get_string_value(reader);
      g_barbar_sway_language_set_keyboard(sway, name);
      json_reader_end_member(reader);

      json_reader_read_member(reader, "xkb_active_layout_name");
      const char *layout = json_reader_get_string_value(reader);
      g_barbar_sway_language_set_layout(sway, layout);
      json_reader_end_member(reader);
      json_reader_end_element(reader);
    }
  }
  g_object_unref(reader);
  g_object_unref(parser);
}

static void input_cb(GObject *object, GAsyncResult *res, gpointer data) {
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_oneshot_finish(res, NULL, &str, &len, &error);

  if (error) {
    g_printerr("Failed to get languages: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (ret) {
    g_barbar_sway_handle_inputs(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void g_barbar_sway_language_start(BarBarSensor *sensor) {
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(sensor);

  sway->sub = BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"mode\"]"));

  g_barbar_sway_ipc_oneshot(SWAY_GET_INPUTS, TRUE, NULL, input_cb, sway, "");
}

/**
 * g_barbar_sway_language_new:
 *
 * Returs: (transfer full): a new sensor
 */
BarBarSensor *g_barbar_sway_language_new(void) {
  BarBarSwayLanguage *sensor;

  sensor = g_object_new(BARBAR_TYPE_SWAY_LANGUAGE, NULL);
  return BARBAR_SENSOR(sensor);
}
