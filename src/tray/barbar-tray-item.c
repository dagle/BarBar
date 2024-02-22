#include "barbar-tray-item.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarTrayItem {
  GtkWidget parent_instance;

  const gchar *object_path;
  const gchar *bus_name;

  GtkWidget *image;
};

enum {
  PROP_0,

  PROP_NAME,
  PROP_PATH,

  NUM_PROPERTIES,
};

static GParamSpec *tray_items_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarTrayItem, g_barbar_tray_item, GTK_TYPE_WIDGET)

static void g_barbar_tray_item_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  BarBarTrayItem *item = BARBAR_TRAY_ITEM(object);

  switch (property_id) {
  case PROP_NAME:
    item->bus_name = strdup(g_value_get_string(value));
    break;
  case PROP_PATH:
    item->object_path = strdup(g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_tray_item_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarTrayItem *item = BARBAR_TRAY_ITEM(object);

  switch (property_id) {
  case PROP_NAME:
    g_value_set_string(value, item->bus_name);
    break;
  case PROP_PATH:
    g_value_set_string(value, item->object_path);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

// static void g_barbar_tray_root(GtkWidget *widget);
static void g_barbar_tray_item_constructed(GObject *object);
static void g_barbar_tray_item_class_init(BarBarTrayItemClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_tray_item_set_property;
  gobject_class->get_property = g_barbar_tray_item_get_property;
  gobject_class->constructed = g_barbar_tray_item_constructed;

  tray_items_props[PROP_NAME] = g_param_spec_string(
      "name", "busname", "busname for the item", NULL,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  tray_items_props[PROP_PATH] = g_param_spec_string(
      "path", "objectpath", "objectpath for the item", NULL,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    tray_items_props);
  // widget_class->root = g_barbar_tray_root;

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "river-tag");
}

static void item_callback(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  const char *icon_name;
  const char *theme_path;

  GtkIconTheme *theme;

  BarBarTrayItem *widget = BARBAR_TRAY_ITEM(data);

  StatusNotifierItem *item = status_notifier_item_proxy_new_finish(res, &error);

  if (error) {
    g_printerr("tray item result: %s\n", error->message);
  }

  icon_name = status_notifier_item_get_icon_name(item);
  widget->image = gtk_image_new_from_icon_name(icon_name);
  gtk_widget_set_parent(widget->image, GTK_WIDGET(widget));
}

static void g_barbar_tray_item_constructed(GObject *object) {
  BarBarTrayItem *item = BARBAR_TRAY_ITEM(object);

  status_notifier_item_proxy_new_for_bus(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, item->bus_name,
      item->object_path, NULL, item_callback, item);
}

static void g_barbar_tray_item_init(BarBarTrayItem *self) {}

BarBarTrayItem *g_barbar_tray_item_new(const char *bus_name,
                                       const char *object_path) {

  return g_object_new(BARBAR_TYPE_TRAY_ITEM, "name", bus_name, "path",
                      object_path, NULL);
}
