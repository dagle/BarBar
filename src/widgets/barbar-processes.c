#include "barbar-processes.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <math.h>

/**
 * BarBarRiverProcesses:
 *
 * A widget to display a list of processes depending on cpu usage
 *
 */
struct _BarBarCpuProcesses {
  GtkWidget parent_instance;

  guint interval;
  guint source_id;
};

enum {
  PROP_0,

  PROP_INTERVAL,
  PROP_NUMBER,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 10000

G_DEFINE_TYPE(BarBarCpuProcesses, g_barbar_cpu_processes, GTK_TYPE_WIDGET)

static GParamSpec *processes_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_cpu_processes_root(GtkWidget *widget);

static void g_barbar_cpu_processes_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {}

static void g_barbar_cpu_processes_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {}

static void g_barbar_cpu_processes_class_init(BarBarCpuProcessesClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_cpu_processes_set_property;
  gobject_class->get_property = g_barbar_cpu_processes_get_property;

  /**
   * BarBarCpuProcesses:interval:
   *
   * How often we should fetch the process list
   */
  processes_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarCpuProcesses:number:
   *
   * How many processes we should display
   */
  processes_props[PROP_NUMBER] =
      g_param_spec_uint("number", NULL, NULL, 0, G_MAXUINT32, DEFAULT_INTERVAL,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  widget_class->root = g_barbar_cpu_processes_root;

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "process-list");
}

static void g_barbar_cpu_processes_init(BarBarCpuProcesses *self) {}

static gboolean g_barbar_cpu_processes_update(gpointer data) {
  BarBarCpuProcesses *self = BARBAR_CPU_PROCESSES(data);

  double total, idle;

  glibtop_cpu cpu;

  glibtop_init();

  glibtop_get_cpu(&cpu);

  total = ((unsigned long)cpu.total) ? ((double)cpu.total) : 1.0;
  idle = ((unsigned long)cpu.idle) ? ((double)cpu.idle) : 1.0;

  // self->percent = (1.0 - (idle - self->prev_idle) / (total -
  // self->prev_total));
  //
  // self->prev_idle = idle;
  // self->prev_total = total;

  // g_object_notify_by_pspec(G_OBJECT(self), cpu_props[CPU_PROP_PERCENT]);
  // g_signal_emit(G_OBJECT(self), cpu_signals[TICK], 0);
  return G_SOURCE_CONTINUE;
}

static void g_barbar_cpu_processes_root(GtkWidget *widget) {
  BarBarCpuProcesses *cpu = BARBAR_CPU_PROCESSES(widget);

  if (cpu->source_id > 0) {
    g_source_remove(cpu->source_id);
  }
  g_barbar_cpu_processes_update(cpu);
  cpu->source_id = g_timeout_add_full(0, cpu->interval,
                                      g_barbar_cpu_processes_update, cpu, NULL);
}
