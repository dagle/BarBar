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

#include "json-glib/json-glib.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <stdint.h>

G_BEGIN_DECLS

/* typedef struct _BarBarHyprlandIpc BarBarHyprlandIpc; */
/* typedef struct _BarBarHyprlandIpcAsyncData BarBarHyprlandIpcAsyncData; */

typedef void (*BarBarHyprlandSubscribeCallback)(uint32_t type, char *args,
                                                gpointer data);

// event type
typedef enum {
  HYPRLAND_WORKSPACE,
  HYPRLAND_WORKSPACEV2,
  HYPRLAND_FOCUSEDMON,
  HYPRLAND_ACTIVEWINDOW,
  HYPRLAND_ACTIVEWINDOWV2,
  HYPRLAND_FULLSCREEN,
  HYPRLAND_MONITORREMOVED,
  HYPRLAND_MONITORADDED,
  HYPRLAND_MONITORADDEDV2,
  HYPRLAND_CREATEWORKSPACE,
  HYPRLAND_CREATEWORKSPACEV2,
  HYPRLAND_DESTROYWORKSPACE,
  HYPRLAND_DESTROYWORKSPACEV2,
  HYPRLAND_MOVEWORKSPACE,
  HYPRLAND_MOVEWORKSPACEV2,
  HYPRLAND_RENAMEWORKSPACE,
  HYPRLAND_ACTIVESPECIAL,
  HYPRLAND_ACTIVELAYOUT,
  HYPRLAND_OPENWINDOW,
  HYPRLAND_CLOSEWINDOW,
  HYPRLAND_MOVEWINDOW,
  HYPRLAND_MOVEWINDOWV2,
  HYPRLAND_OPENLAYER,
  HYPRLAND_CLOSELAYER,
  HYPRLAND_SUBMAP,
  HYPRLAND_CHANGEFLOATINGMODE,
  HYPRLAND_URGENT,
  HYPRLAND_MINIMIZE,
  HYPRLAND_SCREENCAST,
  HYPRLAND_WINDOWTITLE,
  HYPRLAND_IGNOREGROUPLOCK,
  HYPRLAND_LOCKGROUPS
} BarBarHyprlandEvent;

struct BarBarHyprlandListner {
  /* GSocketConnection *connection; */
  BarBarHyprlandSubscribeCallback cb;
  gpointer *data;
};

#define BARBAR_HYPERLAND_REQUEST_SOCKET ".socket.sock"
#define BARBAR_HYPERLAND_EVENT_SOCKET ".socket2.sock"

GSocketAddress *g_barbar_hyprland_ipc_address(const char *socket,
                                              GError **error);

GSocketConnection *g_barbar_hyprland_ipc_controller(GError **err);

gboolean g_barbar_hyprland_ipc_send_command(GSocketConnection *ipc, char *msg,
                                            GError **err);

gchar *g_barbar_hyprland_ipc_message_resp(GSocketConnection *ipc, GError **err);

GSocketConnection *
g_barbar_hyprland_ipc_listner(BarBarHyprlandSubscribeCallback cb, gpointer data,
                              GDestroyNotify destroy, GError **error);

void g_barbar_hyprland_ipc_oneshot(GCancellable *cancellable,
                                   GAsyncReadyCallback callback,
                                   const char *msg, gpointer data);

JsonParser *g_barbar_hyprland_ipc_oneshot_finish(GAsyncResult *result,
                                                 GError **error);

G_END_DECLS
