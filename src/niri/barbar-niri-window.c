#include "niri/barbar-niri-window.h"
#include "gdk/wayland/gdkwayland.h"
#include "gtk/gtk.h"
#include "gtk4-layer-shell.h"
#include "json-glib/json-glib.h"
#include "niri/barbar-niri-subscribe.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include <wayland-client-protocol.h>

/**
 * BarBarNiriWindow:
 *
 * A widget to display the niri workspaces as a list of buttons
 */
struct _BarBarNiriWindow {
  GtkWidget parent_instance;

  char *output_name;
  char *window;

  GtkWidget *label;

  BarBarNiriSubscribe *sub;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zxdg_output_v1 *xdg_output;

  GList *windows;
};

struct window {
  int id;
  char *title;
  int workspace_id;
  gboolean mark;
};

void free_window(struct window *window) {
  free(window->title);
  free(window);
}

enum {
  PROP_0,

  PROP_OUTPUT,
  PROP_WINDOW,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarNiriWindow, g_barbar_niri_window, GTK_TYPE_WIDGET);

static GParamSpec *niri_window_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_niri_window_start(GtkWidget *widget);

static void g_barbar_niri_window_set_output(BarBarNiriWindow *niri,
                                            const gchar *output) {
  g_return_if_fail(BARBAR_IS_NIRI_WINDOW(niri));

  if (g_set_str(&niri->output_name, output)) {
    g_object_notify_by_pspec(G_OBJECT(niri), niri_window_props[PROP_OUTPUT]);
  }
}

static void g_barbar_niri_window_set_window(BarBarNiriWindow *niri,
                                            const gchar *window) {
  g_return_if_fail(BARBAR_IS_NIRI_WINDOW(niri));

  if (g_set_str(&niri->window, window)) {
    g_object_notify_by_pspec(G_OBJECT(niri), niri_window_props[PROP_WINDOW]);
  }
}

static void g_barbar_niri_window_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarNiriWindow *niri = BARBAR_NIRI_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    // g_barbar_sway_workspace_set_output(sway, g_value_get_string(value));
    // break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_niri_window_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {
  BarBarNiriWindow *niri = BARBAR_NIRI_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, niri->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_niri_window_finalize(GObject *object) {
  BarBarNiriWindow *workspace = BARBAR_NIRI_WINDOW(object);

  g_clear_object(&workspace->sub);
  g_free(workspace->output_name);
  g_free(workspace->window);

  G_OBJECT_CLASS(g_barbar_niri_window_parent_class)->finalize(object);
}

static void g_barbar_niri_window_class_init(BarBarNiriWindowClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_niri_window_set_property;
  gobject_class->get_property = g_barbar_niri_window_get_property;
  gobject_class->finalize = g_barbar_niri_window_finalize;

  /**
   * BarBarNiriWindow:output:
   *
   * What screen this is monitoring
   */
  niri_window_props[PROP_OUTPUT] =
      g_param_spec_string("output", NULL, NULL, NULL, G_PARAM_READWRITE);

  /**
   * BarBarNiriWindow:window:
   *
   * What screen this is monitoring
   */
  niri_window_props[PROP_WINDOW] =
      g_param_spec_string("window", NULL, NULL, NULL, G_PARAM_READABLE);

  widget_class->root = g_barbar_niri_window_start;

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    niri_window_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "niri-window");
}

