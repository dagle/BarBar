#include "barbar-hyprland-workspace.h"
#include "barbar-hyprland-ipc.h"
#include "glib-object.h"
#include "gtk4-layer-shell.h"
#include "hyprland/barbar-hyprland-service.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

// TODO:  We need to match against names and not just numbers
// because if the namespace is named, it isn't a number

/**
 * BarBarHyprlandWorkspace:
 * A widget to display the hyprland workspaces as a list of buttons
 */
struct _BarBarHyprlandWorkspace {
  GtkWidget parent_instance;

  char *output_name;
  gboolean focused;
  int active_workspace; // Is the current active workspace, may not be our
                        // workspace

  BarBarHyprlandService *service;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zxdg_output_v1 *xdg_output;

  // struct wl_output *output;

  GList *workspaces; // A list of workspaces;
};

struct MonitorEntry {
  int id;
  GtkWidget *button;
};

struct entry {
  int id;
  int num;

  GtkWidget *button;
};

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarHyprlandWorkspace, g_barbar_hyprland_workspace,
              GTK_TYPE_WIDGET)

static GParamSpec *hypr_workspace_props[NUM_PROPERTIES] = {
    NULL,
};

static guint click_signal;

static void g_barbar_hyprland_workspace_map(GtkWidget *widget);
static void g_barbar_hyprland_add_workspace(BarBarHyprlandWorkspace *hypr,
                                            int id, const char *name);
static void g_barbar_hyprland_remove_workspace(BarBarHyprlandWorkspace *hypr,
                                               int id);

static void g_barbar_hyprland_move_workspace(BarBarHyprlandWorkspace *hypr,
                                             int id, const char *workspacename,
                                             const char *monitor_name);

static void g_barbar_hyprland_rename_workspace(BarBarHyprlandWorkspace *hypr,
                                               int id, const char *name);
static void default_clicked_handler(BarBarHyprlandWorkspace *hypr,
                                    guint workspace, gpointer user_data);

// GtkWidget *g_barbar_hyprland_add_button(BarBarHyprlandWorkspace *hypr,
//                                     struct workspace *workspace);

static void
g_barbar_hyprland_workspace_set_output(BarBarHyprlandWorkspace *hypr,
                                       const gchar *output) {
  g_return_if_fail(BARBAR_IS_HYPRLAND_WORKSPACE(hypr));

  g_free(hypr->output_name);

  if (output) {
    hypr->output_name = g_strdup(output);
  }
  g_object_notify_by_pspec(G_OBJECT(hypr), hypr_workspace_props[PROP_OUTPUT]);
}

static void g_barbar_hyprland_workspace_set_property(GObject *object,
                                                     guint property_id,
                                                     const GValue *value,
                                                     GParamSpec *pspec) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_hyprland_workspace_set_output(hypr, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_hyprland_workspace_get_property(GObject *object,
                                                     guint property_id,
                                                     GValue *value,
                                                     GParamSpec *pspec) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, hypr->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
g_barbar_hyprland_workspace_class_init(BarBarHyprlandWorkspaceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_hyprland_workspace_set_property;
  gobject_class->get_property = g_barbar_hyprland_workspace_get_property;
  widget_class->root = g_barbar_hyprland_workspace_map;

  // this shouldn't need to be here in the future
  hypr_workspace_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    hypr_workspace_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "hypr-workspace");
}

static void g_barbar_hyprland_workspace_init(BarBarHyprlandWorkspace *self) {}

static void default_clicked_handler(BarBarHyprlandWorkspace *sway, guint id,
                                    gpointer user_data) {
  char *str = g_strdup_printf("dispatch workspace %d", id);
  g_barbar_hyprland_ipc_oneshot(NULL, NULL, str, NULL);
  g_free(str);
}

static struct MonitorEntry *get_button(BarBarHyprlandWorkspace *hypr, int id) {
  GList *list;
  if (id <= 0) {
    return NULL;
  }

  for (list = hypr->workspaces; list; list = list->next) {
    struct MonitorEntry *e = list->data;
    if (e->id == id) {
      return e;
    }
  }
  return NULL;
}

