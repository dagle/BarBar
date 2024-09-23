#include "niri/barbar-niri-subscribe.h"
#include "gio/gio.h"
#include "niri/barbar-niri-ipc.h"
#include <json-glib/json-glib.h>

struct _BarBarNiriSubscribe {
  GObject parent_instance;

  GSocketConnection *ipc;
  GDataInputStream *data_input_stream;
  char *interest;
};

enum {
  PROP_0,

  NUM_PROPERTIES,
};

#define REQUEST "\"EventStream\"\n"

G_DEFINE_TYPE(BarBarNiriSubscribe, g_barbar_niri_subscribe, G_TYPE_OBJECT)

static GParamSpec *niri_sub_props[NUM_PROPERTIES] = {
    NULL,
};

static guint event_signal;

static void g_barbar_niri_subscribe_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  BarBarNiriSubscribe *sub = BARBAR_NIRI_SUBSCRIBE(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_niri_subscribe_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {
  // BarBarNiriSubscribe *sub = BARBAR_NIRI_SUBSCRIBE(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
g_barbar_niri_subscribe_class_init(BarBarNiriSubscribeClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  gobject_class->set_property = g_barbar_niri_subscribe_set_property;
  gobject_class->get_property = g_barbar_niri_subscribe_get_property;

  event_signal =
      g_signal_new("event", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST, 0,
                   NULL, NULL, NULL, G_TYPE_NONE, 1, JSON_TYPE_PARSER);
}

static void g_barbar_niri_subscribe_init(BarBarNiriSubscribe *self) {}

static void sub_cb(GObject *object, GAsyncResult *res, gpointer data) {
  GDataInputStream *stream = G_DATA_INPUT_STREAM(object);
  GError *error = NULL;
  gsize len;
  BarBarNiriSubscribe *self = BARBAR_NIRI_SUBSCRIBE(data);
  JsonParser *parser;
  gboolean ret;

  char *line = g_data_input_stream_read_line_finish(stream, res, &len, &error);

  if (error) {
    g_printerr("Niri failed to subscribe for workspace events: %s\n",
               error->message);
    g_error_free(error);
    return;
  }

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, line, len, &error);

  if (error) {
    g_printerr("Niri failed to parse json data: %s, %s\n", line,
               error->message);
    g_error_free(error);
    g_free(line);
    return;
  }

  g_signal_emit(self, event_signal, 0, parser);

  g_free(line);
  g_object_unref(parser);
  g_data_input_stream_read_line_async(self->data_input_stream, 8192, NULL,
                                      sub_cb, self);
}

/**
 * g_barbar_niri_subscribe_connect:
 * @self: a `BarBarNiriSubscribe`
 * @error: (out) (optional):  a #GError, or %NULL
 *
 * Tries to connect to a niri subscribtion. Will set error on error.
 * After connected it will start to emit messages.
 *
 */
void g_barbar_niri_subscribe_connect(BarBarNiriSubscribe *self,
                                     GError **error) {
  GError *err = NULL;
  self->ipc = g_barbar_niri_ipc_connect(&err);

  if (err) {
    g_propagate_error(error, err);
    return;
  }

  GOutputStream *output_stream;
  GInputStream *input_stream;

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(self->ipc));

  gsize written;
  g_output_stream_write_all(output_stream, REQUEST, sizeof(REQUEST), &written,
                            NULL, &err);

  if (err) {
    g_propagate_error(error, err);
    return;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(self->ipc));
  self->data_input_stream = g_data_input_stream_new(input_stream);

  g_data_input_stream_read_line_async(self->data_input_stream, 8192, NULL,
                                      sub_cb, self);
}

/**
 * g_barbar_niri_subscribe_new:
 *
 * creates a new object where we can listen for events
 *
 * Returns: A new `BarBarNiriSubscribe` object
 */
GObject *g_barbar_niri_subscribe_new(void) {
  GObject *object = g_object_new(BARBAR_TYPE_NIRI_SUBSCRIBE, NULL);
  return object;
}
