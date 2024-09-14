#include "barbar-processes.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/procio.h>
#include <glibtop/proclist.h>
#include <glibtop/procmem.h>
#include <glibtop/procstate.h>
#include <glibtop/proctime.h>
#include <glibtop/sysinfo.h>
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

  GList *processes;
  GtkWidget *grid;

  gboolean show_header;

  gboolean seperate_cpu;
  guint cpu_count;
  guint memory;

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
  double cpu_amount;
  double mem_amount;
  double io_amount;
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
  PROP_SEPERATE_CPU,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 30000

G_DEFINE_TYPE(BarBarCpuProcesses, g_barbar_cpu_processes, GTK_TYPE_WIDGET)

static void g_barbar_processes_reset(BarBarCpuProcesses *self);
static gboolean g_barbar_cpu_processes_update(gpointer data);

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

/**
 * g_barbar_cpu_processes_set_number:
 * @cpu: a `BarBarRiverProcesses`
 * @number: number of processes
 *
 * Sets the amount of processes we should display.
 */
void g_barbar_cpu_processes_set_number(BarBarCpuProcesses *cpu, guint number) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->number == number) {
    return;
  }

  cpu->number = number;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_NUMBER]);
  if (gtk_widget_get_parent(GTK_WIDGET(cpu)) != NULL) {
    g_barbar_cpu_processes_update(cpu);
  }
}
/**
 * g_barbar_cpu_processes_set_interval:
 * @cpu: a `BarBarRiverProcesses`
 * @interval: how often we should update
 *
 */
void g_barbar_cpu_processes_set_interval(BarBarCpuProcesses *cpu,
                                         guint interval) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->interval == interval) {
    return;
  }

  cpu->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_INTERVAL]);
}

/**
 * g_barbar_cpu_processes_set_order:
 * @cpu: a `BarBarRiverProcesses`
 * @order: What metric to sort processes
 *
 * Sorts processes by order
 */
void g_barbar_cpu_processes_set_order(BarBarCpuProcesses *cpu,
                                      BarBarProcessOrder order) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->order == order) {
    return;
  }

  cpu->order = order;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_ORDER]);
  g_barbar_processes_reset(cpu);
  if (gtk_widget_get_parent(GTK_WIDGET(cpu)) != NULL) {
    g_barbar_cpu_processes_update(cpu);
  }
}

/**
 * g_barbar_cpu_processes_set_seperate_cpu:
 * @cpu: a `BarBarRiverProcesses`
 * @seperate: if cpus should be seprated
 *
 * Sets if cpus should be seperated. If your computer has 4 cpus and a process
 * uses 100% of one core, it will be reported as 100%. If turned off, it will
 * reported as 25%.
 *
 */
