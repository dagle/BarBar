#include "barbar-tray.h"
#include "barbar-watcher.h"
#include <stdio.h>

// static int num = 1;
struct _BarBarTray {
  GtkWidget parent;

  GList *items;
  BarBarStatusWatcher *watcher;
};

enum {
  PROP_0,

  PROP_STATES,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarTray, g_barbar_tray, GTK_TYPE_WIDGET)

static GParamSpec *tray_props[NUM_PROPERTIES] = {
    NULL,
};

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
  // tray_props[PROP_STATES] = g_param_spec_double("critical-temp", NULL, NULL,
  // 0.0, 300.0, 80.0,
  //                            G_PARAM_CONSTRUCT);
  // g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
  // tray_props);
}

static void g_barbar_tray_init(BarBarTray *self) {}

static void item_reg(BarBarStatusWatcher *watcher, gpointer data) {}

static void g_barbar_tray_host_name_aquired(GDBusConnection *connection,
                                            const gchar *name,
                                            gpointer user_data) {
  BarBarTray *tray = BARBAR_TRAY(user_data);
  BarBarStatusWatcher *watcher = g_barbar_status_watcher_new();
  g_signal_connect(watcher, "item-registerd", G_CALLBACK(item_reg), NULL);
  tray->watcher = watcher;
}

void g_barbar_tray_update(BarBarTray *self) {
  g_bus_own_name(G_BUS_TYPE_SESSION, self->name, G_BUS_NAME_OWNER_FLAGS_NONE,
                 NULL, g_barbar_tray_host_name_aquired, NULL, self, NULL);
  // sigc::mem_fun(*this, &Host::busAcquired))),
}
