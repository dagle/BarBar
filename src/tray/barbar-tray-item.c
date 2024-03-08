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
  GtkWidget *popover;

  StatusNotifierItem *item;
  GtkGesture *gesture;

  gboolean active;
  guint pending;

  GSimpleActionGroup *actions;
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

static GdkPixbuf *into_pixbuf(GVariant *var, int height, int width);
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

static void set_icon(BarBarTrayItem *self);

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

void barbar_tray_item_set_icon_theme(BarBarTrayItem *item, const char *name) {
  if (item->icon_theme) {
    g_clear_pointer(&item->icon_theme, g_object_unref);
  }

  if (name && strlen(name) > 0) {
    const char *const path[] = {name, NULL};
    item->icon_theme = gtk_icon_theme_new();
    gtk_icon_theme_set_resource_path(item->icon_theme, path);
  }
}

void barbar_tray_item_set_icon_name(BarBarTrayItem *item, const char *name) {
  if (!g_strcmp0(item->icon_name, name)) {
    return;
  }

  g_free(item->icon_name);

  item->icon_name = g_strdup(name);
}

void barbar_tray_item_set_icon_pixmap(BarBarTrayItem *item, GVariant *var) {
  int height = gtk_widget_get_height(GTK_WIDGET(item));
  int width = gtk_widget_get_width(GTK_WIDGET(item));

  if (item->icon_pixmap) {
    g_object_unref(item->icon_pixmap);
  }

  item->icon_pixmap = into_pixbuf(var, height, width);
}

void barbar_tray_item_set_overlay_icon_name(BarBarTrayItem *item,
                                            const char *name) {
  if (!g_strcmp0(item->overlay_name, name)) {
    return;
  }

  g_free(item->overlay_name);

  item->overlay_name = g_strdup(name);
}

void barbar_tray_item_set_overlay_pixmap(BarBarTrayItem *item, GVariant *var) {
  int height = gtk_widget_get_height(GTK_WIDGET(item));
  int width = gtk_widget_get_width(GTK_WIDGET(item));

  if (item->overlay_pixmap) {
    g_object_unref(item->overlay_pixmap);
  }

  item->overlay_pixmap = into_pixbuf(var, height, width);
}

void barbar_tray_item_set_attention_icon_name(BarBarTrayItem *item,
                                              const char *name) {
  if (!g_strcmp0(item->attention_name, name)) {
    return;
  }

  g_free(item->attention_name);

  item->attention_name = g_strdup(name);
}

void barbar_tray_item_set_attention_icon_pixmap(BarBarTrayItem *item,
                                                GVariant *var) {
  int height = gtk_widget_get_height(GTK_WIDGET(item));
  int width = gtk_widget_get_width(GTK_WIDGET(item));

  if (item->attention_pixmap) {
    g_object_unref(item->attention_pixmap);
  }

  item->attention_pixmap = into_pixbuf(var, height, width);
}

void barbar_tray_item_set_attention_movie_name(BarBarTrayItem *item,
                                               const char *name) {

  if (!g_strcmp0(item->attention_movie, name)) {
    return;
  }

  g_free(item->attention_movie);

  item->attention_movie = g_strdup(name);
}

void barbar_tray_item_set_tooltip(BarBarTrayItem *item, const char *name) {}
void barbar_tray_item_set_menu(BarBarTrayItem *item, const char *path) {
  if (item->menu) {
    const char *menu_path = g_barbar_dbus_menu_path(item->menu);
    if (!g_strcmp0(menu_path, path)) {
      return;
    }
    g_object_unref(item->menu);
  }
  item->menu = g_barbar_dbus_menu_new(item->bus_name, path);
}
void barbar_tray_item_set_is_menu(BarBarTrayItem *item, gboolean b) {}

