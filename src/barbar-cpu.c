#include "barbar-cpu.h"
#include "barbar-bar.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <math.h>
#include <stdio.h>

struct _BarBarCpu {
  GtkWidget parent_instance;

  GtkWidget *label;

  double prev_total;
  double prev_idle;

  // An array for doubles;
  GArray *states;
};

enum {
  PROP_0,

  PROP_STATES,

  NUM_PROPERTIES,
};

static GParamSpec *cpu_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarCpu, g_barbar_cpu, GTK_TYPE_WIDGET)

static void g_barbar_cpu_constructed(GObject *object);

static void g_barbar_cpu_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {}

static void g_barbar_cpu_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarCpu *cpu = BARBAR_CPU(object);

  switch (property_id) {
    //  case PROP_STATES:
    // g_value_get_string(value);
    //    // g_barbar_disk_set_path(disk, g_value_get_string(value));
    //    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

/**
 * g_barbar_cpu_set_states:
 * @cpu: a #BarBarCpu
 * @states: (array zero-terminated=1): an array of doubles, terminated by a
 *%NULL element
 *
 * Set the the states for the cpu.
 *
 * The states are used to format different levels of load differently
 *
 **/
void g_barbar_cpu_set_states(BarBarCpu *cpu, const double states[]) {
  GArray *garray = g_array_new(FALSE, FALSE, sizeof(int));

  g_array_free(cpu->states, TRUE);

  for (; states; states++) {
    g_array_append_val(garray, states);
  }
  cpu->states = garray;
}

static void g_barbar_cpu_class_init(BarBarCpuClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_cpu_set_property;
  gobject_class->get_property = g_barbar_cpu_get_property;
  gobject_class->constructed = g_barbar_cpu_constructed;
  cpu_props[PROP_STATES] = g_param_spec_double("critical-temp", NULL, NULL, 0.0,
                                               300.0, 80.0, G_PARAM_CONSTRUCT);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, cpu_props);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}

static void g_barbar_cpu_init(BarBarCpu *self) {}

static void g_barbar_cpu_constructed(GObject *object) {
  BarBarCpu *cpu = BARBAR_CPU(object);

  G_OBJECT_CLASS(g_barbar_cpu_parent_class)->constructed(object);

  cpu->label = gtk_label_new("");
  gtk_widget_set_parent(cpu->label, GTK_WIDGET(cpu));
}

static gboolean g_barbar_cpu_update(gpointer data) {
  BarBarCpu *self = BARBAR_CPU(data);

  double total, idle;
  glibtop_cpu cpu;

  glibtop_init();

  glibtop_get_cpu(&cpu);

  total = ((unsigned long)cpu.total) ? ((double)cpu.total) : 1.0;
  idle = ((unsigned long)cpu.idle) ? ((double)cpu.idle) : 1.0;

  printf("load: %f\n", idle * 100 / total);
  return G_SOURCE_CONTINUE;
}

void g_barbar_cpu_start(BarBarCpu *cpu) {
  g_barbar_cpu_update(cpu);
  g_timeout_add_full(0, 30 * 1000, g_barbar_cpu_update, cpu, NULL);
}
