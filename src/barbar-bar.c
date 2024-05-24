#include "barbar-bar.h"
#include <gtk/gtk.h>

#include <gtk4-layer-shell.h>
/**
 * BarBarBar:
 *
 * BarBarBar is a statusbar widget for layered shell. It will create a window
 * and set the bar to the foreground.
 *
 */

struct _BarBarBar {
  GtkWindow parent_instance;

  int left_margin;
  int right_margin;
  int top_margin;
  int bottom_margin;
  uint screen_num;

  int height;

  BarBarBarPosition pos;
};

G_DEFINE_TYPE(BarBarBar, g_barbar_bar, GTK_TYPE_WINDOW)

enum {
  PROP_0,

  PROP_LEFT_MARGIN,
  PROP_RIGHT_MARGIN,
  PROP_TOP_MARGIN,
  PROP_BOTTOM_MARGIN,
  PROP_BAR_HEIGHT,

  PROP_SCREEN_NUM,

  PROP_BAR_POS,
  NUM_PROPERTIES,
};

// TODO: set_size(width, height)

GType g_barbar_position_get_type(void) {

  static gsize barbar_bar_role_type;
  if (g_once_init_enter(&barbar_bar_role_type)) {

    static GEnumValue pattern_types[] = {
        {BARBAR_POS_TOP, "BARBAR_POS_TOP", "top"},
        {BARBAR_POS_BOTTOM, "BARBAR_POS_BOTTOM", "bot"},
        {BARBAR_POS_LEFT, "BARBAR_POS_LEFT", "left"},
        {BARBAR_POS_RIGHT, "BARBAR_POS_RIGHT", "right"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarBarPosition", pattern_types);
    g_once_init_leave(&barbar_bar_role_type, type);
  }
  return barbar_bar_role_type;
}

static GParamSpec *bar_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_bar_set_pos_internal(BarBarBar *bar,
                                          BarBarBarPosition pos) {
  GtkWindow *gtk_window = GTK_WINDOW(bar);

  for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
    gtk_layer_set_anchor(gtk_window, i, TRUE);
  }

  switch (bar->pos) {
  case BARBAR_POS_TOP:
    gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
    break;
  case BARBAR_POS_BOTTOM:
    gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_TOP, FALSE);
    break;
  case BARBAR_POS_LEFT:
    gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
    break;
  case BARBAR_POS_RIGHT:
    gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
    break;
  }
}

static void g_barbar_bar_set_pos(BarBarBar *bar, BarBarBarPosition pos) {
  g_return_if_fail(BARBAR_IS_BAR(bar));

  if (bar->pos == pos) {
    return;
  }

  bar->pos = pos;

  g_barbar_bar_set_pos_internal(bar, pos);

  g_object_notify_by_pspec(G_OBJECT(bar), bar_props[PROP_BAR_POS]);
}

