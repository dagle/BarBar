#include "barbar-wireplumber.h"
#include "wp/core.h"
#include <stdio.h>

struct _BarBarWireplumber {
  BarBarSensor parent_instance;

  char *path;
  double value;

  gint exit_code;

  WpCore *core;
  WpObjectManager *om;
  guint pending_plugins;
};

enum {
  PROP_0,

  PROP_DEVICE,
  PROP_VALUE,

  NUM_PROPERTIES,
};

static GParamSpec *wireplumber_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarWireplumber, g_barbar_wireplumber, BARBAR_TYPE_SENSOR)

void g_barbar_wireplumber_set_device(BarBarWireplumber *self,
                                     const char *device) {
  g_return_if_fail(BARBAR_IS_WIREPLUMBER(self));

  if (device && self->path) {
    if (!strcmp(self->path, device)) {
      return;
    }
  }
  g_free(self->path);

  self->path = strdup(device);

  g_object_notify_by_pspec(G_OBJECT(self), wireplumber_props[PROP_DEVICE]);
}

void g_barbar_wireplumber_set_value(BarBarWireplumber *self, double value) {
  g_return_if_fail(BARBAR_IS_WIREPLUMBER(self));

  if (self->value == value) {
    return;
  }

  self->value = value;

  g_object_notify_by_pspec(G_OBJECT(self), wireplumber_props[PROP_VALUE]);
}

