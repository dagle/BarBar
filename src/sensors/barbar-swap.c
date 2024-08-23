#include "barbar-swap.h"
#include <glibtop.h>
#include <glibtop/swap.h>
#include <stdio.h>

/**
 * BarBarSwap:
 *
 * A simple swap sensor
 */
struct _BarBarSwap {
  BarBarIntervalSensor parent_instance;

  glibtop_swap swap;
  double percent;
};

enum {
  PROP_SWAP_0,

  PROP_SWAP_PERCENT,

  PROP_SWAP_TOTAL,
  PROP_SWAP_USED,
  PROP_SWAP_FREE,

  PROP_NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

static guint swap_signals[NUM_SIGNALS];

G_DEFINE_TYPE(BarBarSwap, g_barbar_swap, BARBAR_TYPE_INTERVAL_SENSOR)

static gboolean g_barbar_swap_tick(BarBarIntervalSensor *sensor);

static GParamSpec *swap_props[PROP_NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_swap_set_property(GObject *object, guint property_id,
                                       const GValue *value, GParamSpec *pspec) {
  BarBarSwap *swap = BARBAR_SWAP(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_swap_get_property(GObject *object, guint property_id,
                                       GValue *value, GParamSpec *pspec) {
  BarBarSwap *swap = BARBAR_SWAP(object);

  switch (property_id) {
  case PROP_SWAP_PERCENT:
    g_value_set_double(value, swap->percent);
    break;
  case PROP_SWAP_TOTAL:
    g_value_set_uint64(value, swap->swap.total);
    break;
  case PROP_SWAP_FREE:
    g_value_set_uint64(value, swap->swap.free);
    break;
  case PROP_SWAP_USED:
    g_value_set_uint64(value, swap->swap.used);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_swap_class_init(BarBarSwapClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  interval_class->tick = g_barbar_swap_tick;

  gobject_class->set_property = g_barbar_swap_set_property;
  gobject_class->get_property = g_barbar_swap_get_property;

  /**
   * BarBarSwap:percent:
   *
   * How much of the swap is used.
   */
  swap_props[PROP_SWAP_PERCENT] =
      g_param_spec_double("percent", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);
  /**
   * BarBarSwap:total:
   *
   * How much of the swap is used.
   */
  swap_props[PROP_SWAP_TOTAL] = g_param_spec_uint64(
      "total", NULL, NULL, 0, G_MAXUINT64, 0, G_PARAM_READABLE);
  /**
   * BarBarSwap:used:
   *
   * Total size of the swap.
   */
  swap_props[PROP_SWAP_USED] = g_param_spec_uint64(
      "used", NULL, NULL, 0, G_MAXUINT64, 0, G_PARAM_READABLE);
  /**
   * BarBarSwap:free:
   *
   * Total size of the swap.
   */
  swap_props[PROP_SWAP_FREE] = g_param_spec_uint64(
      "free", NULL, NULL, 0, G_MAXUINT64, 0, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, PROP_NUM_PROPERTIES,
                                    swap_props);
  /**
   * BarBarSwap::tick:
   * @sensor: This sensor
   *
   * Emit that swap has updated
   */
  swap_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_SWAP,                       /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_swap_init(BarBarSwap *self) {}

static gboolean g_barbar_swap_tick(BarBarIntervalSensor *sensor) {
  BarBarSwap *self = BARBAR_SWAP(sensor);

  glibtop_get_swap(&self->swap);

  self->percent = self->swap.used / (double)self->swap.total;

  g_object_notify_by_pspec(G_OBJECT(self), swap_props[PROP_SWAP_PERCENT]);
  g_object_notify_by_pspec(G_OBJECT(self), swap_props[PROP_SWAP_TOTAL]);
  g_object_notify_by_pspec(G_OBJECT(self), swap_props[PROP_SWAP_USED]);
  g_object_notify_by_pspec(G_OBJECT(self), swap_props[PROP_SWAP_FREE]);
  g_signal_emit(G_OBJECT(self), swap_signals[TICK], 0);
  return G_SOURCE_CONTINUE;
}
