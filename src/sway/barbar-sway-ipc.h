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

#ifndef _BARBAR_SWAY_IPC_H_
#define _BARBAR_SWAY_IPC_H_

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <stdint.h>

G_BEGIN_DECLS

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

typedef struct _BarBarSwayIpc BarBarSwayIpc;
typedef struct _BarBarSwayIpcAsyncData BarBarSwayIpcAsyncData;

typedef void (*BarBarSwaySubscribeCallback)(gchar *payload, uint32_t length,
                                            uint32_t type, gpointer data);

struct _BarBarSwayIpc {
  GSocketConnection *connection;

  BarBarSwayIpcAsyncData *subscribe_data;
};

struct _BarBarSwayIpcAsyncData {
  // A pointer to the class pointer
  // A bit like a parrent pointer
  gpointer *data;
  uint32_t type;
  uint32_t plen;
  gchar *header;

  gchar *payload;
  BarBarSwaySubscribeCallback callback;
};

gboolean g_barbar_sway_ipc_send(BarBarSwayIpc *ipc, guint type,
                                const char *payload);

gssize g_barbar_sway_ipc_read(BarBarSwayIpc *ipc, gchar **payload,
                              GError **error);

void g_barbar_sway_ipc_subscribe(BarBarSwayIpc *ipc, const char *payload,
                                 gpointer data,
                                 BarBarSwaySubscribeCallback callback);

BarBarSwayIpc *g_barbar_sway_ipc_connect(GError **err);

G_END_DECLS

#endif /* _BARBAR_SWAY_IPC_H_ */
