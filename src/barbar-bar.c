#include "barbar-bar.h"
#include <gtk4-layer-shell.h>

struct _BarBarBar {
	GObject parent;

	int left_margin;

};

G_DEFINE_TYPE (BarBarBar, g_barbar_bar, G_TYPE_OBJECT)

enum {
	PROP_0,

	PROP_LEFT_MARGIN,
	PROP_RIGHT_MARGIN,
	PROP_TOP_MARGIN,
	PROP_BOT_MARGIN,

	PROP_BAR_POS,
	NUM_PROPERTIES,
};

static GParamSpec *bar_props[NUM_PROPERTIES] = { NULL, };

static void
g_barbar_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
	BarBarBar *bar = BarBar_Bar(object);

	if (property_id == PROP_BAR) {
	} else {
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
g_barbar_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {

	BarBarBar *bar = BarBar_Bar(object);

	if (property_id == PROP_BAR) {
	} else {
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void g_barbar_bar_class_init(BarBarBarClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->set_property = g_barbar_set_property;
  gobject_class->get_property = g_barbar_get_property;

  bar_props[PROP_LEFT_MARGIN] = g_param_spec_uint ("left-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_RIGHT_MARGIN] = g_param_spec_uint ("right-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_TOP_MARGIN] = g_param_spec_uint ("top-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  bar_props[PROP_BOT_MARGIN] = g_param_spec_uint ("bot-margin", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READWRITE);
  // bar_props[PROP_BAR_POS] = g_param_spec_enum ("bar-pos", NULL, NULL, BARBAR_POS_TOP, G_PARAM_READWRITE);
}

static void g_barbar_bar_init(BarBarBar *self){
}

static void
activate (GtkApplication* app, void *_data) {
    (void)_data;

    // Create a normal GTK window however you like
    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (gtk_window);

    // Order below normal windows
    gtk_layer_set_layer (gtk_window, GTK_LAYER_SHELL_LAYER_TOP);

    // Push other windows out of the way
    gtk_layer_auto_exclusive_zone_enable (gtk_window);

	// gtk_layer_auto_exclusive_zone_enable(gtk_window);
	
    // We don't need to get keyboard input
    // gtk_layer_set_keyboard_mode (gtk_window, GTK_LAYER_SHELL_KEYBOARD_MODE_NONE); // NONE is default

    // The margins are the gaps around the window's edges
    // Margins and anchors can be set like this...
    gtk_layer_set_margin (gtk_window, GTK_LAYER_SHELL_EDGE_LEFT, 0);
    // gtk_layer_set_margin (gtk_window, GTK_LAYER_SHELL_EDGE_RIGHT, 0);
    // gtk_layer_set_margin (gtk_window, GTK_LAYER_SHELL_EDGE_TOP, 20);
    // gtk_layer_set_margin (gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM, 0); // 0 is default

    // ... or like this
    // Anchors are if the window is pinned to each edge of the output
    static const gboolean anchors[] = {TRUE, TRUE, FALSE, TRUE};
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (gtk_window, i, anchors[i]);
    }
	gtk_layer_set_anchor (gtk_window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
	// gtk_layer_set_anchor (gtk_window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
	// gtk_layer_set_anchor (gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
	// gtk_layer_set_anchor (gtk_window, GTK_LAYER_SHELL_EDGE_TOP, FALSE);

    // Set up a widget
    GtkWidget *label = gtk_label_new ("");
    gtk_label_set_markup (GTK_LABEL (label),
                          "<span font_desc=\"10.0\">"
                              "GTK Layer\nShell example!"
                          "</span>");
    gtk_window_set_child (gtk_window, label);
    gtk_window_present (gtk_window);
}

/**
 * g_barbar_run:
 * @bar: A #BarBarBar
 * @argc: The argc from main()
 * @argv: The argv from main()
 * Returns: The exit status.
 */
int g_barbar_run(BarBarBar *bar, int argc, char **argv) {
    GtkApplication * app = gtk_application_new ("com.github.wmww.gtk4-layer-shell.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activate), bar);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}

/**
 * g_barbar_bars_run:
 * @bars: a %NULL-terminated list of #BarBarBar to start
 * @argc: The argc from main()
 * @argv: The argv from main()
 * Returns: The exit status.
 */
int g_barbar_bars_run(BarBarBar **bars, int argc, char **argv) {

	return 0;
}

BarBarBar *
g_barbar_bar_new (void)
{
	BarBarBar *bar;
	
	bar = g_object_new (BARBAR_TYPE_BAR, NULL);
	
	return bar;
}
