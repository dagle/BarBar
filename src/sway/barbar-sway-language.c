#include "sway/barbar-sway-language.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"

/**
 * BarBarSwayLanguage:
 *
 * A sensor to display the keyboard and language
 */
struct _BarBarSwayLanguage {
  BarBarSensor parent_instance;

  GSocketConnection *ipc;
  BarBarSwaySubscribe *sub;
  // struct xkb_context *ctx;

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
  PROP_VARIANT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayLanguage, g_barbar_sway_language, BARBAR_TYPE_SENSOR)

static GParamSpec *sway_language_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_sway_language_start(BarBarSensor *sensor);

static void g_barbar_sway_workspace_set_identifier(BarBarSwayLanguage *sway,
                                                   const gchar *identifier) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));

  if (!g_strcmp0(sway->identifier, identifier)) {
    return;
  }

  g_free(sway->identifier);
  sway->identifier = strdup(identifier);

  g_object_notify_by_pspec(G_OBJECT(sway),
                           sway_language_props[PROP_IDENTIFIER]);
}

static void g_barbar_sway_workspace_set_keyboard(BarBarSwayLanguage *sway,
                                                 const gchar *keyboard) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));

  if (!g_strcmp0(sway->keyboard, keyboard)) {
    return;
  }

  g_free(sway->keyboard);
  sway->keyboard = g_strdup(keyboard);

  g_object_notify_by_pspec(G_OBJECT(sway), sway_language_props[PROP_KEYBOARD]);
}

static void g_barbar_sway_workspace_set_language(BarBarSwayLanguage *sway,
                                                 const gchar *language,
                                                 size_t length) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));

  // TODO: fix compare

  if (!g_strcmp0(sway->language, language)) {
    return;
  }

  g_free(sway->language);
  sway->language = g_strndup(language, length);
  printf("language: %s\n", sway->language);

  g_object_notify_by_pspec(G_OBJECT(sway), sway_language_props[PROP_LANGUAGE]);
}

static void g_barbar_sway_workspace_set_variant(BarBarSwayLanguage *sway,
                                                const gchar *variant) {
  g_return_if_fail(BARBAR_IS_SWAY_LANGUAGE(sway));

  if (!g_strcmp0(sway->variant, variant)) {
    return;
  }

  g_free(sway->variant);
  sway->variant = g_strdup(variant);

  g_object_notify_by_pspec(G_OBJECT(sway), sway_language_props[PROP_VARIANT]);
}

// const char *find_first_opening_parenthesis(const char *str) {
//   const char *before_space = str;
//   while (*str != '\0') {
//     if (*str == '(') {
//       return before_space;
//     } else if (!isspace(*str)) {
//       before_space = str;
//     }
//     str++;
//   }
//   return NULL;
// }
//
static void g_barbar_sway_workspace_set_layout(BarBarSwayLanguage *sway,
                                               const gchar *layout) {
  if (!layout) {
    return;
  }
  // TODO: Can we do this without trying to implement this our selfs

  // struct xkb_keymap *kb =
  //     xkb_keymap_new_from_string(sway->ctx, layout,
  //     XKB_KEYMAP_FORMAT_TEXT_V1,
  //                                XKB_KEYMAP_COMPILE_NO_FLAGS);
  // xkb_layout_index_t idx = xkb_keymap_num_layouts(kb);
  //
  // for (int i = 0; i < idx; i++) {
  //
  //   const char *str = xkb_keymap_layout_get_name(kb, i);
  //   printf("kb: %s\n", str);
  // }
  //
  // xkb_keymap_unref(kb);
}

static void g_barbar_sway_language_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(object);

  switch (property_id) {
  case PROP_IDENTIFIER:
    g_barbar_sway_workspace_set_identifier(sway, g_value_get_string(value));
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
  case PROP_VARIANT:
    g_value_set_string(value, sway->variant);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_language_class_init(BarBarSwayLanguageClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_sway_language_start;

  gobject_class->set_property = g_barbar_sway_language_set_property;
  gobject_class->get_property = g_barbar_sway_language_get_property;

  sway_language_props[PROP_IDENTIFIER] = g_param_spec_string(
      "identifer", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  sway_language_props[PROP_KEYBOARD] =
      g_param_spec_string("keyboard", NULL, NULL, NULL, G_PARAM_READABLE);
  sway_language_props[PROP_LANGUAGE] =
      g_param_spec_string("language", NULL, NULL, NULL, G_PARAM_READABLE);
  sway_language_props[PROP_VARIANT] =
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
        g_barbar_sway_workspace_set_identifier(sway, identifier);
      }
      if (!g_strcmp0(sway->identifier, identifier)) {
        json_reader_read_member(reader, "name");
        const char *name = json_reader_get_string_value(reader);
        g_barbar_sway_workspace_set_keyboard(sway, name);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "xkb_active_layout_name");
        const char *layout = json_reader_get_string_value(reader);
        g_barbar_sway_workspace_set_layout(sway, layout);
        json_reader_end_member(reader);
        json_reader_end_element(reader);
        break;
      }
    }
    json_reader_end_element(reader);
  }
  g_object_unref(reader);
  g_object_unref(parser);
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
    if (g_strcmp0(sway->identifier, identifier)) {
      json_reader_read_member(reader, "name");
      const char *name = json_reader_get_string_value(reader);
      // set language
      json_reader_end_member(reader);

      json_reader_read_member(reader, "xkb_active_layout_name");
      const char *layout = json_reader_get_string_value(reader);
      // set layout
      json_reader_end_member(reader);
      json_reader_end_element(reader);
    }
  }
  g_object_unref(reader);
  g_object_unref(parser);
}

static void input_cb(GObject *object, GAsyncResult *res, gpointer data) {
  GInputStream *stream = G_INPUT_STREAM(object);
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_read_finish(stream, res, NULL, &str, &len, &error);

  if (error) {
    g_printerr("Failed to get workspaces: %s\n", error->message);
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

  GInputStream *input_stream;
  GOutputStream *output_stream;
  GError *error = NULL;

  // sway->ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  // if (!sway->ctx) {
  //   g_printerr("Sway language: Couldn't initialize xkb context");
  //   return;
  // }

  sway->ipc = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    g_printerr("Sway language: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  sway->sub = BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"input\"]"));

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(sway->ipc));

  g_barbar_sway_ipc_send(output_stream, SWAY_GET_INPUTS, "", &error);

  if (error != NULL) {
    g_printerr("Sway language: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  if (error != NULL) {
    g_printerr("Sway language: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(sway->ipc));

  g_barbar_sway_ipc_read_async(input_stream, NULL, input_cb, sway);
}