static void g_barbar_niri_window_init(BarBarNiriWindow *self) {
  self->label = gtk_label_new(NULL);
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {

  BarBarNiriWindow *niri = BARBAR_NIRI_WINDOW(data);
  if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    niri->xdg_output_manager =
        wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
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

static void xdg_output_handle_logical_position(
    void *data, struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y) {}

static void xdg_output_handle_logical_size(void *data,
                                           struct zxdg_output_v1 *xdg_output,
                                           int32_t width, int32_t height) {}
static void xdg_output_handle_done(void *data,
                                   struct zxdg_output_v1 *xdg_output) {}

static void xdg_output_handle_name(void *data,
                                   struct zxdg_output_v1 *xdg_output,
                                   const char *name) {
  BarBarNiriWindow *niri = BARBAR_NIRI_WINDOW(data);
  g_barbar_niri_window_set_output(niri, name);
}

static void xdg_output_handle_description(void *data,
                                          struct zxdg_output_v1 *xdg_output,
                                          const char *description) {}

static const struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_handle_logical_position,
    .logical_size = xdg_output_handle_logical_size,
    .done = xdg_output_handle_done,
    .name = xdg_output_handle_name,
    .description = xdg_output_handle_description,
};

const char *safe_json_get_string_value2(JsonReader *reader) {

  if (json_reader_get_null_value(reader)) {
    return NULL;
  }
  return json_reader_get_string_value(reader);
}

static struct window *get_window(GList *windows, gint64 id) {
  for (; windows; windows = windows->next) {
    struct window *entry;
    entry = windows->data;
    if (entry->id == id) {
      return entry;
    }
  }
  return NULL;
}

static void sweep(BarBarNiriWindow *niri) {
  GList *workspace = niri->windows;

  for (; workspace; workspace = workspace->next) {
    struct window *entry;
    entry = workspace->data;
    if (!entry->mark) {
      free_window(entry);
      niri->windows = g_list_remove_link(niri->windows, workspace);
    } else {
      entry->mark = FALSE;
    }
  }
}

static gboolean g_barbar_niri_window_set_windows(BarBarNiriWindow *niri,
                                                 JsonReader *reader) {
  struct window *window;

  json_reader_read_member(reader, "is_focused");
  gboolean focused = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "id");
  gint64 id = json_reader_get_int_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "title");
  const char *title = safe_json_get_string_value2(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "workspace_id");
  gint64 workspace_id = json_reader_get_int_value(reader);
  json_reader_end_member(reader);

  window = get_window(niri->windows, id);

  if (!window) {
    window = calloc(1, sizeof(struct window));
    window->id = id;
    niri->windows = g_list_insert(niri->windows, window, 0);
  }

  free(window->title);
  window->title = g_strdup(title);
  window->workspace_id = workspace_id;
  window->mark = TRUE;

  if (focused) {
    gtk_label_set_label(GTK_LABEL(niri->label), title);
    return TRUE;
  }
  return FALSE;
}

static void event_listner(BarBarNiriSubscribe *sub, JsonParser *parser,
                          gpointer data) {
  BarBarNiriWindow *niri = BARBAR_NIRI_WINDOW(data);
  gboolean label_set = FALSE;

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  if (json_reader_read_member(reader, "WindowsChanged")) {
    json_reader_read_member(reader, "windows");

    int n = json_reader_count_elements(reader);

    for (int i = 0; i < n; i++) {
      json_reader_read_element(reader, i);
      label_set |= g_barbar_niri_window_set_windows(niri, reader);
      json_reader_end_element(reader);
    }

    json_reader_end_member(reader);

    if (!label_set) {
      gtk_label_set_label(GTK_LABEL(niri->label), NULL);
    }
    sweep(niri);
  }
  json_reader_end_member(reader); // end changed

  if (json_reader_read_member(reader, "WindowOpenedOrChanged")) {
    json_reader_read_member(reader, "windows");

    g_barbar_niri_window_set_windows(niri, reader);
    json_reader_end_member(reader);
  }
  json_reader_end_member(reader); // end opened or changed

  if (json_reader_read_member(reader, "WindowFocusChanged")) {
    struct window *window;
    json_reader_read_member(reader, "id");

    if (json_reader_get_null_value(reader)) {
      gtk_label_set_label(GTK_LABEL(niri->label), NULL);
    } else {
      guint64 id = json_reader_get_int_value(reader);

      window = get_window(niri->windows, id);

      if (window) {
        gtk_label_set_label(GTK_LABEL(niri->label), window->title);
      }
    }
    json_reader_end_member(reader); // id
  }
  json_reader_end_member(reader); // end opened or changed
}

static void g_barbar_niri_window_start(GtkWidget *widget) {
  BarBarNiriWindow *niri = BARBAR_NIRI_WINDOW(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_output *output;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;
  GError *error = NULL;

  GTK_WIDGET_CLASS(g_barbar_niri_window_parent_class)->root(widget);

  gdk_display = gdk_display_get_default();
  niri->sub = BARBAR_NIRI_SUBSCRIBE(g_barbar_niri_subscribe_new());

  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(niri), GTK_TYPE_WINDOW));
  // doesn't need to be a layer window
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    printf("Parent window not found!\n");
    return;
  }

  monitor = gtk_layer_get_monitor(window);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, niri);
  wl_display_roundtrip(wl_display);

  if (!niri->xdg_output_manager) {
    g_warning("Couldn't init the xdg output manager");
    return;
  }

  niri->xdg_output =
      zxdg_output_manager_v1_get_xdg_output(niri->xdg_output_manager, output);

  zxdg_output_v1_add_listener(niri->xdg_output, &xdg_output_listener, niri);
  wl_display_roundtrip(wl_display);

  g_signal_connect(niri->sub, "event", G_CALLBACK(event_listner), niri);

  g_barbar_niri_subscribe_connect(niri->sub, &error);
  if (error) {
    g_printerr("can't connect: %s\n", error->message);
    return;
  }
}