static void set_properties(BarBarTrayItem *item, const char *prop,
                           GVariant *value) {

  if (!strcmp(prop, "IconThemePath")) {
    const char *name = g_variant_get_string(value, NULL);
    barbar_tray_item_set_icon_theme(item, name);
  } else if (!strcmp(prop, "IconName")) {
    const char *name = g_variant_get_string(value, NULL);
    barbar_tray_item_set_icon_name(item, name);
  } else if (!strcmp(prop, "IconPixmap")) {
    barbar_tray_item_set_icon_pixmap(item, value);
  } else if (!strcmp(prop, "OverlayIconName")) {
    const char *name = g_variant_get_string(value, NULL);
    barbar_tray_item_set_overlay_icon_name(item, name);
  } else if (!strcmp(prop, "OverlayIconPixmap")) {
    barbar_tray_item_set_overlay_pixmap(item, value);
  } else if (!strcmp(prop, "AttentionIconName")) {
    const char *name = g_variant_get_string(value, NULL);
    barbar_tray_item_set_attention_icon_name(item, name);
  } else if (!strcmp(prop, "AttentionIconPixmap")) {
    barbar_tray_item_set_attention_icon_pixmap(item, value);
  } else if (!strcmp(prop, "AttentionMovieName")) {
    const char *name = g_variant_get_string(value, NULL);
    barbar_tray_item_set_attention_movie_name(item, name);
  } else if (!strcmp(prop, "ToolTip")) {
    // barbar_tray_item_set_tooltip(item);
  } else if (!strcmp(prop, "Menu")) {
    char *path;
    g_variant_get(value, "o", &path);
    barbar_tray_item_set_menu(item, path);
  } else if (!strcmp(prop, "ItemIsMenu")) {
    gboolean b = g_variant_get_boolean(value);
    barbar_tray_item_set_is_menu(item, b);
  }
}

static void properties_result(GObject *object, GAsyncResult *res,
                              gpointer data) {

  GDBusConnection *connection = G_DBUS_CONNECTION(object);
  BarBarTrayItem *self = BARBAR_TRAY_ITEM(data);
  GError *error = NULL;
  GVariantIter iter;

  GVariant *variant = g_dbus_connection_call_finish(connection, res, &error);
  GVariant *arg0 = g_variant_get_child_value(variant, 0);
  g_variant_iter_init(&iter, arg0);

  GVariant *value;
  char *prop;

  while (g_variant_iter_loop(&iter, "{sv}", &prop, &value)) {
    set_properties(self, prop, value);
  }

  set_icon(self);

  self->pending = 0;
}

static int get_all_properties(gpointer data) {
  BarBarTrayItem *self = BARBAR_TRAY_ITEM(data);

  g_dbus_connection_call(
      g_dbus_proxy_get_connection(G_DBUS_PROXY(self->item)), self->bus_name,
      self->object_path, "org.freedesktop.DBus.Properties", "GetAll",
      g_variant_new("(s)",
                    g_dbus_proxy_get_interface_name(G_DBUS_PROXY(self->item))),
      g_variant_type_new("(a{sv})"), G_DBUS_CALL_FLAGS_NONE, -1, NULL,
      properties_result, self);
  return G_SOURCE_REMOVE;
}

static void schedule_property(BarBarTrayItem *item) {
  if (!item->pending) {
    item->pending = g_timeout_add(100, get_all_properties, item);
  }
}

static void update_property(GDBusProxy *self, gchar *sender_name,
                            gchar *signal_name, GVariant *parameters,
                            gpointer user_data) {
  BarBarTrayItem *item = BARBAR_TRAY_ITEM(user_data);
  schedule_property(item);
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
      icon_theme, name, NULL, size, scale, GTK_TEXT_DIR_LTR, 1);
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

static void click_pressed_cb(GtkGestureClick *gesture, guint n_press, double x,
                             double y, gpointer data) {

  BarBarTrayItem *self = BARBAR_TRAY_ITEM(data);
  GtkGestureSingle *click = GTK_GESTURE_SINGLE(gesture);
  gint i = gtk_gesture_single_get_current_button(click);

  if (i == 1) {
    status_notifier_item_call_activate(self->item, 0, 0, NULL, NULL, NULL);
  } else if (i == 3) {
    if (self->popover) {
      if (!self->active) {
        gtk_popover_popup(GTK_POPOVER(self->popover));
        self->active = TRUE;
      } else {
        gtk_popover_popdown(GTK_POPOVER(self->popover));
        self->active = FALSE;
      }
    }
  }
}

