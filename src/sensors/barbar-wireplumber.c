#include "barbar-wireplumber.h"
#include "wp/core.h"
#include "wp/log.h"
#include <stdio.h>

/**
 * BarBarWireplumber:
 *
 * A wireplumber sensor to read and set volume
 *
 */

struct _BarBarWireplumber {
  BarBarSensor parent_instance;

  char *name_id;
  double volume;
  gboolean mute;

  gint exit_code;
  GPtrArray *apis;

  WpCore *core;
  WpObjectManager *om;
  guint pending_plugins;

  guint32 id;
  WpPlugin *mixer_api;
  WpPlugin *def_nodes_api;
};

enum {
  PROP_0,

  PROP_ID,
  PROP_PERCENT,
  // PROP_MUTED,

  NUM_PROPERTIES,
};

static GParamSpec *wireplumber_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarWireplumber, g_barbar_wireplumber, BARBAR_TYPE_SENSOR)

void g_barbar_wireplumber_start(BarBarSensor *sensor);
void g_barbar_wireplumber_set_device(BarBarWireplumber *self,
                                     const char *device) {
  g_return_if_fail(BARBAR_IS_WIREPLUMBER(self));

  if (device && self->name_id) {
    if (!strcmp(self->name_id, device)) {
      return;
    }
  }
  g_free(self->name_id);

  self->name_id = strdup(device);

  g_object_notify_by_pspec(G_OBJECT(self), wireplumber_props[PROP_ID]);
}

void g_barbar_wireplumber_set_value(BarBarWireplumber *self, double volume) {
  g_return_if_fail(BARBAR_IS_WIREPLUMBER(self));
  gboolean ret;
  GVariant *variant;

  if (self->volume == volume) {
    return;
  }

  variant = g_variant_new_double(volume);
  g_signal_emit_by_name(self->mixer_api, "set-volume", self->id, variant, &ret);
  if (ret) {
    self->volume = volume;
    g_object_notify_by_pspec(G_OBJECT(self), wireplumber_props[PROP_PERCENT]);
  }
}

static void g_barbar_wireplumber_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarWireplumber *wireplumber = BARBAR_WIREPLUMBER(object);

  switch (property_id) {
  case PROP_ID:
    g_barbar_wireplumber_set_device(wireplumber, g_value_get_string(value));
    break;
  case PROP_PERCENT:
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
  case PROP_ID:
    g_value_set_string(value, wireplumber->name_id);
    break;
  case PROP_PERCENT:
    g_value_set_double(value, wireplumber->volume);
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
  wireplumber_props[PROP_ID] = g_param_spec_string(
      "name-id", NULL, NULL, "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  wireplumber_props[PROP_PERCENT] = g_param_spec_double(
      "percent", NULL, NULL, 0, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    wireplumber_props);
}

static void g_barbar_wireplumber_init(BarBarWireplumber *self) {}

