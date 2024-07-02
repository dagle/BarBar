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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

// static void g_barbar_sway_mode_finalize(GObject *object) {
//   BarBarSwayMode *mode = BARBAR_SWAY_MODE(object);
//
//   g_free(mode->mode);
//   g_clear_object(&mode->sub);
//
//   G_OBJECT_CLASS(g_barbar_sway_mode_parent_class)->finalize(object);
// }
//
static void
g_barbar_sway_scratchpad_class_init(BarBarSwayScratchpadClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_sway_scratchpad_start;
  //
  gobject_class->set_property = g_barbar_sway_scratchpad_set_property;
  gobject_class->get_property = g_barbar_sway_scratchpad_get_property;
  // gobject_class->finalize = g_barbar_sway_mode_finalize;

  /**
   * BarBarSwayScratchpad:app:
   *
   * Name of the app in scratchpad area
   */
  sway_props[PROP_APP] =
      g_param_spec_string("app", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
   * BarBarSwayScratchpad:app:
   *
   * The current sway mode
   */
  sway_props[PROP_COUNT] =
      g_param_spec_uint("app", NULL, NULL, 0, 1000, 0, G_PARAM_READABLE);

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

static void g_barbar_sway_handle_scratchpad(BarBarSwayScratchpad *sway,
                                            gchar *payload, gssize len) {}

static void event_listner(BarBarSwaySubscribe *sub, guint type,
                          const char *payload, guint len, gpointer data) {
  BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(data);

  g_barbar_sway_ipc_oneshot(SWAY_GET_TREE, TRUE, NULL, scratchpad_state_cb,
                            sway, "");
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

static void scratchpad_setup_cb(GObject *object, GAsyncResult *res,
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

static void g_barbar_sway_scratchpad_start(BarBarSensor *sensor) {
  BarBarSwayScratchpad *sway = BARBAR_SWAY_SCRATCHPAD(sensor);
  GError *error = NULL;
  GInputStream *input_stream;
  GOutputStream *output_stream;

  sway->ipc = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    g_printerr("Sway workspace: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  sway->sub =
      BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"window\"]"));

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(sway->ipc));

  g_barbar_sway_ipc_send(output_stream, SWAY_GET_WORKSPACES, "", &error);

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
  g_barbar_sway_ipc_read_async(input_stream, NULL, scratchpad_state_cb, sway);
}
