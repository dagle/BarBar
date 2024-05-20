/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Per Odlund
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <stdint.h>

G_BEGIN_DECLS

// request type
typedef enum {
  SWAY_RUN_COMMAND,
  SWAY_GET_WORKSPACES,
  SWAY_SUBSCRIBE,
  SWAY_GET_OUTPUTS,
  SWAY_GET_TREE,
  SWAY_GET_MARKS,
  SWAY_GET_BAR_CONFIG,
  SWAY_GET_VERSION,
  SWAY_GET_BINDING_MODES,
  SWAY_GET_CONFIG,
  SWAY_TICK,
  SWAY_SEND_TICK,
  SWAY_SYNC,
  SWAY_GET_BINDING_STATE,
  SWAY_GET_INPUTS = 100,
  SWAY_GET_SEATS = 101,
} BarBarSwayMessageType;

typedef enum {
  SWAY_WORKSPACE_EVENT = 0x80000000,
  SWAY_OUTPUT_EVENT = 0x80000001,
  SWAY_MODE_EVENT = 0x80000002,
  SWAY_WINDOW_EVENT = 0x80000003,
  SWAY_BARCONFIG_EVENT = 0x80000004,
  SWAY_BINDING_EVENT = 0x80000005,
  SWAY_SHUTDOWN_EVENT = 0x80000006,
  SWAY_TICK_EVENT = 0x80000007,
  SWAY_BARSTATE_EVENT = 0x80000014,
  SWAY_INPUT_EVENT = 0x80000015,
} BarBarSwayEventType;

typedef struct _BarBarSwayIpc BarBarSwayIpc;

GSocketConnection *g_barbar_sway_ipc_connect(GError **error);

gboolean g_barbar_sway_ipc_send(GOutputStream *output_stream, guint type,
                                const char *payload, GError **error);

gssize g_barbar_sway_ipc_read(GInputStream *input_stream, guint32 *type,
                              gchar **payload, gsize *length, GError **error);

void g_barbar_sway_ipc_send_async(GOutputStream *output_stream, guint type,
                                  const char *payload,
                                  GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer data);

gboolean g_barbar_sway_ipc_send_finish(GOutputStream *stream,
                                       GAsyncResult *result, GError **error);

gsize g_barbar_sway_ipc_send_printf_finish(GOutputStream *stream,
                                           GAsyncResult *result,
                                           GError **error);

void g_barbar_sway_ipc_send_vprintf_async(GOutputStream *output_stream,
                                          guint type, GCancellable *cancellable,
                                          GAsyncReadyCallback callback,
                                          gpointer data, const char *format,
                                          va_list args);

void g_barbar_sway_ipc_send_printf_async(GOutputStream *output_stream,
                                         guint type, GCancellable *cancellable,
                                         GAsyncReadyCallback callback,
                                         gpointer data, const char *format,
                                         ...);

void g_barbar_sway_ipc_read_async(GInputStream *input_stream,
                                  GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer data);

gboolean g_barbar_sway_ipc_read_finish(GInputStream *stream,
                                       GAsyncResult *result, guint32 *type,
                                       char **contents, gsize *length,
                                       GError **error);

gboolean g_barbar_sway_message_is_success(const char *buf, gssize len);

void g_barbar_sway_ipc_command(const char *format, ...);

G_END_DECLS
