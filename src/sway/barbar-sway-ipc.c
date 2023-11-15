#include "sway/barbar-sway-ipc.h"
#include <stdint.h>
#include <stdio.h>

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

void g_barbar_sway_ipc_read_get_header_async(GObject *object,
                                             GAsyncResult *result,
                                             gpointer data) {
  uint32_t type;
  uint32_t paylen;
  GInputStream *input_stream = G_INPUT_STREAM(object);
  GError *error = NULL;
  gchar *header;
  gssize bytes_read = g_input_stream_read_finish(input_stream, result, &error);
  header = g_async_result_get_user_data(result);

  if (bytes_read != 14) {
    return;
  }

  memcpy(&paylen, header + 6, 4);
  memcpy(&type, header + 10, 4);

  char *ppayload = malloc(paylen);

  // g_input_stream_read_async(input_stream, header, sizeof(header), 0,
  //                           cancallable, callback, data);

  // bytes_read = g_input_stream_read(input_stream, ppayload, paylen, NULL,
  // &error); if (bytes_read > 0) {
  //   *payload = ppayload;
  // } else {
  //   g_free(ppayload);
  // }
}

// void g_barbar_sway_ipc_read_header_async(GSocketConnection *connection,
//                                          GCancellable *cancallable,
//                                          GAsyncReadyCallback callback,
//                                          gpointer data) {
//   guint8 header[14];
//   gssize bytes_read;
//   gboolean ret;
//
//   GInputStream *input_stream;
//   input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
//
//   g_input_stream_read_async(input_stream, header, sizeof(header), 0,
//                             cancallable, callback, data);
// }

gboolean is_success(gssize size, const char *buf) { return size > 0; }

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
