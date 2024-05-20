#include "sway/barbar-sway-ipc.h"
#include "barbar-error.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include <stdint.h>
#include <stdio.h>

typedef struct SwayIpc {
  guint32 type;
  guint8 header[14];
  gchar *payload;
  gssize length;
} SwayIpc;

static void g_sway_ipc_free(SwayIpc *ipc) {
  g_free(ipc->payload);
  g_free(ipc);
}

typedef struct SwaySendIpc {
  gsize bytes;
  char *message;
} SwaySendIpc;

void g_sway_async_send_free(SwaySendIpc *send) {
  g_free(send->message);
  g_free(send);
}

typedef struct SwayIpcCmd {
  GSocketClient *socket_client;
  char *message;
  GSocketConnection *connection;
} SwayIpcCmd;

static void g_sway_ipc_cmd_free(SwayIpcCmd *ipc) {
  if (ipc->socket_client) {
    g_object_unref((ipc->socket_client));
  }
  if (ipc->connection) {
    g_object_unref((ipc->socket_client));
  }
  g_free(ipc->message);
  g_free(ipc);
}

char *g_barbar_sway_ipc_get_header(SwayIpc *ipc) {
  char *str = g_malloc0(15);
  memcpy(str, ipc->header, 14);

  return str;
}

const char *g_barbar_sway_ipc_get_payload(SwayIpc *ipc) { return ipc->payload; }

guint32 g_barbar_sway_ipc_get_type(SwayIpc *ipc) { return ipc->type; }

/**
 * g_barbar_sway_ipc_connect:
 * @error: (out) (optional):  a #GError, or %NULL
 *
 * Create a new connection to the sway ipc.
 *
 * Returns: (transfer full): A #GSocketConnection or %NULL on failure
 */
GSocketConnection *g_barbar_sway_ipc_connect(GError **error) {
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

/**
 * g_barbar_sway_ipc_send:
 * @output_stream: a #GOutputStream to send data over
 * @type: type of message to send
 * @payload: data to send
 * @error: (out) (optional):  a #GError, or %NULL
 *
 * Send a message over the sway ipc.
 *
 * Returns: %TRUE on success and %FALSE on failure
 */
gboolean g_barbar_sway_ipc_send(GOutputStream *output_stream, guint type,
                                const char *payload, GError **error) {
  uint32_t paylen;
  uint32_t length;
  char *message;
  gboolean ret;

  paylen = strlen(payload);
  // magic-string + 4bytes payload length + 4bytes type
  length = 6 + 4 + 4 + paylen;
  message = calloc(length, sizeof(char));

  memcpy(message, "i3-ipc", 6);
  memcpy(message + 6, &paylen, sizeof(paylen));
  memcpy(message + 10, &type, sizeof(type));
  memcpy(message + 14, payload, paylen);

  ret = g_output_stream_write_all(output_stream, message, length, NULL, NULL,
                                  error);
  g_free(message);

  return ret;
}

void ipc_callback(GObject *source, GAsyncResult *res, gpointer data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source);
  gsize bytes;
  GError *error = NULL;
  GTask *task = data;

  g_output_stream_write_all_finish(stream, res, &bytes, &error);
  SwaySendIpc *send = g_task_get_task_data(task);

  if (error) {
    g_task_return_error(task, error);
    g_object_unref(task);
    return;
  }

  if (bytes < send->bytes) {
    g_task_return_new_error(task, BARBAR_ERROR, BARBAR_ERROR_BAD_SWAY_IPC,
                            "Not all bytes written");
    g_object_unref(task);
    return;
  }

  g_task_return_boolean(task, TRUE);
  g_object_unref(task);
}

gboolean g_barbar_sway_ipc_send_finish(GOutputStream *stream,
                                       GAsyncResult *result, GError **error) {
  GTask *task;
  // SwaySendIpc *ipc;

  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(g_task_is_valid(result, stream), FALSE);

  task = G_TASK(result);
  return g_task_propagate_boolean(task, error);
}

