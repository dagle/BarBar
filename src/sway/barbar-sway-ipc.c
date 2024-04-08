#include "sway/barbar-sway-ipc.h"
#include "barbar-error.h"
#include <stdint.h>
#include <stdio.h>

typedef struct SwayIpc {
  guint32 type;
  guint8 header[14];
  gchar *payload;
  gssize length;

  guint ref_count;
} SwayIpc;

void g_sway_ipc_unref(SwayIpc *ipc) {
  gboolean is_zero;

  is_zero = g_atomic_int_dec_and_test((int *)&ipc->ref_count);

  if (G_UNLIKELY(is_zero)) {
    g_free(ipc->payload);
    g_free(ipc);
  }
}

SwayIpc *g_sway_ipc_ref(SwayIpc *ipc) {
  g_atomic_int_inc((int *)&ipc->ref_count);

  return ipc;
}

char *g_barbar_sway_ipc_get_header(SwayIpc *ipc) {
  char *str = g_malloc0(15);
  memcpy(str, ipc->header, 14);

  return str;
}

const char *g_barbar_sway_ipc_get_payload(SwayIpc *ipc) { return ipc->payload; }

guint32 g_barbar_sway_ipc_get_type(SwayIpc *ipc) { return ipc->type; }

static void callback_header(GObject *object, GAsyncResult *result,
                            gpointer data);
static void callback_payload(GObject *object, GAsyncResult *result,
                             gpointer data);

BarBarSwayIpc *g_barbar_sway_ipc_connect(GError **error) {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  BarBarSwayIpc *ipc;

  const char *socket_path = getenv("SWAYSOCK");

  if (!socket_path) {
    // TODO: Error stuff
    return NULL;
  }

  socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);

  // g_object_unref(address);

  if (connection == NULL || *error != NULL) {
    return NULL;
  }
  ipc = g_malloc0(sizeof(BarBarSwayIpc));
  ipc->connection = connection;

  return ipc;
}

GSocketConnection *g_barbar_sway_ipc_connect2(GError **error) {
  GSocketClient *socket_client;
  GSocketConnection *connection;

  const char *socket_path = getenv("SWAYSOCK");

  if (!socket_path) {
    // TODO: Error stuff
    return NULL;
  }

  socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);

  // g_object_unref(address);
  // g_object_unref(socket_client);

  if (connection == NULL || *error != NULL) {
    return NULL;
  }

  return connection;
}

gboolean g_barbar_sway_ipc_send2(GOutputStream *output_stream, guint type,
                                 const char *payload) {
  uint32_t paylen;
  uint32_t length;
  char *message;
  gboolean ret;
  GError *err = NULL;

  paylen = strlen(payload);
  // magic-string + 4bytes payload length + 4bytes type
  length = 6 + 4 + 4 + paylen;
  message = calloc(length, sizeof(char));

  memcpy(message, "i3-ipc", 6);
  memcpy(message + 6, &paylen, sizeof(paylen));
  memcpy(message + 10, &type, sizeof(type));
  memcpy(message + 14, payload, paylen);

  ret = g_output_stream_write_all(output_stream, message, length, NULL, NULL,
                                  &err);
  g_free(message);
  return ret;
}

gboolean g_barbar_sway_ipc_send(BarBarSwayIpc *ipc, guint type,
                                const char *payload) {
  GOutputStream *output_stream;
  uint32_t paylen;
  uint32_t length;
  char *message;
  gboolean ret;
  GError *err = NULL;

  if (ipc->subscribe_data) {
    // error
    return FALSE;
  }

  paylen = strlen(payload);
  // magic-string + 4bytes payload length + 4bytes type
  length = 6 + 4 + 4 + paylen;
  message = calloc(length, sizeof(char));

  memcpy(message, "i3-ipc", 6);
  memcpy(message + 6, &paylen, sizeof(paylen));
  memcpy(message + 10, &type, sizeof(type));
  memcpy(message + 14, payload, paylen);

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(ipc->connection));

  ret = g_output_stream_write_all(output_stream, message, length, NULL, NULL,
                                  &err);
  g_free(message);
  return ret;
}

// void g_barbar_sway_ipc_send_async(BarBarSwayIpc *ipc, guint type,
//                                   const char *payload,
//                                   GCancellable *cancallable,
//                                   GAsyncReadyCallback callback, gpointer
//                                   data) {
//   GOutputStream *output_stream;
//   uint32_t paylen;
//   uint32_t length;
//   char *message;
//
//   paylen = strlen(payload);
//   // magic-string + 4bytes payload length + 4bytes type
//   length = 6 + 4 + 4 + paylen;
//   message = calloc(length, sizeof(char));
//
//   memcpy(message, "i3-ipc", 6);
//   memcpy(message + 6, &paylen, sizeof(paylen));
//   memcpy(message + 10, &type, sizeof(type));
//   memcpy(message + 14, payload, paylen);
//
//   output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
//
//   g_output_stream_write_all_async(output_stream, message, length, 0,
//                                   cancallable, callback, data);
// }