static void g_barbar_wireplumber_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarWireplumber *wireplumber = BARBAR_WIREPLUMBER(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_wireplumber_set_device(wireplumber, g_value_get_string(value));
    break;
  case PROP_VALUE:
    g_barbar_wireplumber_set_value(wireplumber, g_value_get_double(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_wireplumber_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {
  BarBarWireplumber *wireplumber = BARBAR_WIREPLUMBER(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_value_set_string(value, wireplumber->path);
    break;
  case PROP_VALUE:
    g_value_set_double(value, wireplumber->value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_wireplumber_class_init(BarBarWireplumberClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_wireplumber_start;
  gobject_class->set_property = g_barbar_wireplumber_set_property;
  gobject_class->get_property = g_barbar_wireplumber_get_property;
  wireplumber_props[PROP_DEVICE] = g_param_spec_string(
      "device", NULL, NULL, "/", G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  wireplumber_props[PROP_VALUE] = g_param_spec_double(
      "value", NULL, NULL, 0, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    wireplumber_props);
}

static void g_barbar_wireplumber_init(BarBarWireplumber *self) {}

// static gboolean g_barbar_wireplumber_update(gpointer data) {
//   BarBarWireplumber *wp = BARBAR_WIREPLUMBER(data);
//   printf("value: %f\n", wp->value);
//   return TRUE;
// }

// g_timeout_add_full(0, 1000, g_barbar_wireplumber_update, wp, NULL);

// static void update_volume(BarBarWireplumber *wp) {}

// static void manager_installed(WpObjectManager *om, gpointer data) {
//   BarBarWireplumber *wp = BARBAR_WIREPLUMBER(data);
//   WpPlugin *default_nodes = wp_plugin_find(wp->core, "default-nodes-api");
//
//   if (!default_nodes) {
//
//     return;
//   }
//
//   WpPlugin *mixer = wp_plugin_find(wp->core, "mixer-api");
//
//   if (!default_nodes) {
//
//     return;
//   }
//
//   char *node_name;
//   guint id;
//   g_signal_emit_by_name(default_nodes, "get-default-configured-node-name",
//                         "Audio/Sink", &node_name);
//   g_signal_emit_by_name(default_nodes, "get-default-node", "Audio/Sink",
//   &id);
//
//   update_volume(wp);
//
//   // g_object_unref(nodes);
//   // g_object_unref(mixer);
// }

static void plugin_activated(GObject *object, GAsyncResult *res,
                             gpointer data) {}

static void on_plugin_loaded(WpCore *core, GAsyncResult *res, gpointer *data) {
  BarBarWireplumber *wp = BARBAR_WIREPLUMBER(data);
  WpPlugin *plugin;
  GError *error;

  if (!wp_core_load_component_finish(core, res, &error)) {
    g_printerr("Wireplumber: failed to load compenent: %s\n", error->message);
    wp->exit_code = 1;
    return;
  }

  if (--wp->pending_plugins == 0) {
    g_autoptr(WpPlugin) mixer_api = wp_plugin_find(core, "mixer-api");
    g_object_set(mixer_api, "scale", 1 /* cubic */, NULL);
    wp_core_install_object_manager(wp->core, wp->om);
  }
}

static void do_print_volume(BarBarWireplumber *self, WpPipewireObject *proxy) {
  g_autoptr(WpPlugin) mixer_api = wp_plugin_find(self->core, "mixer-api");
  GVariant *variant = NULL;
  gboolean mute = FALSE;
  gdouble volume = 1.0;
  guint32 id = wp_proxy_get_bound_id(WP_PROXY(proxy));

  g_signal_emit_by_name(mixer_api, "get-volume", id, &variant);
  if (!variant) {
    fprintf(stderr, "Node %d does not support volume\n", id);
    return;
  }
  g_variant_lookup(variant, "volume", "d", &volume);
  g_variant_lookup(variant, "mute", "b", &mute);
  g_clear_pointer(&variant, g_variant_unref);

  printf("Volume: %.2f%s", volume, mute ? " [MUTED]\n" : "\n");
}

static gboolean valid_id(gint id) { return id <= 0 || id >= G_MAXUINT32; }

static void get_volume_run(BarBarWireplumber *self) {
  g_autoptr(WpPlugin) def_nodes_api = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(WpPipewireObject) proxy = NULL;
  guint32 id;

  def_nodes_api = wp_plugin_find(self->core, "default-nodes-api");

  g_signal_emit_by_name(def_nodes_api, "get-default-node", "Audio/Sink", &id);

  if (!valid_id(id)) {
    fprintf(stderr, "'%d' is not a valid ID (returned by default-nodes-api)",
            id);
    return;
  }
  // if (!translate_id(def_nodes_api, cmdline.get_volume.id, &id, &error)) {
  //   fprintf(stderr, "Translate ID error: %s\n\n", error->message);
  //   goto out;
  // }

  proxy = wp_object_manager_lookup(self->om, WP_TYPE_GLOBAL_PROXY,
                                   WP_CONSTRAINT_TYPE_G_PROPERTY, "bound-id",
                                   "=u", id, NULL);
  if (!proxy) {
    fprintf(stderr, "Node '%d' not found\n", id);
    return;
  }

  do_print_volume(self, proxy);
}

void g_barbar_wireplumber_start(BarBarSensor *sensor) {
  BarBarWireplumber *wp = BARBAR_WIREPLUMBER(sensor);
  wp_init(WP_INIT_PIPEWIRE);
  wp->core = wp_core_new(NULL, NULL);
  wp->om = wp_object_manager_new();

  wp_object_manager_add_interest(wp->om, WP_TYPE_NODE,
                                 WP_CONSTRAINT_TYPE_PW_PROPERTY, "media.class",
                                 "=s", "Audio/Sink", NULL);
  // maybe do this in the future
  // wp_object_manager_add_interest (wp->om, WP_TYPE_CLIENT, NULL);
  // wp_object_manager_request_object_features (self->om, WP_TYPE_GLOBAL_PROXY,
  //     WP_PIPEWIRE_OBJECT_FEATURES_MINIMAL);

  wp.pending_plugins++;
  wp_core_load_component(wp->core, "libwireplumber-module-default-nodes-api",
                         "module", NULL, NULL, NULL, on_plugin_loaded, &wp);

  wp.pending_plugins++;
  wp_core_load_component(wp->core, "libwireplumber-module-mixer-api", "module",
                         NULL, NULL, NULL, on_plugin_loaded, &wp);

  /* connect */
  if (!wp_core_connect(wp->core)) {
    fprintf(stderr, "Could not connect to PipeWire\n");
    return;
  }

  g_signal_connect_swapped(wp->om, "installed", G_CALLBACK(get_volume_run), wp);
}
