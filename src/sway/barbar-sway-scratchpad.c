#include "barbar-sway-scratchpad.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"

/**
 * BarBarSwayScratchpad:
 *
 * A sensor to show scratchpad information in sway.
 *
 */
struct _BarBarSwayScratchpad {
  BarBarSensor parent_instance;
  GSocketConnection *ipc;

  char *app;
  gint count;

  BarBarSwaySubscribe *sub;
};

enum {
  PROP_0,

  PROP_APP,
  PROP_COUNT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayScratchpad, g_barbar_sway_scratchpad,
              BARBAR_TYPE_SENSOR)

static GParamSpec *sway_props[NUM_PROPERTIES] = {
    NULL,
};

static void scratchpad_state_cb(GObject *object, GAsyncResult *res,
                                gpointer data);

static void g_barbar_sway_scratchpad_start(BarBarSensor *sensor);

static void g_barbar_sway_scratchpad_set_property(GObject *object,
                                                  guint property_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec) {

  // BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_scratchpad_get_property(GObject *object,
                                                  guint property_id,
                                                  GValue *value,
                                                  GParamSpec *pspec) {
  BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(object);

  switch (property_id) {
  case PROP_APP:
    g_value_set_string(value, sway->app);
    break;
  case PROP_COUNT:
    g_value_set_uint(value, sway->count);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_mode_finalize(GObject *object) {
  BarBarSwayScratchpad *pad = BARBAR_SWAY_SCRATCHPAD(object);

  g_free(pad->app);
  g_clear_pointer(&pad->sub, g_object_unref);

  G_OBJECT_CLASS(g_barbar_sway_scratchpad_parent_class)->finalize(object);
}

static void
g_barbar_sway_scratchpad_class_init(BarBarSwayScratchpadClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_sway_scratchpad_start;
  //
  gobject_class->set_property = g_barbar_sway_scratchpad_set_property;
  gobject_class->get_property = g_barbar_sway_scratchpad_get_property;
  gobject_class->finalize = g_barbar_sway_mode_finalize;

  /**
   * BarBarSwayScratchpad:app:
   *
   * Name of the currently active app in scratchpad area
   */
  sway_props[PROP_APP] =
      g_param_spec_string("app", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
   * BarBarSwayScratchpad:app:
   *
   * The how many apps in the scratchpad
   */
  sway_props[PROP_COUNT] =
      g_param_spec_uint("count", NULL, NULL, 0, 1000, 0, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, sway_props);

  // /**
  //  * BarBarSwayMode::tick:
  //  * @sensor: This sensor
  //  *
  //  * Emit that cpu has updated
  //  */
  // sway_signals[TICK] =
  //     g_signal_new("tick",                                 /* signal_name */
  //                  BARBAR_TYPE_CPU,                        /* itype */
  //                  G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
  //                  0,                                      /* class_offset */
  //                  NULL,                                   /* accumulator */
  //                  NULL,                                   /* accu_data */
  //                  NULL,                                   /* c_marshaller */
  //                  G_TYPE_NONE,                            /* return_type */
  //                  0                                       /* n_params */
  //     );
}

static void g_barbar_sway_scratchpad_init(BarBarSwayScratchpad *self) {}

static void event_listner(BarBarSwaySubscribe *sub, guint type,
                          const char *payload, guint len, gpointer data) {
  BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(data);

  g_barbar_sway_ipc_oneshot(SWAY_GET_TREE, TRUE, NULL, scratchpad_state_cb,
                            sway, "");
}

static void g_barbar_sway_handle_scratchpad(BarBarSwayScratchpad *sway,
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
      if (!g_strcmp0(sway->identifier, identifier)) {
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
      }
    }
    json_reader_end_element(reader);
  }
  g_object_unref(reader);
  g_object_unref(parser);
}

static void scratchpad_state_cb(GObject *object, GAsyncResult *res,
                                gpointer data) {
  BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_oneshot_finish(res, NULL, &str, &len, &error);

  if (error) {
    g_printerr("Sway mode: Failed to get current mode: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (ret) {
    g_barbar_sway_handle_scratchpad(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void get_window(BarBarSwayScratchpad *self) {
  GError *error = NULL;
  GInputStream *input_stream;
  GOutputStream *output_stream;
}

// static void scratchpad_setup_cb(GObject *object, GAsyncResult *res,
//                                 gpointer data) {
//
//   BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(data);
//   GError *error = NULL;
//   char *str = NULL;
//   gsize len;
//
//   gboolean ret =
//       g_barbar_sway_ipc_oneshot_finish(res, NULL, &str, &len, &error);
//
//   if (error) {
//     g_printerr("Sway mode: Failed to get current mode: %s\n",
//     error->message); g_error_free(error); return;
//   }
//
//   if (ret) {
//     g_barbar_sway_handle_scratchpad(sway, str, len);
//
//     g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
//     g_barbar_sway_subscribe_connect(sway->sub, &error);
//   }
//
//   g_free(str);
// }
//

static void tree_cb(GObject *object, GAsyncResult *res, gpointer data) {
  BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_oneshot_finish(res, NULL, &str, &len, &error);

  if (error) {
    g_printerr("Failed to get scratchpad: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (ret) {
    g_barbar_sway_handle_scratchpad(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void g_barbar_sway_scratchpad_start(BarBarSensor *sensor) {
  BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(sensor);

  sway->sub =
      BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"workspace\"]"));

  g_barbar_sway_ipc_oneshot(SWAY_GET_TREE, TRUE, NULL, tree_cb, sway, "");
}

/**
 * g_barbar_sway_scratchpad_new:
 *
 * Returs: (transfer full): a new sensor
 */
BarBarSensor *g_barbar_sway_scratchpad_new(void) {
  BarBarSwayScratchpad *sensor;

  sensor = g_object_new(BARBAR_TYPE_SWAY_SCRATCHPAD, NULL);
  return BARBAR_SENSOR(sensor);
}
