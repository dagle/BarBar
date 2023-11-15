#include "sway/barbar-sway-workspace.h"
#include "sway/barbar-sway-ipc.h"
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarSwayWorkspace {
  GtkWidget parent_instance;

  // struct zriver_status_manager_v1 *status_manager;
  // struct wl_seat *seat;
  // struct wl_output *output;
  // struct zriver_seat_status_v1 *seat_listener;
  // gboolean focused;

  // This should be configureable, this isn't hardcoded
  GtkWidget *buttons[10];
};

enum {
  PROP_0,

  PROP_DEVICE,

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

static void g_barbar_sway_workspace_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {}

static void g_barbar_sway_workspace_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {}

static void
g_barbar_sway_workspace_class_init(BarBarSwayWorkspaceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sway_workspace_set_property;
  gobject_class->get_property = g_barbar_sway_workspace_get_property;
  gobject_class->constructed = g_barbar_sway_workspace_constructed;
  sway_workspace_props[PROP_DEVICE] =
      g_param_spec_uint("tagnums", NULL, NULL, 0, 9, 9, G_PARAM_READWRITE);

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
static void g_barbar_sway_workspace_constructed(GObject *object) {
  BarBarSwayWorkspace *sway = BARBAR_SWAY_WORKSPACE(object);
  GtkWidget *btn;
  char str[2];
  for (uint32_t i = 0; i < 10; i++) {
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label("");
    // gtk_widget_set_name(btn, "tags");
    gtk_widget_set_parent(btn, GTK_WIDGET(sway));
    // g_signal_connect(btn, "clicked", G_CALLBACK(clicked), river);
    sway->buttons[i] = btn;
  }
  // sway->label = gtk_label_new("");
  // gtk_widget_set_parent(sway->label, GTK_WIDGET(sway));
  // sway->focused = FALSE;
}

static void default_clicked_handler(BarBarSwayWorkspace *sway, guint tag,
                                    gpointer user_data) {
  // send a clicky clock
}

struct workspace {
  int id;
  int num;

  gboolean visible;
  gboolean urgent;
  gboolean focused;

  const char *name;
  const char *output;
};

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
  struct workspace current;

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);
  print_workspace("init", &current);
  // do stuff
}
void g_barbar_sway_workspace_empty(BarBarSwayWorkspace *sway,
                                   JsonReader *reader) {

  struct workspace current;

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);
  // do stuff
}
void g_barbar_sway_workspace_focus(BarBarSwayWorkspace *sway,
                                   JsonReader *reader) {
  struct workspace old;
  struct workspace current;

  json_reader_read_member(reader, "old");
  g_barbar_sway_read_workspace(reader, &old);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);
  print_workspace("unfocus", &old);
  print_workspace("focus", &current);
  // do stuff
}
void g_barbar_sway_workspace_move(BarBarSwayWorkspace *sway,
                                  JsonReader *reader) {
  struct workspace old;
  struct workspace current;

  json_reader_read_member(reader, "old");
  g_barbar_sway_read_workspace(reader, &old);
  json_reader_end_member(reader);

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);
  // do stuff
}
void g_barbar_sway_workspace_rename(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {
  // struct workspace old;
  struct workspace current;

  // do we get an old part?

  // json_reader_read_member(reader, "old");
  // g_barbar_sway_read_workspace(reader, &old);
  // json_reader_end_member(reader);
  //
  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);
  // do stuff
}
void g_barbar_sway_workspace_urgent(BarBarSwayWorkspace *sway,
                                    JsonReader *reader) {

  struct workspace current;

  json_reader_read_member(reader, "current");
  g_barbar_sway_read_workspace(reader, &current);
  json_reader_end_member(reader);
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
    printf("json error: %s\n", err->message);
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  // printf("%.*s\n", len, payload);

  json_reader_read_member(reader, "change");
  const char *change = json_reader_get_string_value(reader);
  json_reader_end_member(reader);

  // all of these should have if ours
  if (!strcmp(change, "init")) {
    g_barbar_sway_workspace_add(sway, reader);
    // read current and create the new workspace

  } else if (!strcmp(change, "empty")) {
    g_barbar_sway_workspace_empty(sway, reader);
    // read current and delete the workspace

  } else if (!strcmp(change, "focus")) {
    g_barbar_sway_workspace_focus(sway, reader);
    // read old and unfocus it
    // read current and focus it, remove urgent

  } else if (!strcmp(change, "move")) {
    g_barbar_sway_workspace_move(sway, reader);
    // read old and delete the workspace, if ours
    // read current and create the workspace, if ours

  } else if (!strcmp(change, "rename")) {
    g_barbar_sway_workspace_rename(sway, reader);
    // read current and update the name

  } else if (!strcmp(change, "urgent")) {
    g_barbar_sway_workspace_urgent(sway, reader);
    // read current update the css

  } else if (!strcmp(change, "reload")) {
    g_barbar_sway_workspace_reload(sway, reader);
    // restart the module
  }
  // these are the different events we need to handle
  // init empty focus move rename urgent reload

  // json_reader_read_member(reader, "old");
  //
  // json_reader_read_member(reader, "num");
  // int old_num = json_reader_get_int_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "name");
  // const char *old_name = json_reader_get_string_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "visible");
  // gboolean old_vis = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "focused");
  // gboolean old_focused = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "urgent");
  // gboolean old_urgent = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "output");
  // const char *old_output = json_reader_get_string_value(reader);
  // json_reader_end_member(reader);
  //
  // json_reader_end_member(reader);
  // printf("---------------------------\n");
  // printf("old workspace: %d %s %d %d %d %s\n", old_num, old_name, old_vis,
  //        old_focused, old_urgent, old_output);
  //
  // json_reader_read_member(reader, "current");
  //
  // json_reader_read_member(reader, "num");
  // int current_num = json_reader_get_int_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "name");
  // const char *current_name = json_reader_get_string_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "visible");
  // gboolean current_vis = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "focused");
  // gboolean current_focused = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "urgent");
  // gboolean current_urgent = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "output");
  // const char *current_output = json_reader_get_string_value(reader);
  // json_reader_end_member(reader);
  //
  // json_reader_end_member(reader);
  //
  // printf("---------------------------\n");
  // printf("current workspace: %d %s %d %d %d %s\n", current_num, current_name,
  //        current_vis, current_focused, current_urgent, current_output);
  //
  // printf("\n\n\n");

  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "name");
  // const char *name = json_reader_get_string_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "visible");
  // gboolean vis = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "focused");
  // gboolean focused = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "urgent");
  // gboolean urgent = json_reader_get_boolean_value(reader);
  // json_reader_end_member(reader);
  // json_reader_read_member(reader, "output");
  // const char *output = json_reader_get_string_value(reader);
  // json_reader_end_member(reader);

  // gtk_button_set_label(GTK_BUTTON(sway->buttons[num - 1]), name);
  // gtk_widget_set_visible(sway->buttons[num - 1], true);
}

