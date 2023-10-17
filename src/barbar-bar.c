#include "barbar-bar.h"
#include <gtk4-layer-shell.h>

struct _BarBarBar {
  GObject parent;

  int left_margin;
  int right_margin;
  int top_margin;
  int bottom_margin;

  int height;

  BarBarPosition pos;
};

G_DEFINE_TYPE(BarBarBar, g_barbar_bar, G_TYPE_OBJECT)

enum {
  PROP_0,

  PROP_LEFT_MARGIN,
  PROP_RIGHT_MARGIN,
  PROP_TOP_MARGIN,
  PROP_BOTTOM_MARGIN,

  PROP_BAR_POS,
  NUM_PROPERTIES,
};

static GType g_barbar_position_get_type(void) {
  static GType barbar_position_type = 0;

  if (!barbar_position_type) {
    static GEnumValue pattern_types[] = {
        {BARBAR_POS_TOP, "top", "top"},
        {BARBAR_POS_BOTTOM, "bottom", "bot"},
        {BARBAR_POS_LEFT, "left", "left"},
        {BARBAR_POS_RIGHT, "right", "right"},
        {0, NULL, NULL},
    };

    barbar_position_type =
        g_enum_register_static("BarBarPosition", pattern_types);
  }
  return barbar_position_type;
}

static GParamSpec *bar_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
  BarBarBar *bar = BarBar_Bar(object);

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
    bar->pos = g_value_get_enum(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {

  BarBarBar *bar = BarBar_Bar(object);

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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
  // } else {
  // 	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  // }
}

static void g_barbar_bar_class_init(BarBarBarClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_set_property;
  gobject_class->get_property = g_barbar_get_property;

  bar_props[PROP_LEFT_MARGIN] = g_param_spec_uint(
      "left-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_RIGHT_MARGIN] = g_param_spec_uint(
      "right-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_TOP_MARGIN] = g_param_spec_uint(
      "top-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_BOTTOM_MARGIN] = g_param_spec_uint(
      "bottom-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_BAR_POS] =
      g_param_spec_enum("bar-pos", NULL, NULL, BARBAR_TYPE_POSITION,
                        BARBAR_POS_TOP, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, bar_props);
}

static void g_barbar_bar_init(BarBarBar *self) {}

static void activate(GtkApplication *app, void *data) {
  g_return_if_fail(app);
  g_return_if_fail(data);

  BarBarBar *bar = BarBar_Bar(data);

  // Create a normal GTK window however you like
  GtkWindow *gtk_window = GTK_WINDOW(gtk_application_window_new(app));

  // Before the window is first realized, set it up to be a layer surface
  gtk_layer_init_for_window(gtk_window);

  // Order below normal windows
  gtk_layer_set_layer(gtk_window, GTK_LAYER_SHELL_LAYER_TOP);

  // Push other windows out of the way
  gtk_layer_auto_exclusive_zone_enable(gtk_window);

  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_LEFT, bar->left_margin);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_RIGHT,
                       bar->right_margin);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_TOP, bar->top_margin);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM,
                       bar->bottom_margin); // 0
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

  // Set up a widget
  GtkWidget *label = gtk_label_new("");
  gtk_label_set_markup(GTK_LABEL(label), "<span font_desc=\"10.0\">"
                                         "GTK Layer\nShell example!"
                                         "</span>");
  gtk_window_set_child(gtk_window, label);
  gtk_window_present(gtk_window);
}

/**
 * g_barbar_run:
 * @bar: A #BarBarBar
 * @argc: The argc from main()
 * @argv: The argv from main()
 * Returns: The exit status.
 */
int g_barbar_run(BarBarBar *bar, int argc, char **argv) {
  GtkApplication *app = gtk_application_new(
      "com.github.wmww.gtk4-layer-shell.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), bar);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}

/**
 * g_barbar_bars_run:
 * @bars: a %NULL-terminated list of #BarBarBar to start
 * @argc: The argc from main()
 * @argv: The argv from main()
 * Returns: The exit status.
 */
int g_barbar_bars_run(BarBarBar **bars, int argc, char **argv) { return 0; }

BarBarBar *g_barbar_bar_new(void) {
  BarBarBar *bar;

  bar = g_object_new(BARBAR_TYPE_BAR, "bar-pos", BARBAR_POS_LEFT, NULL);

  return bar;
}
