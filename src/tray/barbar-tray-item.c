#include "barbar-tray-item.h"
#include "barbar-dbusmenu.h"
#include "status-notifier.h"
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

struct _BarBarTrayItem {
  GtkWidget parent_instance;

  gchar *object_path;
  gchar *bus_name;

  gchar *icon_name;
  GdkPixbuf *icon_pixmap;

  GtkIconTheme *icon_theme;
  GtkIconTheme *default_theme;

  gchar *attention_name;
  GdkPixbuf *attention_pixmap;
  gchar *attention_movie;

  gchar *overlay_name;
  GdkPixbuf *overlay_pixmap;

  BarBarDBusMenu *menu;

  GtkWidget *image;
  GtkWidget *button;
};

enum {
  PROP_0,

  PROP_NAME,
  PROP_PATH,

  // PROP_DEFAULT_THEME,
  // PROP_OVERRIDE_THEME,

  NUM_PROPERTIES,
};

static GParamSpec *tray_items_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarTrayItem, g_barbar_tray_item, GTK_TYPE_WIDGET)
// static void dump_item(BarBarTrayItem *self);

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
  // case PROP_DEFAULT_THEME:
  //   item->default_theme = g_object_ref(g_value_get_object(value));
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
  // tray_items_props[PROP_DEFAULT_THEME] = g_param_spec_object(
  //     "default-theme", "default-theme", "default-theme for the icons", NULL,
  //     G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  // tray_items_props[PROP_OVERRIDE_THEME] = g_param_spec_boolean(
  //     "override-theme", "override-theme", "If the icon ", FALSE,
  //     G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

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

// returns the first at least the default size
static int select_size(GtkIconTheme *icon_theme, const char *name,
                       int default_size) {
  int suggested = 0;
  int *sizes = gtk_icon_theme_get_icon_sizes(icon_theme, name);
  if (sizes) {
    while (*sizes) {
      int size = *sizes;
      sizes++;
      if (size == default_size) {
        return default_size;
      }
      suggested = MAX(suggested, size);
      if (suggested > default_size) {
        return suggested;
      }
    }
  } else {
    return default_size;
  }

  if (suggested > 0) {
    return suggested;
  } else {
    return default_size;
  }
}

static GtkWidget *load_icon(GtkIconTheme *icon_theme, const char *name,
                            int height, int scale) {
  GtkWidget *image;
  if (!icon_theme) {
    return NULL;
  }

  if (!name) {
    return NULL;
  }

  int size = select_size(icon_theme, name, height);

  GtkIconPaintable *paintable = gtk_icon_theme_lookup_icon(
      icon_theme, name, NULL, size, scale, GTK_TEXT_DIR_LTR, 2);
  image = gtk_image_new_from_paintable(GDK_PAINTABLE(paintable));
  g_object_unref(paintable);
  return image;
}

static GtkWidget *load_icon_fallback(BarBarTrayItem *self, const char *name,
                                     int height, int scale) {
  GtkWidget *image;
  image = load_icon(self->icon_theme, name, height, scale);
  if (image) {
    return image;
  }

  image = load_icon(self->default_theme, name, height, scale);
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

  image = load_icon_fallback(self, self->attention_name, height, scale);
  if (image) {
    return image;
  }

  image = load_pixmap(self->attention_pixmap);
  if (image) {
    return image;
  }
  image = load_icon_fallback(self, self->icon_name, height, scale);
  printf("icon: %p\n", image);

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
  self->button = gtk_menu_button_new();
  gtk_menu_button_set_child(GTK_MENU_BUTTON(self->button), self->image);
  GtkWidget *popover =
      gtk_popover_menu_new_from_model(G_MENU_MODEL(self->menu));
  // printf("image: %p\n", self->image);
  gtk_widget_set_parent(self->button, GTK_WIDGET(self));
  gtk_menu_button_set_popover(GTK_MENU_BUTTON(self->button), popover);
  // gtk_widget_set_parent(popover, GTK_WIDGET(self));
  // gtk_menu_button_set_popover(self->button, popover);
  // apply_overlay(self, icon);
}

// static void dump_item(BarBarTrayItem *self) {
//   printf("Item {\n");
//   printf("icon_name: %s\n", self->icon_name);
//   printf("attention_name: %s\n", self->attention_name);
//   printf("attention_movie: %s\n", self->attention_movie);
//   printf("overlay_name: %s\n", self->overlay_name);
//   printf("}\n");
// }

