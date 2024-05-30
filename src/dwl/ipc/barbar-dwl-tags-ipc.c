#include "barbar-dwl-tags-ipc.h"
#include "dwl-ipc-unstable-v2-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarDwlTagsIpc:
 * A widget to display the title for current application for the
 * associated screen.
 */
struct _BarBarDwlTagsIpc {
  GtkWidget parent_instance;

  struct zdwl_ipc_manager_v2 *ipc_manager;
  struct zdwl_ipc_output_v2 *ipc_output;
  char *output_name;
  uint nums;
  struct wl_output *output;
  gboolean active;

  GtkWidget *buttons[32];
};

enum {
  PROP_0,

  PROP_OUTPUT,
  PROP_TAGNUMS,

  NUM_PROPERTIES,
};

static GtkBuildableIface *parent_buildable_iface;

static void
g_barbar_dwl_tags_ipc_buildable_interface_init(GtkBuildableIface *iface);
static void g_barbar_dwl_tags_add_button(BarBarDwlTagsIpc *self,
                                         GtkWidget *child);
static void g_barbar_dwl_tag_root(GtkWidget *widget);
static void default_clicked_handler(BarBarDwlTagsIpc *dwl, guint tag,
                                    gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(
    BarBarDwlTagsIpc, g_barbar_dwl_tags_ipc, GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                          g_barbar_dwl_tags_ipc_buildable_interface_init))

static GParamSpec *dwl_tags_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_tags_add_child(GtkBuildable *buildable,
                                        GtkBuilder *builder, GObject *child,
                                        const char *type) {
  g_return_if_fail(GTK_IS_WIDGET(child));

  BarBarDwlTagsIpc *self = BARBAR_DWL_TAGS_IPC(buildable);

  if (g_strcmp0(type, "tag") == 0) {
    g_barbar_dwl_tags_add_button(self, GTK_WIDGET(child));
  } else {
    parent_buildable_iface->add_child(buildable, builder, child, type);
  }
}

static void
g_barbar_dwl_tags_ipc_buildable_interface_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_dwl_tags_add_child;
}

static void g_barbar_dwl_tags_set_output(BarBarDwlTagsIpc *dwl,
                                         const gchar *output) {
  g_return_if_fail(BARBAR_IS_DWL_TAGS_IPC(dwl));

  g_free(dwl->output_name);

  dwl->output_name = g_strdup(output);
  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_tags_props[PROP_OUTPUT]);
}

static void g_barbar_dwl_tags_set_tagnums(BarBarDwlTagsIpc *dwl,
                                          guint tagnums) {
  g_return_if_fail(BARBAR_IS_DWL_TAGS_IPC(dwl));

  if (dwl->nums == tagnums) {
    return;
  }

  dwl->nums = tagnums;

  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_tags_props[PROP_TAGNUMS]);
}

static void g_barbar_dwl_tag_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(object);
  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_dwl_tags_set_output(dwl, g_value_get_string(value));
    break;
  case PROP_TAGNUMS:
    g_barbar_dwl_tags_set_tagnums(dwl, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_tag_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(object);

  switch (property_id) {
  case PROP_TAGNUMS:
    g_value_set_uint(value, dwl->nums);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static guint click_signal;

static void g_barbar_dwl_tags_ipc_class_init(BarBarDwlTagsIpcClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_tag_set_property;
  gobject_class->get_property = g_barbar_dwl_tag_get_property;

  widget_class->root = g_barbar_dwl_tag_root;

  /**
   * BarBarDwlTagsIpc:output:
   *
   * What screen we want this be connected to.
   * This is because gtk4 not having support for
   * wl_output interface v4
   */
  dwl_tags_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, "WL-1", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarDwlTagsIpc:tagnums:
   *
   * How many tags we should show
   */
  dwl_tags_props[PROP_TAGNUMS] = g_param_spec_uint(
      "tagnums", NULL, NULL, 0, 32, 9,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  click_signal =
      g_signal_new_class_handler("clicked", BARBAR_TYPE_DWL_TAGS_IPC,
                                 G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                                 G_CALLBACK(default_clicked_handler), NULL,
                                 NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dwl_tags_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "dwl-tag");
}

static void clicked(GtkButton *self, gpointer user_data) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(user_data);

  guint tag = 0;
  for (int i = 0; i < dwl->nums; i++) {
    if (self == GTK_BUTTON(dwl->buttons[i])) {
      tag = i;
      break;
    }
  }

  g_signal_emit(dwl, click_signal, 0, tag);
}

static void default_clicked_handler(BarBarDwlTagsIpc *dwl, guint tag,
                                    gpointer user_data) {

  zdwl_ipc_output_v2_set_tags(dwl->ipc_output, 1 << tag, 1);
}

static void handle_occupied(BarBarDwlTagsIpc *dwl, uint32_t occupied) {
  for (size_t id = 0; id < dwl->nums; ++id) {
    uint32_t mask = 1 << id;

    if (mask & occupied) {
      gtk_widget_add_css_class(dwl->buttons[id], "occupied");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "occupied");
    }
  }
}
static void handle_selected(BarBarDwlTagsIpc *dwl, uint32_t selected) {
  for (size_t id = 0; id < dwl->nums; ++id) {
    uint32_t mask = 1 << id;

    if (mask & selected) {
      gtk_widget_add_css_class(dwl->buttons[id], "focused");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "focused");
    }
  }
}