static void g_barbar_hyprland_workspace_workspace_callback(
    BarBarHyprlandService *service, guint id, const char *name, gpointer data) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);
  struct MonitorEntry *entry;

  entry = get_button(hypr, hypr->active_workspace);
  if (entry) {
    if (hypr->focused) {
      gtk_widget_remove_css_class(entry->button, "focused");
    } else {
      gtk_widget_remove_css_class(entry->button, "visible");
    }
  } else {
    printf("workspace entry is null\n");
  }

  hypr->active_workspace = id;

  entry = get_button(hypr, hypr->active_workspace);

  if (entry) {
    if (hypr->focused) {
      gtk_widget_add_css_class(entry->button, "focused");
    } else {
      gtk_widget_add_css_class(entry->button, "visible");
    }
  } else {
    printf("workspace entry is null\n");
  }
}

static void g_barbar_hyprland_workspace_urgent_callback(
    BarBarHyprlandService *service, const char *address, gpointer data) {
  // BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);
}

static void g_barbar_hyprland_workspace_focused_monitor_callback(
    BarBarHyprlandService *service, const char *monitor, const char *workspace,
    gpointer data) {

  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  if (!hypr->output_name || !g_strcmp0(hypr->output_name, monitor)) {
    hypr->focused = TRUE;
  } else {
    hypr->focused = FALSE;
  }
}

static void g_barbar_hyprland_workspace_create_workspace_callback(
    BarBarHyprlandService *service, guint id, const char *name, gpointer data) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  g_barbar_hyprland_add_workspace(hypr, id, name);
}

static void g_barbar_hyprland_workspace_destroy_workspace_callback(
    BarBarHyprlandService *service, guint id, const char *name, gpointer data) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  g_barbar_hyprland_remove_workspace(hypr, id);
}

static void g_barbar_hyprland_workspace_move_workspace_callback(
    BarBarHyprlandService *service, guint id, const char *workspace,
    const char *monitor, gpointer data) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  g_barbar_hyprland_move_workspace(hypr, id, workspace, monitor);
}

static void g_barbar_hyprland_workspace_rename_workspace_callback(
    BarBarHyprlandService *service, guint id, const char *name, gpointer data) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  g_barbar_hyprland_rename_workspace(hypr, id, name);
}

struct MonitorEntry *new_entry(int id, const char *name) {

  struct MonitorEntry *entry = g_malloc0(sizeof(struct MonitorEntry));

  entry->id = id;
  entry->button = gtk_button_new_with_label(name);
  return entry;
}

void free_entry(struct MonitorEntry *entry) {
  g_clear_pointer(&entry->button, g_object_unref);
  g_free(entry);
}

static gint entry_compare_func(gconstpointer a, gconstpointer b) {
  const struct MonitorEntry *wrka = a;
  const struct MonitorEntry *wrkb = b;

  return wrka->id - wrkb->id;
}

static void parse_initional_workspaces(BarBarHyprlandWorkspace *hypr,
                                       JsonParser *parser) {
  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  if (json_reader_is_array(reader)) {
    int n = json_reader_count_elements(reader);
    for (int j = 0; j < n; j++) {
      json_reader_read_element(reader, j);
      json_reader_read_member(reader, "id");
      int id = json_reader_get_int_value(reader);
      json_reader_end_member(reader);

      json_reader_read_member(reader, "name");
      const char *name = json_reader_get_string_value(reader);
      json_reader_end_member(reader);

      // json_reader_read_member(reader, "monitor");
      // const char *monitor = json_reader_get_string_value(reader);
      // json_reader_end_member(reader);
      //
      // json_reader_read_member(reader, "monitorID");
      // int monitor_id = json_reader_get_int_value(reader);
      // json_reader_end_member(reader);
      json_reader_end_element(reader);

      // if (!g_strcmp0(monitor, "DVI-D-1")) {
      g_barbar_hyprland_add_workspace(hypr, id, name);
      // }
      // printf("json: id: %d, name: %s, monitor: %s, monitor_id: %d\n", id,
      // name,
      //        monitor, monitor_id);
    }
  }
  g_object_unref(reader);
}