// get the first pixel buffer that is large enough
// we assume they are sorted, for now.
static GVariant *find_buficon(GVariant *var, int height, int width) {
  GVariant *best_tuple = NULL;
  if (g_variant_is_of_type(var, G_VARIANT_TYPE("a(iiay)"))) {
    gsize n_elements = g_variant_n_children(var);
    for (gsize i = 0; i < n_elements; i++) {
      GVariant *tuple = g_variant_get_child_value(var, i);

      gint32 rheight, rwidth;
      GVariant *pixels;

      g_variant_get(tuple, "(ii@ay)", &rheight, &rwidth, &pixels);

      if (height >= rheight && width >= rwidth) {
        return tuple;
      }

      if (best_tuple) {
        g_variant_unref(tuple);
        best_tuple = tuple;
      }
    }
  }
  return best_tuple;
}

static GdkPixbuf *into_pixbuf(GVariant *var, int height, int width) {
  GVariant *pixels = NULL;

  if (!var) {
    return NULL;
  }

  GVariant *tuple = find_buficon(var, height, width);

  if (tuple) {
    g_variant_get(tuple, "(ii@ay)", &height, &width, &pixels);

    gsize data_size;
    const guint8 *data =
        g_variant_get_fixed_array(pixels, &data_size, sizeof(guint8));
    // GInputStream *image_stream =
    // g_memory_input_stream_new_from_data(data, data_size, NULL);
    GBytes *image_bytes = g_bytes_new(data, data_size);
    GInputStream *image_stream =
        g_memory_input_stream_new_from_bytes(image_bytes);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_stream(image_stream, NULL, NULL);

    g_variant_unref(tuple);
    g_bytes_unref(image_bytes);
    g_input_stream_close(image_stream, NULL, NULL);

    return pixbuf;
  }
  return NULL;
}

static char *safe_strdup(const char *str) {
  if (!str || strlen(str) == 0) {
    return NULL;
  }
  return strdup(str);
}

static void populate(BarBarTrayItem *self, StatusNotifierItem *item) {
  int height = gtk_widget_get_height(GTK_WIDGET(self));
  int width = gtk_widget_get_width(GTK_WIDGET(self));
  // TODO:

  // if (self->icon_theme) {
  // }
  // g_object_unref(self->icon_theme);
  if (self->icon_theme) {
    g_clear_pointer(&self->icon_theme, g_object_unref);
  }

  const char *icon_theme = status_notifier_item_get_icon_theme_path(item);
  if (icon_theme && strlen(icon_theme) > 0) {
    const char *const *path = {&icon_theme, NULL};
    self->icon_theme = gtk_icon_theme_new();
    // gtk_icon_theme_add_search_path(self->icon_theme, icon_theme);
    gtk_icon_theme_set_resource_path(self->icon_theme, path);
  }

  self->icon_name = safe_strdup(status_notifier_item_get_icon_name(item));

  if (self->icon_pixmap) {
    g_object_unref(self->icon_pixmap);
  }
  self->icon_pixmap =
      into_pixbuf(status_notifier_item_get_icon_pixmap(item), height, width);

  g_free(self->attention_name);
  self->attention_name =
      safe_strdup(status_notifier_item_get_attention_icon_name(item));

  if (self->attention_pixmap) {
    g_object_unref(self->attention_pixmap);
  }
  self->attention_pixmap = into_pixbuf(
      status_notifier_item_get_attention_icon_pixmap(item), height, width);

  g_free(self->attention_movie);
  self->attention_movie =
      safe_strdup(status_notifier_item_get_attention_movie_name(item));

  g_free(self->overlay_name);
  self->overlay_name =
      safe_strdup(status_notifier_item_get_overlay_icon_name(item));

  if (self->overlay_pixmap) {
    g_object_unref(self->overlay_pixmap);
  }
  self->overlay_pixmap = into_pixbuf(
      status_notifier_item_get_overlay_icon_pixmap(item), height, width);

  const gchar *path = status_notifier_item_get_menu(item);

  if (path) {
    self->menu = g_barbar_dbus_menu_new(self->bus_name, path);
  }
}

static void item_callback(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;

  BarBarTrayItem *self = BARBAR_TRAY_ITEM(data);

  StatusNotifierItem *item = status_notifier_item_proxy_new_finish(res, &error);

  if (error) {
    g_printerr("tray item result: %s\n", error->message);
  }

  populate(self, item);
  set_icon(self);

  g_signal_connect(item, "g-signal", G_CALLBACK(update_property), self);
}

static void g_barbar_tray_item_constructed(GObject *object) {
  BarBarTrayItem *item = BARBAR_TRAY_ITEM(object);

  // If no default theme is set, we set a
  if (!item->default_theme) {
    item->default_theme = gtk_icon_theme_get_for_display(
        gtk_widget_get_display(GTK_WIDGET(item)));
  }

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
