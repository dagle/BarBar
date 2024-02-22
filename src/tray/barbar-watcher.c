#include "barbar-watcher.h"
#include "barbar-dbusmenu.h"
#include "barbar-tray-item.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarStatusWatcher {
  GtkWidget parent_instance;

  guint id;
  StatusNotifierWatcher *skeleton;

  BarBarDbusMenu *menu;

  GList *items;
  GList *hosts;
};

enum {
  PROP_0,

  NUM_PROPERTIES,
};

static GParamSpec *watcher_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarStatusWatcher, g_barbar_status_watcher, GTK_TYPE_WIDGET)

static void g_barbar_watcher_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {}

static void g_barbar_watcher_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {}

static void g_barbar_tray_root(GtkWidget *widget);
static void g_barbar_watcher_constructed(GObject *object);
static void
g_barbar_status_watcher_class_init(BarBarStatusWatcherClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_watcher_set_property;
  gobject_class->get_property = g_barbar_watcher_get_property;
  // gobject_class->constructed = g_barbar_watcher_constructed;

  widget_class->root = g_barbar_tray_root;

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  // gtk_widget_class_set_css_name(widget_class, "tray");
  gtk_widget_class_set_css_name(widget_class, "river-tag");
}

static gboolean
g_barbar_tray_handle_register_host(StatusNotifierWatcher *skeleton,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *service, gpointer data) {
  return TRUE;
}

static gboolean
g_barbar_tray_handle_register_item(StatusNotifierWatcher *skeleton,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *service, gpointer data) {
  BarBarTrayItem *item;
  const gchar *object_path;
  const gchar *bus_name;
  BarBarStatusWatcher *watcher = BARBAR_STATUS_WATCHER(data);

  if (*service == '/') {
    object_path = service;
    bus_name = g_dbus_method_invocation_get_sender(invocation);
  } else {
    bus_name = service;
    object_path = "/StatusNotifierItem";
  }

  if (g_dbus_is_name(bus_name) == FALSE) {
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
        "D-Bus bus name '%s' is not valid", bus_name);
    return TRUE;
  }

  status_notifier_watcher_complete_register_item(watcher->skeleton, invocation);

  gchar *tmp = g_strdup_printf("%s%s", bus_name, object_path);
  status_notifier_watcher_emit_item_registered(watcher->skeleton, tmp);
  g_free(tmp);

  item = g_barbar_tray_item_new(bus_name, object_path);

  gtk_widget_set_parent(GTK_WIDGET(item), GTK_WIDGET(watcher));

  watcher->items = g_list_append(watcher->items, item);

  return TRUE;
}

static void on_bus_acquired(GDBusConnection *connection, const gchar *name,
                            gpointer user_data) {
  GError *error = NULL;
  BarBarStatusWatcher *watcher = BARBAR_STATUS_WATCHER(user_data);
  watcher->skeleton = status_notifier_watcher_skeleton_new();

  g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(watcher->skeleton),
                                   connection, "/StatusNotifierWatcher",
                                   &error);

  if (error != NULL) {
    g_printerr("Failure to setup watcher: %s", error->message);
    return;
  }
  g_print("got bus %s\n", name);

  g_signal_connect(watcher->skeleton, "handle-register-item",
                   G_CALLBACK(g_barbar_tray_handle_register_item), watcher);
  g_signal_connect(watcher->skeleton, "handle-register-host",
                   G_CALLBACK(g_barbar_tray_handle_register_host), watcher);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name,
                             gpointer user_data) {
  g_print("Acquired the name %s\n", name);
}

static void on_name_lost(GDBusConnection *connection, const gchar *name,
                         gpointer user_data) {
  g_print("Lost the name %s\n", name);
}

static void g_barbar_tray_root(GtkWidget *widget) {
  GTK_WIDGET_CLASS(g_barbar_status_watcher_parent_class)->root(widget);

  BarBarStatusWatcher *watcher = BARBAR_STATUS_WATCHER(widget);

  watcher->id = g_bus_own_name(
      G_BUS_TYPE_SESSION, "org.kde.StatusNotifierWatcher",
      G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT | G_BUS_NAME_OWNER_FLAGS_REPLACE,
      on_bus_acquired, on_name_acquired, on_name_lost, widget, NULL);
}

static void g_barbar_status_watcher_init(BarBarStatusWatcher *self) {}

BarBarStatusWatcher *g_barbar_status_watcher_new(void) {
  return g_object_new(BARBAR_TYPE_STATUS_WATCHER, NULL);
}

void g_barbar_status_watcher_update(BarBarStatusWatcher *self) {}