static struct MonitorEntry *get_button_name(BarBarHyprlandWorkspace *hypr,
                                            const char *name) {
  GList *list;
  if (!name) {
    return NULL;
  }

  for (list = hypr->workspaces; list; list = list->next) {
    struct MonitorEntry *e = list->data;
    const char *ename = gtk_button_get_label(GTK_BUTTON(e->button));
    if (!strcmp(ename, name)) {
      return e;
    }
  }
  return NULL;
}

static void clicked(GtkButton *self, gpointer user_data) {
  GList *list;
  struct MonitorEntry *entry = NULL;
  BarBarHyprlandWorkspace *hypr = user_data;

  for (list = hypr->workspaces; list; list = list->next) {
    struct MonitorEntry *e = list->data;
    if (self == GTK_BUTTON(e->button)) {
      entry = e;
      break;
    }
  }

  if (entry) {
    g_signal_emit(hypr, click_signal, 0, entry->id);
  }
}

static void g_barbar_hyprland_add_workspace(BarBarHyprlandWorkspace *hypr,

                                            int id, const char *name) {
  struct MonitorEntry *entry = new_entry(id, name);
  hypr->workspaces =
      g_list_insert_sorted(hypr->workspaces, entry, entry_compare_func);

  GList *list;

  for (list = hypr->workspaces; list; list = list->next) {
    struct MonitorEntry *e = list->data;
    if (e->id == entry->id) {
      break;
    }
  }
  gtk_widget_set_parent(entry->button, GTK_WIDGET(hypr));
  g_signal_connect(entry->button, "clicked", G_CALLBACK(clicked), hypr);
  if (list->prev) {
    struct entry *prev = list->prev->data;
    gtk_widget_insert_after(entry->button, GTK_WIDGET(hypr), prev->button);
  }
}

static void g_barbar_hyprland_remove_workspace(BarBarHyprlandWorkspace *hypr,
                                               int id) {
  GList *list;
  for (list = hypr->workspaces; list; list = list->next) {
    struct MonitorEntry *entry = list->data;
    if (entry->id == id) {
      break;
    }
  }

  if (list) {
    struct MonitorEntry *entry = list->data;
    gtk_widget_unparent(entry->button);
    g_free(entry);

    hypr->workspaces = g_list_remove_link(hypr->workspaces, list);
  }
}

static void g_barbar_hyprland_move_workspace(BarBarHyprlandWorkspace *hypr,
                                             int id, const char *workspacename,
                                             const char *monitor_name) {
  GList *list;
  // for (list = hypr->workspaces; list; list = list->next) {
  //   struct MonitorEntry *entry = list->data;
  //   if (entry->id == id) {
  //     break;
  //   }
  // }
  //
  // if (list) {
  //   struct MonitorEntry *entry = list->data;
  //   gtk_button_set_label(GTK_BUTTON(entry->button), name);
  // }
}

static void g_barbar_hyprland_rename_workspace(BarBarHyprlandWorkspace *hypr,
                                               int id, const char *name) {

  GList *list;
  for (list = hypr->workspaces; list; list = list->next) {
    struct MonitorEntry *entry = list->data;
    if (entry->id == id) {
      break;
    }
  }

  if (list) {
    struct MonitorEntry *entry = list->data;
    gtk_button_set_label(GTK_BUTTON(entry->button), name);
  }
}

static void parse_active_workspace(BarBarHyprlandWorkspace *hypr,
                                   JsonParser *parser) {
  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "monitor");
  const char *monitor = json_reader_get_string_value(reader);
  if (hypr->output_name && g_strcmp0(hypr->output_name, monitor)) {
    json_reader_end_member(reader);
    hypr->active_workspace = 0;
    hypr->focused = FALSE;
    goto cleanup;
  }
  json_reader_end_member(reader);

  json_reader_read_member(reader, "id");
  int id = json_reader_get_int_value(reader);
  struct MonitorEntry *entry = get_button(hypr, id);
  if (entry) {
    gtk_widget_add_css_class(entry->button, "focused");
  }
  hypr->active_workspace = id;
  hypr->focused = TRUE;
  json_reader_end_member(reader);

