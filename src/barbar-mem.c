#include "barbar-mem.h"
#include <glibtop.h>
#include <glibtop/mem.h>
#include <math.h>
#include <stdio.h>

struct _BarBarMem {
  GObject parent;

  char *label;
};

enum {
  PROP_0,

  PROP_STATES,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarMem, g_barbar_mem, GTK_TYPE_WIDGET)

static GParamSpec *mem_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_mem_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {}

static void g_barbar_mem_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {}

static void g_barbar_mem_class_init(BarBarMemClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_mem_set_property;
  gobject_class->get_property = g_barbar_mem_get_property;
  mem_props[PROP_STATES] =
      g_param_spec_string("path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, mem_props);
}

static void g_barbar_mem_init(BarBarMem *self) {}

void g_barbar_mem_update(BarBarMem *self) {}
