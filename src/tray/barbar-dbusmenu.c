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

  // GtkWidget *menu;
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
      if (sector) {
        // free
        g_menu_append_section(menu, NULL, G_MENU_MODEL(sector));
      }
      sector = g_menu_new();

      // g_menu_append_section(GMenu *menu, const gchar *label, GMenuModel
      // *section)

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
  return menu;
}

static GMenuItem *parse_item(GVariant *entry) {
  GVariantIter iter;
  gchar *prop;
  GVariant *value;
  GVariant *child;

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
    }

    if (strcmp("enabled", prop) == 0) {
    }
    // if shortcut, add shortcut
    // toggle-type / toggle-state
  }

  GVariant *submenu = g_variant_get_child_value(entry, 2);
  GMenu *menu = parse_menu(submenu);
  if (menu) {
    g_menu_item_set_submenu(menu_item, G_MENU_MODEL(menu));
  }

  return menu_item;
}

static gboolean parse_root(BarBarDBusMenu *self, GVariant *layout) {
  GVariant *submenu = g_variant_get_child_value(layout, 2);
  GMenu *menu = parse_menu(submenu);
  if (menu) {
    self->internal = menu;
    int added = g_menu_model_get_n_items(G_MENU_MODEL(self->internal));
    g_menu_model_items_changed(G_MENU_MODEL(self), 0, 0, added);
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
  StatusNotifierComCanonicalDbusmenu *proxy =
      STATUS_NOTIFIER_COM_CANONICAL_DBUSMENU(object);

  success = status_notifier_com_canonical_dbusmenu_call_get_layout_finish(
      proxy, &revision, &layout, res, &error);

  if (error) {
    g_printerr("Couldn't fetch layout: %s", error->message);
    g_error_free(error);
  }

  parse_root(menu, layout);
}

static void menu_callback(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  BarBarDBusMenu *menu = BARBAR_DBUS_MENU(data);

  StatusNotifierComCanonicalDbusmenu *dbus_menu =
      status_notifier_com_canonical_dbusmenu_proxy_new_for_bus_finish(res,
                                                                      &error);
  static const char *const arg_propertyNames[] = {"type",
                                                  "label",
                                                  "visible",
                                                  "enabled",
                                                  "children-display",
                                                  "accessible-desc",
                                                  NULL};

  status_notifier_com_canonical_dbusmenu_call_get_layout(
      dbus_menu, 0, -1, arg_propertyNames, NULL, layout_callback, menu);
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

static void menu_changed(GMenuModel *model, gint position, gint removed,
                         gint added, gpointer user_data) {

  BarBarDBusMenu *menu = user_data;
  g_menu_model_items_changed(G_MENU_MODEL(menu), position, removed, added);
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
