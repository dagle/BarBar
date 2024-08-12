#include "barbar-collector.h"

/**
 * BarBarCollector:
 *
 * A sensor to display the amount of data transfered
 * over an interface
 *
 */
struct _BarBarColllector {
  int i;
};

enum {
  PROP_0,

  PROP_VALUE,
  PROP_COLLECTED,
  PROP_UNIT,
  PROP_DYNAMIC,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarCollector, g_barbar_network, BARBAR_TYPE_SENSOR)

static GParamSpec *network_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_collector_add_value(BarBarCollector *object,
                                         const GValue *value) {}

static void g_barbar_collector_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  BarBarCollector *network = BARBAR_COLLECTOR(object);
  switch (property_id) {
  case PROP_VALUE:
    g_barbar_collector_add_value(, value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_collector_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarCollector *network = BARBAR_COLLECTOR(object);

  switch (property_id) {
  case PROP_INTERFACE:
    g_value_set_string(value, network->interface);
    break;
  case PROP_UP_SPEED:
    g_value_set_uint64(value, network->up_speed);
    break;
  case PROP_DOWN_SPEED:
    g_value_set_uint64(value, network->down_speed);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}
