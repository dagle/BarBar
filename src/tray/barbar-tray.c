#include "barbar-tray.h"
#include "barbar-tray-item.h"
#include "barbar-watcher.h"
#include "glib-object.h"
#include <stdio.h>

struct _BarBarTray {
  GtkWidget parent;

  BarBarTrayDefaultRule rule;
  GList *items;
  BarBarStatusWatcher *watcher;
};

enum {
  PROP_0,

  PROP_RULE_MODE,

  NUM_PROPERTIES,
};

GType g_barbar_default_rule_get_type(void) {

  static gsize barbar_default_rule_type;
  if (g_once_init_enter(&barbar_default_rule_type)) {

    static GEnumValue pattern_types[] = {
        {BARBAR_TRAY_ALLOW, "BARBAR_TRAY_ALLOW", "allow"},
        {BARBAR_TRAY_BLOCK, "BARBAR_TRAY_BLOCK", "block"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarTrayDefaultRule", pattern_types);
    g_once_init_leave(&barbar_default_rule_type, type);
  }
  return barbar_default_rule_type;
}

G_DEFINE_TYPE(BarBarTray, g_barbar_tray, GTK_TYPE_WIDGET)

static GParamSpec *tray_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_layout_start(GtkWidget *widget);

static void g_barbar_tray_set_rule(BarBarTray *tray,
                                   BarBarTrayDefaultRule rule) {
  g_return_if_fail(BARBAR_IS_TRAY(tray));

  if (tray->rule == rule) {
    return;
  }

  tray->rule = rule;
  g_object_notify_by_pspec(G_OBJECT(tray), tray_props[PROP_RULE_MODE]);
}

static void g_barbar_tray_set_property(GObject *object, guint property_id,
                                       const GValue *value, GParamSpec *pspec) {
  BarBarTray *tray = BARBAR_TRAY(object);

  switch (property_id) {
  case PROP_RULE_MODE:
    g_barbar_tray_set_rule(tray, g_value_get_enum(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_tray_get_property(GObject *object, guint property_id,
                                       GValue *value, GParamSpec *pspec) {
  BarBarTray *tray = BARBAR_TRAY(object);

  switch (property_id) {
  case PROP_RULE_MODE:
    g_value_set_enum(value, tray->rule);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_tray_class_init(BarBarTrayClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_tray_set_property;
  gobject_class->get_property = g_barbar_tray_get_property;
  widget_class->root = g_barbar_dwl_layout_start;

  tray_props[PROP_RULE_MODE] = g_param_spec_enum(
      "rule-mode", "rule mode", "how to apply rules for the tray",
      BARBAR_TYPE_DEFAULT_RULE, BARBAR_TRAY_ALLOW,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, tray_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "tray");
}

static void item_register(BarBarStatusWatcher *watcher, char *bus_name,
                          char *object_path, gpointer data) {
  BarBarTray *tray = BARBAR_TRAY(data);
  BarBarTrayItem *item = g_barbar_tray_item_new(bus_name, object_path);
  tray->items = g_list_append(tray->items, item);
  gtk_widget_set_parent(GTK_WIDGET(item), GTK_WIDGET(tray));
}

static void item_unregister(BarBarStatusWatcher *watcher, char *bus_name,
                            char *object_path, gpointer data) {
  BarBarTray *tray = BARBAR_TRAY(data);
  BarBarTrayItem *item;

  GList *items = tray->items;

  while (items) {
    item = items->data;

    if (g_barbar_tray_item_equal(item, bus_name, object_path)) {
      tray->items = g_list_remove(tray->items, item);
      gtk_widget_unparent(GTK_WIDGET(item));
      return;
    }
    items = items->next;
  }
}

static void g_barbar_tray_init(BarBarTray *self) {}

static void g_barbar_dwl_layout_start(GtkWidget *widget) {
  BarBarTray *tray = BARBAR_TRAY(widget);
  GTK_WIDGET_CLASS(g_barbar_tray_parent_class)->root(widget);

  tray->watcher = g_barbar_status_watcher_new();

  g_signal_connect(tray->watcher, "item-register", G_CALLBACK(item_register),
                   tray);
  g_signal_connect(tray->watcher, "item-unregister",
                   G_CALLBACK(item_unregister), tray);
}

GtkWidget *g_barbar_tray_new(void) {
  BarBarTray *tray;

  tray = g_object_new(BARBAR_TYPE_TRAY, NULL);

  return GTK_WIDGET(tray);
}
