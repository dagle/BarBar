#include "barbar-separator.h"

/**
 * BarBarSeparator:
 *
 * A seperator that almost works like the gtk version of separator
 * but acts greedy in the sense that it tries to take up as much space as
 * possible.
 */
struct _BarBarSeparator {
  GtkWidget parent_instance;

  GtkOrientation orientation;
};

enum { PROP_0, PROP_ORIENTATION };

G_DEFINE_TYPE_WITH_CODE(BarBarSeparator, g_barbar_separator, GTK_TYPE_WIDGET,
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_ORIENTABLE, NULL))

static void barbar_separator_set_orientation(BarBarSeparator *self,
                                             GtkOrientation orientation) {
  if (self->orientation != orientation) {
    self->orientation = orientation;

    // gtk_widget_update_orientation(GTK_WIDGET(self), orientation);
    gtk_widget_queue_resize(GTK_WIDGET(self));

    gtk_accessible_update_property(GTK_ACCESSIBLE(self),
                                   GTK_ACCESSIBLE_PROPERTY_ORIENTATION,
                                   orientation, -1);

    g_object_notify(G_OBJECT(self), "orientation");
  }
}

static void g_barbar_separator_set_property(GObject *object, guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarSeparator *separator = BARBAR_SEPARATOR(object);

  switch (prop_id) {
  case PROP_ORIENTATION:
    barbar_separator_set_orientation(separator, g_value_get_enum(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void g_barbar_separator_get_property(GObject *object, guint prop_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarSeparator *separator = BARBAR_SEPARATOR(object);

  switch (prop_id) {
  case PROP_ORIENTATION:
    g_value_set_enum(value, separator->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void g_barbar_separator_measure(GtkWidget *widget,
                                       GtkOrientation orientation, int for_size,
                                       int *minimum, int *natural,
                                       int *minimum_baseline,
                                       int *natural_baseline) {

  BarBarSeparator *self = BARBAR_SEPARATOR(widget);
  if (for_size != -1 && (self->orientation == orientation)) {
    *minimum = *natural = for_size;
  } else {
    *minimum = *natural = 1;
  }
}

static void g_barbar_separator_class_init(BarBarSeparatorClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  object_class->set_property = g_barbar_separator_set_property;
  object_class->get_property = g_barbar_separator_get_property;

  widget_class->measure = g_barbar_separator_measure;

  g_object_class_override_property(object_class, PROP_ORIENTATION,
                                   "orientation");

  gtk_widget_class_set_css_name(widget_class, "barbar-separator");
  gtk_widget_class_set_accessible_role(widget_class,
                                       GTK_ACCESSIBLE_ROLE_SEPARATOR);
}

static void g_barbar_separator_init(BarBarSeparator *separator) {
  separator->orientation = GTK_ORIENTATION_HORIZONTAL;
}
