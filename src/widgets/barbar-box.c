#include "barbar-box.h"

/**
 * BarBarBox:
 *
 * A empty widget used to describe a box. It's mostly use to just
 */
struct _BarBarBox {
  GtkWidget parent_instance;

  uint size;
  uint level;

  // GtkOrientation orientation;
};

enum {
  PROP_0,

  PROP_LEVEL,
  PROP_SIZE,
  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarBox, g_barbar_box, GTK_TYPE_WIDGET)

// Maybe this is horrible design
void g_barbar_box_set_level(BarBarBox *box, uint value) {
  g_return_if_fail(BARBAR_IS_BOX(box));

  if (box->level == value) {
    return;
  }

  box->level = value;

  g_object_notify_by_pspec(G_OBJECT(box), properties[PROP_LEVEL]);
}

uint g_barbar_box_get_value(BarBarBox *box) { return box->level; }

static void g_barbar_box_measure(GtkWidget *widget, GtkOrientation orientation,
                                 int for_size, int *minimum, int *natural,
                                 int *minimum_baseline, int *natural_baseline) {
  BarBarBox *self = BARBAR_BOX(widget);
  if (for_size != -1) {
    // printf("for_size: %d\n", for_size);
    *minimum = *natural = for_size;
  } else {
    *minimum = *natural = self->size;
  }
}

static void g_barbar_box_set_size(BarBarBox *box, guint size) {
  g_return_if_fail(BARBAR_IS_BOX(box));

  if (box->size == size) {
    return;
  }

  box->size = size;

  g_object_notify_by_pspec(G_OBJECT(box), properties[PROP_SIZE]);
}

static void g_barbar_box_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {
  BarBarBox *box = BARBAR_BOX(object);

  switch (property_id) {
  case PROP_SIZE:
    g_barbar_box_set_size(box, g_value_get_uint(value));
    break;
  case PROP_LEVEL:
    g_barbar_box_set_level(box, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_box_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarBox *box = BARBAR_BOX(object);

  switch (property_id) {
  case PROP_SIZE:
    g_value_set_uint(value, box->size);
    break;
  case PROP_LEVEL:
    g_value_set_uint(value, box->level);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_box_class_init(BarBarBoxClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  object_class->set_property = g_barbar_box_set_property;
  object_class->get_property = g_barbar_box_get_property;

  widget_class->measure = g_barbar_box_measure;
  // widget_class->get_request_mode = g_barbar_rotary_get_request_mode;

  properties[PROP_SIZE] = g_param_spec_uint(
      "size", NULL, NULL, 1, G_MAXUINT, 2,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  properties[PROP_LEVEL] = g_param_spec_uint(
      "value", NULL, NULL, 0, G_MAXUINT, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, NUM_PROPERTIES, properties);

  gtk_widget_class_set_css_name(widget_class, "barbar-box");
}

static void g_barbar_box_init(BarBarBox *box) {}

GtkWidget *g_barbar_box_new(uint size) {
  BarBarBox *bar;

  bar = g_object_new(BARBAR_TYPE_BOX, "size", size, NULL);

  return GTK_WIDGET(bar);
}