static void handle_urgent(BarBarDwlTagsIpc *dwl, uint32_t urgent) {
  for (size_t id = 0; id < dwl->nums; ++id) {
    uint32_t mask = 1 << id;

    if (mask & urgent) {
      gtk_widget_add_css_class(dwl->buttons[id], "urgent");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "urgent");
    }
  }
}

static void g_barbar_dwl_tags_add_button(BarBarDwlTagsIpc *self,
                                         GtkWidget *child) {
  GtkWidget *btn;

  uint32_t i = 0;
  for (; i < 32; i++) {
    if (self->buttons[i]) {
      continue;
    }
    btn = gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(btn), child);
    gtk_widget_set_parent(btn, GTK_WIDGET(self));
    g_signal_connect(btn, "clicked", G_CALLBACK(clicked), self);
    self->buttons[i] = btn;
    break;
  }
  i++;
  if (self->nums < i) {
    self->nums = i;
    g_object_notify_by_pspec(G_OBJECT(self), dwl_tags_props[PROP_TAGNUMS]);
  }
}

static void g_barbar_dwl_tags_ipc_init(BarBarDwlTagsIpc *self) {}

static void g_barbar_dwl_tags_defaults(BarBarDwlTagsIpc *self) {
  GtkWidget *btn;
  char str[2];
  for (uint32_t i = 0; i < self->nums; i++) {
    if (self->buttons[i]) {
      continue;
    }
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label(str);
    gtk_widget_set_parent(btn, GTK_WIDGET(self));
    g_signal_connect(btn, "clicked", G_CALLBACK(clicked), self);
    self->buttons[i] = btn;
  }
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(data);

  if (strcmp(interface, zdwl_ipc_manager_v2_interface.name) == 0) {
    dwl->ipc_manager = wl_registry_bind(
        registry, name, &zdwl_ipc_manager_v2_interface, version);
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

static void toggle_visibility(void *data,
                              struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2) {}

static void active(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                   uint32_t active) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(data);
  dwl->active = active;
}

static void tag(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                uint32_t tag, uint32_t state, uint32_t clients,
                uint32_t focused) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(data);

  if (state & ZDWL_IPC_OUTPUT_V2_TAG_STATE_ACTIVE) {
    gtk_widget_add_css_class(dwl->buttons[tag], "focused");
  } else {
    gtk_widget_add_css_class(dwl->buttons[tag], "unfocused");
  }
  if (clients > 0) {
    gtk_widget_add_css_class(dwl->buttons[tag], "occupied");
  } else {
    gtk_widget_remove_css_class(dwl->buttons[tag], "occupied");
  }
  if (state & ZDWL_IPC_OUTPUT_V2_TAG_STATE_URGENT) {
    gtk_widget_add_css_class(dwl->buttons[tag], "urgent");
  } else {
    gtk_widget_remove_css_class(dwl->buttons[tag], "urgent");
  }
}

static void layout(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                   uint32_t layout) {}

static void title(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                  const char *title) {}

static void appid(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                  const char *appid) {}

static void layout_symbol(void *data,
                          struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                          const char *layout) {}

static void frame(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2) {}

static void fullscreen(void *data,
                       struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                       uint32_t is_fullscreen) {}

static void floating(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                     uint32_t is_floating) {}

static const struct zdwl_ipc_output_v2_listener ipc_output_listener = {
    .toggle_visibility = toggle_visibility,
    .active = active,
    .tag = tag,
    .layout = layout,
    .title = title,
    .appid = appid,
    .layout_symbol = layout_symbol,
    .frame = frame,
    .fullscreen = fullscreen,
    .floating = floating,
};

static void g_barbar_dwl_tag_root(GtkWidget *widget) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_dwl_tags_ipc_parent_class)->root(widget);
  g_barbar_dwl_tags_defaults(dwl);

  gdk_display = gdk_display_get_default();

  // This shouldn't need to be done because layered shell requires wayland
  g_return_if_fail(gdk_display);
  g_return_if_fail(GDK_IS_WAYLAND_DISPLAY(gdk_display));

  // We try to find the main screen for this widget, this should only
  // be done if no screen is specified
  GtkWindow *window =
      GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(dwl), GTK_TYPE_WINDOW));
  if (window == NULL || !gtk_layer_is_layer_window(window)) {
    // print an error
    printf("Parent window not found!\n");
    return;
  }
  monitor = gtk_layer_get_monitor(window);
  dwl->output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, dwl);
  wl_display_roundtrip(wl_display);

  dwl->ipc_output =
      zdwl_ipc_manager_v2_get_output(dwl->ipc_manager, dwl->output);

  zdwl_ipc_output_v2_add_listener(dwl->ipc_output, &ipc_output_listener, dwl);

  wl_display_roundtrip(wl_display);
}
