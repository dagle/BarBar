#include "barbar-bar.h"
#include "barbar-enum.h"
#include <gtk/gtk.h>

/**
 * BarBarBar:
 *
 * BarBarBar is a statusbar widget for layered shell. It will create a window
 * and set the bar to the foreground.
 *
 */

struct _BarBarBar {
  GtkWindow parent_instance;

  int margins[GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER];
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

/**
 * g_barbar_bar_set_screen_pos:
 * @bar: a `BarBarBar`
 * @pos: where to anchor the screen
 *
 * Where to put the screen.
 */
void g_barbar_bar_set_pos(BarBarBar *bar, BarBarBarPosition pos) {
  g_return_if_fail(BARBAR_IS_BAR(bar));

  if (bar->pos == pos) {
    return;
  }

  bar->pos = pos;

  g_barbar_bar_set_pos_internal(bar, pos);

  g_object_notify_by_pspec(G_OBJECT(bar), bar_props[PROP_BAR_POS]);
}

static void g_barbar_bar_set_screen_num_internal(BarBarBar *bar, uint num) {
  GdkDisplay *gdk_display = gdk_display_get_default();
  GListModel *monitors = gdk_display_get_monitors(gdk_display);
  GtkWindow *gtk_window = GTK_WINDOW(bar);
  uint screen_num;

  screen_num = bar->screen_num;
  if (bar->screen_num > g_list_model_get_n_items(monitors)) {
    g_printerr("Screen index %d doesn't exist, setting bar on screen 0",
               bar->screen_num);
    screen_num = 0;
  }

  GdkMonitor *monitor = g_list_model_get_item(monitors, screen_num);
  if (monitor) {
    gtk_layer_set_monitor(gtk_window, monitor);
  }
}

/**
 * g_barbar_bar_set_screen_num:
 * @bar: a `BarBarBar`
 * @num: screen index
 *
 * What screen to display the bar on.
 */
void g_barbar_bar_set_screen_num(BarBarBar *bar, uint num) {
  g_return_if_fail(BARBAR_IS_BAR(bar));

  if (bar->screen_num == num) {
    return;
  }
  bar->screen_num = num;

  g_barbar_bar_set_screen_num_internal(bar, num);

  g_object_notify_by_pspec(G_OBJECT(bar), bar_props[PROP_SCREEN_NUM]);
}

void g_barbar_bar_set_height(BarBarBar *bar, uint height) {
  g_return_if_fail(BARBAR_IS_BAR(bar));

  if (bar->height == height) {
    return;
  }
  bar->height = height;
  g_object_notify_by_pspec(G_OBJECT(bar), bar_props[PROP_BAR_HEIGHT]);
}

/**
 * g_barbar_bar_set_margin:
 * @bar: a `BarBarBar`
 * @edge: from what edge the margin should basd
 * @margin: how big margin we should have
 *
 * Creates a margin between the bar and an edge of the screen
 *
 */
void g_barbar_bar_set_margin(BarBarBar *bar, GtkLayerShellEdge edge,
                             uint margin) {
  g_return_if_fail(BARBAR_IS_BAR(bar));
  GtkWindow *gtk_window = GTK_WINDOW(bar);

  gtk_layer_set_margin(gtk_window, edge, bar->margins[edge]);

  if (bar->margins[edge] == margin) {
    return;
  }

  bar->margins[edge] = margin;
  g_object_notify_by_pspec(G_OBJECT(bar), bar_props[PROP_LEFT_MARGIN + edge]);
}

static void g_barbar_bar_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {
  BarBarBar *bar = BARBAR_BAR(object);

  switch (property_id) {
  case PROP_LEFT_MARGIN:
    g_barbar_bar_set_margin(bar, GTK_LAYER_SHELL_EDGE_LEFT,
                            g_value_get_uint(value));
    break;
  case PROP_RIGHT_MARGIN:
    g_barbar_bar_set_margin(bar, GTK_LAYER_SHELL_EDGE_RIGHT,
                            g_value_get_uint(value));
    break;
  case PROP_TOP_MARGIN:
    g_barbar_bar_set_margin(bar, GTK_LAYER_SHELL_EDGE_TOP,
                            g_value_get_uint(value));
    break;
  case PROP_BOTTOM_MARGIN:
    g_barbar_bar_set_margin(bar, GTK_LAYER_SHELL_EDGE_BOTTOM,
                            g_value_get_uint(value));
    break;
  case PROP_BAR_POS:
    g_barbar_bar_set_pos(bar, g_value_get_enum(value));
    break;
  case PROP_BAR_HEIGHT:
    g_barbar_bar_set_height(bar, g_value_get_uint(value));
    break;
  case PROP_SCREEN_NUM:
    g_barbar_bar_set_screen_num(bar, g_value_get_uint(value));
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
    g_value_set_uint(value, bar->margins[GTK_LAYER_SHELL_EDGE_LEFT]);
    break;
  case PROP_RIGHT_MARGIN:
    g_value_set_uint(value, bar->margins[GTK_LAYER_SHELL_EDGE_RIGHT]);
    break;
  case PROP_TOP_MARGIN:
    g_value_set_uint(value, bar->margins[GTK_LAYER_SHELL_EDGE_TOP]);
    break;
  case PROP_BOTTOM_MARGIN:
    g_value_set_uint(value, bar->margins[GTK_LAYER_SHELL_EDGE_BOTTOM]);
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

static void g_barbar_bar_root(GObject *widget);

static void g_barbar_bar_class_init(BarBarBarClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_bar_set_property;
  gobject_class->get_property = g_barbar_bar_get_property;
  gobject_class->constructed = g_barbar_bar_root;

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
  gtk_layer_set_namespace(gtk_window, "bar");
  gtk_layer_set_layer(gtk_window, GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_auto_exclusive_zone_enable(gtk_window);
}

static void g_barbar_bar_root(GObject *widget) {
  G_OBJECT_CLASS(g_barbar_bar_parent_class)->constructed(widget);
  BarBarBar *bar = BARBAR_BAR(widget);

  // maybe we can move these to init
  g_barbar_bar_set_screen_num_internal(bar, bar->screen_num);
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
