#include "barbar-dwl-layout-ipc.h"
#include "dwl-ipc-unstable-v2-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarDwlLayoutIpc:
 * A widget to display the layout for the associated screen.
 */
struct _BarBarDwlLayoutIpc {
  GtkWidget parent_instance;

  struct zdwl_ipc_manager_v2 *ipc_manager;
  struct zdwl_ipc_output_v2 *ipc_output;
  struct wl_output *output;

  GtkWidget *button;
  uint32_t layout_idx;
  uint32_t max_layout;
};

enum {
  PROP_0,

  PROP_MAXLAYOUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarDwlLayoutIpc, g_barbar_dwl_layout_ipc, GTK_TYPE_WIDGET)

static GParamSpec *dwl_layout_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_layout_ipc_root(GtkWidget *widget);

static void g_barbar_dwl_layout_set_maxlayout(BarBarDwlLayoutIpc *dwl,
                                              guint max_layout) {
  g_return_if_fail(BARBAR_IS_DWL_LAYOUT_IPC(dwl));
  printf("aaeoeeoau\n");

  if (dwl->max_layout == max_layout) {
    return;
  }
  if (max_layout == 0) {
    max_layout = 1;
  }

  dwl->max_layout = max_layout;

  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_layout_props[PROP_MAXLAYOUT]);
}

static void g_barbar_dwl_layout_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(object);
  switch (property_id) {
  case PROP_MAXLAYOUT:
    g_barbar_dwl_layout_set_maxlayout(dwl, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_layout_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(object);

  switch (property_id) {
  case PROP_MAXLAYOUT:
    g_value_set_uint(value, dwl->max_layout);
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_layout_finalize(GObject *object) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(object);

  // g_free(dwl->output_name);
  zdwl_ipc_output_v2_destroy(dwl->ipc_output);
  zdwl_ipc_manager_v2_destroy(dwl->ipc_manager);

  G_OBJECT_CLASS(g_barbar_dwl_layout_ipc_parent_class)->finalize(object);
}

static void g_barbar_dwl_layout_ipc_class_init(BarBarDwlLayoutIpcClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_layout_set_property;
  gobject_class->get_property = g_barbar_dwl_layout_get_property;
  gobject_class->finalize = g_barbar_dwl_layout_finalize;

  widget_class->root = g_barbar_dwl_layout_ipc_root;

  /**
   * BarBarDwlLayoutIpc:max-layout:
   *
   * How many layouts we should be able to toggle between before wrapping
   */
  dwl_layout_props[PROP_MAXLAYOUT] = g_param_spec_uint(
      "max-layout", NULL, NULL, 0, 100, 3,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "dwl-layout");
}
static void clicked(GtkButton *self, gpointer data) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(data);

  dwl->layout_idx = (dwl->layout_idx + 1) % dwl->max_layout;
  zdwl_ipc_output_v2_set_layout(dwl->ipc_output, dwl->layout_idx + 1);
}

static void g_barbar_dwl_layout_ipc_init(BarBarDwlLayoutIpc *self) {
  self->max_layout = 3;
  self->button = gtk_button_new_with_label("");
  g_signal_connect(self->button, "clicked", G_CALLBACK(clicked), self);
  gtk_widget_set_parent(self->button, GTK_WIDGET(self));
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(data);

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
                   uint32_t active) {}

static void tag(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                uint32_t tag, uint32_t state, uint32_t clients,
                uint32_t focused) {}

static void layout(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                   uint32_t layout) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(data);
  dwl->layout_idx = layout;
}

static void title(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                  const char *title) {}

static void appid(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                  const char *appid) {}

static void layout_symbol(void *data,
                          struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                          const char *layout) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(data);
  gtk_button_set_label(GTK_BUTTON(dwl->button), layout);
}

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

static void g_barbar_dwl_layout_ipc_root(GtkWidget *widget) {
  BarBarDwlLayoutIpc *dwl = BARBAR_DWL_LAYOUT_IPC(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_dwl_layout_ipc_parent_class)->root(widget);

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
