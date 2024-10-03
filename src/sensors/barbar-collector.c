#include "barbar-collector.h"
#include "glib-object.h"

/**
 * BarBarCollector:
 *
 * A sensor to display the amount of data transfered
 * over an interface
 *
 */
struct _BarBarCollector {
  gboolean dynamic;
};

enum {
  PROP_0,

  PROP_VALUE,
  PROP_COLLECTED,
  PROP_UNIT,
  PROP_DYNAMIC,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarCollector, g_barbar_collector, BARBAR_TYPE_SENSOR)

static GParamSpec *collector_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_collector_add_value(BarBarCollector *object,
                                         const GValue *value) {}

static void g_barbar_collector_set_dynamic(BarBarCollector *collector,
                                           gboolean enable) {
  g_return_if_fail(BARBAR_IS_COLLECTOR(collector));

  if (collector->dynamic == enable)
    return;

  collector->dynamic = enable;

  g_object_notify_by_pspec(G_OBJECT(collector), collector_props[PROP_DYNAMIC]);
}

static void g_barbar_collector_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {

  BarBarCollector *collector = BARBAR_COLLECTOR(object);
  switch (property_id) {
  case PROP_VALUE:
    g_barbar_collector_add_value(collector, value);
    break;
  case PROP_DYNAMIC:
    g_barbar_collector_set_dynamic(collector, g_value_get_boolean(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_collector_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarCollector *collector = BARBAR_COLLECTOR(object);

  switch (property_id) {
  case PROP_VALUE:
    break;
  case PROP_COLLECTED:
    break;
  case PROP_UNIT:
    break;
  case PROP_DYNAMIC:
    g_value_set_boolean(value, collector->dynamic);
    break;
  // case PROP_INTERFACE:
  //   g_value_set_string(value, collector->interface);
  //   break;
  // case PROP_UP_SPEED:
  //   g_value_set_uint64(value, collector->up_speed);
  //   break;
  // case PROP_DOWN_SPEED:
  //   g_value_set_uint64(value, collector->down_speed);
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}
void g_barbar_collector_start(BarBarSensor *sensor) {}

static void g_barbar_collector_class_init(BarBarCollectorClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_collector_start;

  /**
   * BarBarCollector:Value:
   *
   * The last added value to the collection
   */
  collector_props[PROP_VALUE] = g_param_spec_variant(
      "value", "value", "the last added value", G_VARIANT_TYPE_ANY, NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarCollector:Collected:
   *
   * The name of the interface
   */
  collector_props[PROP_COLLECTED] = g_param_spec_variant(
      "collected", "collected", "all values collected so far",
      G_VARIANT_TYPE_ANY, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarCollector:Unit:
   *
   * The name of the interface
   */
  collector_props[PROP_UNIT] = g_param_spec_string(
      "unit", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarCollector:Dynamic:
   *
   * If the unit should be dynamically updated. So instead of saying
   * 12000 it would be 12k.
   */
  collector_props[PROP_DYNAMIC] =
      g_param_spec_boolean("dynamic", "dynamic", "dynamic units", NULL,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    collector_props);
}

static void g_barbar_collector_init(BarBarCollector *self) {}

/**
 * g_barbar_collector_new:
 *
 * Returns: (transfer full): a `BarBarCollector`
 */
BarBarSensor *g_barbar_collector_new(void) {
  BarBarCollector *collector;

  collector = g_object_new(BARBAR_TYPE_COLLECTOR, NULL);

  return BARBAR_SENSOR(collector);
}
