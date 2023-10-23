#include "barbar-network.h"
#include <stdio.h>

struct _BarBarNetwork {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  char *interface;
  int family;
};

enum {
  PROP_0,

  // PROP_STATES,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarNetwork, g_barbar_network, G_TYPE_OBJECT)

static GParamSpec *network_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_network_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {}

static void g_barbar_network_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  BarBarNetwork *network = BARBAR_NETWORK(object);

  switch (property_id) {
    //  case PROP_STATES:
    // g_value_get_string(value);
    //    // g_barbar_disk_set_path(disk, g_value_get_string(value));
    //    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_network_class_init(BarBarNetworkClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_network_set_property;
  gobject_class->get_property = g_barbar_network_get_property;
  // network_props[PROP_STATES] = g_param_spec_double("critical-temp", NULL, NULL, 0.0, 300.0, 80.0,
  //                            G_PARAM_CONSTRUCT);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    network_props);
}

static void g_barbar_network_init(BarBarNetwork *self) {}

void g_barbar_network_update(BarBarNetwork *self) {}
