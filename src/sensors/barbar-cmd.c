#include "barbar-cmd.h"
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdio.h>

/**
 * BarBarCmd:
 *
 * Run a command every interval
 */
struct _BarBarCmd {
  BarBarIntervalSensor parent_instance;

  gchar **cmd;
  char *value;
};

enum {
  CMD_PROP_0,

  CMD_PROP_CMD,
  CMD_PROP_VALUE,

  CMD_NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

G_DEFINE_TYPE(BarBarCmd, g_barbar_cmd, BARBAR_TYPE_INTERVAL_SENSOR)

static GParamSpec *cmd_props[CMD_NUM_PROPERTIES] = {
    NULL,
};

static gboolean g_barbar_cmd_tick(BarBarIntervalSensor *sensor);

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

static void g_barbar_cmd_set_value(BarBarCmd *self, char *value) {
  g_return_if_fail(BARBAR_IS_CMD(self));

  g_free(self->value);
  self->value = value;

  g_object_notify_by_pspec(G_OBJECT(self), cmd_props[CMD_PROP_VALUE]);
}

static void g_barbar_cmd_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {

  BarBarCmd *cmd = BARBAR_CMD(object);

  switch (property_id) {
  case CMD_PROP_CMD:
    g_barbar_cmd_set_cmd(cmd, g_value_get_string(value));
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
  case CMD_PROP_VALUE:
    g_value_set_string(value, cmd->value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cmd_start(BarBarSensor *sensor);

static void g_barbar_cmd_class_init(BarBarCmdClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  interval_class->tick = g_barbar_cmd_tick;

  gobject_class->set_property = g_barbar_cmd_set_property;
  gobject_class->get_property = g_barbar_cmd_get_property;

  // cmd_props[CMD_PROP_CMD] = g_param_spec_boxed(
  //     "command", "cmd", NULL, G_TYPE_STRV,
  //     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarCmd:cmd:
   *
   * Command to be executed
   */
  cmd_props[CMD_PROP_CMD] = g_param_spec_string(
      "cmd", "command", NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarCmd:value:
   *
   * Return value from the command
   */
  cmd_props[CMD_PROP_VALUE] =
      g_param_spec_string("value", NULL, NULL, NULL, G_PARAM_READABLE);

  // cmd_props[CMD_PROP_VALUE] = g_param_spec_object(
  //     "value", NULL, NULL, GTK_TYPE_EXPRESSION, G_PARAM_READABLE);

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

static gboolean g_barbar_cmd_tick(BarBarIntervalSensor *sensor) {
  BarBarCmd *self = BARBAR_CMD(sensor);
  GError *err = NULL;

  GSubprocess *proc = g_subprocess_newv(
      (const gchar *const *)self->cmd,
      G_SUBPROCESS_FLAGS_SEARCH_PATH_FROM_ENVP | G_SUBPROCESS_FLAGS_STDOUT_PIPE,
      &err);

  if (err) {
    g_printerr("Command: failed: %s\n", err->message);
    g_error_free(err);
    // g_object_unref(proc);
    return G_SOURCE_REMOVE;
  }

  g_subprocess_communicate_utf8_async(proc, NULL, NULL, g_barbar_cmd_callback,
                                      self);

  return G_SOURCE_CONTINUE;
}

/**
 * g_barbar_cmd_new:
 *
 * Returns: (transfer full): a `BarBarCmd`
 */
BarBarSensor *g_barbar_cmd_new(const char *command) {
  BarBarCmd *cmd;

  cmd = g_object_new(BARBAR_TYPE_CMD, "command", command, NULL);

  return BARBAR_SENSOR(cmd);
}
