#include "sway/barbar-sway-ipc.h"
#include <stdint.h>
#include <stdio.h>

GSocketConnection *g_barbar_sway_ipc_connect(GError **error) {
  GSocketClient *socket_client;
  GSocketConnection *connection;
  // GError *error = NULL;

  const char *socket_path = getenv("SWAYSOCK");

  if (!socket_path) {
    printf("apa!\n");
    // TODO: Error stuff
    return NULL;
  }

  socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  connection = g_socket_client_connect(
      socket_client, G_SOCKET_CONNECTABLE(address), NULL, error);

  if (connection == NULL || *error != NULL) {
    return NULL;
  }

  return connection;
  // g_io_stream_close(G_IO_STREAM(connection), NULL, NULL);
}

gboolean g_barbar_sway_ipc_send(GSocketConnection *connection, guint type,
                                const char *payload) {
  GOutputStream *output_stream;
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

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));

  ret = g_output_stream_write_all(output_stream, message, length, NULL, NULL,
                                  &err);
  return ret;
}

void g_barbar_sway_ipc_send_async(GSocketConnection *connection, guint type,
                                  const char *payload,
                                  GCancellable *cancallable,
                                  GAsyncReadyCallback callback, gpointer data) {
  GOutputStream *output_stream;
  uint32_t paylen;
  uint32_t length;
  char *message;

  paylen = strlen(payload);
  // magic-string + 4bytes payload length + 4bytes type
  length = 6 + 4 + 4 + paylen;
  message = calloc(length, sizeof(char));

  memcpy(message, "i3-ipc", 6);
  memcpy(message + 6, &paylen, sizeof(paylen));
  memcpy(message + 10, &type, sizeof(type));
  memcpy(message + 14, payload, paylen);

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));

  g_output_stream_write_all_async(output_stream, message, length, 0,
                                  cancallable, callback, data);
}

gssize g_barbar_sway_ipc_read(GSocketConnection *connection, gchar **payload,
                              GError **error) {
  guint8 header[14];
  gssize bytes_read;
  gboolean ret;

  GInputStream *input_stream;
  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

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

void g_barbar_sway_ipc_read_header_async(GSocketConnection *connection,
                                         GCancellable *cancallable,
                                         GAsyncReadyCallback callback,
                                         gpointer data) {
  guint8 header[14];
  gssize bytes_read;
  gboolean ret;

  GInputStream *input_stream;
  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

  g_input_stream_read_async(input_stream, header, sizeof(header), 0,
                            cancallable, callback, data);
}

void g_barbar_sway_ipc_subscribe(GSocketConnection *connection,
                                 const char *interest) {
  gchar *payload;
  g_barbar_sway_ipc_send(connection, SWAY_SUBSCRIBE, interest);
  g_barbar_sway_ipc_read(connection, &payload, NULL);
  printf("payload: %s\n", payload);
  while (TRUE) {
    g_barbar_sway_ipc_read(connection, &payload, NULL);
    printf("payload: %s\n", payload);
  }
}
