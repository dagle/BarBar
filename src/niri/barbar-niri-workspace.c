#include "niri/barbar-niri-workspace.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk4-layer-shell.h"
#include "niri/barbar-niri-subscribe.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarNiriWorkspace:
 *
 * A widget to display the niri workspaces as a list of buttons
 */
struct _BarBarNiriWorkspace {
  GtkWidget parent_instance;

  char *output_name;

  BarBarNiriSubscribe *sub;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zxdg_output_v1 *xdg_output;

  GList *workspaces; // A list of workspaces;
};

struct workspace {
  int num;
  int id;
  gboolean mark;

  GtkWidget *button;
};

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarNiriWorkspace, g_barbar_niri_workspace, GTK_TYPE_WIDGET)

static GParamSpec *niri_workspace_props[NUM_PROPERTIES] = {
    NULL,
};

static guint click_signal;

static void g_barbar_niri_workspace_start(GtkWidget *widget);

static void default_clicked_handler(BarBarNiriWorkspace *niri, guint workspace,
                                    gpointer user_data);

static void g_barbar_niri_workspace_set_output(BarBarNiriWorkspace *niri,
                                               const gchar *output) {
  g_return_if_fail(BARBAR_IS_NIRI_WORKSPACE(niri));

  if (g_set_str(&niri->output_name, output)) {
    g_object_notify_by_pspec(G_OBJECT(niri), niri_workspace_props[PROP_OUTPUT]);
  }
}

static void g_barbar_niri_workspace_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  BarBarNiriWorkspace *niri = BARBAR_NIRI_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    // g_barbar_sway_workspace_set_output(sway, g_value_get_string(value));
    // break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_niri_workspace_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {
  BarBarNiriWorkspace *niri = BARBAR_NIRI_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, niri->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_niri_workspace_finalize(GObject *object) {
  BarBarNiriWorkspace *workspace = BARBAR_NIRI_WORKSPACE(object);

  g_clear_object(&workspace->sub);
  g_list_free_full(workspace->workspaces, g_free);
  g_free(workspace->output_name);

  G_OBJECT_CLASS(g_barbar_niri_workspace_parent_class)->finalize(object);
}

static void
g_barbar_niri_workspace_class_init(BarBarNiriWorkspaceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_niri_workspace_set_property;
  gobject_class->get_property = g_barbar_niri_workspace_get_property;
  gobject_class->finalize = g_barbar_niri_workspace_finalize;

  /**
   * BarBarNiriWorkspace:output:
   *
   * What screen this is monitoring
   */
  niri_workspace_props[PROP_OUTPUT] =
      g_param_spec_string("output", NULL, NULL, NULL, G_PARAM_READWRITE);

  widget_class->root = g_barbar_niri_workspace_start;

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    niri_workspace_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "niri-workspace");
}

static void g_barbar_niri_workspace_init(BarBarNiriWorkspace *self) {}