void g_barbar_sway_ipc_send_async(GOutputStream *output_stream, guint type,
                                  const char *payload,
                                  GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer data) {
  GTask *task;

  uint32_t paylen;
  uint32_t length;
  char *message;

  SwaySendIpc *ipc = g_malloc0(sizeof(SwaySendIpc));
  task = g_task_new(output_stream, cancellable, callback, data);
  g_task_set_task_data(task, ipc, (GDestroyNotify)g_sway_async_send_free);

  paylen = strlen(payload);
  // magic-string + 4bytes payload length + 4bytes type
  length = 6 + 4 + 4 + paylen;
  message = calloc(length, sizeof(char));

  memcpy(message, "i3-ipc", 6);
  memcpy(message + 6, &paylen, sizeof(paylen));
  memcpy(message + 10, &type, sizeof(type));
  memcpy(message + 14, payload, paylen);
  ipc->bytes = paylen;
  ipc->message = message;

  g_output_stream_write_all_async(output_stream, message, length, 0,
                                  cancellable, callback, task);
}

void printf_cb(GObject *source, GAsyncResult *res, gpointer data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source);
  GError *error = NULL;
  GTask *task = data;

  g_barbar_sway_ipc_send_finish(stream, res, &error);

  if (error) {
    g_task_return_error(task, error);
    g_object_unref(task);
    return;
  }

  char *str = g_task_get_task_data(task);

  g_task_return_int(task, strlen(str));
  g_object_unref(task);
}

gsize g_barbar_sway_ipc_send_printf_finish(GOutputStream *stream,
                                           GAsyncResult *result,
                                           GError **error) {
  GTask *task;

  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(g_task_is_valid(result, stream), FALSE);

  task = G_TASK(result);
  return g_task_propagate_int(task, error);
}
void g_barbar_sway_ipc_send_vprintf_async(GOutputStream *output_stream,
                                          guint type, GCancellable *cancellable,
                                          GAsyncReadyCallback callback,
                                          gpointer data, const char *format,
                                          va_list args) {
  GTask *task;
  task = g_task_new(output_stream, cancellable, callback, data);

  char *buffer;
  buffer = g_strdup_vprintf(format, args);

  g_task_set_task_data(task, buffer, (GDestroyNotify)g_free);
  g_barbar_sway_ipc_send_async(output_stream, type, buffer, cancellable,
                               printf_cb, task);
}

void g_barbar_sway_ipc_send_printf_async(GOutputStream *output_stream,
                                         guint type, GCancellable *cancellable,
                                         GAsyncReadyCallback callback,
                                         gpointer data, const char *format,
                                         ...) {
  va_list args;

  va_start(args, format);
  g_barbar_sway_ipc_send_vprintf_async(output_stream, type, cancellable,
                                       callback, data, format, args);
  va_end(args);
}

/**
 * g_barbar_sway_ipc_read:
 * @input_stream: a #GOutputStream to send data over
 * @type: (out) (optional): type of message to send
 * @payload: (out) (transfer full): data to send
 * @length: (out) (optional): length of the massage
 * @error: (out) (optional):  a #GError, or %NULL
 *
 * Read a message from the sway ipc, synchronously.
 *
 * Returns: The number of bytes read
 */
gssize g_barbar_sway_ipc_read(GInputStream *input_stream, guint32 *type,
                              gchar **payload, gsize *length, GError **error) {
  guint8 header[14];
  gssize bytes_read;

  bytes_read =
      g_input_stream_read(input_stream, header, sizeof(header), NULL, error);

  if (bytes_read != 14) {
    g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_SWAY_IPC,
                "Bad header length");
    return -1;
  }

  uint32_t mtype;
  uint32_t paylen;

  memcpy(&paylen, header + 6, 4);
  memcpy(&mtype, header + 10, 4);

  char *ppayload = malloc(paylen);

  bytes_read = g_input_stream_read(input_stream, ppayload, paylen, NULL, error);
  if (bytes_read > 0) {
    *payload = ppayload;
    if (length) {
      *length = bytes_read;
    }
    if (type) {
      *type = mtype;
    }
  } else {
    g_free(ppayload);
  }

  return bytes_read;
}

static void read_payload_cb(GObject *source, GAsyncResult *res, gpointer data) {
  gssize size;
  GError *error = NULL;
  GTask *task = data;
  GInputStream *stream = G_INPUT_STREAM(source);

  size = g_input_stream_read_finish(stream, res, &error);

  if (error) {
    g_task_return_error(task, error);
    return;
  }

  g_task_return_boolean(task, TRUE);
  g_object_unref(task);
}