gssize g_barbar_sway_ipc_read(BarBarSwayIpc *ipc, gchar **payload,
                              GError **error) {
  guint8 header[14];
  gssize bytes_read;

  if (ipc->subscribe_data) {
    // error
    return -1;
  }

  GInputStream *input_stream;
  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc->connection));

  bytes_read =
      g_input_stream_read(input_stream, header, sizeof(header), NULL, error);

  // The header size should always be 14 bytes
  if (bytes_read != 14) {
    // error
    return -1;
  }

  uint32_t type;
  uint32_t paylen;

  memcpy(&paylen, header + 6, 4);
  memcpy(&type, header + 10, 4);

  char *ppayload = malloc(paylen);

  bytes_read = g_input_stream_read(input_stream, ppayload, paylen, NULL, error);
  if (bytes_read > 0) {
    *payload = ppayload;
  } else {
    g_free(ppayload);
  }

  return bytes_read;
}

void read_payload_cb(GObject *source, GAsyncResult *res, gpointer data) {
  gssize size;
  GBytes *bytes;
  GError *error = NULL;
  GTask *task = data;
  GInputStream *stream = G_INPUT_STREAM(source);

  size = g_input_stream_read_finish(stream, res, &error);
  SwayIpc *ipc = g_task_get_task_data(task);

  if (error) {
    g_task_return_error(task, error);
    return;
  }

  g_task_return_boolean(task, TRUE);
  g_object_unref(task);
}

void read_header_cb(GObject *source, GAsyncResult *res, gpointer data) {
  gssize size;
  GError *error = NULL;
  GInputStream *stream = G_INPUT_STREAM(source);
  GTask *task = data;

  size = g_input_stream_read_finish(stream, res, &error);

  SwayIpc *ipc = g_task_get_task_data(task);

  if (error) {
    g_task_return_error(task, error);
    g_object_unref(task);
    return;
  }

  if (size < 14) {
    g_task_return_new_error(task, BARBAR_ERROR, BARBAR_ERROR_BAD_SWAY_IPC,
                            "Bad header length");
    g_object_unref(task);
    return;
  }

  if (strncmp((char *)ipc->header, "i3-ipc", 6)) {
    g_task_return_new_error(task, BARBAR_ERROR, BARBAR_ERROR_BAD_SWAY_IPC,
                            "Bad header magic");
  }

  guint32 type;
  guint32 paylen;

  memcpy(&paylen, ipc->header + 6, 4);
  memcpy(&type, ipc->header + 10, 4);

  ipc->type = type;
  ipc->payload = g_malloc0(paylen);

  g_input_stream_read_async(stream, ipc->payload, paylen, 8192, NULL,
                            read_payload_cb, data);
}

void g_barbar_sway_ipc_read_async(GInputStream *input_stream,
                                  GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer data) {
  GTask *task;
  SwayIpc *ipc = g_malloc0(sizeof(SwayIpc));

  task = g_task_new(input_stream, cancellable, callback, data);
  g_task_set_task_data(task, ipc, (GDestroyNotify)g_sway_ipc_unref);

  g_input_stream_read_async(input_stream, ipc->header, 14, 8192, cancellable,
                            read_header_cb, data);
}

/**
 * g_barbar_sway_ipc_read_finish:
 * @stream: input #GInputStream
 * @result: a #GAsyncResult
 * @type: (out) (optional): Type of message in the response or %NULL if the type
 * is not needed
 * @contents: (out) (transfer full) (element-type guint8) (array length=length):
 * a location to place the contents of the file
 * @length: (out) (optional): a location to place the length of the contents of
 * the file, Length of the content or %NULL if length is not needed
 * @error: a #GError, or %NULL
 *
 * Finishes an asynchronous sway ipc response that was started
 * with g_barbar_sway_ipc_read_async(). The data is always
 * zero-terminated. The returned @contents should be freed with g_free()
 * when no longer needed.
 *
 * Returns: %TRUE if the ipc was successfully communicated (the ipc content can
 * still contain an error). If %FALSE and @error is present, it will be set
 * appropriately.
 */
gboolean g_barbar_sway_ipc_read_finish(GInputStream *stream,
                                       GAsyncResult *result, guint32 *type,
                                       char **contents, gsize *length,
                                       GError **error) {
  GTask *task;
  SwayIpc *ipc;

  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(g_task_is_valid(result, stream), FALSE);

  task = G_TASK(result);

  if (!g_task_propagate_boolean(task, error)) {
    if (type)
      *type = 0;
    return FALSE;
  }

  ipc = g_task_get_task_data(task);

  if (type) {
    *type = ipc->type;
  }

  *contents = ipc->payload;
  ipc->payload = NULL;

  return g_task_propagate_boolean(task, error);
}

gboolean is_success(gssize size, const char *buf) { return size > 0; }

/**
 *g_barbar_sway_ipc_subscribe:
 */
BarBarSwayIpcAsyncData *
g_barbar_sway_subscribe_data_new(gpointer data,
                                 BarBarSwaySubscribeCallback callback) {
  BarBarSwayIpcAsyncData *adata = g_malloc0(sizeof(BarBarSwayIpcAsyncData));

  adata->header = g_malloc0(14 * sizeof(gchar));
  adata->data = data;
  adata->callback = callback;
  return adata;
}

