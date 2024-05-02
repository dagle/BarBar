#include "barbar-cpu.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <math.h>
#include <stdio.h>

/**
 * BarBarCpu:
 *
 * A simple cpu sensor
 */
struct _BarBarCpu {
  BarBarSensor parent_instance;

  double prev_total;
  double prev_idle;
  double percent;

  guint interval;
  guint source_id;
};

enum {
  CPU_PROP_0,

  CPU_PROP_INTERVAL,
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

// update every 10 sec
#define DEFAULT_INTERVAL 10000

static guint cpu_signals[NUM_SIGNALS];

static GParamSpec *cpu_props[CPU_NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarCpu, g_barbar_cpu, BARBAR_TYPE_SENSOR)

static void g_barbar_cpu_start(BarBarSensor *sensor);
static void g_barbar_cpu_constructed(GObject *object);
static gboolean g_barbar_cpu_update(gpointer data);

static void g_barbar_cpu_set_interval(BarBarCpu *cpu, guint inteval) {
  g_return_if_fail(BARBAR_IS_CPU(cpu));

  if (cpu->interval == inteval) {
    return;
  }

  cpu->interval = inteval;

  g_object_notify_by_pspec(G_OBJECT(cpu), cpu_props[CPU_PROP_INTERVAL]);
}

static void g_barbar_cpu_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {

  BarBarCpu *cpu = BARBAR_CPU(object);

  switch (property_id) {
  case CPU_PROP_INTERVAL:
    g_barbar_cpu_set_interval(cpu, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cpu_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarCpu *cpu = BARBAR_CPU(object);

  switch (property_id) {
  case CPU_PROP_INTERVAL:
    g_value_set_uint(value, cpu->interval);
    break;
  case CPU_PROP_PERCENT:
    g_value_set_double(value, cpu->percent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cpu_class_init(BarBarCpuClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_cpu_start;

  gobject_class->set_property = g_barbar_cpu_set_property;
  gobject_class->get_property = g_barbar_cpu_get_property;
  gobject_class->constructed = g_barbar_cpu_constructed;

  /**
   * BarBarCpu:interval:
   *
   * How often the cpu should be pulled for info
   */
  cpu_props[CPU_PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

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

static void g_barbar_cpu_init(BarBarCpu *self) {
  self->interval = DEFAULT_INTERVAL;
}

static void g_barbar_cpu_constructed(GObject *obj) {
  BarBarCpu *self = BARBAR_CPU(obj);
  G_OBJECT_CLASS(g_barbar_cpu_parent_class)->constructed(obj);
}

static gboolean g_barbar_cpu_update(gpointer data) {
  BarBarCpu *self = BARBAR_CPU(data);

  double total, idle;

  glibtop_cpu cpu;

  glibtop_init();

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

static void g_barbar_cpu_start(BarBarSensor *sensor) {
  BarBarCpu *cpu = BARBAR_CPU(sensor);
  if (cpu->source_id > 0) {
    g_source_remove(cpu->source_id);
  }
  g_barbar_cpu_update(cpu);
  cpu->source_id =
      g_timeout_add_full(0, cpu->interval, g_barbar_cpu_update, cpu, NULL);
}