static void read_header_cb(GObject *source, GAsyncResult *res, gpointer data) {
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
    g_object_unref(task);
    return;
  }

  guint32 type;
  guint32 paylen;

  memcpy(&paylen, ipc->header + 6, 4);
  memcpy(&type, ipc->header + 10, 4);

  ipc->type = type;
  ipc->payload = g_malloc0(paylen);
  ipc->length = paylen;

  g_input_stream_read_async(stream, ipc->payload, paylen, 8192, NULL,
                            read_payload_cb, data);
}

/**
 * g_barbar_sway_ipc_read_async:
 * @input_stream: a #GInputStream
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: (scope async): a #GAsyncReadyCallback
 *     to call when the request is satisfied
 * @data: the data to pass to callback function
 *
 * Read data from the sway ipc async
 *
 */
void g_barbar_sway_ipc_read_async(GInputStream *input_stream,
                                  GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer data) {
  GTask *task;
  SwayIpc *ipc = g_malloc0(sizeof(SwayIpc));

  task = g_task_new(input_stream, cancellable, callback, data);
  g_task_set_task_data(task, ipc, (GDestroyNotify)g_sway_ipc_free);

  g_input_stream_read_async(input_stream, ipc->header, 14, 8192, cancellable,
                            read_header_cb, task);
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
  gboolean res;

  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(g_task_is_valid(result, stream), FALSE);

  task = G_TASK(result);
  res = g_task_propagate_boolean(task, error);
  if (!res) {
    if (type) {
      *type = 0;
    }
    if (length) {
      *length = 0;
    }
    return FALSE;
  }

  ipc = g_task_get_task_data(task);

  if (type) {
    *type = ipc->type;
  }

  *contents = ipc->payload;
  *length = ipc->length;
  ipc->payload = NULL;

  return res;
}

/**
 * g_barbar_sway_message_is_success:
 * @buf: a string containing a json message
 * @len: the length of the buffer
 *
 * Check the json for success. Usefull for messages that just a
 * return a single success value, like subscribe.
 *
 * Returns: %TRUE if the response is a success
 */
gboolean g_barbar_sway_message_is_success(const char *buf, gssize len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, buf, len, &err);

  if (!ret) {
    g_printerr("Failed to parse json: %s", err->message);
    g_object_unref(parser);
    return FALSE;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "success");
  gboolean success = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  g_object_unref(reader);
  g_object_unref(parser);

  return success;
}

static void cmd_close(GObject *source, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  SwayIpcCmd *ipc = data;
  g_io_stream_close_finish(G_IO_STREAM(source), res, &error);

  if (error) {
    g_printerr("Couldn't close socket: %s\n", error->message);
    g_error_free(error);
  }
  g_sway_ipc_cmd_free(ipc);
}

static void cmd_cb(GObject *source, GAsyncResult *res, gpointer data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source);
  SwayIpcCmd *ipc = data;
  GError *error = NULL;

  g_barbar_sway_ipc_send_finish(stream, res, &error);

  if (error) {
    g_printerr("Couldn't send message: %s\n", error->message);
    g_error_free(error);
  }

  g_io_stream_close_async(G_IO_STREAM(ipc->socket_client), 0, NULL, cmd_close,
                          ipc);
  g_sway_ipc_cmd_free(ipc);
}

static void cmd_connect(GObject *source, GAsyncResult *res, gpointer data) {
  SwayIpcCmd *ipc = data;
  GError *error = NULL;
  GSocketClient *socket_client = G_SOCKET_CLIENT(source);
  GOutputStream *output_stream;

  ipc->connection = g_socket_client_connect_finish(socket_client, res, &error);

  if (error) {
    g_sway_ipc_cmd_free(ipc);
    g_error_free(error);
    return;
  }
  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(socket_client));

  g_barbar_sway_ipc_send_async(output_stream, SWAY_RUN_COMMAND, ipc->message,
                               NULL, cmd_cb, ipc);
}

void g_barbar_sway_ipc_command(const char *format, ...) {
  va_list args;
  // GTask *task;
  SwayIpcCmd *ipc = g_malloc0(sizeof(SwayIpcCmd));

  const char *socket_path = getenv("SWAYSOCK");

  if (!socket_path) {
    // TODO: Error stuff
    return;
  }

  ipc->socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  va_start(args, format);
  ipc->message = g_strdup_vprintf(format, args);
  va_end(args);

  g_socket_client_connect_async(ipc->socket_client,
                                G_SOCKET_CONNECTABLE(address), NULL,
                                cmd_connect, ipc);
}
