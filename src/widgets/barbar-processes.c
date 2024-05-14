#include "barbar-processes.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/proclist.h>
#include <glibtop/procstate.h>
#include <glibtop/proctime.h>
#include <math.h>
#include <stdlib.h>

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

  guint number;
  GtkWidget *label;

  guint interval;
  guint delta;
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

  PROP_DELTA,
  PROP_INTERVAL,
  PROP_NUMBER,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 30000
#define DEFAULT_DELTA 1000

G_DEFINE_TYPE(BarBarCpuProcesses, g_barbar_cpu_processes, GTK_TYPE_WIDGET)

static GParamSpec *processes_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_cpu_processes_root(GtkWidget *widget);

static void g_barbar_cpu_processes_set_number(BarBarCpuProcesses *cpu,
                                              guint number) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->number == number) {
    return;
  }

  cpu->number = number;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_NUMBER]);
}

static void g_barbar_cpu_processes_set_delta(BarBarCpuProcesses *cpu,
                                             guint delta) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->delta == delta) {
    return;
  }

  cpu->delta = delta;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_DELTA]);
}

static void g_barbar_cpu_processes_set_interval(BarBarCpuProcesses *cpu,
                                                guint interval) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->interval == interval) {
    return;
  }

  cpu->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_INTERVAL]);
}

