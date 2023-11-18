#include "dwl/barbar-dwl-tags.h"
#include "dwl-ipc-unstable-v2-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarDwlTag {
  GtkWidget parent_instance;

  struct zdwl_ipc_manager_v2 *ipc_manager;
  struct zdwl_ipc_output_v2 *ipc_output;

  struct wl_seat *seat;
  gboolean active;

  GtkWidget *buttons[9];
};

enum {
  PROP_0,

  PROP_TAGNUMS,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarDwlTag, g_barbar_dwl_tag, GTK_TYPE_WIDGET)

static GParamSpec *river_tags_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_tag_constructed(GObject *object);
static void default_clicked_handler(BarBarDwlTag *dwl, guint tag,
                                    gpointer user_data);

static void g_barbar_dwl_tag_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {}

static void g_barbar_dwl_tag_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {}

static guint click_signal;

static void g_barbar_dwl_tag_class_init(BarBarDwlTagClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_tag_set_property;
  gobject_class->get_property = g_barbar_dwl_tag_get_property;
  gobject_class->constructed = g_barbar_dwl_tag_constructed;
  river_tags_props[PROP_TAGNUMS] =
      g_param_spec_uint("tagnums", NULL, NULL, 0, 9, 9, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    river_tags_props);

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "river-tag");
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarDwlTag *dwl = BARBAR_DWL_TAG(data);
  if (strcmp(interface, zdwl_ipc_manager_v2_interface.name) == 0) {
    if (version >= ZDWL_IPC_MANAGER_V2_TAGS_SINCE_VERSION) {
      dwl->ipc_manager = wl_registry_bind(
          registry, name, &zdwl_ipc_manager_v2_interface, version);
    }
  }
  if (strcmp(interface, wl_seat_interface.name) == 0) {
    dwl->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
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

static void clicked(GtkButton *self, gpointer user_data) {
  BarBarDwlTag *dwl = BARBAR_DWL_TAG(user_data);

  guint tag;
  for (int i = 0; i < 9; i++) {
    if (self == GTK_BUTTON(dwl->buttons[i])) {
      tag = i;
    }
  }
  tag = 1 << tag;

  g_signal_emit(dwl, click_signal, 0, tag);
}

static void default_clicked_handler(BarBarDwlTag *dwl, guint tag,
                                    gpointer user_data) {
  struct zriver_command_callback_v1 *callback;
  char buf[4];

  snprintf(buf, 4, "%d", tag);

  // zriver_control_v1_add_argument(river->control, "set-focused-tags");
  // zriver_control_v1_add_argument(river->control, buf);
  // callback = zriver_control_v1_run_command(river->control, river->seat);
  // zriver_command_callback_v1_add_listener(callback,
  // &command_callback_listener,
  //                                         NULL);
}
void printBits(unsigned int num) {
  for (int bit = 0; bit < (sizeof(unsigned int) * 8); bit++) {
    printf("%i ", num & 0x01);
    num = num >> 1;
  }
  printf("\n");
}

static void toggle_visibility(void *data,
                              struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2) {}

static void active(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                   uint32_t active) {}

static void tag(void *data, struct zdwl_ipc_output_v2 *zdwl_ipc_output_v2,
                uint32_t tag, uint32_t state, uint32_t clients,
                uint32_t focused) {
  BarBarDwlTag *dwl = BARBAR_DWL_TAG(data);

  printf("tag: ");
  printBits(tag);

  printf("state: ");
  printBits(state);
  if (state & ZDWL_IPC_OUTPUT_V2_TAG_STATE_ACTIVE) {
  } else if (state & ZDWL_IPC_OUTPUT_V2_TAG_STATE_URGENT) {
  } else {
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

static const struct zdwl_ipc_output_v2_listener zdwl_ipc_output_v2_listener = {
    .toggle_visibility = toggle_visibility,
    .active = active,
    .tag = tag,
    .layout = layout,
    .title = title,
    .appid = appid,
    .layout_symbol = layout_symbol,
    .frame = frame,
};

static void g_barbar_dwl_tag_init(BarBarDwlTag *self) {}
static void g_barbar_dwl_tag_constructed(GObject *object) {
  GtkWidget *btn;
  BarBarDwlTag *dwl = BARBAR_DWL_TAG(object);
  char str[2];
  for (uint32_t i = 0; i < 9; i++) {
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label(str);
    // gtk_widget_set_name(btn, "tags");
    gtk_widget_set_parent(btn, GTK_WIDGET(dwl));
    // g_signal_connect(btn, "clicked", G_CALLBACK(clicked), dwl);
    dwl->buttons[i] = btn;
  }
}

void g_barbar_dwl_tag_start(BarBarDwlTag *dwl) {
  GdkDisplay *gdk_display;
  GdkMonitor *monitor;
  struct wl_registry *wl_registry;
  struct wl_output *output;
  struct wl_display *wl_display;

  gdk_display = gdk_display_get_default();

  // This shouldn't need to be done because layered shell requires wayland
  g_return_if_fail(gdk_display);
  g_return_if_fail(GDK_IS_WAYLAND_DISPLAY(gdk_display));

  // We try to find the main screen for this widget, this should only
  // be done if no screen is specified
  GtkNative *native = gtk_widget_get_native(GTK_WIDGET(dwl));
  GdkSurface *surface = gtk_native_get_surface(native);
  monitor = gdk_display_get_monitor_at_surface(gdk_display, surface);
  output = gdk_wayland_monitor_get_wl_output(monitor);

  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, dwl);
  wl_display_roundtrip(wl_display);

  if (!dwl->ipc_manager) {
    return;
  }

  // output_status_ = zdwl_ipc_manager_v2_get_output(status_manager_, output);
  // zdwl_ipc_output_v2_add_listener(output_status_,
  // &output_status_listener_impl, this);

  dwl->ipc_output = zdwl_ipc_manager_v2_get_output(dwl->ipc_manager, output);
  zdwl_ipc_output_v2_add_listener(dwl->ipc_output, &zdwl_ipc_output_v2_listener,
                                  dwl);
  wl_display_roundtrip(wl_display);

  // dwl->output_status = zriver_status_manager_v1_get_river_output_status(
  //     dwl->status_manager, output);
  //
  // zriver_output_status_v1_add_listener(dwl->output_status,
  //                                      &output_status_listener, dwl);
  // wl_display_roundtrip(wl_display);
  //
  // zriver_status_manager_v1_destroy(dwl->status_manager);
  //
  // dwl->status_manager = NULL;
}