static void on_plugin_loaded(WpCore *core, GAsyncResult *res, gpointer *data) {
  BarBarWireplumber *wp = BARBAR_WIREPLUMBER(data);
  GError *error = NULL;

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

static void get_volume_internal(BarBarWireplumber *self,
                                WpPipewireObject *proxy) {
  GVariant *variant = NULL;
  guint32 id = wp_proxy_get_bound_id(WP_PROXY(proxy));

  g_signal_emit_by_name(self->mixer_api, "get-volume", id, &variant);
  if (!variant) {
    fprintf(stderr, "Node %d does not support volume\n", id);
    return;
  }
  g_variant_lookup(variant, "volume", "d", &self->volume);
  g_variant_lookup(variant, "mute", "b", &self->mute);

  g_clear_pointer(&variant, g_variant_unref);
  g_object_notify_by_pspec(G_OBJECT(self), wireplumber_props[PROP_PERCENT]);
  // g_object_notify_by_pspec(G_OBJECT(self), wireplumber_props[PROP_MUTED]);
}

static gboolean valid_id(gint id) { return id > 0 || id < G_MAXUINT32; }

static void mixer_changed(BarBarWireplumber *self, guint32 id) {
  g_autoptr(WpPipewireObject) proxy = NULL;
  if (id == self->id) {
    g_autoptr(WpPipewireObject) proxy = NULL;

    proxy = wp_object_manager_lookup(self->om, WP_TYPE_GLOBAL_PROXY,
                                     WP_CONSTRAINT_TYPE_G_PROPERTY, "bound-id",
                                     "=u", self->id, NULL);
    if (!proxy) {
      fprintf(stderr, "Node '%d' not found\n", self->id);
      return;
    }
    get_volume_internal(self, proxy);
  }
}
static void default_node_changed(BarBarWireplumber *self) {
  // TODO:
}

static void manager_installed(BarBarWireplumber *self) {
  g_autoptr(WpPipewireObject) proxy = NULL;

  self->def_nodes_api = wp_plugin_find(self->core, "default-nodes-api");

  self->mixer_api = wp_plugin_find(self->core, "mixer-api");

  if (!self->def_nodes_api) {
    printf("no nodes api\n");
    return;
  }

  g_signal_emit_by_name(self->def_nodes_api, "get-default-node", "Audio/Sink",
                        &self->id);
  g_signal_emit_by_name(self->def_nodes_api, "get-default-configured-node-name",
                        "Audio/Sink", &self->name_id);

  g_object_notify_by_pspec(G_OBJECT(self), wireplumber_props[PROP_ID]);

  if (!valid_id(self->id)) {
    fprintf(stderr, "'%d' is not a valid ID (returned by default-nodes-api)",
            self->id);
    return;
  }
  proxy = wp_object_manager_lookup(self->om, WP_TYPE_GLOBAL_PROXY,
                                   WP_CONSTRAINT_TYPE_G_PROPERTY, "bound-id",
                                   "=u", self->id, NULL);
  if (!proxy) {
    fprintf(stderr, "Node '%d' not found\n", self->id);
    return;
  }

  get_volume_internal(self, proxy);

  g_signal_connect_swapped(self->mixer_api, "changed",
                           G_CALLBACK(mixer_changed), self);
  g_signal_connect_swapped(self->def_nodes_api, "changed",
                           G_CALLBACK(default_node_changed), self);
}

// static void on_plugin_activated(WpObject *p, GAsyncResult *res,
//                                 BarBarWireplumber *wp) {
//   g_autoptr(GError) error = NULL;
//
//   if (!wp_object_activate_finish(p, res, &error)) {
//     fprintf(stderr, "%s", error->message);
//     return;
//   }
//
//   if (--wp->pending_plugins == 0) {
//     wp_core_install_object_manager(wp->core, wp->om);
//   }
// }

void g_barbar_wireplumber_start(BarBarSensor *sensor) {
  BarBarWireplumber *wp = BARBAR_WIREPLUMBER(sensor);
  wp_init(WP_INIT_PIPEWIRE);
  wp->core = wp_core_new(NULL, NULL, NULL);
  wp->om = wp_object_manager_new();

  wp->apis = g_ptr_array_new_with_free_func(g_object_unref);

  wp_object_manager_add_interest(wp->om, WP_TYPE_NODE,
                                 WP_CONSTRAINT_TYPE_PW_PROPERTY, "media.class",
                                 "=s", "Audio/Sink", NULL);

  wp->pending_plugins++;
  wp_core_load_component(wp->core, "libwireplumber-module-default-nodes-api",
                         "module", NULL, NULL, NULL,
                         (GAsyncReadyCallback)on_plugin_loaded, wp);

  wp->pending_plugins++;
  wp_core_load_component(wp->core, "libwireplumber-module-mixer-api", "module",
                         NULL, NULL, NULL,
                         (GAsyncReadyCallback)on_plugin_loaded, wp);

  /* connect */
  if (!wp_core_connect(wp->core)) {
    fprintf(stderr, "Could not connect to PipeWire\n");
    return;
  }

  g_signal_connect_swapped(wp->om, "installed", G_CALLBACK(manager_installed),
                           wp);

  // g_signal_connect_swapped(wp->core, "disconnected",
  //                          (GCallback)g_main_loop_quit, ctl.loop);
}
