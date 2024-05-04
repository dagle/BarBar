#include "barbar-processes.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/proclist.h>
#include <glibtop/procstate.h>
#include <glibtop/proctime.h>
#include <math.h>

/**
 * BarBarRiverProcesses:
 *
 * A widget to display a list of processes depending on cpu usage
 *
 */
struct _BarBarCpuProcesses {
  GtkWidget parent_instance;

  // double prev_total;
  // double prev_utime;
  // double prev_stime;
  GList *lines;

  GtkWidget *label;

  guint interval;
  guint source_id;
};

struct procline {
  GtkWidget *box;
  GtkWidget *name;
  GtkWidget *pid;
  GtkWidget *cpu;
  GtkWidget *mem;
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

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "process-list");
}

static void g_barbar_cpu_processes_init(BarBarCpuProcesses *self) {
  self->label = gtk_label_new("test");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

static gboolean g_barbar_cpu_processes_update(gpointer data) {
  BarBarCpuProcesses *self = BARBAR_CPU_PROCESSES(data);
  glibtop_proclist buf;
  guint64 total;
  glibtop_cpu cpu;
  pid_t *pids;

  glibtop_init();

  glibtop_get_cpu(&cpu);

  // TODO: is 0, 0 right?
  pids = glibtop_get_proclist(&buf, 0, 0);
  total = ((unsigned long)cpu.total) ? ((double)cpu.total) : 1.0;
  // guint64 total_delta = total - self->prev_total;

  for (int i = 0; i < buf.number; ++i) {
    glibtop_proc_time ptime;
    glibtop_proc_time prev_ptime;
    glibtop_proc_state pstate;
    guint64 utime_delta;
    guint64 stime_delta;

    pid_t p = pids[i];

    // get the proc time twice so we can calculate the usage
    glibtop_get_proc_time(&ptime, p);
    glibtop_get_proc_state(&pstate, p);

    // double usage = (ptime.utime + ptime.stime) / (double)total_delta;
    // printf("process %s usage: %f\n", pstate.cmd, usage);
  }

  // self->prev_total = total;

  return G_SOURCE_CONTINUE;
}

static void g_barbar_cpu_processes_root(GtkWidget *widget) {
  BarBarCpuProcesses *cpu = BARBAR_CPU_PROCESSES(widget);

  if (cpu->source_id > 0) {
    g_source_remove(cpu->source_id);
  }
  g_barbar_cpu_processes_update(cpu);
  // cpu->source_id = g_timeout_add_full(0, cpu->interval,
  //                                     g_barbar_cpu_processes_update, cpu,
  //                                     NULL);
}
