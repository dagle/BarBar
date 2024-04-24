#include "sway/barbar-sway-window.h"
#include "sway/barbar-sway-ipc.h"
#include "sway/barbar-sway-subscribe.h"
#include <gdk/wayland/gdkwayland.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarSwayWindow {
  GtkWidget parent_instance;

  char *output_name;
  GSocketConnection *ipc;
  BarBarSwaySubscribe *sub;

  struct wl_output *output;

  GtkWidget *label;
};

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarSwayWindow, g_barbar_sway_window, GTK_TYPE_WIDGET)

static GParamSpec *sway_window_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_sway_window_start(GtkWidget *widget);

static void g_barbar_sway_window_constructed(GObject *object);

static void g_barbar_sway_window_set_output(BarBarSwayWindow *sway,
                                            const gchar *output) {
  g_return_if_fail(BARBAR_IS_SWAY_WINDOW(sway));

  g_free(sway->output_name);

  sway->output_name = strdup(output);
  g_object_notify_by_pspec(G_OBJECT(clock), sway_window_props[PROP_OUTPUT]);
}

static void g_barbar_sway_window_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_sway_window_set_output(sway, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_window_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(object);

  switch (property_id) {
  case PROP_OUTPUT:
    g_value_set_string(value, sway->output_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_sway_window_class_init(BarBarSwayWindowClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_sway_window_set_property;
  gobject_class->get_property = g_barbar_sway_window_get_property;
  gobject_class->constructed = g_barbar_sway_window_constructed;
  widget_class->root = g_barbar_sway_window_start;

  sway_window_props[PROP_OUTPUT] =
      g_param_spec_string("output", NULL, NULL, NULL, G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    sway_window_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "sway-window");
}

static void g_barbar_sway_window_init(BarBarSwayWindow *self) {}
static void g_barbar_sway_window_constructed(GObject *object) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(object);

  sway->label = gtk_label_new("");
  gtk_widget_set_parent(sway->label, GTK_WIDGET(sway));
}

static void g_barbar_sway_handle_window_change(const char *payload,
                                               uint32_t len, uint32_t type,
                                               gpointer data) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway window: Failed to parse json: %s", err->message);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  json_reader_read_member(reader, "change");
  const char *change = json_reader_get_string_value(reader);
  json_reader_end_member(reader);

  // if (!strcmp(change, "init")) {
  //   g_barbar_sway_workspace_add(sway, reader);
  // }
  json_reader_read_member(reader, "container");
  json_reader_read_member(reader, "focused");
  gboolean focused = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);
  if (focused) {
    json_reader_read_member(reader, "name");
    const char *name = json_reader_get_string_value(reader);
    json_reader_end_member(reader);
    gtk_label_set_text(GTK_LABEL(sway->label), name);
  }
  json_reader_end_member(reader);
  g_object_unref(reader);
}
static void g_barbar_sway_set_focused_tree(BarBarSwayWindow *sway,
                                           JsonReader *reader) {
  json_reader_read_member(reader, "focused");
  gboolean focused = json_reader_get_boolean_value(reader);
  json_reader_end_member(reader);

  if (focused) {
    json_reader_read_member(reader, "name");
    const char *name = json_reader_get_string_value(reader);
    json_reader_end_member(reader);

    gtk_label_set_text(GTK_LABEL(sway->label), name);
  } else {
    json_reader_read_member(reader, "nodes");
    if (json_reader_is_array(reader)) {
      int n = json_reader_count_elements(reader);
      for (int i = 0; i < n; i++) {
        json_reader_read_element(reader, i);
        g_barbar_sway_set_focused_tree(sway, reader);
        json_reader_end_element(reader);
      }
    }
    json_reader_end_member(reader);
  }
}

