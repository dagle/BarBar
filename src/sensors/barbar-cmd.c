#include "barbar-cmd.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarCmd {
  BarBarSensor parent;

  gchar **cmd;
  char *value;

  guint interval;
  guint source_id;
};

enum {
  CMD_PROP_0,

  CMD_PROP_CMD,
  CMD_PROP_VALUE,
  CMD_PROP_INTERVAL,

  CMD_NUM_PROPERTIES,
};

// update every 10 sec by default
#define DEFAULT_INTERVAL 10000

G_DEFINE_TYPE(BarBarCmd, g_barbar_cmd, BARBAR_TYPE_SENSOR)

static GParamSpec *cmd_props[CMD_NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_cmd_set_cmd(BarBarCmd *self, const char *cmd) {
  g_return_if_fail(BARBAR_IS_CMD(self));
  GError *err = NULL;
  int argc;
  char **parsed_cmd = NULL;

  g_strfreev(self->cmd);

  gboolean ret = g_shell_parse_argv(cmd, &argc, &parsed_cmd, &err);
  if (!ret) {
    g_printerr("Command: failed to parse %s: %s\n", cmd, err->message);
  }
  self->cmd = parsed_cmd;

  g_object_notify_by_pspec(G_OBJECT(self), cmd_props[CMD_PROP_CMD]);
}

void g_barbar_cmd_set_value(BarBarCmd *self, char *value) {
  g_return_if_fail(BARBAR_IS_CMD(self));

  g_free(self->value);
  self->value = value;

  g_object_notify_by_pspec(G_OBJECT(self), cmd_props[CMD_PROP_VALUE]);
}
void g_barbar_cmd_set_interval(BarBarCmd *self, uint interval) {
  g_return_if_fail(BARBAR_IS_CMD(self));

  self->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(self), cmd_props[CMD_PROP_INTERVAL]);
}

static void g_barbar_cmd_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {

  BarBarCmd *cmd = BARBAR_CMD(object);

  switch (property_id) {
  case CMD_PROP_CMD:
    g_barbar_cmd_set_cmd(cmd, g_value_get_string(value));
    break;
  case CMD_PROP_INTERVAL:
    g_barbar_cmd_set_interval(cmd, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cmd_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarCmd *cmd = BARBAR_CMD(object);

  switch (property_id) {
  case CMD_PROP_CMD:
    g_value_set_boxed(value, cmd->cmd);
    break;
  case CMD_PROP_INTERVAL:
    g_value_set_uint(value, cmd->interval);
    break;
  case CMD_PROP_VALUE:
    g_value_set_string(value, cmd->value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cmd_start(BarBarSensor *sensor);

// static void g_barbar_cmd_constructed(GObject *obj) {
//   BarBarCmd *self = BARBAR_CMD(obj);
//   G_OBJECT_CLASS(g_barbar_cmd_parent_class)->constructed(obj);
//
//   if (!self->interval) {
//     self->interval = DEFAULT_INTERVAL;
//   }
//
//   g_barbar_cmd_start(self, NULL);
// }

static void g_barbar_cmd_class_init(BarBarCmdClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_cmd_set_property;
  gobject_class->get_property = g_barbar_cmd_get_property;
  sensor_class->start = g_barbar_cmd_start;

  // cmd_props[CMD_PROP_CMD] = g_param_spec_boxed(
  //     "command", "cmd", NULL, G_TYPE_STRV,
  //     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  cmd_props[CMD_PROP_CMD] = g_param_spec_string(
      "command", "cmd", NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  cmd_props[CMD_PROP_VALUE] =
      g_param_spec_string("value", NULL, NULL, NULL, G_PARAM_READABLE);
  cmd_props[CMD_PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_properties(gobject_class, CMD_NUM_PROPERTIES,
                                    cmd_props);
}

static void g_barbar_cmd_init(BarBarCmd *self) {}

static void g_barbar_cmd_callback(GObject *object, GAsyncResult *res,
                                  gpointer data) {

  GSubprocess *proc = G_SUBPROCESS(object);
  BarBarCmd *self = BARBAR_CMD(data);
  GError *err = NULL;
  gboolean ret;
  char *stdout_buf;

  ret =
      g_subprocess_communicate_utf8_finish(proc, res, &stdout_buf, NULL, &err);

  if (ret) {
    gint len = strlen(stdout_buf);
    if (len > 1) {
      stdout_buf[len - 1] = '\0';
      g_barbar_cmd_set_value(self, stdout_buf);
    }
  }

  g_object_unref(proc);
}

static gboolean g_barbar_cmd_update(gpointer data) {
  BarBarCmd *self = BARBAR_CMD(data);
  GError *err = NULL;

  GSubprocess *proc = g_subprocess_newv(
      self->cmd,
      G_SUBPROCESS_FLAGS_SEARCH_PATH_FROM_ENVP | G_SUBPROCESS_FLAGS_STDOUT_PIPE,
      &err);

  if (err) {
    g_printerr("Command: failed: %s\n", err->message);
    g_error_free(err);
    g_object_unref(proc);
    return G_SOURCE_REMOVE;
  }

  g_subprocess_communicate_utf8_async(proc, NULL, NULL, g_barbar_cmd_callback,
                                      self);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_cmd_start(BarBarSensor *sensor) {
  BarBarCmd *cmd = BARBAR_CMD(sensor);
  if (cmd->source_id > 0) {
    g_source_remove(cmd->source_id);
  }
  g_barbar_cmd_update(cmd);
  cmd->source_id =
      g_timeout_add_full(0, cmd->interval, g_barbar_cmd_update, cmd, NULL);
}
