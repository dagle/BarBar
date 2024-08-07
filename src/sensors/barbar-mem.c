#include "barbar-mem.h"
#include <glibtop.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <stdio.h>

/**
 * BarBarMem:
 *
 * A simple memory sensor
 */
struct _BarBarMem {
  BarBarIntervalSensor parent_instance;

  double percent;

  guint interval;
  guint source_id;
};

enum {
  MEM_PROP_0,

  MEM_PROP_PERCENT,

  // MEM_PROP_TOTAL,
  // MEM_PROP_USED,
  // MEM_PROP_FREE,
  // MEM_PROP_SHARED,
  // MEM_PROP_BUFFER,
  // MEM_PROP_CACHE,
  // MEM_PROP_USER,
  // MEM_PROP_LOCKED,

  MEM_NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

static guint mem_signals[NUM_SIGNALS];

G_DEFINE_TYPE(BarBarMem, g_barbar_mem, BARBAR_TYPE_INTERVAL_SENSOR)

static gboolean g_barbar_mem_tick(BarBarIntervalSensor *sensor);

static GParamSpec *mem_props[MEM_NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_mem_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {
  BarBarMem *mem = BARBAR_MEM(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mem_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarMem *mem = BARBAR_MEM(object);

  switch (property_id) {
  case MEM_PROP_PERCENT:
    g_value_set_double(value, mem->percent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

// static void g_barbar_mem_constructed(GObject *obj);

static void g_barbar_mem_class_init(BarBarMemClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  interval_class->tick = g_barbar_mem_tick;

  gobject_class->set_property = g_barbar_mem_set_property;
  gobject_class->get_property = g_barbar_mem_get_property;

  /**
   * BarBarMem:percent:
   *
   * How much of the memory is used.
   */
  mem_props[MEM_PROP_PERCENT] =
      g_param_spec_double("percent", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);
  g_object_class_install_properties(gobject_class, MEM_NUM_PROPERTIES,
                                    mem_props);
  /**
   * BarBarMem::tick:
   * @sensor: This sensor
   *
   * Emit that mem has updated
   */
  mem_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_MEM,                        /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_mem_init(BarBarMem *self) {}

static gboolean g_barbar_mem_tick(BarBarIntervalSensor *sensor) {
  BarBarMem *self = BARBAR_MEM(sensor);

  glibtop_mem mem;

  glibtop_get_mem(&mem);

  self->percent =
      (1.0 - ((double)(mem.total - mem.user)) / ((double)mem.total));

  g_object_notify_by_pspec(G_OBJECT(self), mem_props[MEM_PROP_PERCENT]);
  g_signal_emit(G_OBJECT(self), mem_signals[TICK], 0);
  return G_SOURCE_CONTINUE;
}
