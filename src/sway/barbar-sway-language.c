#include "sway/barbar-sway-language.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"

/**
 * BarBarSwayLanguage:
 *
 * A widget to display the keyboard and language
 */
struct _BarBarSwayLanguage {
  GtkWidget parent_instance;

  GSocketConnection *ipc;
  BarBarSwaySubscribe *sub;

  char *identifier;
  char *language;
  char *variant;
};

enum {
  PROP_0,

  PROP_IDENTIFIER,
  PROP_LANGUAGE,
  PROP_VARIANT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayLanguage, g_barbar_sway_language, BARBAR_TYPE_SENSOR)

static GParamSpec *sway_language_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_sway_workspace_set_name(BarBarSwayLanguage *sway,
                                             const gchar *output) {}

static void g_barbar_sway_workspace_set_variant(BarBarSwayLanguage *sway,
                                                const gchar *output) {}

static void g_barbar_sway_language_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(object);

  switch (property_id) {
  // case PROP_OUTPUT:
  //   g_barbar_sway_workspace_set_output(sway, g_value_get_string(value));
  //   break;
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
  // case PROP_OUTPUT:
  //   g_value_set_string(value, sway->output_name);
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_language_class_init(BarBarSwayLanguageClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_sway_language_set_property;
  gobject_class->get_property = g_barbar_sway_language_get_property;
}

static void g_barbar_sway_language_init(BarBarSwayLanguage *self) {}

static void g_barbar_sway_handle_workspaces(BarBarSwayLanguage *sway,
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
        sway->identifier = g_strdup(identifier);
      }
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
    // g_barbar_sway_handle_workspaces(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void g_barbar_sway_workspace_start(BarBarSensor *sensor) {
  BarBarSwayLanguage *sway = BARBAR_SWAY_LANGUAGE(sensor);

  GInputStream *input_stream;
  GOutputStream *output_stream;
  GError *error = NULL;

  sway->ipc = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    g_printerr("Sway workspace: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  sway->sub = BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"input\"]"));

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(sway->ipc));

  g_barbar_sway_ipc_send(output_stream, SWAY_GET_INPUTS, "", &error);

  if (error != NULL) {
    g_printerr("Sway workspace: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  if (error != NULL) {
    g_printerr("Sway workspace: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(sway->ipc));

  g_barbar_sway_ipc_read_async(input_stream, NULL, input_cb, sway);
}
