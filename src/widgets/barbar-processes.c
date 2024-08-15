#include "barbar-processes.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/procio.h>
#include <glibtop/proclist.h>
#include <glibtop/procmem.h>
#include <glibtop/procstate.h>
#include <glibtop/proctime.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * BarBarRiverProcesses:
 *
 * A widget to display a list of processes depending on cpu usage
 *
 */
struct _BarBarCpuProcesses {
  GtkWidget parent_instance;

  /* line showing for each process */
  // GList *lines;

  /*  */

  GArray *processes;
  GtkWidget *grid;

  gboolean show_header;

  guint number;
  guint64 previous_total;

  guint interval;
  guint source_id;
  BarBarProcessOrder order;
};

typedef struct proc_info {
  pid_t pid;
  glibtop_proc_state state;

  // Maybe make these pointers and malloc these, lets do this later
  double amount;
  glibtop_proc_time ptime;
  glibtop_proc_mem mem;
  glibtop_proc_io io;

  // maybe these 2
  guint64 utime;
  guint64 stime;

  guint64 old_utime;
  guint64 old_stime;

  gboolean mark;

} proc_info;

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
  PROP_HEADERS,
  PROP_NUMBER,
  PROP_ORDER,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 1000

G_DEFINE_TYPE(BarBarCpuProcesses, g_barbar_cpu_processes, GTK_TYPE_WIDGET)

GType g_barbar_procces_order_get_type(void) {

  static gsize barbar_process_order_type;
  if (g_once_init_enter(&barbar_process_order_type)) {

    static GEnumValue pattern_types[] = {
        {BARBAR_ORDER_MEM, "BARBAR_ORDER_MEM", "mem"},
        {BARBAR_ORDER_CPU, "BARBAR_ORDER_CPU", "cpu"},
        {BARBAR_ORDER_IO, "BARBAR_ORDER_IO", "io"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarProcessOrder", pattern_types);
    g_once_init_leave(&barbar_process_order_type, type);
  }
  return barbar_process_order_type;
}

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

static void g_barbar_cpu_processes_set_interval(BarBarCpuProcesses *cpu,
                                                guint interval) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->interval == interval) {
    return;
  }

  cpu->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_INTERVAL]);
}

static void g_barbar_cpu_processes_set_order(BarBarCpuProcesses *cpu,
                                             BarBarProcessOrder order) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->order == order) {
    return;
  }

  cpu->order = order;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_ORDER]);
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
  case PROP_INTERVAL:
    g_barbar_cpu_processes_set_interval(cpu, g_value_get_uint(value));
    break;
  case PROP_ORDER:
    g_barbar_cpu_processes_set_order(cpu, g_value_get_enum(value));
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
  case PROP_ORDER:
    g_value_set_enum(value, cpu->order);
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
   * BarBarCpuProcesses:headers:
   *
   */
  processes_props[PROP_HEADERS] =
      g_param_spec_boolean("headers", "Headers", NULL, TRUE, G_PARAM_READWRITE);

  /**
   * BarBarCpuProcesses:interval:
   *
   * How often we should fetch the process list
   */
  processes_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarCpuProcesses:number:
   *
   * How many processes we should display
   */
  processes_props[PROP_NUMBER] =
      g_param_spec_uint("number", NULL, NULL, 0, G_MAXUINT32, DEFAULT_INTERVAL,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  processes_props[PROP_ORDER] = g_param_spec_enum(
      "order", NULL, NULL, BARBAR_TYPE_PROCESS_ORDER, BARBAR_ORDER_CPU,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    processes_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "process-list");
}

static void insert_row(BarBarCpuProcesses *self, proc_info *proc, int row);

static proc_info *new_info(GArray *processes, pid_t pid) {
  // TODO:
  return NULL;
}

static proc_info *find_info(GArray *processes, pid_t pid) {
  for (int i = 0; i < processes->len; i++) {
    proc_info *proc = &g_array_index(processes, proc_info, i);
    if (proc->pid == pid) {
      return proc;
    }
  }
  return NULL;
}

static proc_info *get_info(GArray *processes, pid_t pid) {
  proc_info *info = find_info(processes, pid);
  return info ? info : new_info(processes, pid);
}

static void sweep(GArray *processes, guint32 tick) {
  for (int i = 0; i < processes->len; ++i) {
    proc_info *proc = &g_array_index(processes, proc_info, i);
    if (proc->mark) {
    }
  }
}