static void g_barbar_bar_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {
  BarBarBar *bar = BARBAR_BAR(object);

  switch (property_id) {
  case PROP_LEFT_MARGIN:
    bar->left_margin = g_value_get_uint(value);
    break;
  case PROP_RIGHT_MARGIN:
    bar->right_margin = g_value_get_uint(value);
    break;
  case PROP_TOP_MARGIN:
    bar->top_margin = g_value_get_uint(value);
    break;
  case PROP_BOTTOM_MARGIN:
    bar->bottom_margin = g_value_get_uint(value);
    break;
  case PROP_BAR_POS:
    g_barbar_bar_set_pos(bar, g_value_get_enum(value));
    break;
  case PROP_BAR_HEIGHT:
    bar->height = g_value_get_uint(value);
    break;
  case PROP_SCREEN_NUM:
    bar->screen_num = g_value_get_uint(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_bar_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {

  BarBarBar *bar = BARBAR_BAR(object);

  switch (property_id) {
  case PROP_LEFT_MARGIN:
    g_value_set_uint(value, bar->left_margin);
    break;
  case PROP_RIGHT_MARGIN:
    g_value_set_uint(value, bar->right_margin);
    break;
  case PROP_TOP_MARGIN:
    g_value_set_uint(value, bar->top_margin);
    break;
  case PROP_BOTTOM_MARGIN:
    g_value_set_uint(value, bar->bottom_margin);
    break;
  case PROP_BAR_POS:
    g_value_set_enum(value, bar->pos);
    break;
  case PROP_BAR_HEIGHT:
    g_value_set_uint(value, bar->height);
    break;
  case PROP_SCREEN_NUM:
    g_value_set_uint(value, bar->screen_num);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_bar_constructed(GObject *object);

// GObject g_barbar_bar_constructor(GType type, guint n_construct_properties,
//                                  GObjectConstructParam *construct_properties)
//                                  {
//
//   GObject *object;
//
//   return NULL;
//
// }

static void g_barbar_bar_class_init(BarBarBarClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_bar_set_property;
  gobject_class->get_property = g_barbar_bar_get_property;
  // gobject_class->constructor = g_barbar_bar_constructor;
  gobject_class->constructed = g_barbar_bar_constructed;

  /**
   * BarBarBar:left-margin:
   *
   * Padding from the right
   */
  bar_props[PROP_LEFT_MARGIN] =
      g_param_spec_uint("left-margin", NULL, NULL, 0, G_MAXUINT, 0,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarBar:right-margin:
   *
   * Padding from the right
   */
  bar_props[PROP_RIGHT_MARGIN] =
      g_param_spec_uint("right-margin", NULL, NULL, 0, G_MAXUINT, 0,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarBar:top-margin:
   *
   * Padding from the top
   */
  bar_props[PROP_TOP_MARGIN] =
      g_param_spec_uint("top-margin", NULL, NULL, 0, G_MAXUINT, 0,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarBar:bottom-margin:
   *
   * Padding from the bottom
   */
  bar_props[PROP_BOTTOM_MARGIN] =
      g_param_spec_uint("bottom-margin", NULL, NULL, 0, G_MAXUINT, 0,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarBar:bar-height:
   *
   * How large should the bar be
   */
  bar_props[PROP_BAR_HEIGHT] =
      g_param_spec_uint("bar-height", NULL, NULL, 0, G_MAXUINT, 20,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarBar:screen-num:
   *
   * What screen to display the bar on.
   */
  bar_props[PROP_SCREEN_NUM] =
      g_param_spec_uint("screen-num", NULL, NULL, 0, G_MAXUINT, 0,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarBar:bar-pos:
   *
   * [enum@BarBarBar.Position] Where to display the bar on the screen. Default
   * is top.
   */
  bar_props[PROP_BAR_POS] = g_param_spec_enum(
      "bar-pos", "barposition", "the position of the bar", BARBAR_TYPE_POSITION,
      BARBAR_POS_TOP,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, bar_props);
  gtk_widget_class_set_css_name(widget_class, "bar");
}

static void g_barbar_bar_init(BarBarBar *self) {
  GtkWindow *gtk_window = GTK_WINDOW(self);

  gtk_layer_init_for_window(gtk_window);
}

static void g_barbar_bar_constructed(GObject *object) {
  G_OBJECT_CLASS(g_barbar_bar_parent_class)->constructed(object);
  BarBarBar *bar = BARBAR_BAR(object);

  GtkWindow *gtk_window = GTK_WINDOW(object);

  gtk_layer_set_namespace(gtk_window, "bar");
  gtk_layer_set_layer(gtk_window, GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_auto_exclusive_zone_enable(gtk_window);

  // something like this, make it possible to identify the monitor by name
  GdkDisplay *gdk_display = gdk_display_get_default();
  GListModel *monitors = gdk_display_get_monitors(gdk_display);
  if (bar->screen_num <= g_list_model_get_n_items(monitors)) {
    GdkMonitor *monitor = g_list_model_get_item(monitors, bar->screen_num);
    if (monitor) {
      gtk_layer_set_monitor(gtk_window, monitor);
    }
  }

  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_LEFT, bar->left_margin);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_RIGHT,
                       bar->right_margin);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_TOP, bar->top_margin);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM,
                       bar->bottom_margin);

  g_barbar_bar_set_pos_internal(bar, bar->pos);
}

/**
 * g_barbar_bar_new:
 *
 * Creates a new `BarBarBar`.
 *
 * Returns: (transfer full): a new `BarBarBar`.
 */
GtkWidget *g_barbar_bar_new(void) {
  BarBarBar *bar;

  bar = g_object_new(BARBAR_TYPE_BAR, NULL);

  return GTK_WIDGET(bar);
}