static void callback_payload(GObject *object, GAsyncResult *result,
                             gpointer data) {
  GInputStream *input_stream;
  input_stream = G_INPUT_STREAM(object);
  GError *error = NULL;
  BarBarSwayIpcAsyncData *async = data;

  gssize bytes_read = g_input_stream_read_finish(input_stream, result, &error);
  if (bytes_read > 0) {
    async->callback(async->payload, async->plen, async->type, async->data);
    g_free(async->payload);
    g_input_stream_read_async(input_stream, async->header, 14,
                              G_PRIORITY_DEFAULT, NULL, callback_header, async);
  }
}

static void callback_header(GObject *object, GAsyncResult *result,
                            gpointer data) {
  GInputStream *input_stream;
  input_stream = G_INPUT_STREAM(object);
  GError *error = NULL;
  BarBarSwayIpcAsyncData *async = data;

  gssize bytes_read = g_input_stream_read_finish(input_stream, result, &error);
  if (bytes_read == 14) {

    memcpy(&async->plen, async->header + 6, 4);
    memcpy(&async->type, async->header + 10, 4);

    async->payload = g_malloc(async->plen);
    g_input_stream_read_async(input_stream, async->payload, async->plen,
                              G_PRIORITY_DEFAULT, NULL, callback_payload,
                              async);
  } else {
    printf("bytes_read: %ld\n", bytes_read);
    printf("ERROR!!!\n");
  }
}

// g_barbar_sway_ipc_read_finish(stream, res, &error);

gboolean is_success2(const char *buf, gssize len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, buf, len, &err);

  if (!ret) {
    g_printerr("Failed to parse json: %s", err->message);
    return FALSE;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "success");
  gboolean success = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  return success;
}
void event_cb(GObject *object, GAsyncResult *res, gpointer data) {
  GInputStream *stream = G_INPUT_STREAM(object);
  GError *error = NULL;
  GTask *task = data;

  SwayIpc *ipc = g_barbar_sway_ipc_read_finish(stream, res, &error);

  if (error) {
    // if we have get an error, we shutdown the loop
    g_task_return_error(task, error);
    return;
  }

  g_task_return_boolean(task, TRUE);
  g_barbar_sway_ipc_read_async(stream, NULL, event_cb, data);
}

void sub_cb(GObject *object, GAsyncResult *res, gpointer data) {
  GInputStream *stream = G_INPUT_STREAM(object);
  GError *error = NULL;
  GTask *task = data;
  char *str = NULL;
  gsize len;

  // g_task_had_error
  // if (g_task_propagate_error (task, error)) {

  // return FALSE;
  // }
  gboolean ret =
      g_barbar_sway_ipc_read_finish(stream, res, NULL, &str, &len, &error);

  if (error) {
    g_task_return_error(task, error);
    return;
  }

  if (is_success2(str, len)) {
    g_barbar_sway_ipc_read_async(stream, NULL, event_cb, data);
  }
  g_free(str);
}

GSocketConnection *g_barbar_sway_ipc_subscribe2(const char *intrest,
                                                GCancellable *cancellable,
                                                GAsyncReadyCallback callback,
                                                gpointer data) {
  GError *error = NULL;
  GInputStream *input_stream;
  GOutputStream *output_stream;
  GTask *task;

  GSocketConnection *con = g_barbar_sway_ipc_connect2(&error);

  task = g_task_new(input_stream, cancellable, callback, data);

  if (error) {
    // TODO: handle error
    // We should fire off a single callback with just the error
    g_task_return_error(task, error);
    return NULL;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(con));
  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(con));

  g_barbar_sway_ipc_send2(output_stream, SWAY_SUBSCRIBE, intrest);
  g_barbar_sway_ipc_read_async(input_stream, NULL, sub_cb, task);

  return con;
  // check that the content is success
  // if (is_success(len, buf)) {
  //   async = g_barbar_sway_subscribe_data_new(data, callback);
  //   ipc->subscribe_data = async;
  //   g_input_stream_read_async(input_stream, async->header, 14,
  //                             G_PRIORITY_DEFAULT, NULL, callback_header,
  //                             async);
  // }
}

void g_barbar_sway_ipc_subscribe(BarBarSwayIpc *ipc, const char *intrest,
                                 gpointer data,
                                 BarBarSwaySubscribeCallback callback) {
  char *buf = NULL;
  GError *error = NULL;
  gssize len;
  GInputStream *input_stream;
  BarBarSwayIpcAsyncData *async;
  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc->connection));

  g_barbar_sway_ipc_send(ipc, SWAY_SUBSCRIBE, intrest);
  len = g_barbar_sway_ipc_read(ipc, &buf, &error);
  // check that the content is success
  if (is_success(len, buf)) {
    async = g_barbar_sway_subscribe_data_new(data, callback);
    ipc->subscribe_data = async;
    g_input_stream_read_async(input_stream, async->header, 14,
                              G_PRIORITY_DEFAULT, NULL, callback_header, async);
  }
}
