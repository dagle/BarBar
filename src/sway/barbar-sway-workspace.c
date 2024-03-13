#include "sway/barbar-sway-workspace.h"
#include "sway/barbar-sway-ipc.h"
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

struct _BarBarSwayWorkspace {
  GtkWidget parent_instance;

  char *output_name;

  struct wl_output *output;

  // struct zriver_status_manager_v1 *status_manager;
  // struct wl_seat *seat;
  // struct wl_output *output;
  // struct zriver_seat_status_v1 *seat_listener;
  // gboolean focused;

  // This should be configureable, this isn't hardcoded
  // GtkWidget *buttons[10];
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
  int id;
  int num;

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

static void g_barbar_sway_workspace_constructed(GObject *object);
static void default_clicked_handler(BarBarSwayWorkspace *sway, guint workspace,
                                    gpointer user_data);

GtkWidget *g_barbar_sway_add_button(BarBarSwayWorkspace *sway,
                                    struct workspace *workspace);

static void g_barbar_sway_workspace_set_output(BarBarSwayWorkspace *sway,
                                               const gchar *output) {
  g_return_if_fail(BARBAR_IS_SWAY_WORKSPACE(sway));

  g_free(sway->output_name);

  sway->output_name = strdup(output);
  g_object_notify_by_pspec(G_OBJECT(clock), sway_workspace_props[PROP_OUTPUT]);
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

static void
g_barbar_sway_workspace_class_init(BarBarSwayWorkspaceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sway_workspace_set_property;
  gobject_class->get_property = g_barbar_sway_workspace_get_property;
  gobject_class->constructed = g_barbar_sway_workspace_constructed;

  sway_workspace_props[PROP_OUTPUT] =
      g_param_spec_string("output", NULL, NULL, NULL, G_PARAM_READWRITE);

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
static void g_barbar_sway_workspace_constructed(GObject *object) {}

static void default_clicked_handler(BarBarSwayWorkspace *sway, guint tag,
                                    gpointer user_data) {
  // send a clicky clock
}

void print_workspace(char *name, struct workspace *workspace) {
  printf("%s: %d %s %d %d %d %s\n", name, workspace->num, workspace->name,
         workspace->visible, workspace->urgent, workspace->focused,
         workspace->output);
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
  workspace->name = json_reader_get_string_value(reader);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "output");
  workspace->output = json_reader_get_string_value(reader);
  json_reader_end_member(reader);
}

void g_barbar_sway_workspace_add(BarBarSwayWorkspace *sway,
                                 JsonReader *reader) {
  struct workspace current = {0};

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  g_barbar_sway_add_button(sway, &current);
}

static void g_barbar_workspace_free(struct workspace *workspace) {}

void g_barbar_sway_workspace_empty(BarBarSwayWorkspace *sway,
                                   JsonReader *reader) {

  GList *list;
  struct workspace current = {0};

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  for (list = sway->workspaces; list; list = list->next) {
    struct entry *e = list->data;
    if (e->num == current.num) {
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

void g_barbar_sway_workspace_focus(BarBarSwayWorkspace *sway,
                                   JsonReader *reader) {
  GList *list;
  struct workspace old = {0};
  struct workspace current = {0};

  json_reader_read_member(reader, "old");
  g_barbar_sway_read_workspace(reader, &old);
  json_reader_end_member(reader);

  for (list = sway->workspaces; list; list = list->next) {
    struct entry *e = list->data;
    if (e->num == old.num) {
      break;
    }
  }
  if (list) {
    struct entry *entry = list->data;
    gtk_widget_remove_css_class(entry->button, "focused");
  }

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  for (list = sway->workspaces; list; list = list->next) {
    struct entry *e = list->data;
    if (e->num == current.num) {
      break;
    }
  }

  if (list) {
    struct entry *entry = list->data;
    gtk_widget_add_css_class(entry->button, "focused");
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

  // the workspace is moving away from us
  if (!strcmp(old.output, sway->output_name)) {
    for (list = sway->workspaces; list; list = list->next) {
      struct entry *e = list->data;
      if (e->num == current.num) {
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
  } else if (!strcmp(old.output, sway->output_name)) {
    g_barbar_sway_add_button(sway, &current);
  }
}
void g_barbar_sway_workspace_rename(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {
  struct workspace current = {0};

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  // change the name
}

void g_barbar_sway_workspace_urgent(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {

  GList *list;
  struct workspace current = {0};

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);

  for (list = sway->workspaces; list; list = list->next) {
    struct entry *e = list->data;
    if (e->num == current.num) {
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

void g_barbar_sway_workspace_reload(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {
  // reload everythg
}

static void g_barbar_sway_handle_workspaces_change(gchar *payload, uint32_t len,
                                                   uint32_t type,
                                                   gpointer data) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(data);

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway workspace: Failed to parse json: %s", err->message);
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
    printf("%.*s\n", len, payload);
    g_barbar_sway_workspace_rename(sway, reader);
  } else if (!strcmp(change, "urgent")) {
    g_barbar_sway_workspace_urgent(sway, reader);
  } else if (!strcmp(change, "reload")) {
    g_barbar_sway_workspace_reload(sway, reader);
  }
  g_object_unref(reader);
}

static gint compare_func(gconstpointer a, gconstpointer b) {
  const struct workspace *wrka = a;
  const struct workspace *wrkb = b;

  return wrka->num - wrkb->num;
}

GtkWidget *g_barbar_sway_add_button(BarBarSwayWorkspace *sway,
                                    struct workspace *workspace) {
  GtkWidget *btn;
  GList *list;

  struct entry *entry = g_malloc0(sizeof(struct entry));
  entry->num = workspace->num;
  entry->id = workspace->id;
  sway->workspaces =
      g_list_insert_sorted(sway->workspaces, entry, compare_func);

  for (list = sway->workspaces; list; list = list->next) {
    struct entry *e = list->data;
    if (e->num == entry->num) {
      break;
    }
  }

  btn = gtk_button_new_with_label(workspace->name);

  gtk_widget_set_parent(btn, GTK_WIDGET(sway));
  if (list->prev) {
    struct entry *prev = list->prev->data;
    gtk_widget_insert_after(btn, GTK_WIDGET(sway), prev->button);
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
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  gint i = json_reader_count_elements(reader);

  for (int j = 0; j < i; j++) {
    struct workspace workspace = {0};
    GtkWidget *button;
    json_reader_read_element(reader, j);
    g_barbar_sway_read_workspace(reader, &workspace);
    button = g_barbar_sway_add_button(sway, &workspace);

    if (workspace.focused) {
      gtk_widget_add_css_class(button, "focused");
    }

    json_reader_end_element(reader);
  }
}

void g_barbar_sway_workspace_start(BarBarSwayWorkspace *sway) {
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;

  GError *error = NULL;
  gchar *buf = NULL;
  int len;
  BarBarSwayIpc *ipc;

  const char *intrest = "[\"workspace\"]";

  gdk_display = gdk_display_get_default();

  GtkNative *native = gtk_widget_get_native(GTK_WIDGET(sway));
  GdkSurface *surface = gtk_native_get_surface(native);
  monitor = gdk_display_get_monitor_at_surface(gdk_display, surface);
  sway->output = gdk_wayland_monitor_get_wl_output(monitor);

  // TODO: We need to get the output->name, we can't really do that atm
  // because gtk4 binds to the wl_output interface version 3, which doesn't
  // support this. This will change in future. For now the user needs to specify
  // the output. This will be fixed in the future.

  ipc = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    g_printerr("Sway workspace: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }
  // g_barbar_sway_ipc_subscribe(connection, payload);
  g_barbar_sway_ipc_send(ipc, SWAY_GET_WORKSPACES, "");
  len = g_barbar_sway_ipc_read(ipc, &buf, NULL);
  if (len > 0) {
    g_barbar_sway_handle_workspaces(sway, buf, len);

    g_free(buf);
  }

  g_barbar_sway_ipc_subscribe(ipc, intrest, sway,
                              g_barbar_sway_handle_workspaces_change);

  // g_barbar_sway_ipc_close(ipc);
}