static void g_barbar_cpu_processes_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec) {

  BarBarCpuProcesses *cpu = BARBAR_CPU_PROCESSES(object);
  switch (property_id) {
  case PROP_NUMBER:
    g_barbar_cpu_processes_set_number(cpu, g_value_get_uint(value));
    break;
  case PROP_DELTA:
    g_barbar_cpu_processes_set_delta(cpu, g_value_get_uint(value));
    break;
  case PROP_INTERVAL:
    g_barbar_cpu_processes_set_interval(cpu, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cpu_processes_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {

  BarBarCpuProcesses *cpu = BARBAR_CPU_PROCESSES(object);

  switch (property_id) {
  case PROP_INTERVAL:
    g_value_set_uint(value, cpu->interval);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cpu_processes_class_init(BarBarCpuProcessesClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_cpu_processes_set_property;
  gobject_class->get_property = g_barbar_cpu_processes_get_property;
  widget_class->root = g_barbar_cpu_processes_root;

  /**
   * BarBarCpuProcesses:interval:
   *
   * How often we should fetch the process list
   */
  processes_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarCpuProcesses:Delta:
   *
   * How long processes should be monitored
   */
  processes_props[PROP_DELTA] =
      g_param_spec_uint("delta", NULL, NULL, 0, G_MAXUINT32, DEFAULT_DELTA,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarCpuProcesses:number:
   *
   * How many processes we should display
   */
  processes_props[PROP_NUMBER] =
      g_param_spec_uint("number", NULL, NULL, 0, G_MAXUINT32, DEFAULT_INTERVAL,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    processes_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "process-list");
}

static void g_barbar_cpu_processes_init(BarBarCpuProcesses *self) {
  self->interval = DEFAULT_INTERVAL;
  self->interval = DEFAULT_DELTA;

  self->label = gtk_label_new("test");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

struct state {
  // int number;
  pid_t *pids;
  glibtop_proc_time *ptime;
  glibtop_proc_state *pstate;

  glibtop_proc_time *delta_ptime;
  glibtop_proc_state *delta_pstate;
};

struct delta {
  pid_t pid;

  glibtop_proc_time ptime;
  glibtop_proc_time delta_ptime;

  double time;
  glibtop_proc_state pstate;

  // glibtop_proc_state delta_pstate;
};

// static void g_barbar_state_sort2(struct state *state) {}

static gint g_barbar_state_sort(gconstpointer a, gconstpointer b) {
  const struct delta *delta_a = a;
  const struct delta *delta_b = b;
  if (!delta_a) {
    printf("a is null!\n");
    return -1;
  }
  if (!delta_a) {
    printf("a is null!\n");
    return 1;
  }
  return delta_b->time - delta_a->time;
}

// let curr_stat = proc.curr_proc.stat();
// let prev_stat = &proc.prev_stat;
//
// let curr_time = curr_stat.utime + curr_stat.stime;
// let prev_time = prev_stat.utime + prev_stat.stime;
// let usage_ms = (curr_time - prev_time) * 1000 / procfs::ticks_per_second();
// let interval_ms = proc.interval.as_secs() * 1000 +
// u64::from(proc.interval.subsec_millis()); let usage = usage_ms as f64 * 100.0
// / interval_ms as f64;
//
// let fmt_content = format!("{usage:.1}");
// let raw_content = (usage * 1000.0) as u32;
//
// self.fmt_contents.insert(proc.pid, fmt_content);
// self.raw_contents.insert(proc.pid, raw_content);

// Data should be something else
static void g_barbar_cpu_processes_delta(gpointer data) {
  GArray *array = data;
  for (int i = 0; i < array->len; ++i) {
    struct delta *delta = &g_array_index(array, struct delta, i);
    glibtop_get_proc_time(&delta->delta_ptime, delta->pid);
    guint64 d = (delta->delta_ptime.stime - delta->ptime.stime) +
                (delta->delta_ptime.utime - delta->ptime.utime);
    guint64 usage_ms = d * 1000 / delta->delta_ptime.frequency;
    guint64 interval_ms = DEFAULT_DELTA;
    double usage = (double)usage_ms * 100.0 / (double)interval_ms;
    delta->time = usage;
  }
  g_array_sort(array, g_barbar_state_sort);

  for (int i = 0; i < array->len; ++i) {
    struct delta *delta = &g_array_index(array, struct delta, i);
    printf("%s - %f\n", delta->pstate.cmd, delta->time);
    if (i > 5) {
      break;
    }
  }
  fprintf(stdout, "No. of clock ticks per sec : %ld\n", sysconf(_SC_CLK_TCK));
  g_array_unref(array);
}

static gboolean g_barbar_cpu_processes_update(gpointer data) {
  BarBarCpuProcesses *self = BARBAR_CPU_PROCESSES(data);
  glibtop_proclist buf;
  glibtop_cpu cpu;
  pid_t *pids;
  GArray *array;

  glibtop_get_cpu(&cpu);

  pids = glibtop_get_proclist(&buf, 0, 0);

  struct delta *delta = calloc(sizeof(struct delta), buf.number);

  // total = ((unsigned long)cpu.total) ? ((double)cpu.total) : 1.0;
  // guint64 total_delta = total - self->prev_total;
  printf("num: %ld\n", buf.number);

  for (int i = 0; i < buf.number; ++i) {
    delta[i].pid = pids[i];

    // get the proc time twice so we can calculate the usage
    glibtop_get_proc_time(&delta[i].ptime, delta[i].pid);
    glibtop_get_proc_state(&delta[i].pstate, delta[i].pid);
  }
  array = g_array_new_take(delta, buf.number, FALSE, sizeof(struct delta));

  // g_array_set_clear_func(array,);

  g_free(pids);

  // printf("interval: %d\n", self->interval);

  g_timeout_add_once(self->interval, g_barbar_cpu_processes_delta, array);

  // self->prev_total = total;

  return G_SOURCE_CONTINUE;
}

static void g_barbar_cpu_processes_root(GtkWidget *widget) {
  BarBarCpuProcesses *cpu = BARBAR_CPU_PROCESSES(widget);
  glibtop_init();

  if (cpu->source_id > 0) {
    g_source_remove(cpu->source_id);
  }
  g_barbar_cpu_processes_update(cpu);
  // cpu->source_id = g_timeout_add_full(0, cpu->interval,
  //                                     g_barbar_cpu_processes_update, cpu,
  //                                     NULL);
}
