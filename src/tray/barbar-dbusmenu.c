#include "barbar-dbusmenu.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarDBusMenu {
  GtkWidget parent_instance;
};

enum {
  PROP_0,

  NUM_PROPERTIES,
};

static GParamSpec *dbus_menu_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarDBusMenu, g_barbar_dbus_menu, GTK_TYPE_WIDGET)

static void g_barbar_dbus_menu_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(object);

  switch (property_id) {
  // case PROP_NAME:
  // item->bus_name = strdup(g_value_get_string(value));
  // break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dbus_menu_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(object);

  switch (property_id) {
  // case PROP_NAME:
  //   g_value_set_string(value, item->bus_name);
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dbus_menu_class_init(BarBarDBusMenuClass *class) {

  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_dbus_menu_set_property;
  gobject_class->get_property = g_barbar_dbus_menu_get_property;
}

static void g_barbar_dbus_menu_init(BarBarDBusMenu *self) {}