static void g_barbar_sway_initial_window(BarBarSwayWindow *sway, gchar *payload,
                                         uint32_t len) {
  JsonParser *parser;
  gboolean ret;
  GError *err = NULL;

  parser = json_parser_new();
  ret = json_parser_load_from_data(parser, payload, len, &err);

  if (!ret) {
    g_printerr("Sway window: Failed to parse json: %s", err->message);
    return;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  g_barbar_sway_set_focused_tree(sway, reader);

  g_object_unref(reader);
}

static void event_listner(BarBarSwaySubscribe *sub, guint type,
                          const char *payload, guint length, gpointer data) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);
  g_barbar_sway_handle_window_change(payload, length, type, sway);
}

static void tree_cb(GObject *object, GAsyncResult *res, gpointer data) {
  GInputStream *stream = G_INPUT_STREAM(object);
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(data);
  GError *error = NULL;
  char *str = NULL;
  gsize len;

  gboolean ret =
      g_barbar_sway_ipc_read_finish(stream, res, NULL, &str, &len, &error);

  if (error) {
    g_printerr("Failed to get workspaces: %s\n", error->message);
    return;
  }
  if (ret) {
    g_barbar_sway_initial_window(sway, str, len);

    g_signal_connect(sway->sub, "event", G_CALLBACK(event_listner), sway);
    g_barbar_sway_subscribe_connect(sway->sub, &error);
  }

  g_free(str);
}

static void g_barbar_sway_window_start(GtkWidget *widget) {
  BarBarSwayWindow *sway = BARBAR_SWAY_WINDOW(widget);
  GInputStream *input_stream;
  GOutputStream *output_stream;
  GError *error = NULL;

  GTK_WIDGET_CLASS(g_barbar_sway_window_parent_class)->root(widget);

  sway->ipc = g_barbar_sway_ipc_connect(&error);
  if (error != NULL) {
    g_printerr("Sway window: Couldn't connect to the sway ipc %s",
               error->message);
    return;
  }

  sway->sub =
      BARBAR_SWAY_SUBSCRIBE(g_barbar_sway_subscribe_new("[\"window\"]"));

  output_stream = g_io_stream_get_output_stream(G_IO_STREAM(sway->ipc));

  g_barbar_sway_ipc_send(output_stream, SWAY_GET_TREE, "", &error);

  if (error != NULL) {
    g_printerr("Sway window: Couldn't send the tree command: %s\n",
               error->message);
    return;
  }

  input_stream = g_io_stream_get_input_stream(G_IO_STREAM(sway->ipc));

  g_barbar_sway_ipc_read_async(input_stream, NULL, tree_cb, sway);

  // const char *intrest = "[\"window\"]";

  // gdk_display = gdk_display_get_default();
  //
  // GtkNative *native = gtk_widget_get_native(GTK_WIDGET(sway));
  // GdkSurface *surface = gtk_native_get_surface(native);
  // monitor = gdk_display_get_monitor_at_surface(gdk_display, surface);
  // sway->output = gdk_wayland_monitor_get_wl_output(monitor);
  //
  // // TODO: We need to get the output->name, we can't really do that atm
  // // because gtk4 binds to the wl_output interface version 3, which doesn't
  // // support this. This will change in future. For now the user needs to
  // specify
  // // the output. This will be fixed in the future.
  //
  // ipc = g_barbar_sway_ipc_connect(&error);
  // if (error != NULL) {
  //   g_printerr("Sway window: Couldn't connect to the sway ipc %s",
  //              error->message);
  //   return;
  // }
  //
  // g_barbar_sway_ipc_send(ipc, SWAY_GET_TREE, "");
  // len = g_barbar_sway_ipc_read(ipc, &buf, NULL);
  // if (len > 0) {
  //   g_barbar_sway_initial_window(sway, buf, len);
  //   g_free(buf);
  // }

  // g_barbar_sway_ipc_subscribe(ipc, intrest, sway,
  //                             g_barbar_sway_handle_window_change);

  // g_barbar_sway_ipc_close(ipc);
}
