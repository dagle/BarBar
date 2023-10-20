#include "barbar-tray.h"
#include <stdio.h>

struct _BarBarTray {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  int icon_size;
  int spacing;
  gboolean reverse_direction;
  gboolean show_passive;
  gboolean wild;

  // A list of clients we should listen to
  GArray clients;
};

enum {
  PROP_0,

  // PROP_STATES,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarTray, g_barbar_tray, G_TYPE_OBJECT)

static GParamSpec *tray_props[NUM_PROPERTIES] = {
    NULL,
};

static GParamSpec *elem = NULL;

static void g_barbar_tray_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
}

static void g_barbar_tray_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
  BarBarTray *disk = BARBAR_TRAY(object);

  switch (property_id) {
 //  case PROP_STATES:
	// g_value_get_string(value);
 //    // g_barbar_disk_set_path(disk, g_value_get_string(value));
 //    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }

}

static void g_barbar_tray_class_init(BarBarTrayClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_tray_set_property;
  gobject_class->get_property = g_barbar_tray_get_property;
  elem = g_param_spec_double(
      "critical-temp", NULL, NULL, 0.0, 300.0, 80.0, G_PARAM_CONSTRUCT);
  // tray_props[PROP_STATES] = g_param_spec_value_array(
  //     "states", NULL, NULL, elem, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, tray_props);
}

static void g_barbar_tray_init(BarBarTray *self) {
}

void g_barbar_tray_update(BarBarTray *self) {
}
