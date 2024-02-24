#include "barbar-tray-item.h"
#include <gio/gio.h>
#include <stdio.h>

struct _BarBarTrayItem {
  GtkWidget parent_instance;

  const gchar *object_path;
  const gchar *bus_name;

  gchar *icon_name;
  GdkPixbuf *icon_pixmap;

  gchar *attention_name;
  GdkPixbuf *attention_pixmap;
  gchar *attention_movie;

  gchar *overlay_name;
  GdkPixbuf *overlay_pixmap;

  GtkIconTheme *icon_theme;

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

static void update_property(GDBusProxy *self, gchar *sender_name,
                            gchar *signal_name, GVariant *parameters,
                            gpointer user_data) {

  BarBarTrayItem *item = BARBAR_TRAY_ITEM(user_data);

  // It seems like we don't trust clint signal the update correctly,
  // so on each update we update all
}

static GtkWidget *load_icon(BarBarTrayItem *self, const char *name, int height,
                            int scale) {
  GtkWidget *image;
  if (!name) {
    return NULL;
  }

  GtkIconPaintable *paintable = gtk_icon_theme_lookup_icon(
      self->icon_theme, name, NULL, height, scale, GTK_TEXT_DIR_LTR, 0);
  image = gtk_image_new_from_paintable(GDK_PAINTABLE(paintable));
  g_object_unref(paintable);
  return image;
}

static GtkWidget *load_pixmap(GdkPixbuf *pixmap) {
  GtkWidget *image;
  if (!pixmap) {
    return NULL;
  }

  GdkTexture *texture = gdk_texture_new_for_pixbuf(pixmap);
  if (texture) {
    image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
    g_object_unref(texture);
    return image;
  }
  return NULL;
}

static GtkWidget *select_icon(BarBarTrayItem *self) {
  GtkWidget *image = NULL;

  int scale = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  int height = gtk_widget_get_height(GTK_WIDGET(self));

  image = load_icon(self, self->attention_name, height, scale);
  if (image) {
    return image;
  }

  image = load_pixmap(self->attention_pixmap);
  if (image) {
    return image;
  }
  image = load_icon(self, self->icon_name, height, scale);

  if (image) {
    return image;
  }

  image = load_pixmap(self->icon_pixmap);
  if (image) {
    return image;
  }

  // TODO: return a default image

  return NULL;
}

static void apply_overlay(BarBarTrayItem *self, GtkWidget *icon) {
  GtkWidget *overlay;
  GtkWidget *overlay_image = NULL;

  int scale = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  int height = gtk_widget_get_height(GTK_WIDGET(self));

  if (self->overlay_name) {
    gboolean exist =
        gtk_icon_theme_has_icon(self->icon_theme, self->overlay_name);

    if (exist) {
      GtkIconPaintable *paintable =
          gtk_icon_theme_lookup_icon(self->icon_theme, self->overlay_name, NULL,
                                     height, scale, GTK_TEXT_DIR_LTR, 0);
      g_object_unref(paintable);
      overlay_image = gtk_image_new_from_paintable(GDK_PAINTABLE(paintable));
    }
  } else if (self->overlay_pixmap) {
    GdkTexture *texture = gdk_texture_new_for_pixbuf(self->overlay_pixmap);
    if (texture) {
      overlay_image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
      g_object_unref(texture);
    }
  }

  if (overlay_image != NULL) {
    overlay = gtk_overlay_new();
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), icon);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), overlay_image);
    gtk_widget_set_parent(self->image, GTK_WIDGET(self));
    g_object_unref(icon);
    g_object_unref(overlay_image);
    self->image = overlay;
  } else {
    gtk_widget_set_parent(self->image, GTK_WIDGET(self));
    self->image = icon;
  }
}

static void set_icon(BarBarTrayItem *self) {
  GtkWidget *icon = select_icon(self);
  self->image = icon;
  gtk_widget_set_parent(self->image, GTK_WIDGET(self));

  // apply_overlay(self, icon);
}

static void item_callback(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;

  BarBarTrayItem *self = BARBAR_TRAY_ITEM(data);

  StatusNotifierItem *item = status_notifier_item_proxy_new_finish(res, &error);

  if (error) {
    g_printerr("tray item result: %s\n", error->message);
  }

  self->icon_name = strdup(status_notifier_item_get_icon_name(item));
  self->icon_theme =
      gtk_icon_theme_get_for_display(gtk_widget_get_display(GTK_WIDGET(self)));
  // overlay_icon = status_notifier_item_get_overlay_icon_name(item);
  // attention_icon = status_notifier_item_get_attention_icon_name(item);

  // icon_name = status_notifier_item_get_icon_name(item);
  // self->image = gtk_image_new_from_icon_name(icon_name);
  //
  set_icon(self);

  g_signal_connect(item, "g-signal", G_CALLBACK(update_property), self);
  // gtk_widget_set_parent(self->image, GTK_WIDGET(self));
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
