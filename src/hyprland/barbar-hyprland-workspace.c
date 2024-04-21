#include "barbar-hyprland-workspace.h"
#include "barbar-hyprland-ipc.h"
#include "gtk4-layer-shell.h"
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

  GSocketConnection *listener;

  // struct wl_output *output;

  GList *workspaces; // A list of workspaces;
};

struct MonitorEntry {
  int id;
  GtkWidget *button;
};

// struct workspace {
//   int id;
//   int num;
//
//   gboolean focused;
//
//   gboolean visible;
//   gboolean urgent;
//
//   char *name;
//   char *output;
// };

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

static void default_clicked_handler(BarBarHyprlandWorkspace *sway, guint tag,
                                    gpointer user_data) {
  // send a clicky clock
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

static void g_barbar_hyprland_workspace_callback(uint32_t type, char *args,
                                                 gpointer data) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);

  switch (type) {
  case HYPRLAND_WORKSPACEV2: {
    struct MonitorEntry *entry;
    gchar **tokens = g_strsplit(args, ",", -1);
    int id = atoi(tokens[0]);

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
    g_strfreev(tokens);
    break;
  }
  case HYPRLAND_URGENT: {
    // WINDOWADDRESS
    break;
  }
  case HYPRLAND_FOCUSEDMON: {
    // MONNAME,WORKSPACENAME
    gchar **tokens = g_strsplit(args, ",", -1);
    const char *monname = tokens[0];

    // do we need to do anything more? HYPRLAND_WORKSPACEV2 will happen?
    if (!hypr->output_name || !g_strcmp0(hypr->output_name, monname)) {
      hypr->focused = TRUE;
    } else {
      hypr->focused = FALSE;
    }

    break;
  }
  case HYPRLAND_CREATEWORKSPACEV2: {
    // WORKSPACEID,WORKSPACENAME
    gchar **tokens = g_strsplit(args, ",", -1);

    int id = atoi(tokens[0]);
    g_barbar_hyprland_add_workspace(hypr, id, tokens[1]);
    g_strfreev(tokens);
    break;
  }
  case HYPRLAND_DESTROYWORKSPACEV2: {
    // WORKSPACEID,WORKSPACENAME
    gchar **tokens = g_strsplit(args, ",", -1);

    int id = atoi(tokens[0]);
    g_barbar_hyprland_remove_workspace(hypr, id);

    g_strfreev(tokens);
    break;
  }
  case HYPRLAND_MOVEWORKSPACEV2: {
    // WORKSPACEID,WORKSPACENAME,MONNAME
    gchar **tokens = g_strsplit(args, ",", -1);

    int id = atoi(tokens[0]);
    g_barbar_hyprland_move_workspace(hypr, id, tokens[1], tokens[2]);
    g_strfreev(tokens);
    break;
  }
  case HYPRLAND_RENAMEWORKSPACE: {
    // WORKSPACEID,NEWNAME
    gchar **tokens = g_strsplit(args, ",", -1);
    int id = atoi(tokens[0]);
    g_barbar_hyprland_rename_workspace(hypr, id, tokens[1]);
    g_strfreev(tokens);
    break;
  }
  }
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
                                       GSocketConnection *ipc) {
  JsonParser *parser;
  GInputStream *input_stream;
  GError *err = NULL;
  parser = json_parser_new();

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc));
  gboolean ret = json_parser_load_from_stream(parser, input_stream, NULL, &err);

  if (!ret) {
    g_printerr("Hyprland workspace: Failed to parse json: %s", err->message);
  }

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

      json_reader_read_member(reader, "monitor");
      const char *monitor = json_reader_get_string_value(reader);
      json_reader_end_member(reader);

      json_reader_read_member(reader, "monitorID");
      int monitor_id = json_reader_get_int_value(reader);
      json_reader_end_member(reader);
      json_reader_end_element(reader);

      printf("monitor: %s\n", monitor);

      // if (!g_strcmp0(monitor, "DVI-D-1")) {
      g_barbar_hyprland_add_workspace(hypr, id, name);
      // }
      // printf("json: id: %d, name: %s, monitor: %s, monitor_id: %d\n", id,
      // name,
      //        monitor, monitor_id);
    }
  }
  g_object_unref(reader);
  g_object_unref(parser);
}

static void clicked(GtkButton *self, gpointer user_data) {
  struct MonitorEntry *entry = user_data;
  GError *error = NULL;

  GSocketConnection *ipc = g_barbar_hyprland_ipc_controller(&error);
  if (error) {
    g_printerr("Hyprland workspace: Error connecting to the ipc: %s",
               error->message);
    return;
  }

  char *str = g_strdup_printf("dispatch workspace %d", entry->id);
  g_barbar_hyprland_ipc_send_command(ipc, str, &error);
  g_free(str);
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
  g_signal_connect(entry->button, "clicked", G_CALLBACK(clicked), entry);
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

// TODO: when we can listen for the output version 4
//
// static void noop() {}
//
// static const struct wl_output_listener wl_output_listener = {
//     .name = wl_output_handle_name,
//     .geometry = noop,
//     .mode = noop,
//     .scale = noop,
//     .description = noop,
//     .done = noop,
// };
//
static void parse_active_workspace(BarBarHyprlandWorkspace *hypr,
                                   GSocketConnection *ipc) {
  JsonParser *parser;
  GInputStream *input_stream;
  GError *err = NULL;
  parser = json_parser_new();

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(ipc));
  gboolean ret = json_parser_load_from_stream(parser, input_stream, NULL, &err);

  if (!ret) {
    g_printerr("Hyprland workspace: Failed to parse json: %s", err->message);
  }

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
  g_object_unref(parser);
}

static void g_barbar_hyprland_workspace_map(GtkWidget *widget) {
  GError *error = NULL;

  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(widget);

  GSocketConnection *ipc = g_barbar_hyprland_ipc_controller(&error);

  if (error) {
    g_printerr("Hyprland workspace: Error connecting to the ipc: %s",
               error->message);
    return;
  }

  g_barbar_hyprland_ipc_send_command(ipc, "j/workspaces", &error);
  if (error) {
    g_printerr("Hyprland workspace: Error sending command for initial "
               "workspaces: %s",
               error->message);
    return;
  }

  parse_initional_workspaces(hypr, ipc);
  g_object_unref(ipc);

  ipc = g_barbar_hyprland_ipc_controller(&error);

  if (error) {
    g_printerr("Hyprland workspace: Error connecting to the ipc: %s",
               error->message);
    return;
  }

  g_barbar_hyprland_ipc_send_command(ipc, "j/activeworkspace", &error);
  if (error) {
    g_printerr(
        "Hyprland workspace: Error sending command for initial workspace: %s",
        error->message);
    return;
  }
  parse_active_workspace(hypr, ipc);

  g_object_unref(ipc);

  hypr->listener = g_barbar_hyprland_ipc_listner(
      g_barbar_hyprland_workspace_callback, hypr, NULL, &error);

  if (error) {
    g_printerr("error setting up listner: %s\n", error->message);
  }
}
