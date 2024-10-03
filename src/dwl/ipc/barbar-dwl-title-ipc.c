#include "barbar-dwl-title-ipc.h"
#include "dwl-ipc-unstable-v2-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarDwlTitleIpc:
 * A widget to display the title for current application for the
 * associated screen.
 */
struct _BarBarDwlTitleIpc {
  GtkWidget parent_instance;

  struct zdwl_ipc_manager_v2 *ipc_manager;
  struct zdwl_ipc_output_v2 *ipc_output;
  struct wl_output *output;

  GtkWidget *label;
};

enum {
  PROP_0,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarDwlTitleIpc, g_barbar_dwl_title_ipc, GTK_TYPE_WIDGET)

static GParamSpec *dwl_title_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_title_start(GtkWidget *widget);

static void g_barbar_dwl_title_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarDwlTitleIpc *dwl = BARBAR_DWL_TITLE_IPC(object);
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

// zdwl_ipc_output_v2_set_tags
static void g_barbar_dwl_title_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarDwlTitleIpc *dwl = BARBAR_DWL_TITLE_IPC(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_title_finalize(GObject *object) {
  BarBarDwlTitleIpc *dwl = BARBAR_DWL_TITLE_IPC(object);

  zdwl_ipc_output_v2_destroy(dwl->ipc_output);
  zdwl_ipc_manager_v2_destroy(dwl->ipc_manager);

  G_OBJECT_CLASS(g_barbar_dwl_title_ipc_parent_class)->finalize(object);
}

static void g_barbar_dwl_title_ipc_class_init(BarBarDwlTitleIpcClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_title_set_property;
  gobject_class->get_property = g_barbar_dwl_title_get_property;
  gobject_class->finalize = g_barbar_dwl_title_finalize;

  widget_class->root = g_barbar_dwl_title_start;

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "dwl-title");
}

static void g_barbar_dwl_title_ipc_init(BarBarDwlTitleIpc *self) {
  self->label = gtk_label_new("");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarDwlTitleIpc *dwl = BARBAR_DWL_TITLE_IPC(data);

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
                   uint32_t layout) {}

static void title(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                  const char *title) {
  BarBarDwlTitleIpc *dwl = BARBAR_DWL_TITLE_IPC(data);
  gtk_label_set_label(GTK_LABEL(dwl->label), title);
}

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

static void g_barbar_dwl_title_start(GtkWidget *widget) {
  BarBarDwlTitleIpc *dwl = BARBAR_DWL_TITLE_IPC(widget);
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_registry *wl_registry;
  struct wl_display *wl_display;

  GTK_WIDGET_CLASS(g_barbar_dwl_title_ipc_parent_class)->root(widget);

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

/**
 * g_barbar_dwl_title_ipc_new:
 *
 * Returns: (transfer full): a `BarBarDwlTitleIpc`
 */
GtkWidget *g_barbar_dwl_title_ipc_new(void) {
  BarBarDwlTitleIpc *dwl;

  dwl = g_object_new(BARBAR_TYPE_DWL_TITLE_IPC, NULL);

  return GTK_WIDGET(dwl);
}