static void g_barbar_populate_headers(BarBarCpuProcesses *self) {
  // self->headers = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *process = gtk_label_new("process");
  gtk_widget_set_halign(process, GTK_ALIGN_START);
  gtk_widget_set_hexpand(process, TRUE);
  // gtk_label_set_xalign(GTK_LABEL(process), 0);
  // GtkWidget *cpu = gtk_label_new("cpu");
  // gtk_label_set_xalign(GTK_LABEL(cpu), 0.5);
  //
  GtkWidget *cpu = gtk_label_new("cpu");
  gtk_widget_set_halign(cpu, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(cpu, TRUE);

  GtkWidget *mem = gtk_label_new("mem");
  gtk_widget_set_halign(mem, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(mem, TRUE);

  GtkWidget *io = gtk_label_new("io");
  gtk_widget_set_halign(io, GTK_ALIGN_END);
  gtk_widget_set_hexpand(io, TRUE);

  gtk_grid_attach(GTK_GRID(self->grid), process, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(self->grid), cpu, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(self->grid), mem, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(self->grid), io, 3, 0, 1, 1);
}

static void insert_row(BarBarCpuProcesses *self, proc_info *proc, int row) {
  GtkWidget *widget;
  char str[8];
  row++;

  widget = gtk_grid_get_child_at(GTK_GRID(self->grid), 0, row);
  if (widget) {
    GtkWidget *process = gtk_grid_get_child_at(GTK_GRID(self->grid), 0, row);
    gtk_label_set_text(GTK_LABEL(process), proc->state.cmd);

    GtkWidget *cpu = gtk_grid_get_child_at(GTK_GRID(self->grid), 1, row);
    snprintf(str, 7, "%.2f", proc->amount);
    gtk_label_set_text(GTK_LABEL(cpu), str);

    GtkWidget *mem = gtk_grid_get_child_at(GTK_GRID(self->grid), 2, row);
    snprintf(str, 7, "%lu", proc->mem.rss);
    gtk_label_set_text(GTK_LABEL(mem), str);

    GtkWidget *io = gtk_grid_get_child_at(GTK_GRID(self->grid), 3, row);
    snprintf(str, 7, "%lu", proc->io.disk_wchar);
    gtk_label_set_text(GTK_LABEL(io), str);

  } else {
    GtkWidget *process = gtk_label_new(proc->state.cmd);
    gtk_widget_set_halign(process, GTK_ALIGN_START);
    gtk_widget_set_hexpand(process, TRUE);

    snprintf(str, 7, "%.2f", proc->amount);
    GtkWidget *cpu = gtk_label_new(str);
    gtk_widget_set_halign(cpu, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(cpu, TRUE);

    snprintf(str, 7, "%lu", proc->mem.rss);
    GtkWidget *mem = gtk_label_new(str);
    gtk_widget_set_halign(mem, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(mem, TRUE);

    snprintf(str, 7, "%lu", proc->io.disk_wchar);
    GtkWidget *io = gtk_label_new(str);
    gtk_widget_set_halign(io, GTK_ALIGN_END);
    gtk_widget_set_hexpand(io, TRUE);

    gtk_grid_attach(GTK_GRID(self->grid), process, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(self->grid), cpu, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(self->grid), mem, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(self->grid), io, 3, row, 1, 1);
  }
}

static void g_barbar_cpu_processes_init(BarBarCpuProcesses *self) {
  self->interval = DEFAULT_INTERVAL;
  self->show_header = TRUE;

  self->grid = gtk_grid_new();
  gtk_widget_set_hexpand(self->grid, true);

  g_barbar_populate_headers(self);

  gtk_widget_set_parent(self->grid, GTK_WIDGET(self));
}

static gint g_barbar_cpu_sort(gconstpointer a, gconstpointer b) {
  const proc_info *proc_a = a;
  const proc_info *proc_b = b;

  if (proc_b->amount > proc_a->amount) {
    return 1;
  }
  if (proc_a->amount > proc_b->amount) {
    return -1;
  }
  return 0;
}

static gint g_barbar_mem_sort(gconstpointer a, gconstpointer b) {
  const proc_info *proc_a = a;
  const proc_info *proc_b = b;

  return proc_a->mem.rss - proc_b->mem.rss;
}

static gint g_barbar_io_sort(gconstpointer a, gconstpointer b) {
  const proc_info *proc_a = a;
  const proc_info *proc_b = b;

  return proc_a->io.disk_wchar - proc_b->io.disk_wchar;
}

static void get_proctime(proc_info *proc, guint64 total) {
  // float mul = 100.0;
  // if (top_cpu_separate.get(*state)) mul *= info.cpu_count;
  //
  glibtop_get_proc_time(&proc->ptime, proc->pid);
  // proc->amount = mul * (proc->pti me.rtime) / (double)total;
  // proc->amount = (proc->ptime.cutime + proc->ptime.cstime) / (double)total;
  proc->amount = (proc->ptime.rtime) / (double)total;
}

static void sweep(GArray *processes) {
  //
}

static void get_metrics(BarBarCpuProcesses *self, pid_t *pids,
                        glibtop_proclist *buf, guint64 total) {

  guint selected = MIN(self->number, buf->number);
  switch (self->order) {
  case BARBAR_ORDER_MEM:
    for (int i = 0; i < buf->number; ++i) {
      proc_info *info = get_info(self->processes, pids[i]);
      glibtop_get_proc_mem(&info->mem, info->pid);
    }

    sweep(self->processes);
    g_array_sort(self->processes, g_barbar_mem_sort);

    for (int i = 0; i < selected; ++i) {
      proc_info *proc = &g_array_index(self->processes, proc_info, i);
      get_proctime(proc, total);
      glibtop_get_proc_io(&proc->io, proc->pid);
    }

    break;
  case BARBAR_ORDER_CPU:
    for (int i = 0; i < buf->number; ++i) {
      proc_info *info = get_info(self->processes, pids[i]);
      get_proctime(info, total);
    }

    g_array_sort(self->processes, g_barbar_mem_sort);

    for (int i = 0; i < selected; ++i) {
      proc_info *proc = &g_array_index(self->processes, proc_info, i);

      glibtop_get_proc_mem(&proc->mem, proc->pid);
      glibtop_get_proc_io(&proc->io, proc->pid);
    }
    break;
  case BARBAR_ORDER_IO:
    for (int i = 0; i < buf->number; ++i) {
      proc_info *info = get_info(self->processes, pids[i]);
      glibtop_get_proc_io(&info->io, info->pid);
    }

    g_array_sort(self->processes, g_barbar_mem_sort);

    for (int i = 0; i < selected; ++i) {
      proc_info *proc = &g_array_index(self->processes, proc_info, i);

      get_proctime(proc, total);
      glibtop_get_proc_mem(&proc->mem, proc->pid);
    }

    break;
  }

  for (int i = 0; i < selected; ++i) {
    proc_info *proc = &g_array_index(self->processes, proc_info, i);

    glibtop_get_proc_state(&proc->state, proc->pid);
  }
}

static gboolean g_barbar_cpu_processes_update(gpointer data) {
  BarBarCpuProcesses *self = BARBAR_CPU_PROCESSES(data);
  glibtop_proclist buf;
  glibtop_cpu cpu;
  pid_t *pids;
  GArray *procs;
  guint64 t; /* GLIBTOP_CPU_TOTAL		*/

  glibtop_get_cpu(&cpu);

  pids = glibtop_get_proclist(&buf, 0, 0);

  proc_info *info = calloc(buf.number, sizeof(proc_info));
  procs = g_array_new_take(info, buf.number, FALSE, sizeof(proc_info));

  for (int i = 0; i < procs->len; ++i) {
    proc_info *proc = &g_array_index(procs, proc_info, i);
    proc->pid = pids[i];
  }

  t = cpu.total - self->previous_total;
  self->previous_total = cpu.total;

  get_metrics(self, pids, &buf, t);

  // for (int i = 0; MA)
  for (int i = 0; i < MIN(self->number, procs->len); i++) {
    proc_info *proc = &g_array_index(procs, proc_info, i);
    insert_row(self, proc, i);
  }

  g_array_free(procs, TRUE);

  g_free(pids);

  return G_SOURCE_CONTINUE;
}

static void g_barbar_cpu_processes_root(GtkWidget *widget) {
  BarBarCpuProcesses *cpu = BARBAR_CPU_PROCESSES(widget);
  glibtop_init();

  if (cpu->source_id > 0) {
    g_source_remove(cpu->source_id);
  }
  g_barbar_cpu_processes_update(cpu);
  cpu->source_id = g_timeout_add_full(0, cpu->interval,
                                      g_barbar_cpu_processes_update, cpu, NULL);
}