static void g_barbar_sway_handle_workspaces(BarBarSwayWorkspace *sway,
                                            gchar *payload, gssize len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  // printf("%.*s\n", len, payload);

  if (!ret) {
    printf("json error: %s\n", err->message);
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  gint i = json_reader_count_elements(reader);

  for (int j = 0; j < 10; j++) {
    gtk_widget_set_visible(sway->buttons[j], false);
  }
  for (int j = 0; j < i; j++) {
    json_reader_read_element(reader, j);
    json_reader_read_member(reader, "num");
    int num = json_reader_get_int_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "name");
    const char *name = json_reader_get_string_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "visible");
    gboolean vis = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "focused");
    gboolean focused = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "urgent");
    gboolean urgent = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "output");
    const char *output = json_reader_get_string_value(reader);
    json_reader_end_member(reader);

    gtk_button_set_label(GTK_BUTTON(sway->buttons[num - 1]), name);
    gtk_widget_set_visible(sway->buttons[num - 1], true);

    printf("workspace: %d %s %d %d %d %s\n", num, name, vis, focused, urgent,
           output);

    json_reader_end_element(reader);
  }
}

void g_barbar_sway_workspace_start(BarBarSwayWorkspace *sway) {
  GError *error = NULL;
  gchar *buf = NULL;
  int len;
  BarBarSwayIpc *ipc;

  const char *intrest = "[\"workspace\"]";

  ipc = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    printf("Error: %s\n", error->message);
    // TODO: Error stuff
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
