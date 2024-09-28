#include "sway/barbar-sway-workspace.h"
#include "glib-object.h"
#include "gtk4-layer-shell.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include <ctype.h>
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarSwayWorkspace:
 *
 * A widget to display the sway workspaces as a list of buttons
 */
struct _BarBarSwayWorkspace {
  GtkWidget parent_instance;

  char *output_name;

  BarBarSwaySubscribe *sub;
  struct zxdg_output_manager_v1 *xdg_output_manager;
  struct zxdg_output_v1 *xdg_output;

  GList *workspaces; // A list of workspaces;
};

struct workspace {
  int id;
  int num;

  gboolean visible;
  gboolean urgent;
  gboolean focused;

  char *name;
  char *output;
};

struct entry {
  int num;
  int id;

  GtkWidget *button;
};

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayWorkspace, g_barbar_sway_workspace, GTK_TYPE_WIDGET)

static GParamSpec *sway_workspace_props[NUM_PROPERTIES] = {
    NULL,
};

static guint click_signal;

static void g_barbar_sway_workspace_start(GtkWidget *widget);

static void default_clicked_handler(BarBarSwayWorkspace *sway, guint workspace,
                                    gpointer user_data);

GtkWidget *g_barbar_sway_add_button(BarBarSwayWorkspace *sway,
                                    struct workspace *workspace);

static void g_barbar_sway_workspace_set_output(BarBarSwayWorkspace *sway,
                                               const gchar *output) {
  g_return_if_fail(BARBAR_IS_SWAY_WORKSPACE(sway));

  if (g_set_str(&sway->output_name, output)) {
    g_object_notify_by_pspec(G_OBJECT(sway), sway_workspace_props[PROP_OUTPUT]);
  }
}

static void g_barbar_sway_workspace_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_sway_workspace_set_output(sway, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_workspace_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, sway->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_workspace_finalize(GObject *object) {
  BarBarSwayWorkspace *workspace = BARBAR_SWAY_WORKSPACE(object);

  g_clear_object(&workspace->sub);
  g_list_free_full(workspace->workspaces, g_free);
  g_free(workspace->output_name);

  G_OBJECT_CLASS(g_barbar_sway_workspace_parent_class)->finalize(object);
}

static void
g_barbar_sway_workspace_class_init(BarBarSwayWorkspaceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sway_workspace_set_property;
  gobject_class->get_property = g_barbar_sway_workspace_get_property;
  gobject_class->finalize = g_barbar_sway_workspace_finalize;

  /**
   * BarBarSwayWorkspace:output:
   *
   * What screen this is monitoring
   */
  sway_workspace_props[PROP_OUTPUT] =
      g_param_spec_string("output", NULL, NULL, NULL, G_PARAM_READWRITE);

  widget_class->root = g_barbar_sway_workspace_start;

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    sway_workspace_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "sway-workspace");
}

static void g_barbar_sway_workspace_init(BarBarSwayWorkspace *self) {}

static void default_clicked_handler(BarBarSwayWorkspace *sway, guint tag,
                                    gpointer user_data) {
  g_barbar_sway_ipc_oneshot(SWAY_RUN_COMMAND, FALSE, NULL, NULL, NULL,
                            "%workspace %d", tag);
}

void g_barbar_sway_read_workspace(JsonReader *reader,
                                  struct workspace *workspace) {
  json_reader_read_member(reader, "id");
  workspace->id = json_reader_get_int_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "num");
  workspace->num = json_reader_get_int_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "visible");
  workspace->visible = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "urgent");
  workspace->urgent = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "focused");
  workspace->focused = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "name");
  workspace->name = g_strdup(json_reader_get_string_value(reader));
  json_reader_end_member(reader);

  json_reader_read_member(reader, "output");
  workspace->output = g_strdup(json_reader_get_string_value(reader));
  json_reader_end_member(reader);
}

void g_barbar_sway_workspace_add(BarBarSwayWorkspace *sway,
                                 JsonReader *reader) {
  struct workspace current = {0};

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  if (!g_strcmp0(current.output, sway->output_name)) {
    g_barbar_sway_add_button(sway, &current);
  }
}

void g_barbar_sway_workspace_empty(BarBarSwayWorkspace *sway,
                                   JsonReader *reader) {

  GList *list;
  struct workspace current = {0};

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  if (!g_strcmp0(current.output, sway->output_name)) {
    for (list = sway->workspaces; list; list = list->next) {
      struct entry *e = list->data;
      if (e->id == current.id) {
        break;
      }
    }
    if (list) {
      struct entry *entry = list->data;
      gtk_widget_unparent(entry->button);
      g_free(entry);
      sway->workspaces = g_list_remove_link(sway->workspaces, list);
    }
  }
}

