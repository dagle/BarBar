#include "barbar-river.h"
#include "river-status-unstable-v1-client-protocol.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarRiver {
  GObject parent;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarRiver, g_barbar_river, G_TYPE_OBJECT)

static GParamSpec *river_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_river_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {}

static void g_barbar_river_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {}

static guint click_signal;

static void g_barbar_river_class_init(BarBarRiverClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_river_set_property;
  gobject_class->get_property = g_barbar_river_get_property;
  river_props[PROP_DEVICE] =
      g_param_spec_string("path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, river_props);

  /* TODO: */
  click_signal = g_signal_new(
      "click-signal", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      0 /* class offset.Subclass cannot override the class handler (default
           handler). */
      ,
      NULL /* accumulator */, NULL /* accumulator data */,
      NULL /* C marshaller. g_cclosure_marshal_generic() will be used */,
      G_TYPE_NONE /* return_type */, 0 /* n_params */
  );
}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarRiver *river = BARBAR_RIVER(data);
  if (strcmp(interface, zriver_status_manager_v1_interface.name) == 0) {
    // layout_manager =
    //     wl_registry_bind(registry, name, &river_layout_manager_v3_interface,
    //     2);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct wl_output *wl_output =
        wl_registry_bind(registry, name, &wl_output_interface, 4);

    // GriverOutput *output = create_output(ctx, wl_output, name);
    // g_signal_emit(ctx, griver_signals[GRIVER_ADD_OUTPUT], 0, output);
  }
}

static void registry_handle_global_remove(void *data,
                                          struct wl_registry *registry,
                                          uint32_t name) {
  BarBarRiver *river = BARBAR_RIVER(data);
  // GriverContext *ctx = GRIVER_CONTEXT(data);
  // GriverContextPrivate *priv = g_river_context_get_instance_private(ctx);
  // GriverOutput *output = output_from_global_name(priv, name);
  // if (output != NULL) {
  //   g_signal_emit(ctx, griver_signals[GRIVER_REMOVE_OUTPUT], 0, output);
  //   priv->outputs = g_list_remove(priv->outputs, output);
  //   g_object_unref(output);
  // }
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void g_barbar_river_init(BarBarRiver *self) {
  GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(self));
  GtkWindow *window = GTK_WINDOW(root);
  GdkMonitor *monitor = gtk_layer_get_monitor(window);
  struct wl_output *output = gdk_wayland_monitor_get_wl_output(monitor);

  // struct wl_display *wl_display = wl_display_connect(NULL);
  // struct wl_registry *wl_registry = NULL;
  //
  // wl_registry = wl_display_get_registry(wl_display);
  // wl_registry_add_listener(wl_registry, &registry_listener, self);
  // wl_display_roundtrip(wl_display);
  //
  // if (wl_display_roundtrip(wl_display) < 0) {
  //   g_printerr("initial roundtrip failed\n");
  //   return false;
  // }
}

void g_barbar_river_update(BarBarRiver *river) {}
