#include "sway/barbar-sway-subscribe.h"
#include "sway/barbar-sway-ipc.h"
#include <json-glib/json-glib.h>

struct _BarBarSwaySubscribe {
  GObject parent_instance;

  GSocketConnection *ipc;
  char *interest;
};

enum {
  PROP_0,

  PROP_INTEREST,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwaySubscribe, g_barbar_sway_subscribe, G_TYPE_OBJECT)

static GParamSpec *sway_sub_props[NUM_PROPERTIES] = {
    NULL,
};

static guint event_signal;

/**
 * g_barbar_sway_susbscribe_set_interest:
 * @sub: a `BarBarSwaySubscribe`
 * @interest: interest
 *
 * Set what events we should be listening to
 */
void g_barbar_sway_susbscribe_set_interest(BarBarSwaySubscribe *sub,
                                           const char *interest) {
  g_return_if_fail(BARBAR_IS_SWAY_SUBSCRIBE(sub));

  if (g_set_str(&sub->interest, interest)) {

    g_object_notify_by_pspec(G_OBJECT(sub), sway_sub_props[PROP_INTEREST]);
  }
}

/**
 * g_barbar_sway_susbscribe_get_interest:
 * @sub: a `BarBarSwaySubscribe`
 *
 * Returns: (transfer none): get the events we are listening for
 */
const char *g_barbar_sway_susbscribe_get_interest(BarBarSwaySubscribe *sub) {
  g_return_val_if_fail(BARBAR_IS_SWAY_SUBSCRIBE(sub), NULL);

  return sub->interest;
}

static void g_barbar_sway_subscribe_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  BarBarSwaySubscribe *sub = BARBAR_SWAY_SUBSCRIBE(object);

  switch (property_id) {
  case PROP_INTEREST:
    g_barbar_sway_susbscribe_set_interest(sub, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_subscribe_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {
  // BarBarSwaySubscribe *sub = BARBAR_SWAY_SUBSCRIBE(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void event_cb(GObject *object, GAsyncResult *res, gpointer data) {
  GInputStream *stream = G_INPUT_STREAM(object);
  BarBarSwaySubscribe *self = BARBAR_SWAY_SUBSCRIBE(data);
  GError *error = NULL;
  char *str = NULL;
  guint32 type;
  gsize len;
  JsonParser *parser;

  gboolean ret =
      g_barbar_sway_ipc_read_finish(stream, res, &type, &str, &len, &error);

  if (!ret || error) {
    g_printerr("Sway subscribe ipc error: %s", error->message);
    g_error_free(error);
    return;
  }
  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, str, len, &error);

  if (!ret || error) {
    g_printerr("Sway subscribe couldn't parse json: %s", error->message);
    g_error_free(error);
    g_object_unref(parser);
    return;
  }

  g_signal_emit(self, event_signal, 0, type, parser);
  g_free(str);
  g_object_unref(parser);

  g_barbar_sway_ipc_read_async(stream, NULL, event_cb, data);
}

static void sub_cb(GObject *object, GAsyncResult *res, gpointer data) {
  GInputStream *stream = G_INPUT_STREAM(object);
  BarBarSwaySubscribe *self = BARBAR_SWAY_SUBSCRIBE(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_read_finish(stream, res, NULL, &str, &len, &error);

  if (!ret || error) {
    g_printerr("Sway failed to subscribe for workspace events: %s",
               error->message);
    g_error_free(error);
    return;
  }

  if (g_barbar_sway_message_is_success(str, len)) {
    g_barbar_sway_ipc_read_async(stream, NULL, event_cb, self);
  }
  g_free(str);
}

static void g_barbar_sway_subscribe_finalize(GObject *object) {
  BarBarSwaySubscribe *hypr = BARBAR_SWAY_SUBSCRIBE(object);

  g_free(hypr->interest);

  G_OBJECT_CLASS(g_barbar_sway_subscribe_parent_class)->finalize(object);
}

static void
g_barbar_sway_subscribe_class_init(BarBarSwaySubscribeClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  gobject_class->set_property = g_barbar_sway_subscribe_set_property;
  gobject_class->get_property = g_barbar_sway_subscribe_get_property;
  gobject_class->finalize = g_barbar_sway_subscribe_finalize;

  sway_sub_props[PROP_INTEREST] = g_param_spec_string(
      "interest", NULL, NULL, NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    sway_sub_props);

  event_signal = g_signal_new("event", G_TYPE_FROM_CLASS(class),
                              G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL,
                              G_TYPE_NONE, 2, G_TYPE_UINT, JSON_TYPE_PARSER);
}

static void g_barbar_sway_subscribe_init(BarBarSwaySubscribe *self) {}

/**
 * g_barbar_sway_subscribe_connect:
 * @self: a `BarBarSwaySubscribe`
 * @error: (out) (optional):  a #GError, or %NULL
 *
 * Tries to connect to a sway subscribtion. Will set error on error.
 * After connected it will start to emit messages.
 *
 */
void g_barbar_sway_subscribe_connect(BarBarSwaySubscribe *self,
                                     GError **error) {
  GError *err = NULL;
  self->ipc = g_barbar_sway_ipc_connect(&err);

  if (err) {
    g_propagate_error(error, err);
    return;
  }

  GOutputStream *output_stream;
  GInputStream *input_stream;

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(self->ipc));
  g_barbar_sway_ipc_send(output_stream, SWAY_SUBSCRIBE, self->interest, &err);

  if (err) {
    g_propagate_error(error, err);
    return;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(self->ipc));
  g_barbar_sway_ipc_read_async(input_stream, NULL, sub_cb, self);
}

/**
 * g_barbar_sway_subscribe_new:
 *
 * creates a new object where we can listen for events
 *
 * Returns: A new `BarBarSwaySubscribe` object
 */
GObject *g_barbar_sway_subscribe_new(const char *interest) {
  GObject *object =
      g_object_new(BARBAR_TYPE_SWAY_SUBSCRIBE, "interest", interest, NULL);
  return object;
}
