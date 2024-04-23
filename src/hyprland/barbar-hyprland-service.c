#include "hyprland/barbar-hyprland-service.h"
#include "barbar-error.h"
#include <gio/gio.h>
#include <gio/gunixinputstream.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

struct _BarBarHyprlandService {
  GObject parent_instance;

  GSocketClient *socket_client;
  GSocketConnection *connection;
  GInputStream *input_stream;
  GDataInputStream *data_stream;

  char *submap;
  char *keyboard;
  char *layout;
};

enum {
  WORKSPACEV2,
  FOCUSEDMON,
  ACTIVEWINDOW,
  ACTIVEWINDOWV2,
  FULLSCREEN,
  MONITORREMOVED,
  MONITORADDEDV2,
  CREATEWORKSPACEV2,
  DESTROYWORKSPACEV2,
  MOVEWORKSPACEV2,
  RENAMEWORKSPACE,
  ACTIVESPECIAL,
  ACTIVELAYOUT,
  OPENWINDOW,
  CLOSEWINDOW,
  MOVEWINDOWV2,
  OPENLAYER,
  CLOSELAYER,
  SUBMAP,
  CHANGEFLOATINGMODE,
  URGENT,
  MINIMIZE,
  SCREENCAST,
  WINDOWTITLE,
  IGNOREGROUPLOCK,
  LOCKGROUPS,
  CONFIGRELOADED,
  PIN,
  NUM_SIGNALS,
};

static guint hyprland_service_signals[NUM_SIGNALS];

G_DEFINE_TYPE(BarBarHyprlandService, g_barbar_hyprland_service, G_TYPE_OBJECT)

enum {
  PROP_0,

  PROP_SUBMAP,
  PROP_KEYBOARD,
  PROP_LAYOUT,

  NUM_PROPERTIES,
};

static GParamSpec *hypr_service_props[NUM_PROPERTIES] = {
    NULL,
};

static BarBarHyprlandService *the_singleton = NULL;

static void g_babrab_hyprland_service_set_submap(BarBarHyprlandService *service,
                                                 const char *submap) {
  g_return_if_fail(BARBAR_IS_HYPRLAND_SERVICE(service));

  if (!g_strcmp0(service->submap, submap)) {
    return;
  }

  g_free(service->submap);
  service->submap = g_strdup(submap);

  g_object_notify_by_pspec(G_OBJECT(service), hypr_service_props[PROP_SUBMAP]);
}

static void
g_babrab_hyprland_service_set_keyboard(BarBarHyprlandService *service,
                                       const char *keyboard) {
  g_return_if_fail(BARBAR_IS_HYPRLAND_SERVICE(service));

  if (!g_strcmp0(service->keyboard, keyboard)) {
    return;
  }

  g_free(service->keyboard);
  service->keyboard = g_strdup(keyboard);

  g_object_notify_by_pspec(G_OBJECT(service),
                           hypr_service_props[PROP_KEYBOARD]);
}

static void g_babrab_hyprland_service_set_layout(BarBarHyprlandService *service,
                                                 const char *layout) {
  g_return_if_fail(BARBAR_IS_HYPRLAND_SERVICE(service));

  if (!g_strcmp0(service->layout, layout)) {
    return;
  }

  g_free(service->submap);
  service->layout = g_strdup(layout);

  g_object_notify_by_pspec(G_OBJECT(service), hypr_service_props[PROP_LAYOUT]);
}

static GObject *
g_barbar_hyprland_service_constructor(GType type, guint n_construct_params,
                                      GObjectConstructParam *construct_params) {
  GObject *object;

  if (!the_singleton) {
    object = G_OBJECT_CLASS(g_barbar_hyprland_service_parent_class)
                 ->constructor(type, n_construct_params, construct_params);
    the_singleton = BARBAR_HYPRLAND_SERVICE(object);
  } else
    object = g_object_ref(G_OBJECT(the_singleton));

  return object;
}

