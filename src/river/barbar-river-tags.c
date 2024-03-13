#include "river/barbar-river-tags.h"
#include "barbar-util.h"
#include "river-control-unstable-v1-client-protocol.h"
#include "river-status-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarRiverTag:
 *
 * A widget to display the river tags as a list of buttons
 *
 */
struct _BarBarRiverTag {
  GtkWidget parent_instance;

  struct zriver_status_manager_v1 *status_manager;
  struct zriver_control_v1 *control;
  struct zriver_output_status_v1 *output_status;
  struct wl_seat *seat;

  GtkWidget *buttons[9];
};

enum {
  PROP_0,

  PROP_TAGNUMS,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarRiverTag, g_barbar_river_tag, GTK_TYPE_WIDGET)

static GParamSpec *river_tags_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_river_tag_root(GtkWidget *widget);
static void g_barbar_river_tag_constructed(GObject *object);
static void default_clicked_handler(BarBarRiverTag *river, guint tag,
                                    gpointer user_data);

static void g_barbar_river_tag_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {}

static void g_barbar_river_tag_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {}

static guint click_signal;

static void g_barbar_river_tag_class_init(BarBarRiverTagClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_river_tag_set_property;
  gobject_class->get_property = g_barbar_river_tag_get_property;
  gobject_class->constructed = g_barbar_river_tag_constructed;

  widget_class->root = g_barbar_river_tag_root;

  river_tags_props[PROP_TAGNUMS] = g_param_spec_uint(
      "tagnums", NULL, NULL, 0, 9, 9,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    river_tags_props);

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "river-tag");
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarRiverTag *river = BARBAR_RIVER_TAG(data);
  if (strcmp(interface, zriver_status_manager_v1_interface.name) == 0) {
    if (version >= ZRIVER_OUTPUT_STATUS_V1_LAYOUT_NAME_CLEAR_SINCE_VERSION) {
      river->status_manager = wl_registry_bind(
          registry, name, &zriver_status_manager_v1_interface, version);
    }
  }
  if (strcmp(interface, zriver_control_v1_interface.name) == 0) {
    river->control =
        wl_registry_bind(registry, name, &zriver_control_v1_interface, version);
  }
  if (strcmp(interface, wl_seat_interface.name) == 0) {
    river->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
  }
}

