#include "barbar-seperator.h"

struct _BarBarSeparator {
  GObject parent_instance;

  GtkOrientation orientation;
};

// enum {
//   PROP_0,
//
//   PROP_INTEREST,
//
//   NUM_PROPERTIES,
// };

enum { PROP_0, PROP_ORIENTATION };

G_DEFINE_TYPE(BarBarSeparator, g_barbar_separator, GTK_TYPE_WIDGET)

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

static void barbar_separator_set_property(GObject *object, guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  BarBarSeparator *separator = BARBAR_SEPARATOR(object);

  switch (prop_id) {
  case PROP_ORIENTATION:
    gtk_separator_set_orientation(separator, g_value_get_enum(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void barbar_separator_get_property(GObject *object, guint prop_id,
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

static void gtk_separator_init(BarBarSeparator *separator) {
  separator->orientation = GTK_ORIENTATION_HORIZONTAL;

  // gtk_widget_update_orientation(GTK_WIDGET(separator),
  // separator->orientation);
}

static void gtk_separator_class_init(BarBarSeparatorClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  object_class->set_property = barbar_separator_set_property;
  object_class->get_property = barbar_separator_get_property;

  g_object_class_override_property(object_class, PROP_ORIENTATION,
                                   "orientation");

  gtk_widget_class_set_css_name(widget_class, "separator");
  gtk_widget_class_set_accessible_role(widget_class,
                                       GTK_ACCESSIBLE_ROLE_SEPARATOR);
}