static void g_barbar_hyprland_service_set_property(GObject *object,
                                                   guint property_id,
                                                   const GValue *value,
                                                   GParamSpec *pspec) {
  BarBarHyprlandService *service = BARBAR_HYPRLAND_SERVICE(object);
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_hyprland_service_get_property(GObject *object,
                                                   guint property_id,
                                                   GValue *value,
                                                   GParamSpec *pspec) {

  BarBarHyprlandService *service = BARBAR_HYPRLAND_SERVICE(object);

  switch (property_id) {
  case PROP_SUBMAP:
    g_value_set_string(value, service->submap);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

#define EVENT_TYPE(input, name, stmts)                                         \
  if (!strncmp(input, #name ">>", sizeof(#name) + 1)) {                        \
    args = line + sizeof(#name) + 1;                                           \
    stmts;                                                                     \
    goto end;                                                                  \
  }

static void g_barbar_hyprland_line_reader(GObject *object, GAsyncResult *res,
                                          gpointer data) {
  GDataInputStream *data_stream = G_DATA_INPUT_STREAM(object);
  BarBarHyprlandService *service = BARBAR_HYPRLAND_SERVICE(data);

  GError *error = NULL;
  gsize length;
  gchar *line;

  line =
      g_data_input_stream_read_line_finish(data_stream, res, &length, &error);

  const char *args;

  EVENT_TYPE(line, workspacev2, {
    // we could do this inplace
    gchar **tokens = g_strsplit(args, ",", -1);
    int id = atoi(tokens[0]);

    g_signal_emit(service, hyprland_service_signals[WORKSPACEV2], 0, id,
                  tokens[1]);

    g_strfreev(tokens);
  });

  EVENT_TYPE(line, focusedmon, {
    gchar **tokens = g_strsplit(args, ",", -1);

    g_signal_emit(service, hyprland_service_signals[FOCUSEDMON], 0, tokens[0],
                  tokens[1]);

    g_strfreev(tokens);
  });

  EVENT_TYPE(line, activewindow, {
    gchar **tokens = g_strsplit(args, ",", -1);

    g_signal_emit(service, hyprland_service_signals[ACTIVEWINDOW], 0, tokens[0],
                  tokens[1]);

    g_strfreev(tokens);
  });

  EVENT_TYPE(line, activewindowv2, {
    g_signal_emit(service, hyprland_service_signals[ACTIVEWINDOWV2], 0, args);
  });

  EVENT_TYPE(line, fullscreen, {
    int id = atoi(args);

    g_signal_emit(service, hyprland_service_signals[FULLSCREEN], 0, id);
  });

  EVENT_TYPE(line, monitorremoved, {
    g_signal_emit(service, hyprland_service_signals[MONITORREMOVED], 0, args);
  });

  EVENT_TYPE(line, monitoraddedv2, {
    gchar **tokens = g_strsplit(args, ",", -1);

    g_signal_emit(service, hyprland_service_signals[MONITORADDEDV2], 0,
                  tokens[0], tokens[1], tokens[2]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, createworkspacev2, {
    gchar **tokens = g_strsplit(args, ",", -1);
    int id = atoi(tokens[0]);

    g_signal_emit(service, hyprland_service_signals[CREATEWORKSPACEV2], 0, id,
                  tokens[1]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, destroyworkspacev2, {
    gchar **tokens = g_strsplit(args, ",", -1);
    int id = atoi(tokens[0]);

    g_signal_emit(service, hyprland_service_signals[DESTROYWORKSPACEV2], 0, id,
                  tokens[1]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, moveworkspacev2, {
    gchar **tokens = g_strsplit(args, ",", -1);
    int id = atoi(tokens[0]);

    g_signal_emit(service, hyprland_service_signals[MOVEWORKSPACEV2], 0, id,
                  tokens[1], tokens[2]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, renameworkspace, {
    gchar **tokens = g_strsplit(args, ",", -1);
    int id = atoi(tokens[0]);

    g_signal_emit(service, hyprland_service_signals[RENAMEWORKSPACE], 0, id,
                  tokens[1]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, activespecial, {
    gchar **tokens = g_strsplit(args, ",", -1);

    g_signal_emit(service, hyprland_service_signals[ACTIVESPECIAL], 0,
                  tokens[0], tokens[1], tokens[2]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, activelayout, {
    gchar **tokens = g_strsplit(args, ",", -1);

    g_babrab_hyprland_service_set_keyboard(service, tokens[0]);
    g_babrab_hyprland_service_set_layout(service, tokens[1]);

    g_strfreev(tokens);
  });

  EVENT_TYPE(line, openwindow, {
    gchar **tokens = g_strsplit(args, ",", -1);

    g_signal_emit(service, hyprland_service_signals[OPENWINDOW], 0, tokens[0],
                  tokens[1], tokens[2], tokens[3]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, closewindow, {
    g_signal_emit(service, hyprland_service_signals[CLOSEWINDOW], 0, args);
  });

  EVENT_TYPE(line, movewindowv2, {
    gchar **tokens = g_strsplit(args, ",", -1);

    g_signal_emit(service, hyprland_service_signals[MOVEWINDOWV2], 0, tokens[0],
                  tokens[1], tokens[2]);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, openlayer, {
    g_signal_emit(service, hyprland_service_signals[OPENLAYER], 0, args);
  });

  EVENT_TYPE(line, closelayer, {
    g_signal_emit(service, hyprland_service_signals[CLOSELAYER], 0, args);
  });

  EVENT_TYPE(line, submap, {
    // TODO:check for null
    // g_signal_emit(service, hyprland_service_signals[SUBMAP], 0, args);

    g_babrab_hyprland_service_set_submap(service, args);

    // g_object_notify_by_pspec(G_OBJECT(service),
    //                          hypr_service_props[PROP_SUBMAP]);
  });

  EVENT_TYPE(line, changefloatingmode, {
    int mode = atoi(args);
    g_signal_emit(service, hyprland_service_signals[CHANGEFLOATINGMODE], 0,
                  mode);
  });

  EVENT_TYPE(line, urgent, {
    g_signal_emit(service, hyprland_service_signals[URGENT], 0, args);
  });

  EVENT_TYPE(line, minimize, {
    gchar **tokens = g_strsplit(args, ",", -1);
    int mini = atoi(tokens[1]);

    g_signal_emit(service, hyprland_service_signals[MINIMIZE], 0, tokens[0],
                  mini);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, screencast, {
    gchar **tokens = g_strsplit(args, ",", -1);
    int state = atoi(tokens[0]);
    int window = atoi(tokens[1]);

    g_signal_emit(service, hyprland_service_signals[SCREENCAST], 0, state,
                  window);
    g_strfreev(tokens);
  });

  EVENT_TYPE(line, windowtitle, {
    g_signal_emit(service, hyprland_service_signals[WINDOWTITLE], 0, args);
  });

  EVENT_TYPE(line, ignoregrouplock, {
    int state = atoi(args);

    g_signal_emit(service, hyprland_service_signals[IGNOREGROUPLOCK], 0, state);
  });

  EVENT_TYPE(line, lockgroups, {
    int state = atoi(args);
    g_signal_emit(service, hyprland_service_signals[LOCKGROUPS], 0, state);
  });

  EVENT_TYPE(line, configreloaded, {
    g_signal_emit(service, hyprland_service_signals[CONFIGRELOADED], 0);
  });

  EVENT_TYPE(line, pin, {
    gchar **tokens = g_strsplit(args, ",", -1);
    int state = atoi(tokens[1]);

    g_signal_emit(service, hyprland_service_signals[PIN], 0, tokens[0], state);
    g_strfreev(tokens);
  });

end:
  g_data_input_stream_read_line_async(data_stream, G_PRIORITY_DEFAULT, NULL,
                                      g_barbar_hyprland_line_reader, data);
}

static void g_barbar_hyprland_service_constructed(GObject *self) {
  BarBarHyprlandService *service = BARBAR_HYPRLAND_SERVICE(self);
  GError *error = NULL;

  const char *his = getenv("HYPRLAND_INSTANCE_SIGNATURE");

  if (!his) {
    g_printerr("HYPRLAND_INSTANCE_SIGNATURE not set\n");
    return;
  }

  char *socket_path = g_strdup_printf("/tmp/hypr/%s/.socket2.sock", his);

  service->socket_client = g_socket_client_new();
  GSocketAddress *address = g_unix_socket_address_new(socket_path);

  service->connection = g_socket_client_connect(
      service->socket_client, G_SOCKET_CONNECTABLE(address), NULL, &error);
  g_object_unref(service->socket_client);
  g_object_unref(address);

  g_free(socket_path);

  if (!service->connection) {
    return;
  }

  service->input_stream =
      g_io_stream_get_input_stream(G_IO_STREAM(service->connection));
  service->data_stream = g_data_input_stream_new(service->input_stream);

  g_data_input_stream_read_line_async(service->data_stream, G_PRIORITY_DEFAULT,
                                      NULL, g_barbar_hyprland_line_reader,
                                      service);
}
// static void g_barbar_service_start(BarBarSensor *sensor) {}

static void
g_barbar_hyprland_service_class_init(BarBarHyprlandServiceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  // BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_hyprland_service_set_property;
  gobject_class->get_property = g_barbar_hyprland_service_get_property;

  gobject_class->constructor = g_barbar_hyprland_service_constructor;
  gobject_class->constructed = g_barbar_hyprland_service_constructed;
  // sensor_class->start = g_barbar_service_start;

  hypr_service_props[PROP_SUBMAP] =
      g_param_spec_string("submap", NULL, NULL, NULL, G_PARAM_READABLE);
  hypr_service_props[PROP_KEYBOARD] =
      g_param_spec_string("keyboard", NULL, NULL, NULL, G_PARAM_READABLE);
  hypr_service_props[PROP_LAYOUT] =
      g_param_spec_string("layout", NULL, NULL, NULL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    hypr_service_props);

  /**
   * BarBarHyprlandService::workspace:
   * @service: this object
   * @id: id of the workspace
   * @name: name of the workspace
   *
   * emitted on workspace change. Is emitted ONLY when a user requests a
   * workspace change, and is not emitted on mouse movements (see activemon)
   */
  hyprland_service_signals[WORKSPACEV2] = g_signal_new(
      "workspace", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::focused-monitor:
   * @service: this object
   * @monitorname: name of the monitor
   * @workspacename: name of the workspace
   *
   * emitted on the active monitor being changed.
   */
  hyprland_service_signals[FOCUSEDMON] = g_signal_new(
      "focused-monitor", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::active-window:
   * @service: this object
   * @class: class name of the active window
   * @title: title of the active window
   *
   * emitted on the active window being changed.
   */
  hyprland_service_signals[ACTIVEWINDOW] = g_signal_new(
      "active-window", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  // TODO:
  /**
   * BarBarHyprlandService::active-window-v2:
   * @service: this object
   * @address: Address of the current window
   *
   * emitted on the active window being changed.
   */
  hyprland_service_signals[ACTIVEWINDOWV2] =
      g_signal_new("active-window-v2", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

  // TODO:
  /**
   * BarBarHyprlandService::fullscreen:
   * @service: this object
   * @fullscreen: %TRUE if window is fullscreen
   *
   * emitted when a fullscreen status of a window changes.
   */
  hyprland_service_signals[FULLSCREEN] =
      g_signal_new("fullscreen", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  /**
   * BarBarHyprlandService::monitor-removed:
   * @service: this object
   * @name: class name of the removed movitor
   *
   * emitted when a monitor is removed (disconnected)
   */
  hyprland_service_signals[MONITORREMOVED] =
      g_signal_new("monitor-removed", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::monitor-added:
   * @service: this object
   * @id: id of monitor
   * @name: name of monitor
   * @description: monitor description
   *
   * emitted when a monitor is added (connected)
   */
  hyprland_service_signals[MONITORADDEDV2] = g_signal_new(
      "monitor-added", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::create-workspace:
   * @service: this object
   * @id: class name of the created workspace
   * @name: title of the created workspcace
   *
   * emitted when a workspace is created
   */
  hyprland_service_signals[CREATEWORKSPACEV2] = g_signal_new(
      "create-workspace", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::destroy-workspace:
   * @service: this object
   * @id: class name of the destroyed workspace
   * @name: title of the destroyed workspcace
   *
   * emitted when a workspace is destroyed
   */
  hyprland_service_signals[DESTROYWORKSPACEV2] = g_signal_new(
      "destroy-workspace", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::move-workspace:
   * @service: this object
   * @id: id of the workspace
   * @workspacename: name of the workspace
   * @monitorname: name of the monitor
   *
   * emitted when a workspace is moved to a different monitor.
   */
  hyprland_service_signals[MOVEWORKSPACEV2] = g_signal_new(
      "move-workspace", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::rename-workspace:
   * @service: this object
   * @id: id of the workspace
   * @newname: new name of the workspace
   *
   * emitted when a workspace is renamed
   */
  hyprland_service_signals[RENAMEWORKSPACE] = g_signal_new(
      "rename-workspace", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::activespecial:
   * @service: this object
   * @workspacename: name of the workspace
   * @monitorname: name of the monitor
   *
   * emitted when the special workspace opened in a monitor changes (closing
   * results in an empty WORKSPACENAME)
   */
  hyprland_service_signals[ACTIVESPECIAL] = g_signal_new(
      "activespecial", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::activelayout:
   * @service: this object
   * @keyboardname: name of keyboard device
   * @layoutname: name of keyboard layout.
   *
   * emitted on a layout change of the active keyboard
   */
  hyprland_service_signals[ACTIVELAYOUT] = g_signal_new(
      "active-layout", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::open-window:
   * @service: this object
   * @address: address of the window
   * @workspacename: name of the workspace
   * @class: window class
   * @title: window title
   *
   * emitted when a window is opened
   */
  hyprland_service_signals[OPENWINDOW] =
      g_signal_new("open-window", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 4, G_TYPE_STRING,
                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::close-window:
   * @service: this object
   * @address: address of the window
   *
   * emitted when a window is closed
   */
  hyprland_service_signals[CLOSEWINDOW] =
      g_signal_new("close-window", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::move-window:
   * @service: this object
   * @windowaddress: address of the window
   * @workspaceid: id of workspace
   * @workspacename: id of workspace
   *
   * emitted when a window is moved to a workspace
   */
  hyprland_service_signals[MOVEWINDOWV2] = g_signal_new(
      "move-window", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::open-layer:
   * @service: this object
   * @namespace: name of created layer
   *
   * emitted when a layerSurface is mapped
   */
  hyprland_service_signals[OPENLAYER] =
      g_signal_new("open-layer", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
  /**
   * BarBarHyprlandService::close-layer:
   * @service: this object
   * @namespace: name of closed layer
   *
   * emitted when a layerSurface is unmapped
   */
  hyprland_service_signals[CLOSELAYER] =
      g_signal_new("close-layer", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
  /**
   * BarBarHyprlandService::floating-mode:
   * @service: this object
   * @mode: %TRUE if current mode is floating.
   *
   * emitted when a window changes its floating mode.
   */
  hyprland_service_signals[CHANGEFLOATINGMODE] =
      g_signal_new("floating-mode", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  /**
   * BarBarHyprlandService::urgent:
   * @service: this object
   * @address: address of urgent window
   *
   * emitted when a window requests an urgent state
   */
  hyprland_service_signals[URGENT] =
      g_signal_new("urgent", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::minimize:
   * @service: this object
   * @address: address of urgent window
   * @minimized: %TRUE if the window was minimized
   *
   * emitted when a window requests a change to its minimized state.
   */
  hyprland_service_signals[MINIMIZE] = g_signal_new(
      "minimized", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_BOOLEAN);

  /**
   * BarBarHyprlandService::screencast:
   * @service: this object
   * @state: %TRUE if we started to cast
   * @window: %TRUE if casting a window, %FALSE is casting the screen
   *
   * emitted when a screencopy state of a client changes. Keep in mind there
   * might be multiple separate clients.
   */
  hyprland_service_signals[SCREENCAST] = g_signal_new(
      "screencast", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

  /**
   * BarBarHyprlandService::window-title:
   * @service: this object
   * @address: address of the changed window
   *
   * emitted when a window title changes.
   */
  hyprland_service_signals[WINDOWTITLE] =
      g_signal_new("window-title", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
   * BarBarHyprlandService::ignore-grouplock:
   * @service: this object
   * @ignored: %TRUE if grouplock is ignored
   *
   * emitted when ignoregrouplock is toggled.
   */
  hyprland_service_signals[IGNOREGROUPLOCK] =
      g_signal_new("ignore-grouplock", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  /**
   * BarBarHyprlandService::lock-groups:
   * @service: this object
   * locked: %TRUE if enabled
   *
   * emitted when lockgroups is toggled.
   */
  hyprland_service_signals[LOCKGROUPS] =
      g_signal_new("lock-group", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 0, G_TYPE_BOOLEAN);

  /**
   * BarBarHyprlandService::config-reloaded:
   * @service: this object
   *
   * emitted when the config is done reloading
   */
  hyprland_service_signals[CONFIGRELOADED] =
      g_signal_new("config-reloaded", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  /**
   * BarBarHyprlandService::pin:
   * @service: this object
   * @windowaddress: address of window
   * @pinstate: %TRUE if window is pinned
   *
   * emitted when a window is pinned or unpinned
   */
  hyprland_service_signals[PIN] = g_signal_new(
      "pin", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);
}

static void g_barbar_hyprland_service_init(BarBarHyprlandService *self) {}

BarBarHyprlandService *g_barbar_hyprland_service_new(void) {
  BarBarHyprlandService *hypr =
      g_object_new(BARBAR_TYPE_HYPRLAND_SERVICE, NULL);

  return hypr;
}
