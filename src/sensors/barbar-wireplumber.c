#include "barbar-wireplumber.h"
#include <stdio.h>

struct _BarBarWireplumber {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  char *path;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

static GParamSpec *wireplumber_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarWireplumber, g_barbar_wireplumber, G_TYPE_OBJECT)

void g_barbar_wireplumber_set_path(BarBarWireplumber *bar, const char *path);

static void g_barbar_wireplumber_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarWireplumber *wireplumber = BARBAR_WIREPLUMBER(object);

  // switch (property_id) {
  // case PROP_PATH:
  //   g_barbar_wireplumber_set_path(wireplumber, g_value_get_string(value));
  //   break;
  // default:
  //   G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  // }
}

static void g_barbar_wireplumber_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {
  BarBarWireplumber *wireplumber = BARBAR_WIREPLUMBER(object);

  // switch (property_id) {
  // case PROP_PATH:
  //   g_value_set_string(value, wireplumber->path);
  //   break;
  // default:
  //   G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  // }
}

// void g_barbar_wireplumber_set_path(BarBarWireplumber *bar, const char *path)
// {
//   g_return_if_fail(BARBAR_IS_WIREPLUMBER(bar));
//
//   g_free(bar->path);
//   bar->path = g_strdup(path);
//
//   g_object_notify_by_pspec(G_OBJECT(bar), wireplumber_props[PROP_PATH]);
// }

static void g_barbar_wireplumber_class_init(BarBarWireplumberClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_wireplumber_set_property;
  gobject_class->get_property = g_barbar_wireplumber_get_property;
  wireplumber_props[PROP_DEVICE] =
      g_param_spec_string("path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    wireplumber_props);
}

static void g_barbar_wireplumber_init(BarBarWireplumber *self) {}

void g_barbar_wireplumber_update(BarBarWireplumber *wireplumber) {
  wp_init(WP_INIT_PIPEWIRE);
  WpCore *core = wp_core_new(NULL, NULL);
  WpObjectManager *om = wp_object_manager_new();
  GError *error;

  wp_object_manager_add_interest(om, WP_TYPE_NODE,
                                 WP_CONSTRAINT_TYPE_PW_PROPERTY, "media.class",
                                 "=s", "Audio/Sink", NULL);

  if (!wp_core_load_component(core, "libwireplumber-module-default-nodes-api",
                              "module", NULL, &error)) {
	printf("NULL1\n");
    // throw std::runtime_error(error->message);
  }

  if (!wp_core_load_component(core, "libwireplumber-module-mixer-api", "module",
                              NULL, &error)) {
	printf("NULL2\n");
    // throw std::runtime_error(error->message);
  }
  WpPlugin *pl = wp_plugin_find(core, "default-nodes-api");
  if (!pl) {
	  printf("NULL3\n");
  }

  g_signal_connect_swapped(om, "installed", G_CALLBACK(NULL), NULL);
  printf("Success\n");
  //
  // g_ptr_array_add(apis_, wp_plugin_find(wp_core_, "default-nodes-api"));
  // g_ptr_array_add(apis_, ({
  //                   WpPlugin* p = wp_plugin_find(wp_core_, "mixer-api");
  //                   g_object_set(G_OBJECT(p), "scale", 1 /* cubic */, NULL);
  //                   p;
  //                 }));
}
