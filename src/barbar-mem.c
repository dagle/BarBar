#include "barbar-mem.h"
#include <glibtop.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <math.h>
#include <stdio.h>

struct _BarBarMem {
  GtkWidget parent_instance;

  GtkWidget *label;
  guint interval;

  guint source_id;
};

enum {
  PROP_0,

  PROP_INTERVAL,

  NUM_PROPERTIES,
};

// update every 10 sec
#define DEFAULT_INTERVAL 10000

G_DEFINE_TYPE(BarBarMem, g_barbar_mem, GTK_TYPE_WIDGET)

static GParamSpec *mem_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_mem_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {}

static void g_barbar_mem_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {}

static void g_barbar_mem_class_init(BarBarMemClass *class) {
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_mem_set_property;
  gobject_class->get_property = g_barbar_mem_get_property;
  mem_props[PROP_INTERVAL] =
      g_param_spec_uint("interval", "Interval", "Interval in milli seconds", 0,
                        G_MAXUINT32, DEFAULT_INTERVAL, G_PARAM_READWRITE);
  // mem_props[PROP_STATES] =
  //     g_param_spec_string("path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, mem_props);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}

void g_barbar_mem_start(BarBarMem *mem, gpointer data);

static void g_barbar_mem_init(BarBarMem *self) {
  self->interval = DEFAULT_INTERVAL;
  self->label = gtk_label_new("");

  gtk_widget_set_parent(self->label, GTK_WIDGET(self));

  g_signal_connect(self, "map", G_CALLBACK(g_barbar_mem_start), NULL);
}

static gboolean g_barbar_mem_update(gpointer data) {
  BarBarMem *self = BARBAR_MEM(data);
  double percent;

  glibtop_mem mem;
  // glibtop_swap swap;

  glibtop_get_mem(&mem);
  // glibtop_get_swap(&swap);

  percent =
      100 * (1.0 - ((double)(mem.total - mem.user)) / ((double)mem.total));
  gchar *str = g_strdup_printf("mem: %.0f%%", percent);

  gtk_label_set_label(GTK_LABEL(self->label), str);
  g_free(str);

  return G_SOURCE_CONTINUE;
}

void g_barbar_mem_start(BarBarMem *mem, gpointer data) {
  if (mem->source_id > 0) {
    g_source_remove(mem->source_id);
  }
  g_barbar_mem_update(mem);
  mem->source_id =
      g_timeout_add_full(0, mem->interval, g_barbar_mem_update, mem, NULL);
}