cleanup:
  g_object_unref(reader);
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {

  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);
  if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    hypr->xdg_output_manager =
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
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);
  g_set_str(&hypr->output_name, name);
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

static void active_async(GObject *source_object, GAsyncResult *res,
                         gpointer data) {
  GError *error = NULL;
  JsonParser *parser;
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  parser = g_barbar_hyprland_ipc_oneshot_finish(res, &error);

  if (error) {
    g_printerr("Failed to setup hyprland active workspace: %s\n",
               error->message);
    g_object_unref(parser);
    g_error_free(error);
    return;
  }

  parse_active_workspace(hypr, parser);
  g_object_unref(parser);

  hypr->service = g_barbar_hyprland_service_new();
  g_signal_connect(hypr->service, "workspace",
                   G_CALLBACK(g_barbar_hyprland_workspace_workspace_callback),
                   hypr);
  g_signal_connect(
      hypr->service, "focused-monitor",
      G_CALLBACK(g_barbar_hyprland_workspace_focused_monitor_callback), hypr);
  g_signal_connect(
      hypr->service, "create-workspace",
      G_CALLBACK(g_barbar_hyprland_workspace_create_workspace_callback), hypr);
  g_signal_connect(
      hypr->service, "destroy-workspace",
      G_CALLBACK(g_barbar_hyprland_workspace_destroy_workspace_callback), hypr);
  g_signal_connect(
      hypr->service, "move-workspace",
      G_CALLBACK(g_barbar_hyprland_workspace_move_workspace_callback), hypr);
  g_signal_connect(
      hypr->service, "rename-workspace",
      G_CALLBACK(g_barbar_hyprland_workspace_rename_workspace_callback), hypr);
  g_signal_connect(hypr->service, "urgent",
                   G_CALLBACK(g_barbar_hyprland_workspace_urgent_callback),
                   hypr);
}

static void worspace_async(GObject *source_object, GAsyncResult *res,
                           gpointer data) {
  GError *error = NULL;
  JsonParser *parser;
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  parser = g_barbar_hyprland_ipc_oneshot_finish(res, &error);

  if (error) {
    g_printerr("Failed to setup hyprland workspaces: %s\n", error->message);
    g_object_unref(parser);
    g_error_free(error);
    return;
  }

  parse_initional_workspaces(hypr, parser);
  g_object_unref(parser);
  g_barbar_hyprland_ipc_oneshot(NULL, active_async, "j/activeworkspace", hypr);
}

static void g_barbar_hyprland_workspace_map(GtkWidget *widget) {
  GTK_WIDGET_CLASS(g_barbar_hyprland_workspace_parent_class)->root(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_output *output;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;
  BarBarHyprlandWorkspace *hypr;

  hypr = BARBAR_HYPRLAND_WORKSPACE(widget);

  gdk_display = gdk_display_get_default();

  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(hypr), GTK_TYPE_WINDOW));
  // doesn't need to be a layer window
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    printf("Parent window not found!\n");
    return;
  }

  monitor = gtk_layer_get_monitor(window);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, hypr);
  wl_display_roundtrip(wl_display);

  if (!hypr->xdg_output_manager) {
    g_warning("Couldn't init the xdg output manager");
    return;
  }

  hypr->xdg_output =
      zxdg_output_manager_v1_get_xdg_output(hypr->xdg_output_manager, output);

  zxdg_output_v1_add_listener(hypr->xdg_output, &xdg_output_listener, hypr);
  wl_display_roundtrip(wl_display);

  g_barbar_hyprland_ipc_oneshot(NULL, worspace_async, "j/workspaces", hypr);
}

/**
 * g_barbar_hyprland_workspace_new:
 *
 * Returs: (transfer none): a `BarBarHyprlandWindow`
 */
GtkWidget *g_barbar_hyprland_workspace_new(void) {
  BarBarHyprlandWorkspace *hypr;

  hypr = g_object_new(BARBAR_TYPE_HYPRLAND_WORKSPACE, NULL);

  return GTK_WIDGET(hypr);
}