void g_barbar_cpu_processes_set_seperate_cpu(BarBarCpuProcesses *cpu,
                                             gboolean seperate) {
  g_return_if_fail(BARBAR_IS_CPU_PROCESSES(cpu));

  if (cpu->seperate_cpu == seperate) {
    return;
  }

  cpu->seperate_cpu = seperate;

  g_object_notify_by_pspec(G_OBJECT(cpu), processes_props[PROP_SEPERATE_CPU]);
  if (gtk_widget_get_parent(GTK_WIDGET(cpu)) != NULL) {
    g_barbar_cpu_processes_update(cpu);
  }
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
  case PROP_SEPERATE_CPU:
    g_barbar_cpu_processes_set_seperate_cpu(cpu, g_value_get_boolean(value));
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
  case PROP_SEPERATE_CPU:
    g_value_set_boolean(value, cpu->seperate_cpu);
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
   * If headers of the widget should be displayed.
   * Displaying information of what is displayed in each column.
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

  /**
   * BarBarCpuProcesses:order:
   *
   * What metric to use when sorting processes.
   */
  processes_props[PROP_ORDER] = g_param_spec_enum(
      "order", NULL, NULL, BARBAR_TYPE_PROCESS_ORDER, BARBAR_ORDER_CPU,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarCpuProcesses:seperate-cpu:
   *
   * If we should normalize against one core.
   *
   */
  processes_props[PROP_SEPERATE_CPU] = g_param_spec_boolean(
      "seperate-cpu", NULL, NULL, TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    processes_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "process-list");
}

static void insert_row(BarBarCpuProcesses *self, proc_info *proc, int row);

static proc_info *new_info(GList **processes, pid_t pid) {
  proc_info *info = calloc(1, sizeof(proc_info));
  info->pid = pid;
  *processes = g_list_prepend(*processes, info);
  return info;
}

static proc_info *find_info(GList **processes, pid_t pid) {
  for (GList *l = *processes; l != NULL; l = l->next) {
    proc_info *proc = l->data;
    if (proc->pid == pid) {
      return proc;
    }
  }
  return NULL;
}

static proc_info *get_info(GList **processes, pid_t pid) {
  proc_info *info = find_info(processes, pid);
  return info ? info : new_info(processes, pid);
}

static void sweep(GList **processes) {
  GList *l = *processes;
  while (l != NULL) {
    GList *next = l->next;
    proc_info *proc = l->data;

    if (proc->mark) {
      proc->mark = FALSE;
    } else {
      *processes = g_list_delete_link(*processes, l);
      g_free(proc);
    }
    l = next;
  }
}
static void set_cpu_sort(GtkWidget *widget, gpointer data) {
  BarBarCpuProcesses *self = data;
  g_barbar_cpu_processes_set_order(self, BARBAR_ORDER_CPU);
}

static void set_mem_sort(GtkWidget *widget, gpointer data) {
  BarBarCpuProcesses *self = data;
  g_barbar_cpu_processes_set_order(self, BARBAR_ORDER_MEM);
}

static void set_io_sort(GtkWidget *widget, gpointer data) {
  BarBarCpuProcesses *self = data;
  g_barbar_cpu_processes_set_order(self, BARBAR_ORDER_IO);
}

static void g_barbar_populate_headers(BarBarCpuProcesses *self) {
  GtkWidget *process = gtk_label_new("processes");
  // gtk_button_set_label(GTK_BUTTON(process), "process");
  gtk_widget_set_halign(process, GTK_ALIGN_START);
  gtk_widget_set_hexpand(process, TRUE);

  GtkWidget *cpu = gtk_button_new();
  gtk_button_set_label(GTK_BUTTON(cpu), "cpu");
  gtk_widget_set_halign(cpu, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(cpu, TRUE);
  g_signal_connect(cpu, "clicked", G_CALLBACK(set_cpu_sort), self);

  GtkWidget *mem = gtk_button_new();
  gtk_button_set_label(GTK_BUTTON(mem), "mem");
  gtk_widget_set_halign(mem, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(mem, TRUE);
  g_signal_connect(mem, "clicked", G_CALLBACK(set_mem_sort), self);

  GtkWidget *io = gtk_button_new();
  gtk_button_set_label(GTK_BUTTON(io), "io");
  gtk_widget_set_halign(io, GTK_ALIGN_END);
  gtk_widget_set_hexpand(io, TRUE);
  g_signal_connect(io, "clicked", G_CALLBACK(set_io_sort), self);

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
    snprintf(str, 7, "%.1f%%", proc->cpu_amount);
    gtk_label_set_text(GTK_LABEL(cpu), str);

    GtkWidget *mem = gtk_grid_get_child_at(GTK_GRID(self->grid), 2, row);
    snprintf(str, 7, "%0.1f%%", proc->mem_amount);
    gtk_label_set_text(GTK_LABEL(mem), str);

    GtkWidget *io = gtk_grid_get_child_at(GTK_GRID(self->grid), 3, row);
    snprintf(str, 7, "%lu", proc->io.disk_wchar);
    gtk_label_set_text(GTK_LABEL(io), str);
  } else {
    GtkWidget *process = gtk_label_new(proc->state.cmd);
    gtk_widget_set_halign(process, GTK_ALIGN_START);
    gtk_widget_set_hexpand(process, TRUE);

    snprintf(str, 7, "%.1f%%", proc->cpu_amount);
    GtkWidget *cpu = gtk_label_new(str);
    gtk_widget_set_halign(cpu, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(cpu, TRUE);

    snprintf(str, 7, "%0.1f%%", proc->mem_amount);
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
  glibtop_mem mem;
  const glibtop_sysinfo *info = glibtop_get_sysinfo();

  glibtop_get_mem(&mem);
  self->memory = mem.total;

  self->interval = DEFAULT_INTERVAL;
  self->show_header = TRUE;

  self->cpu_count = info->ncpu;

  self->grid = gtk_grid_new();
  gtk_widget_set_hexpand(self->grid, true);

  g_barbar_populate_headers(self);

  gtk_widget_set_parent(self->grid, GTK_WIDGET(self));
}

static gint g_barbar_cpu_sort(gconstpointer a, gconstpointer b) {
  const proc_info *proc_a = a;
  const proc_info *proc_b = b;

  if (proc_a->cpu_amount < proc_b->cpu_amount) {
    return 1;
  }
  if (proc_a->cpu_amount > proc_b->cpu_amount) {
    return -1;
  }
  return 0;
}

static gint g_barbar_mem_sort(gconstpointer a, gconstpointer b) {
  const proc_info *proc_a = a;
  const proc_info *proc_b = b;

  return proc_b->mem.rss - proc_a->mem.rss;
}

static gint g_barbar_io_sort(gconstpointer a, gconstpointer b) {
  const proc_info *proc_a = a;
  const proc_info *proc_b = b;

  if (proc_b->io.disk_wchar > proc_a->io.disk_wchar) {
    return 1;
  }
  if (proc_b->io.disk_wchar < proc_a->io.disk_wchar) {
    return -1;
  }
  return 0;
}

static void get_proctime(BarBarCpuProcesses *self, proc_info *proc,
                         guint64 total) {
  float mul = 100.0;

  if (self->seperate_cpu) {
    mul *= self->cpu_count;
  }
  // if (top_cpu_separate.get(*state)) mul *= info.cpu_count;
  //
  glibtop_get_proc_time(&proc->ptime, proc->pid);

  proc->utime = proc->ptime.utime - proc->old_utime;
  proc->stime = proc->ptime.stime - proc->old_stime;

  proc->old_stime = proc->ptime.stime;
  proc->old_utime = proc->ptime.utime;

  proc->cpu_amount = mul * (proc->utime + proc->stime) / (double)total;
}
static void get_mem(proc_info *proc, guint64 total) {
  glibtop_get_proc_mem(&proc->mem, proc->pid);

  proc->mem_amount = 100 * proc->mem.rss / (double)total;
}

static void get_io(proc_info *proc, guint64 total) {
  glibtop_get_proc_io(&proc->io, proc->pid);
}

static void get_metrics(BarBarCpuProcesses *self, pid_t *pids,
                        glibtop_proclist *buf, guint64 total) {

  int j;

  guint selected = MIN(self->number, buf->number);
  switch (self->order) {
  case BARBAR_ORDER_MEM:
    for (int i = 0; i < buf->number; ++i) {
      proc_info *info = get_info(&self->processes, pids[i]);
      get_mem(info, self->memory);
      info->mark = TRUE;
    }

    sweep(&self->processes);
    self->processes = g_list_sort(self->processes, g_barbar_mem_sort);

    j = 0;
    for (GList *l = self->processes; l != NULL && j < selected;
         l = l->next, j++) {
      proc_info *proc = l->data;
      get_proctime(self, proc, total);
      glibtop_get_proc_io(&proc->io, proc->pid);
    }

    break;
  case BARBAR_ORDER_CPU:
    for (int i = 0; i < buf->number; ++i) {
      proc_info *info = get_info(&self->processes, pids[i]);
      get_proctime(self, info, total);
      info->mark = TRUE;
    }

    sweep(&self->processes);
    self->processes = g_list_sort(self->processes, g_barbar_cpu_sort);

    j = 0;
    for (GList *l = self->processes; l != NULL && j < selected;
         l = l->next, j++) {
      proc_info *proc = l->data;

      get_mem(proc, self->memory);
      glibtop_get_proc_io(&proc->io, proc->pid);
    }
    break;
  case BARBAR_ORDER_IO:
    for (int i = 0; i < buf->number; ++i) {
      proc_info *info = get_info(&self->processes, pids[i]);
      glibtop_get_proc_io(&info->io, info->pid);
      info->mark = TRUE;
    }

    sweep(&self->processes);
    self->processes = g_list_sort(self->processes, g_barbar_io_sort);

    j = 0;
    for (GList *l = self->processes; l != NULL && j < selected;
         l = l->next, j++) {
      proc_info *proc = l->data;

      get_proctime(self, proc, total);
      get_mem(proc, self->memory);
    }

    break;
  }

  j = 0;
  for (GList *l = self->processes; l != NULL; l = l->next) {
    proc_info *proc = l->data;
    glibtop_get_proc_state(&proc->state, proc->pid);
  }
}
static void g_barbar_processes_reset(BarBarCpuProcesses *self) {
  if (self->processes) {
    g_list_free_full(self->processes, g_free);
  }
  self->processes = NULL;
  self->previous_total = 0;
}

static gboolean g_barbar_cpu_processes_update(gpointer data) {
  BarBarCpuProcesses *self = BARBAR_CPU_PROCESSES(data);
  glibtop_proclist buf;
  glibtop_cpu cpu;
  pid_t *pids;
  guint64 t; /* GLIBTOP_CPU_TOTAL		*/

  glibtop_get_cpu(&cpu);

  pids = glibtop_get_proclist(&buf, 0, 0);

  t = cpu.total - self->previous_total;
  self->previous_total = cpu.total;

  get_metrics(self, pids, &buf, t);

  int i = 0;
  for (GList *l = self->processes; l != NULL && i < self->number;
       l = l->next, i++) {
    proc_info *proc = l->data;
    insert_row(self, proc, i);
  }

  g_free(pids);

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

/**
 * g_barbar_cpu_processes_new:
 *
 * Creates a new `BarBarRiverProcesses`
 */
GtkWidget *g_barbar_cpu_processes_new(void) {
  BarBarCpuProcesses *self;

  self = g_object_new(BARBAR_TYPE_CPU_PROCESSES, NULL);

  return GTK_WIDGET(self);
}
