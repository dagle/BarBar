#include "barbar-watcher.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarStatusWatcher {
  GObject parent;

  gboolean inited;
  GError *init_error;
  StatusNotifierWatcher *watcher;
  // TODO:This should be in parent
  // char *label;
};

static void g_barbar_status_watcher_iface_init(GInitableIface *iface);
G_DEFINE_TYPE_WITH_CODE(
    BarBarStatusWatcher, g_barbar_status_watcher, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, g_barbar_status_watcher_iface_init))

static GObject *constructor(GType type, guint n_construct_params,
                            GObjectConstructParam *construct_params) {
  static GObject *self = NULL;

  if (self == NULL) {
    self = G_OBJECT_CLASS(g_barbar_status_watcher_parent_class)
               ->constructor(type, n_construct_params, construct_params);
    g_object_add_weak_pointer(self, (gpointer)&self);

    // class->watcher = status_notifier_watcher_skeleton_new();
    return self;
  }

  return g_object_ref(self);
}

static gboolean g_barbar_status_watcher_initable(GInitable *initable,
                                                 GCancellable *cancellable,
                                                 GError **out_error) {
  BarBarStatusWatcher *watcher = BARBAR_STATUS_WATCHER(initable);

  if (watcher->inited) {
    if (watcher->init_error) {
      *out_error = g_error_copy(watcher->init_error);
      return FALSE;
    }
    return TRUE;
  } else {
    // TODO
    // g_propagate_error (out_error, g_error_copy (watcher->init_error));
  }
  // g_error_copy

  // if (portal->init_error != NULL)
  //   {
  //     return FALSE;
  //   }
  //
  // g_assert (portal->bus != NULL);
  return TRUE;
}

static void g_barbar_status_watcher_iface_init(GInitableIface *iface) {
  iface->init = g_barbar_status_watcher_initable;
}
void g_barbar_status_watcher_name_acquired_handler(GDBusConnection *connection,
                                                   const gchar *name,
                                                   gpointer user_data);

static void g_barbar_status_watcher_iface_run(BarBarStatusWatcher *watcher) {
  guint i = g_bus_own_name(
      G_BUS_TYPE_SESSION, "org.kde.StatusNotifierWatcher",
      G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT | G_BUS_NAME_OWNER_FLAGS_REPLACE,
      NULL, g_barbar_status_watcher_name_acquired_handler, NULL, NULL, NULL);
  // g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(watcher->watcher),
  // conn->gobj(),
  //                                  "/StatusNotifierWatcher", &error);
}

void g_barbar_status_watcher_name_acquired_handler(GDBusConnection *connection,
                                                   const gchar *name,
                                                   gpointer user_data) {}

// static void g_barbar_status_watcher_name_acquired_handler() {}

// static void g_barbar_status_watcher_name_lost_handler() {}

static void
g_barbar_status_watcher_class_init(BarBarStatusWatcherClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->constructor = constructor;
}

static void g_barbar_status_watcher_init(BarBarStatusWatcher *self) {}

BarBarStatusWatcher *g_barbar_status_watcher_new(void) {
  return g_object_new(BARBAR_TYPE_STATUS_WATCHER, NULL);
}

void g_barbar_status_watcher_update(BarBarStatusWatcher *self) {}