void g_barbar_sway_workspace_focus(BarBarSwayWorkspace *sway,
                                   JsonReader *reader) {
  GList *list;
  struct workspace old = {0};
  struct workspace current = {0};

  json_reader_read_member(reader, "old");
  g_barbar_sway_read_workspace(reader, &old);
  json_reader_end_member(reader);

  if (!g_strcmp0(sway->output_name, old.output)) {

    for (list = sway->workspaces; list; list = list->next) {
      struct entry *e = list->data;
      if (e->id == old.id) {
        break;
      }
    }
    if (list) {
      struct entry *entry = list->data;
      gtk_widget_remove_css_class(entry->button, "focused");
    }
  }

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  if (!g_strcmp0(sway->output_name, current.output)) {
    for (list = sway->workspaces; list; list = list->next) {
      struct entry *e = list->data;
      if (e->id == current.id) {
        break;
      }
    }

    if (list) {
      struct entry *entry = list->data;
      gtk_widget_add_css_class(entry->button, "focused");
    }
  }
}

void g_barbar_sway_workspace_move(BarBarSwayWorkspace *sway,
                                  JsonReader *reader) {
  struct workspace old = {0};
  struct workspace current = {0};
  GList *list;

  json_reader_read_member(reader, "old");
  g_barbar_sway_read_workspace(reader, &old);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  if (!g_strcmp0(old.output, sway->output_name)) {
    for (list = sway->workspaces; list; list = list->next) {
      struct entry *e = list->data;
      if (e->id == current.id) {
        break;
      }
    }
    if (list) {
      struct entry *entry = list->data;
      gtk_widget_unparent(entry->button);
      g_free(entry);
      sway->workspaces = g_list_remove_link(sway->workspaces, list);
    }
    // the workspace is moving to us
  }
  if (!g_strcmp0(current.output, sway->output_name)) {
    g_barbar_sway_add_button(sway, &current);
  }
}
void g_barbar_sway_workspace_rename(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {
  struct workspace current = {0};
  GList *list;

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  if (!g_strcmp0(current.output, sway->output_name)) {
    for (list = sway->workspaces; list; list = list->next) {
      struct entry *e = list->data;
      if (e->id == current.id) {
        break;
      }
    }

    if (list) {
      struct entry *entry = list->data;
      gtk_button_set_label(GTK_BUTTON(entry->button), current.name);
    }
  }
}

void g_barbar_sway_workspace_urgent(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {

  GList *list;
  struct workspace current = {0};

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  if (!g_strcmp0(current.output, sway->output_name)) {
    for (list = sway->workspaces; list; list = list->next) {
      struct entry *e = list->data;
      if (e->id == current.id) {
        break;
      }
    }

    if (list) {
      struct entry *entry = list->data;
      if (current.urgent) {
        gtk_widget_add_css_class(entry->button, "urgent");
      } else {
        gtk_widget_remove_css_class(entry->button, "urgent");
      }
    }
  }
}

void g_barbar_sway_workspace_reload(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {
  // reload everythg
}

static void g_barbar_sway_handle_workspaces_change(uint32_t type,
                                                   JsonParser *parser,
                                                   gpointer data) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(data);

  if (type != SWAY_WORKSPACE_EVENT) {
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "change");
  const char *change = json_reader_get_string_value(reader);
  json_reader_end_member(reader);

  if (!strcmp(change, "init")) {
    g_barbar_sway_workspace_add(sway, reader);
  } else if (!strcmp(change, "empty")) {
    g_barbar_sway_workspace_empty(sway, reader);
  } else if (!strcmp(change, "focus")) {
    g_barbar_sway_workspace_focus(sway, reader);
  } else if (!strcmp(change, "move")) {
    g_barbar_sway_workspace_move(sway, reader);
  } else if (!strcmp(change, "rename")) {
    g_barbar_sway_workspace_rename(sway, reader);
  } else if (!strcmp(change, "urgent")) {
    g_barbar_sway_workspace_urgent(sway, reader);
  } else if (!strcmp(change, "reload")) {
    g_barbar_sway_workspace_reload(sway, reader);
  }
  g_object_unref(reader);
}

static gint compare_func(gconstpointer a, gconstpointer b) {
  const struct entry *wrka = a;
  const struct entry *wrkb = b;

  return wrka->num - wrkb->num;
}

static int g_barbar_sway_workspace_to_num(const char *name) {
  if (isdigit(name[0]) != 0) {
    errno = 0;
    char *endptr = NULL;
    long long parsed_num = strtoll(name, &endptr, 10);
    if (errno != 0 || parsed_num > INT32_MAX || parsed_num < 0 ||
        endptr == name) {
      return -1;
    }
    return (int)parsed_num;
  }
  return -1;
}

GtkWidget *g_barbar_sway_add_button(BarBarSwayWorkspace *sway,
                                    struct workspace *workspace) {
  GtkWidget *btn;
  GList *list;

  struct entry *entry = g_malloc0(sizeof(struct entry));
  entry->id = workspace->id;
  entry->num = workspace->num;
  sway->workspaces =
      g_list_insert_sorted(sway->workspaces, entry, compare_func);

  for (list = sway->workspaces; list; list = list->next) {
    struct entry *e = list->data;
    if (e->id == entry->id) {
      break;
    }
  }

  btn = gtk_button_new_with_label(workspace->name);

  gtk_widget_set_parent(btn, GTK_WIDGET(sway));
  if (list->prev) {
    struct entry *prev = list->prev->data;
    gtk_widget_insert_after(btn, GTK_WIDGET(sway), prev->button);
  }

  if (!workspace->visible) {
    gtk_widget_add_css_class(btn, "invisible");
  }

  if (workspace->focused) {
    gtk_widget_add_css_class(btn, "focused");
  }
  if (workspace->urgent) {
    gtk_widget_add_css_class(btn, "urgent");
  }

  entry->button = btn;
  return btn;
}

static void g_barbar_sway_handle_workspaces(BarBarSwayWorkspace *sway,
                                            gchar *payload, gssize len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway workspace: Failed to parse json: %s", err->message);
    g_error_free(err);
    g_object_unref(parser);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  gint i = json_reader_count_elements(reader);

  for (int j = 0; j < i; j++) {
    struct workspace workspace = {0};
    GtkWidget *button;
    json_reader_read_element(reader, j);
    g_barbar_sway_read_workspace(reader, &workspace);
    if (!g_strcmp0(workspace.output, sway->output_name)) {
      button = g_barbar_sway_add_button(sway, &workspace);
    }

    json_reader_end_element(reader);
  }
  g_object_unref(reader);
  g_object_unref(parser);
}

static void event_listner(BarBarSwaySubscribe *sub, guint type,
                          JsonParser *parser, gpointer data) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(data);
  g_barbar_sway_handle_workspaces_change(type, parser, sway);
}

static void workspaces_cb(GObject *object, GAsyncResult *res, gpointer data) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_oneshot_finish(res, NULL, &str, &len, &error);

  if (error) {
    g_printerr("Sway workspace: Failed to get workspaces: %s\n",
               error->message);
    g_error_free(error);
    return;
  }

  if (ret) {
    g_barbar_sway_handle_workspaces(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {

  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(data);
  if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    sway->xdg_output_manager =
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
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(data);
  g_barbar_sway_workspace_set_output(sway, name);
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

static void g_barbar_sway_workspace_start(GtkWidget *widget) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_output *output;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_sway_workspace_parent_class)->root(widget);

  gdk_display = gdk_display_get_default();

  sway->sub =
      BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"workspace\"]"));

  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(sway), GTK_TYPE_WINDOW));
  // doesn't need to be a layer window
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    printf("Parent window not found!\n");
    return;
  }

  monitor = gtk_layer_get_monitor(window);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, sway);
  wl_display_roundtrip(wl_display);

  if (!sway->xdg_output_manager) {
    g_warning("Couldn't init the xdg output manager");
    return;
  }

  sway->xdg_output =
      zxdg_output_manager_v1_get_xdg_output(sway->xdg_output_manager, output);

  zxdg_output_v1_add_listener(sway->xdg_output, &xdg_output_listener, sway);
  wl_display_roundtrip(wl_display);

  g_barbar_sway_ipc_oneshot(SWAY_GET_WORKSPACES, TRUE, NULL, workspaces_cb,
                            sway, "");
}

/**
 * g_barbar_sway_workspace_new:
 *
 * Returns: (transfer full): a new `BarBarSwayWorkspace`
 */
GtkWidget *g_barbar_sway_workspace_new(void) {
  BarBarSwayWorkspace *ws;

  ws = g_object_new(BARBAR_TYPE_SWAY_WORKSPACE, NULL);

  return GTK_WIDGET(ws);
}
