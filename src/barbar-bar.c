#include "barbar-bar.h"
#include "barbar-clock.h"
#include "barbar-cpu.h"
#include "barbar-disk.h"
#include "barbar-inhibitor.h"
#include "barbar-mpris2.h"

#include "dwl/barbar-dwl-tags.h"
#include "river/barbar-river-layout.h"
#include "river/barbar-river-mode.h"
#include "river/barbar-river-tags.h"
#include "river/barbar-river-view.h"

#include "sway/barbar-sway-window.h"
#include "sway/barbar-sway-workspace.h"

#include "hyprland/barbar-hyprland-workspace.h"

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
  PROP_BAR_HEIGHT,

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
    bar->pos = g_value_get_enum(value);
    break;
  case PROP_BAR_HEIGHT:
    bar->height = g_value_get_uint(value);
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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_bar_constructed(GObject *object) {
  BarBarBar *self = BARBAR_BAR(object);
  G_OBJECT_CLASS(g_barbar_bar_parent_class)->constructed(object);

  self->pos = BARBAR_POS_TOP;
  self->height = 20;
}

static void g_barbar_bar_class_init(BarBarBarClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_bar_set_property;
  gobject_class->get_property = g_barbar_bar_get_property;
  gobject_class->constructed = g_barbar_bar_constructed;

  bar_props[PROP_LEFT_MARGIN] = g_param_spec_uint(
      "left-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_RIGHT_MARGIN] = g_param_spec_uint(
      "right-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_TOP_MARGIN] = g_param_spec_uint(
      "top-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_BOTTOM_MARGIN] = g_param_spec_uint(
      "bottom-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_BAR_HEIGHT] = g_param_spec_uint(
      "bar-height", NULL, NULL, 0, G_MAXUINT, 20, G_PARAM_READWRITE);
  bar_props[PROP_BAR_POS] =
      g_param_spec_enum("bar-pos", NULL, NULL, BARBAR_TYPE_POSITION,
                        BARBAR_POS_TOP, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, bar_props);
}

static void g_barbar_bar_init(BarBarBar *self) {}

static void activate(GtkApplication *app, void *data) {
  g_return_if_fail(app);
  g_return_if_fail(data);

  BarBarBar *bar = BARBAR_BAR(data);

  // Create a normal GTK window however you like
  GtkWindow *gtk_window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_default_size(gtk_window, 0, bar->height);

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
                       bar->bottom_margin);
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
  // gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
  // gtk_window_set_resizable(gtk_window, FALSE);

  GtkCssProvider *css_provider;
  GdkDisplay *display;

  display = gtk_widget_get_display(GTK_WIDGET(gtk_window));

  css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(css_provider,
                                  "/home/dagle/.config/barbar/style.css");

  gtk_style_context_add_provider_for_display(display,
                                             GTK_STYLE_PROVIDER(css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

  BarBarClock *clock = g_object_new(BARBAR_TYPE_CLOCK, "tz", "Europe/Stockholm",
                                    "interval", 1000, NULL);

  BarBarDisk *disk = g_object_new(BARBAR_TYPE_DISK, "path", "/", NULL);
  BarBarCpu *cpu = g_object_new(BARBAR_TYPE_CPU, NULL);
  BarBarMpris *mpris = g_object_new(BARBAR_TYPE_MPRIS, NULL);
  BarBarInhibitor *inhibitor = g_object_new(BARBAR_TYPE_INHIBITOR, NULL);

  // BarBarRiverTag *tags = g_object_new(BARBAR_TYPE_RIVER_TAG, NULL);
  // BarBarRiverView *view = g_object_new(BARBAR_TYPE_RIVER_VIEW, NULL);
  // BarBarRiverLayout *layout = g_object_new(BARBAR_TYPE_RIVER_LAYOUT, NULL);
  // BarBarRiverMode *mode = g_object_new(BARBAR_TYPE_RIVER_MODE, NULL);

  // BarBarSwayWorkspace *workspace =
  //     g_object_new(BARBAR_TYPE_SWAY_WORKSPACE, NULL);
  // BarBarSwayWindow *window = g_object_new(BARBAR_TYPE_SWAY_WINDOW, NULL);

  // BarBarHyprlandWorkspace *workspace =
  //     g_object_new(BARBAR_TYPE_SWAY_WORKSPACE, NULL);
  // BarBarSwayWindow *window = g_object_new(BARBAR_TYPE_SWAY_WINDOW, NULL);
  BarBarDwlTag *tags = g_object_new(BARBAR_TYPE_DWL_TAG, NULL);

  GtkWidget *bbb = gtk_action_bar_new();
  gtk_action_bar_pack_end(GTK_ACTION_BAR(bbb), GTK_WIDGET(clock));
  gtk_action_bar_pack_end(GTK_ACTION_BAR(bbb), GTK_WIDGET(disk));
  gtk_action_bar_pack_end(GTK_ACTION_BAR(bbb), GTK_WIDGET(cpu));
  gtk_action_bar_pack_end(GTK_ACTION_BAR(bbb), GTK_WIDGET(mpris));
  gtk_action_bar_pack_end(GTK_ACTION_BAR(bbb), GTK_WIDGET(inhibitor));

  gtk_action_bar_pack_start(GTK_ACTION_BAR(bbb), GTK_WIDGET(tags));
  // gtk_action_bar_pack_start(GTK_ACTION_BAR(bbb), GTK_WIDGET(workspace));
  // gtk_action_bar_pack_start(GTK_ACTION_BAR(bbb), GTK_WIDGET(window));

  // gtk_action_bar_pack_start(GTK_ACTION_BAR(bbb), GTK_WIDGET(tags));
  // gtk_action_bar_pack_start(GTK_ACTION_BAR(bbb), GTK_WIDGET(layout));
  // gtk_action_bar_pack_start(GTK_ACTION_BAR(bbb), GTK_WIDGET(view));
  // gtk_action_bar_pack_start(GTK_ACTION_BAR(bbb), GTK_WIDGET(mode));
  //

  gtk_window_set_child(gtk_window, GTK_WIDGET(bbb));
  gtk_window_present(gtk_window);

  g_barbar_clock_start(clock);
  g_barbar_disk_start(disk);
  g_barbar_cpu_start(cpu);
  g_barbar_mpris_start(mpris);
  g_barbar_inhibitor_start(inhibitor);
  g_barbar_dwl_tag_start(tags);
  // g_barbar_hyprland_workspace_start(workspace);

  // g_barbar_sway_workspace_start(workspace);
  // g_barbar_sway_window_start(window);

  // g_barbar_river_tag_start(tags);
  // g_barbar_river_view_start(view);
  // g_barbar_river_layout_start(layout);
  // g_barbar_river_mode_start(mode);
}

/**
 * g_barbar_run:
 * @bar: A #BarBarBar
 * @argc: The argc from main()
 * @argv: The argv from main()
 * Returns: The exit status.
 */
int g_barbar_run(BarBarBar *bar, int argc, char **argv, GtkWidget *widget) {
  GtkApplication *app =
      gtk_application_new("com.github.barbar", G_APPLICATION_DEFAULT_FLAGS);
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

  // bar = g_object_new(BARBAR_TYPE_BAR, "bar-pos", BARBAR_POS_BOTTOM,
  //                    "bar-height", 15, NULL);
  bar = g_object_new(BARBAR_TYPE_BAR, "bar-pos", BARBAR_POS_BOTTOM,
                     "bar-height", 15, NULL);

  return bar;
}
