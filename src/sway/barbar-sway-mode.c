#include "barbar-sway-mode.h"
#include "sensors/barbar-sensor.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"

/**
 * BarBarSwayMode:
 *
 * A sensor to display the mode we currently are in.
 * See sway(5) for more info
 *
 */
struct _BarBarSwayMode {
  BarBarSensor parent_instance;

  char *mode;
  BarBarSwaySubscribe *sub;
};

enum {
  PROP_0,

  PROP_MODE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayMode, g_barbar_sway_mode, BARBAR_TYPE_SENSOR)

static GParamSpec *sway_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_sway_mode_start(BarBarSensor *sensor);

static void g_barbar_sway_mode_set_mode(BarBarSwayMode *sway,
                                        const char *mode) {
  g_return_if_fail(BARBAR_IS_SWAY_MODE(sway));

  if (g_set_str(&sway->mode, mode)) {
    g_object_notify_by_pspec(G_OBJECT(sway), sway_props[PROP_MODE]);
  }
}

/**
 * g_barbar_sway_mode_get_mode:
 * @sway: a `BarBarSwayMode`
 *
 * Returns: (transfer none): get the current mode
 */
const char *g_barbar_sway_mode_get_mode(BarBarSwayMode *sway) {
  g_return_val_if_fail(BARBAR_IS_SWAY_MODE(sway), NULL);

  return sway->mode;
}

static void g_barbar_sway_mode_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  // BarBarSwayMode *sway = BARBAR_SWAY_MODE(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_mode_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarSwayMode *sway = BARBAR_SWAY_MODE(object);

  switch (property_id) {
  case PROP_MODE:
    g_value_set_string(value, sway->mode);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_mode_finalize(GObject *object) {
  BarBarSwayMode *mode = BARBAR_SWAY_MODE(object);

  g_free(mode->mode);
  g_clear_object(&mode->sub);

  G_OBJECT_CLASS(g_barbar_sway_mode_parent_class)->finalize(object);
}

static void g_barbar_sway_mode_class_init(BarBarSwayModeClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_sway_mode_start;

  gobject_class->set_property = g_barbar_sway_mode_set_property;
  gobject_class->get_property = g_barbar_sway_mode_get_property;
  gobject_class->finalize = g_barbar_sway_mode_finalize;

  /**
   * BarBarSwayMode:mode:
   *
   * The current sway mode
   */
  sway_props[PROP_MODE] =
      g_param_spec_string("mode", NULL, NULL, NULL, G_PARAM_READABLE);

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

static void g_barbar_sway_mode_init(BarBarSwayMode *self) {}

static void g_barbar_sway_handle_mode(BarBarSwayMode *sway, gchar *payload,
                                      gssize len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway workspace: Failed to parse json: %s", err->message);
    g_error_free(err);
    g_object_unref(parser);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  json_reader_read_member(reader, "name");
  const char *name = json_reader_get_string_value(reader);
  g_barbar_sway_mode_set_mode(sway, name);
  json_reader_end_member(reader);

  g_object_unref(reader);
  g_object_unref(parser);
}

static void event_listner(BarBarSwaySubscribe *sub, guint type,
                          const char *payload, guint len, gpointer data) {
  BarBarSwayMode *sway = BARBAR_SWAY_MODE(data);

  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  if (type != SWAY_MODE_EVENT) {
    return;
  }

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway workspace: Failed to parse json: %s", err->message);
    g_error_free(err);
    g_object_unref(parser);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "change");
  const char *change = json_reader_get_string_value(reader);
  g_barbar_sway_mode_set_mode(sway, change);
  json_reader_end_member(reader);

  g_object_unref(reader);
}

static void binding_state_cb(GObject *object, GAsyncResult *res,
                             gpointer data) {
  BarBarSwayMode *sway = BARBAR_SWAY_MODE(data);
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
    g_barbar_sway_handle_mode(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void g_barbar_sway_mode_start(BarBarSensor *sensor) {
  BarBarSwayMode *sway = BARBAR_SWAY_MODE(sensor);

  sway->sub = BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"mode\"]"));

  g_barbar_sway_ipc_oneshot(SWAY_GET_BINDING_STATE, TRUE, NULL,
                            binding_state_cb, sway, "");
}

/**
 * g_barbar_sway_mode_new:
 *
 * Returns: (transfer full): a new sensor
 */
BarBarSensor *g_barbar_sway_mode_new(void) {
  BarBarSwayMode *sensor;

  sensor = g_object_new(BARBAR_TYPE_SWAY_MODE, NULL);
  return BARBAR_SENSOR(sensor);
}
