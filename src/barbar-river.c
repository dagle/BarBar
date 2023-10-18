#include "barbar-river.h"
#include "river-status-unstable-v1-client-protocol.h"

struct _BarBarRiver {
  GObject parent;

};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarRiver, g_barbar_river, G_TYPE_OBJECT)

static GParamSpec *river_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_river_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
}

static void g_barbar_river_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
}

static guint click_signal;

static void g_barbar_river_class_init(BarBarRiverClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_river_set_property;
  gobject_class->get_property = g_barbar_river_get_property;
  river_props[PROP_DEVICE] = g_param_spec_string(
      "path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, river_props);

  /* TODO: */
  click_signal = g_signal_new ("click-signal",
		  G_TYPE_FROM_CLASS (class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		  0 /* class offset.Subclass cannot override the class handler (default handler). */,
		  NULL /* accumulator */,
		  NULL /* accumulator data */,
		  NULL /* C marshaller. g_cclosure_marshal_generic() will be used */,
		  G_TYPE_NONE /* return_type */,
		  0     /* n_params */
		  );
}

static void g_barbar_river_init(BarBarRiver *self) {
}

void g_barbar_river_update(BarBarRiver *river) {
	
}
