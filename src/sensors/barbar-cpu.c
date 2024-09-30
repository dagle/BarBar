#include "barbar-cpu.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <stdio.h>

/**
 * BarBarCpu:
 *
 * A simple cpu sensor
 */
struct _BarBarCpu {
  BarBarIntervalSensor parent_instance;

  double prev_total;
  double prev_idle;
  double percent;
};

enum {
  CPU_PROP_0,

  CPU_PROP_PERCENT,

  // CPU_PROP_TOTAL,
  // CPU_PROP_USER,
  // CPU_PROP_NICE,
  // CPU_PROP_SYS,
  // CPU_PROP_IDLE,
  // CPU_PROP_IOWAIT,
  // CPU_PROP_IRQ,
  // CPU_PROP_SOFTIRQ,
  // CPU_PROP_FREQUENCY,

  CPU_NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

static guint cpu_signals[NUM_SIGNALS];

static GParamSpec *cpu_props[CPU_NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarCpu, g_barbar_cpu, BARBAR_TYPE_INTERVAL_SENSOR)

static gboolean g_barbar_cpu_tick(BarBarIntervalSensor *sensor);

static void g_barbar_cpu_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {

  // BarBarCpu *cpu = BARBAR_CPU(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cpu_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarCpu *cpu = BARBAR_CPU(object);

  switch (property_id) {
  case CPU_PROP_PERCENT:
    g_value_set_double(value, cpu->percent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cpu_class_init(BarBarCpuClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarIntervalSensorClass *interval_class =
      BARBAR_INTERVAL_SENSOR_CLASS(class);

  interval_class->tick = g_barbar_cpu_tick;

  gobject_class->set_property = g_barbar_cpu_set_property;
  gobject_class->get_property = g_barbar_cpu_get_property;

  /**
   * BarBarCpu:percent:
   *
   * How much of the cpu is used.
   */
  cpu_props[CPU_PROP_PERCENT] =
      g_param_spec_double("percent", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, CPU_NUM_PROPERTIES,
                                    cpu_props);

  /**
   * BarBarCpu::tick:
   * @sensor: This sensor
   *
   * Emit that cpu has updated
   */
  cpu_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_CPU,                        /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_cpu_init(BarBarCpu *self) {}

static gboolean g_barbar_cpu_tick(BarBarIntervalSensor *sensor) {
  BarBarCpu *self = BARBAR_CPU(sensor);

  double total, idle;

  glibtop_cpu cpu;

  glibtop_get_cpu(&cpu);

  total = ((unsigned long)cpu.total) ? ((double)cpu.total) : 1.0;
  idle = ((unsigned long)cpu.idle) ? ((double)cpu.idle) : 1.0;

  self->percent = (1.0 - (idle - self->prev_idle) / (total - self->prev_total));

  self->prev_idle = idle;
  self->prev_total = total;

  g_object_notify_by_pspec(G_OBJECT(self), cpu_props[CPU_PROP_PERCENT]);
  g_signal_emit(G_OBJECT(self), cpu_signals[TICK], 0);
  return G_SOURCE_CONTINUE;
}

BarBarSensor *g_barbar_cpu_new(void) {
  BarBarCpu *cpu;

  cpu = g_object_new(BARBAR_TYPE_CPU, NULL);

  return BARBAR_SENSOR(cpu);
}
