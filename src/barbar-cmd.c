#include "barbar-cmd.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarCmd {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  char *cmd;
};

enum {
  PROP_0,

  PROP_CMD,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarCmd, g_barbar_cmd, G_TYPE_OBJECT)

static GParamSpec *cmd_props[NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_cmd_set_cmd(BarBarCmd *self, const char *cmd) {
  g_return_if_fail(BARBAR_IS_CMD(self));

  g_free(self->cmd);
  self->cmd = g_strdup(self->cmd);

  g_object_notify_by_pspec(G_OBJECT(self), cmd_props[PROP_CMD]);
}

static void g_barbar_cmd_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {}

static void g_barbar_cmd_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarCmd *cmd = BARBAR_CMD(object);

  switch (property_id) {
  case PROP_CMD:
    g_value_get_string(value);
    g_barbar_cmd_set_cmd(cmd, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cmd_class_init(BarBarCmdClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_cmd_set_property;
  gobject_class->get_property = g_barbar_cmd_get_property;
  cmd_props[PROP_CMD] =
      g_param_spec_string("states", NULL, NULL, NULL, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, cmd_props);
}

static void g_barbar_cmd_init(BarBarCmd *self) {}

void g_barbar_cmd_update(BarBarCmd *self) {
  GError *err;
  // g_subprocess_new(G_SUBPROCESS_FLAGS_NONE, &err)
}