static gboolean menu_deactivate_cb(BarBarTrayItem *self) {
  self->active = false;
  return TRUE;
}

static void set_popup(BarBarTrayItem *self) {

  if (self->popover) {
    g_clear_pointer(&self->popover, gtk_widget_unparent);
  }

  self->popover =
      gtk_popover_menu_new_from_model_full(G_MENU_MODEL(self->menu), 0);

  gtk_popover_popdown(GTK_POPOVER(self->popover));
  self->active = FALSE;

  g_signal_connect_swapped(self->popover, "closed",
                           G_CALLBACK(menu_deactivate_cb), self);
}

static void set_icon(BarBarTrayItem *self) {
  GtkWidget *icon = select_icon(self);

  if (self->image) {
    g_clear_pointer(&self->image, gtk_widget_unparent);
  }

  self->image = icon;

  set_popup(self);

  gtk_widget_set_parent(self->popover, GTK_WIDGET(self));
  gtk_widget_set_parent(self->image, GTK_WIDGET(self));
}

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

static void activate(GSimpleAction *action, GVariant *parameter,
                     gpointer user_data) {
  BarBarTrayItem *item = BARBAR_TRAY_ITEM(user_data);

  if (item->menu) {
    g_barbar_dbus_menu_event(item->menu, g_variant_get_int32(parameter));
  }
}

static GActionEntry entries[] = {
    {"activate", activate, "i", NULL, NULL},
};

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

static void item_callback(GObject *object, GAsyncResult *res, gpointer data) {
  GError *error = NULL;
  BarBarTrayItem *self = BARBAR_TRAY_ITEM(data);

  self->item = status_notifier_item_proxy_new_finish(res, &error);
  if (error) {
    g_printerr("tray item result: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (error) {
    g_printerr("call result: %s\n", error->message);
    g_error_free(error);
    return;
  }
  schedule_property(self);

  g_signal_connect(self->item, "g-signal", G_CALLBACK(update_property), self);
}

static void g_barbar_tray_item_constructed(GObject *object) {
  G_OBJECT_CLASS(g_barbar_tray_item_parent_class)->constructed(object);
  BarBarTrayItem *item = BARBAR_TRAY_ITEM(object);

  if (!item->default_theme) {
    item->default_theme = gtk_icon_theme_get_for_display(
        gtk_widget_get_display(GTK_WIDGET(item)));
  }

  status_notifier_item_proxy_new_for_bus(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, item->bus_name,
      item->object_path, NULL, item_callback, item);
}

static void g_barbar_tray_item_init(BarBarTrayItem *self) {
  self->gesture = gtk_gesture_click_new();

  gtk_gesture_single_set_touch_only(GTK_GESTURE_SINGLE(self->gesture), FALSE);
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(self->gesture), 0);

  g_signal_connect(self->gesture, "pressed", G_CALLBACK(click_pressed_cb),
                   self);

  gtk_event_controller_set_propagation_phase(
      GTK_EVENT_CONTROLLER(self->gesture), GTK_PHASE_CAPTURE);

  self->actions = g_simple_action_group_new();

  g_action_map_add_action_entries(G_ACTION_MAP(self->actions), entries,
                                  G_N_ELEMENTS(entries), self);

  gtk_widget_insert_action_group(GTK_WIDGET(self), "trayitem",
                                 G_ACTION_GROUP(self->actions));

  gtk_widget_add_controller(GTK_WIDGET(self),
                            GTK_EVENT_CONTROLLER(self->gesture));
}

BarBarTrayItem *g_barbar_tray_item_new(const char *bus_name,
                                       const char *object_path) {

  return g_object_new(BARBAR_TYPE_TRAY_ITEM, "name", bus_name, "path",
                      object_path, NULL);
}
