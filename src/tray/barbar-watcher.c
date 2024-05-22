#include "barbar-watcher.h"
// #include "barbar-dbusmenu.h"
// #include "barbar-tray-item.h"
#include "status-notifier.h"
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

/**
 * BarBarStatusWatcher:
 *
 * A status watcher, it registers incomming requests
 * for tray icons. A tray hosts would listen for this
 *
 */
struct _BarBarStatusWatcher {
  GObject parent_instance;

  guint id;
  StatusNotifierWatcher *skeleton;

  GList *items;
  GList *hosts;
};

typedef struct WatcherItem {
  char *bus_name;
  char *object_path;
  int name_id;
} WatcherItem;

void item_vanish(GDBusConnection *connection, const gchar *bus_name,
                 gpointer user_data);

static gboolean watcher_exists(GList *items, const char *bus_name,
                               const char *object_path) {
  WatcherItem *item;

  while (items) {
    item = items->data;
    if (!strcmp(bus_name, item->bus_name) &&
        strcmp(object_path, item->object_path)) {
      return TRUE;
    }

    items = items->next;
  }
  return FALSE;
}

static void free_item(WatcherItem *item) {

  if (item->name_id > 0) {
    g_bus_unwatch_name(item->name_id);
  }
  g_free(item->bus_name);
  g_free(item->object_path);
}

static WatcherItem *add_item(BarBarStatusWatcher *watcher, const char *bus_name,
                             const char *object_path) {
  WatcherItem *item = g_malloc0(sizeof(WatcherItem));
  item->bus_name = g_strdup(bus_name);
  item->object_path = g_strdup(object_path);
  item->name_id = g_bus_watch_name(G_BUS_TYPE_SESSION, bus_name,
                                   G_BUS_NAME_WATCHER_FLAGS_NONE, NULL,
                                   item_vanish, watcher, NULL);

  return item;
}

enum {
  PROP_0,

  PROP_ITEMS,

  NUM_PROPERTIES,
};

static GParamSpec *watcher_props[NUM_PROPERTIES] = {
    NULL,
};

enum {
  ITEM_REGISTER,
  ITEM_UNREGISTER,
  NUM_SIGNALS,
};

static guint watcher_signals[NUM_SIGNALS];

G_DEFINE_TYPE(BarBarStatusWatcher, g_barbar_status_watcher, GTK_TYPE_WIDGET)

static void on_bus_acquired(GDBusConnection *connection, const gchar *name,
                            gpointer user_data);
static void on_name_acquired(GDBusConnection *connection, const gchar *name,
                             gpointer user_data);

static void on_name_lost(GDBusConnection *connection, const gchar *name,
                         gpointer user_data);

static void g_barbar_watcher_get_items(BarBarStatusWatcher *watcher,
                                       GValue *value) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ss)"));
  for (GList *list = watcher->items; list != NULL; list = list->next) {
    WatcherItem *item = list->data;
    g_variant_builder_add(&builder, "ss", item->bus_name, item->object_path);
  }
  GVariant *variant = g_variant_builder_end(&builder);
  g_value_take_variant(value, variant);
}

static void g_barbar_watcher_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {}

static void g_barbar_watcher_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {

  BarBarStatusWatcher *watcher = BARBAR_STATUS_WATCHER(object);
  switch (property_id) {
  case PROP_ITEMS:
    g_barbar_watcher_get_items(watcher, value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static BarBarStatusWatcher *the_singleton = NULL;

static GObject *
g_barbar_watcher_constructor(GType type, guint n_construct_params,
                             GObjectConstructParam *construct_params) {
  GObject *object;

  if (!the_singleton) {
    object = G_OBJECT_CLASS(g_barbar_status_watcher_parent_class)
                 ->constructor(type, n_construct_params, construct_params);
    the_singleton = BARBAR_STATUS_WATCHER(object);
  } else {
    object = g_object_ref(G_OBJECT(the_singleton));
  }

  return object;
}

static void g_barbar_watcher_constructed(GObject *self) {
  BarBarStatusWatcher *watcher = BARBAR_STATUS_WATCHER(self);
  G_OBJECT_CLASS(g_barbar_status_watcher_parent_class)->constructed(self);

  watcher->id = g_bus_own_name(
      G_BUS_TYPE_SESSION, "org.kde.StatusNotifierWatcher",
      G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT | G_BUS_NAME_OWNER_FLAGS_REPLACE,
      on_bus_acquired, on_name_acquired, on_name_lost, self, NULL);
}

static void
g_barbar_status_watcher_class_init(BarBarStatusWatcherClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_watcher_set_property;
  gobject_class->get_property = g_barbar_watcher_get_property;

  gobject_class->constructor = g_barbar_watcher_constructor;
  gobject_class->constructed = g_barbar_watcher_constructed;

  watcher_props[PROP_ITEMS] =
      g_param_spec_variant("items", NULL, NULL, ((const GVariantType *)"a(ss)"),
                           NULL, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    watcher_props);

  watcher_signals[ITEM_REGISTER] =
      g_signal_new("item-register", BARBAR_TYPE_STATUS_WATCHER,
                   G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL, NULL,
                   G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  watcher_signals[ITEM_UNREGISTER] =
      g_signal_new("item-unregister", BARBAR_TYPE_STATUS_WATCHER,
                   G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL, NULL,
                   G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);
}

static gboolean
g_barbar_tray_handle_register_host(StatusNotifierWatcher *skeleton,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *service, gpointer data) {
  return TRUE;
}

void item_vanish(GDBusConnection *connection, const gchar *bus_name,
                 gpointer user_data) {
  BarBarStatusWatcher *watcher = user_data;
  GList *items = watcher->items;
  WatcherItem *item;

  while (items) {
    item = items->data;
    if (!strcmp(bus_name, item->bus_name)) {
      g_signal_emit(watcher, watcher_signals[ITEM_UNREGISTER], 0,
                    item->bus_name, item->object_path);
      watcher->items = g_list_remove(watcher->items, item);
      // we can do this right?
      free_item(item);
      return;
    }
    items = items->next;
  }
}

static gboolean
g_barbar_tray_handle_register_item(StatusNotifierWatcher *skeleton,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *service, gpointer data) {
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

  if (watcher_exists(watcher->items, bus_name, object_path)) {
    // g_dbus_method_invocation_return_error(
    //     invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
    //     "Item with bus name '%s' and object path '%s' already exist",
    //     bus_name, object_path);
    status_notifier_watcher_complete_register_item(watcher->skeleton,
                                                   invocation);
    return TRUE;
  }

  status_notifier_watcher_complete_register_item(watcher->skeleton, invocation);

  gchar *tmp = g_strdup_printf("%s%s", bus_name, object_path);
  status_notifier_watcher_emit_item_registered(watcher->skeleton, tmp);
  g_free(tmp);

  WatcherItem *item = add_item(watcher, bus_name, object_path);

  watcher->items = g_list_append(watcher->items, item);

  g_signal_emit(watcher, watcher_signals[ITEM_REGISTER], 0, bus_name,
                object_path);
  g_object_notify_by_pspec(G_OBJECT(watcher), watcher_props[PROP_ITEMS]);

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

static void g_barbar_status_watcher_init(BarBarStatusWatcher *self) {}

BarBarStatusWatcher *g_barbar_status_watcher_new(void) {
  return g_object_new(BARBAR_TYPE_STATUS_WATCHER, NULL);
}

// void g_barbar_status_watcher_update(BarBarStatusWatcher *self) {}
