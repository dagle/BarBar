#include "barbar-mem.h"
#include <glibtop.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <math.h>
#include <stdio.h>

/**
 * BarBarMem:
 *
 * A simple memory sensor
 */
struct _BarBarMem {
  BarBarSensor parent_instance;

  double percent;

  guint interval;
  guint source_id;
};

enum {
  MEM_PROP_0,

  MEM_PROP_INTERVAL,
  MEM_PROP_PERCENT,

  // MEM_PROP_TOTAL,
  // MEM_PROP_USED,
  // MEM_PROP_FREE,
  // MEM_PROP_SHARED,
  // MEM_PROP_BUFFER,
  // MEM_PROP_CACHE,
  // MEM_PROP_USER,
  // MEM_PROP_LOCKED,

  MEM_NUM_PROPERTIES,
};

// update every 10 sec
#define DEFAULT_INTERVAL 10000

// G_DEFINE_TYPE(BarBarMem, g_barbar_mem, GTK_TYPE_WIDGET)
G_DEFINE_TYPE(BarBarMem, g_barbar_mem, BARBAR_TYPE_SENSOR)

static void g_barbar_mem_start(BarBarSensor *sensor);

static GParamSpec *mem_props[MEM_NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_mem_set_interval(BarBarMem *self, uint interval) {
  g_return_if_fail(BARBAR_IS_MEM(self));

  self->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(self), mem_props[MEM_PROP_INTERVAL]);
}

static void g_barbar_mem_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {
  BarBarMem *mem = BARBAR_MEM(object);

  switch (property_id) {
  case MEM_PROP_INTERVAL:
    g_barbar_mem_set_interval(mem, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mem_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarMem *mem = BARBAR_MEM(object);

  switch (property_id) {
  case MEM_PROP_INTERVAL:
    g_value_set_uint(value, mem->interval);
    break;
  case MEM_PROP_PERCENT:
    g_value_set_double(value, mem->percent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mem_constructed(GObject *obj);

static void g_barbar_mem_class_init(BarBarMemClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_mem_start;

  gobject_class->set_property = g_barbar_mem_set_property;
  gobject_class->get_property = g_barbar_mem_get_property;
  gobject_class->constructed = g_barbar_mem_constructed;

  /**
   * BarBarMem:interval:
   *
   * How often memory should be pulled for info
   */
  mem_props[MEM_PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarMem:percent:
   *
   * How much of the memory is used.
   */
  mem_props[MEM_PROP_PERCENT] =
      g_param_spec_double("percent", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);
  g_object_class_install_properties(gobject_class, MEM_NUM_PROPERTIES,
                                    mem_props);
}

static void g_barbar_mem_init(BarBarMem *self) {
  self->interval = DEFAULT_INTERVAL;
}

static void g_barbar_mem_constructed(GObject *obj) {
  BarBarMem *self = BARBAR_MEM(obj);
  G_OBJECT_CLASS(g_barbar_mem_parent_class)->constructed(obj);
}

gboolean g_barbar_mem_update(gpointer data) {
  BarBarMem *self = BARBAR_MEM(data);

  glibtop_mem mem;

  glibtop_get_mem(&mem);

  self->percent =
      (1.0 - ((double)(mem.total - mem.user)) / ((double)mem.total));

  g_object_notify_by_pspec(G_OBJECT(self), mem_props[MEM_PROP_PERCENT]);
  return G_SOURCE_CONTINUE;
}

static void g_barbar_mem_start(BarBarSensor *sensor) {
  BarBarMem *mem = BARBAR_MEM(sensor);
  if (mem->source_id > 0) {
    g_source_remove(mem->source_id);
  }
  g_barbar_mem_update(mem);
  mem->source_id =
      g_timeout_add_full(0, mem->interval, g_barbar_mem_update, mem, NULL);
}
