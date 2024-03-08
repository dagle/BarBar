#include "barbar-dbusmenu.h"
#include "gio/gmenu.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarDBusMenu {
  GtkWidget parent_instance;

  gchar *object_path;
  gchar *bus_name;

  GMenu *internal;
  gint handler_id;

  gint revision;

  StatusNotifierComCanonicalDbusmenu *proxy;
};

enum {
  PROP_0,

  PROP_NAME,
  PROP_PATH,

  NUM_PROPERTIES,
};

static GParamSpec *dbus_menu_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarDBusMenu, g_barbar_dbus_menu, G_TYPE_MENU_MODEL)

static void g_barbar_dbus_menu_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(object);

  switch (property_id) {
  case PROP_NAME:
    menu->bus_name = strdup(g_value_get_string(value));
    break;
  case PROP_PATH:
    menu->object_path = strdup(g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dbus_menu_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(object);

  switch (property_id) {
  case PROP_NAME:
    g_value_set_string(value, menu->bus_name);
    break;
  case PROP_PATH:
    g_value_set_string(value, menu->object_path);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

const char *g_barbar_dbus_menu_path(BarBarDBusMenu *menu) {
  return menu->object_path;
}

static gboolean is_seperator(GVariant *entry) {
  GVariant *child = g_variant_get_child_value(entry, 1);
  GVariant *type = g_variant_lookup_value(child, "type", G_VARIANT_TYPE_STRING);

  if (type) {
    gsize length;
    const gchar *kind = g_variant_get_string(type, &length);
    if (!strcmp("separator", kind)) {
      return TRUE;
    }
  }

  return FALSE;
}

static GMenuItem *parse_item(GVariant *entry);

// create a sector if the current sector isn't
// an empty one and adds the old one to the menu.
static void new_sector(GMenu **menu, GMenu **sector) {
  // we have an empty sector, just use that and don't create a new one
  if (*sector && g_menu_model_get_n_items(G_MENU_MODEL(*sector)) <= 0) {
    return;
  }

  // If we already have a sector, we need to commit it to
  // the menu before create a new one
  if (*sector) {
    if (!menu) {
      *menu = g_menu_new();
    }
    g_menu_append_section(*menu, NULL, G_MENU_MODEL(*sector));
    g_object_unref(*sector);
  }
  *sector = g_menu_new();
}

// A bit ugly but works(tm)
static GMenu *parse_menu(GVariant *submenu) {
  GVariant *child;
  GVariantIter children;
  GMenuItem *item = NULL;
  GMenu *menu = NULL;
  GMenu *sector = NULL;

  g_variant_iter_init(&children, submenu);
  while ((child = g_variant_iter_next_value(&children)) != NULL) {
    if (g_variant_is_of_type(child, G_VARIANT_TYPE_VARIANT)) {
      GVariant *tmp = g_variant_get_variant(child);
      g_variant_unref(child);
      child = tmp;
    }
    if (is_seperator(child)) {
      new_sector(&menu, &sector);
    } else {
      item = parse_item(child);
      if (item) {
        if (!menu) {
          menu = g_menu_new();
        }
        if (sector) {
          g_menu_append_item(sector, item);
        } else {
          g_menu_append_item(menu, item);
        }
      }
    }
  }
  if (sector && menu) {
    g_menu_append_section(menu, NULL, G_MENU_MODEL(sector));
  }
  return menu;
}

static GMenuItem *parse_item(GVariant *entry) {
  GVariantIter iter;
  gchar *prop;
  GVariant *value;
  GVariant *child;
  gboolean visible = TRUE;

  GVariant *idv = g_variant_get_child_value(entry, 0);
  gint id = g_variant_get_int32(idv);

  child = g_variant_get_child_value(entry, 1);

  g_variant_iter_init(&iter, child);

  GMenuItem *menu_item;
  menu_item = g_object_new(G_TYPE_MENU_ITEM, NULL);

  while (g_variant_iter_loop(&iter, "{sv}", &prop, &value)) {
    if (strcmp("label", prop) == 0) {
      gsize length;
      const gchar *label = g_variant_get_string(value, &length);
      if (!label) {
        continue;
      }
      if (g_ascii_strcasecmp("label empty", label)) {
        g_menu_item_set_label(menu_item, label);
      }
    }
    if (strcmp("icon-name", prop) == 0) {
      gsize length;
      const gchar *icon = g_variant_get_string(value, &length);

      GIcon *gicon = g_themed_icon_new(icon);
      g_menu_item_set_icon(menu_item, gicon);
    }

    if (strcmp("enabled", prop) == 0) {
      gboolean enabled = g_variant_get_boolean(value);

      if (enabled) {
        g_menu_item_set_action_and_target_value(menu_item, "trayitem.activate",
                                                g_variant_new_int32(id));
      }
    }

    if (strcmp("visible", prop) == 0) {
      visible = g_variant_get_boolean(value);
    }

    if (strcmp("shortcut", prop) == 0) {
    }
    if (strcmp("toggle-type", prop) == 0) {
    }
    if (strcmp("toggle-state", prop) == 0) {
    }
  }

  GVariant *submenu = g_variant_get_child_value(entry, 2);
  GMenu *menu = parse_menu(submenu);
  if (menu) {
    g_menu_item_set_submenu(menu_item, G_MENU_MODEL(menu));
  }
  // status_notifier_com_canonical_dbusmenu_call_event

  return menu_item;
}

static void event_callback(GObject *object, GAsyncResult *res, gpointer data) {}

void g_barbar_dbus_menu_event(BarBarDBusMenu *menu, int id) {
  GVariant *variant = g_variant_new_int32(0);
  GVariant *nested = g_variant_new_variant(variant);
  guint time = GDK_CURRENT_TIME;

  status_notifier_com_canonical_dbusmenu_call_event(
      menu->proxy, id, "clicked", nested, time, NULL, event_callback, NULL);
}

static gboolean parse_root(BarBarDBusMenu *self, GVariant *layout) {
  GVariant *submenu = g_variant_get_child_value(layout, 2);
  GMenu *menu = parse_menu(submenu);
  if (menu) {
    if (self->internal) {
      int removed = g_menu_model_get_n_items(G_MENU_MODEL(self->internal));
      int added = g_menu_model_get_n_items(G_MENU_MODEL(menu));
      g_object_unref(self->internal);

      self->internal = menu;
      g_menu_model_items_changed(G_MENU_MODEL(self), 0, removed, added);
    } else {
      self->internal = menu;
      int added = g_menu_model_get_n_items(G_MENU_MODEL(self->internal));
      g_menu_model_items_changed(G_MENU_MODEL(self), 0, 0, added);
    }
    return TRUE;
  }
  return FALSE;
}

static void layout_callback(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  guint revision;
  GVariant *layout = NULL;
  gboolean success;

  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(data);

  success = status_notifier_com_canonical_dbusmenu_call_get_layout_finish(
      menu->proxy, &revision, &layout, res, &error);

  if (error) {
    g_printerr("Couldn't fetch layout: %s", error->message);
    g_error_free(error);
  }

  if (revision > menu->revision) {
    menu->revision = revision;
    parse_root(menu, layout);
  }
}

static void update_menu(StatusNotifierComCanonicalDbusmenu *proxy,
                        GVariant *add, GVariant *remove, gpointer data) {
  printf("update menu!\n");
}

static void get_layout(BarBarDBusMenu *menu, int parent) {
  static const char *const arg_propertyNames[] = {"type",
                                                  "label",
                                                  "visible",
                                                  "enabled",
                                                  "children-display",
                                                  "accessible-desc",
                                                  NULL};

  status_notifier_com_canonical_dbusmenu_call_get_layout(
      menu->proxy, 0, -1, arg_propertyNames, NULL, layout_callback, menu);
}

// layout_update (GDBusProxy * proxy, guint revision, gint parent,
// DbusmenuClient * client)
static void layout_update(StatusNotifierComCanonicalDbusmenu *proxy,
                          int revision, int parent, gpointer data) {
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(data);

  if (revision > menu->revision) {
    get_layout(menu, 0);
  }
}

static void menu_callback(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(data);

  menu->proxy = status_notifier_com_canonical_dbusmenu_proxy_new_for_bus_finish(
      res, &error);

  get_layout(menu, 0);

  g_signal_connect(menu->proxy, "items-properties-updated",
                   G_CALLBACK(update_menu), menu);
  g_signal_connect(menu->proxy, "layout-updated", G_CALLBACK(layout_update),
                   menu);
}

static void g_barbar_dbus_menu_constructed(GObject *object) {
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(object);

  G_OBJECT_CLASS(g_barbar_dbus_menu_parent_class)->constructed(object);

  status_notifier_com_canonical_dbusmenu_proxy_new_for_bus(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, menu->bus_name,
      menu->object_path, NULL, menu_callback, menu);
}

static gboolean g_barbar_dbus_menu_is_mutable(GMenuModel *model) {
  return TRUE;
}

static gint g_barbar_dbus_menu_get_n_items(GMenuModel *model) {
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(model);

  if (menu->internal) {
    int i = g_menu_model_get_n_items(G_MENU_MODEL(menu->internal));
    return i;
  } else {
    return 0;
  }
}

static void g_barbar_dbus_menu_get_item_attributes(GMenuModel *model,
                                                   gint position,
                                                   GHashTable **table) {

  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(model);

  return G_MENU_MODEL_GET_CLASS(menu->internal)
      ->get_item_attributes(G_MENU_MODEL(menu->internal), position, table);
}

static void g_barbar_dbus_menu_get_item_links(GMenuModel *model, gint position,
                                              GHashTable **table) {
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(model);

  G_MENU_MODEL_GET_CLASS(menu->internal)
      ->get_item_links(G_MENU_MODEL(menu->internal), position, table);
}

static void g_barbar_dbus_menu_class_init(BarBarDBusMenuClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GMenuModelClass *menu_class = G_MENU_MODEL_CLASS(class);

  gobject_class->set_property = g_barbar_dbus_menu_set_property;
  gobject_class->get_property = g_barbar_dbus_menu_get_property;
  gobject_class->constructed = g_barbar_dbus_menu_constructed;

  menu_class->is_mutable = g_barbar_dbus_menu_is_mutable;
  menu_class->get_n_items = g_barbar_dbus_menu_get_n_items;
  menu_class->get_item_attributes = g_barbar_dbus_menu_get_item_attributes;
  menu_class->get_item_links = g_barbar_dbus_menu_get_item_links;

  dbus_menu_props[PROP_NAME] = g_param_spec_string(
      "name", "busname", "busname for the menu", NULL,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  dbus_menu_props[PROP_PATH] = g_param_spec_string(
      "path", "objectpath", "objectpath for the menu", NULL,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dbus_menu_props);
}

static void g_barbar_dbus_menu_init(BarBarDBusMenu *self) {}

BarBarDBusMenu *g_barbar_dbus_menu_new(const gchar *bus_name,
                                       const gchar *path) {
  return g_object_new(BARBAR_TYPE_DBUS_MENU, "name", bus_name, "path", path,
                      NULL);
}