static void registry_handle_global_remove(void *_data,
                                          struct wl_registry *_registry,
                                          uint32_t _name) {
  (void)_data;
  (void)_registry;
  (void)_name;
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void listen_command_success(
    void *data, struct zriver_command_callback_v1 *zriver_command_callback_v1,
    const char *output) {
  // Do nothing but keep listener to avoid crashing when command was successful
}

static void listen_command_failure(
    void *data, struct zriver_command_callback_v1 *zriver_command_callback_v1,
    const char *output) {
  // log error
}

static const struct zriver_command_callback_v1_listener
    command_callback_listener = {
        .success = listen_command_success,
        .failure = listen_command_failure,
};

static void
listen_focused_tags(void *data,
                    struct zriver_output_status_v1 *zriver_output_status_v1,
                    uint32_t tags) {
  BarBarRiverTag *river = BARBAR_RIVER_TAG(data);
  for (size_t i = 0; i < 9; ++i) {
    if ((1 << i) & tags) {
      gtk_widget_add_css_class(river->buttons[i], "focused");
    } else {
      gtk_widget_remove_css_class(river->buttons[i], "focused");
    }
  }
}
// called when a new tag gets occupied or vacant
static void
listen_view_tags(void *data,
                 struct zriver_output_status_v1 *zriver_output_status_v1,
                 struct wl_array *tag_array) {
  BarBarRiverTag *river = BARBAR_RIVER_TAG(data);
  uint32_t *tag;
  uint32_t tags = 0;
  wl_array_for_each(tag, tag_array) { tags |= *tag; }

  for (size_t i = 0; i < 9; ++i) {
    if ((1 << i) & tags) {
      gtk_widget_add_css_class(river->buttons[i], "occupied");
    } else {
      gtk_widget_remove_css_class(river->buttons[i], "occupied");
    }
  }
}

static void
listen_urgent_tags(void *data,
                   struct zriver_output_status_v1 *zriver_output_status_v1,
                   uint32_t tags) {
  BarBarRiverTag *river = BARBAR_RIVER_TAG(data);
  for (size_t i = 0; i < 9; ++i) {
    if ((1 << i) & tags) {
      gtk_widget_add_css_class(river->buttons[i], "urgent");
    } else {
      gtk_widget_remove_css_class(river->buttons[i], "urgent");
    }
  }
}

static void layout_name(void *data,
                        struct zriver_output_status_v1 *zriver_output_status_v1,
                        const char *name) {}
static void
layout_name_clear(void *data,
                  struct zriver_output_status_v1 *zriver_output_status_v1) {}

static const struct zriver_output_status_v1_listener output_status_listener = {
    .focused_tags = listen_focused_tags,
    .view_tags = listen_view_tags,
    .urgent_tags = listen_urgent_tags,
    .layout_name = layout_name,
    .layout_name_clear = layout_name_clear,
};

static void clicked(GtkButton *self, gpointer user_data) {
  BarBarRiverTag *river = BARBAR_RIVER_TAG(user_data);

  guint tag;
  for (int i = 0; i < 9; i++) {
    if (self == GTK_BUTTON(river->buttons[i])) {
      tag = i;
    }
  }
  tag = 1 << tag;

  g_signal_emit(river, click_signal, 0, tag);
}

static void default_clicked_handler(BarBarRiverTag *river, guint tag,
                                    gpointer user_data) {
  struct zriver_command_callback_v1 *callback;
  char buf[4];

  snprintf(buf, 4, "%d", tag);

  zriver_control_v1_add_argument(river->control, "set-focused-tags");
  zriver_control_v1_add_argument(river->control, buf);
  callback = zriver_control_v1_run_command(river->control, river->seat);
  zriver_command_callback_v1_add_listener(callback, &command_callback_listener,
                                          NULL);
}

static void g_barbar_river_tag_init(BarBarRiverTag *self) {}
static void g_barbar_river_tag_constructed(GObject *object) {
  GtkWidget *btn;
  BarBarRiverTag *river = BARBAR_RIVER_TAG(object);
  char str[2];
  for (uint32_t i = 0; i < 9; i++) {
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label(str);
    gtk_widget_set_parent(btn, GTK_WIDGET(river));
    g_signal_connect(btn, "clicked", G_CALLBACK(clicked), river);
    river->buttons[i] = btn;
  }

  G_OBJECT_CLASS(g_barbar_river_tag_parent_class)->constructed(object);
}

static void g_barbar_river_tag_root(GtkWidget *widget) {
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_registry *wl_registry;
  struct wl_output *output;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_river_tag_parent_class)->root(widget);

  BarBarRiverTag *river = BARBAR_RIVER_TAG(widget);

  gdk_display = gdk_display_get_default();

  // This shouldn't need to be done because layered shell requires wayland
  g_return_if_fail(gdk_display);
  g_return_if_fail(GDK_IS_WAYLAND_DISPLAY(gdk_display));

  // We try to find the main screen for this widget, this should only
  // be done if no screen is specified
  // GtkWindow *window = g_barbar_get_parent_layer_window(GTK_WIDGET(river));
  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(river), GTK_TYPE_WINDOW));
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    printf("Parent window not found!\n");
    return;
  }

  monitor = gtk_layer_get_monitor(window);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, river);
  wl_display_roundtrip(wl_display);

  if (!river->status_manager) {
    return;
  }

  river->output_status = zriver_status_manager_v1_get_river_output_status(
      river->status_manager, output);

  zriver_output_status_v1_add_listener(river->output_status,
                                       &output_status_listener, river);
  wl_display_roundtrip(wl_display);

  zriver_status_manager_v1_destroy(river->status_manager);

  river->status_manager = NULL;
}