static void default_clicked_handler(BarBarNiriWorkspace *niri, guint tag,
                                    gpointer user_data) {
  // g_barbar_sway_ipc_oneshot(SWAY_RUN_COMMAND, FALSE, NULL, NULL, NULL,
  //                           "%workspace %d", tag);
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {

  BarBarNiriWorkspace *niri = BARBAR_NIRI_WORKSPACE(data);
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
  BarBarNiriWorkspace *niri = BARBAR_NIRI_WORKSPACE(data);
  g_barbar_niri_workspace_set_output(niri, name);
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

const char *safe_json_get_string_value(JsonReader *reader) {

  if (json_reader_get_null_value(reader)) {
    return NULL;
  }
  return json_reader_get_string_value(reader);
}

static gint compare_func(gconstpointer a, gconstpointer b) {
  const struct workspace *wrka = a;
  const struct workspace *wrkb = b;

  return wrka->num - wrkb->num;
}

static struct workspace *get_entry(GList *workspace, guint64 id) {
  for (; workspace; workspace = workspace->next) {
    struct workspace *entry;
    entry = workspace->data;
    if (entry->id == id) {
      return entry;
    }
  }
  return NULL;
}

static void sweep(BarBarNiriWorkspace *niri) {
  GList *workspace = niri->workspaces;

  for (; workspace; workspace = workspace->next) {
    struct workspace *entry;
    entry = workspace->data;
    if (!entry->mark) {
      g_free(entry);
      niri->workspaces = g_list_remove_link(niri->workspaces, workspace);
    } else {
      entry->mark = FALSE;
    }
  }
}

static void sort_widgets(BarBarNiriWorkspace *niri) {
  GList *workspace;
  for (workspace = niri->workspaces; workspace; workspace = workspace->next) {
    struct workspace *entry;
    entry = workspace->data;
    if (workspace->prev) {
      struct workspace *prev = workspace->prev->data;
      gtk_widget_insert_after(entry->button, GTK_WIDGET(niri), prev->button);
    }
  }
}

static gboolean g_barbar_niri_workspace_set_workspace(BarBarNiriWorkspace *niri,
                                                      JsonReader *reader) {
  struct workspace *entry = NULL;
  gboolean needs_sort = FALSE;

  json_reader_read_member(reader, "output");
  const char *output = safe_json_get_string_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "id");
  gint64 id = json_reader_get_int_value(reader);
  json_reader_end_member(reader);

  entry = get_entry(niri->workspaces, id);

  if (!entry) {
    entry = malloc(sizeof(struct workspace));
    entry->button = gtk_button_new();
    entry->id = id;
    needs_sort = TRUE;
    gtk_widget_set_parent(entry->button, GTK_WIDGET(niri));
  }

  json_reader_read_member(reader, "idx");
  gint64 idx = json_reader_get_int_value(reader);
  json_reader_end_member(reader);
  if (entry->num != idx) {
    needs_sort = TRUE;
  }
  entry->num = idx;

  json_reader_read_member(reader, "name");
  const char *name = safe_json_get_string_value(reader);
  json_reader_end_member(reader);
  if (name != NULL) {
    gtk_button_set_label(GTK_BUTTON(entry->button), name);
  } else {
    char num[3];
    snprintf(num, sizeof(num), "%d", entry->num);
    gtk_button_set_label(GTK_BUTTON(entry->button), num);
  }

  json_reader_read_member(reader, "is_active");
  gboolean active = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  if (active) {
    gtk_widget_add_css_class(entry->button, "active");
  } else {
    gtk_widget_remove_css_class(entry->button, "active");
  }

  json_reader_read_member(reader, "is_focused");
  gboolean focused = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  if (focused) {
    gtk_widget_add_css_class(entry->button, "focused");
  } else {
    gtk_widget_remove_css_class(entry->button, "focused");
  }

  niri->workspaces =
      g_list_insert_sorted(niri->workspaces, entry, compare_func);
  return needs_sort;
}

static void event_listner(BarBarNiriSubscribe *sub, JsonParser *parser,
                          gpointer data) {
  BarBarNiriWorkspace *niri = BARBAR_NIRI_WORKSPACE(data);
  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  gboolean needs_sort = FALSE;

  if (json_reader_read_member(reader, "WorkspacesChanged")) {
    json_reader_read_member(reader, "workspaces");
    int n = json_reader_count_elements(reader);

    for (int i = 0; i < n; i++) {
      json_reader_read_element(reader, i);
      needs_sort |= g_barbar_niri_workspace_set_workspace(niri, reader);
      json_reader_end_element(reader);
    }

    json_reader_end_member(reader); // end workspaces
    json_reader_end_member(reader); // end changed
    sweep(niri);
    if (needs_sort) {
      sort_widgets(niri);
    }
  }
  if (json_reader_read_member(reader, "WorkspaceActivated")) {
    json_reader_read_member(reader, "id");
    json_reader_end_member(reader);
    json_reader_read_member(reader, "focused");
    json_reader_end_member(reader);

    json_reader_end_member(reader); // end activated
  }
}

static void g_barbar_niri_workspace_start(GtkWidget *widget) {
  BarBarNiriWorkspace *niri = BARBAR_NIRI_WORKSPACE(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_output *output;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;
  GError *error = NULL;

  GTK_WIDGET_CLASS(g_barbar_niri_workspace_parent_class)->root(widget);

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

/**
 * g_barbar_niri_workspace_new:
 *
 * Returns: (transfer full): a new `BarBarNiriWorkspace`
 */
GtkWidget *g_barbar_niri_workspace_new(void) {
  BarBarNiriWorkspace *ws;

  ws = g_object_new(BARBAR_TYPE_NIRI_WORKSPACE, NULL);

  return GTK_WIDGET(ws);
}
