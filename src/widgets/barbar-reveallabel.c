#include "barbar-reveallabel.h"
#include "glib-object.h"
#include "gtk/gtk.h"

struct _BarBarRevealLabel {
  GtkWidget parent_instance;

  char *str;

  guint max_length;

  GtkLabel *label;
  GtkRevealer *revealer;
  // double current_pos;
  GtkRevealerTransitionType transition_type;
  guint transition_duration;

  double current_pos;
  double source_pos;
  double target_pos;

  guint tick_id;
  GtkEventController *hover;
};

enum {
  PROP_0,

  PROP_LABEL,
  PROP_TRANSITION_TYPE,
  PROP_TRANSITION_DURATION,
  PROP_REVEAL_CHILD,
  PROP_CHILD_REVEALED,
  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarRevealLabel, g_barbar_reveal_label, GTK_TYPE_WIDGET)

void g_barbar_reveal_label_set_str(BarBarRevealLabel *revealer,
                                   const char *str) {

  g_return_if_fail(BARBAR_IS_REVEAL_LABEL(revealer));

  if (str == NULL) {
    gtk_label_set_label(revealer->label, NULL);

    gtk_label_set_label(revealer->label, NULL);
  }
}

/**
 * g_barbar_reveal_label_set_label: (attributes
 * org.gtk.Method.set_property=child)
 * @revealer: a `GtkRevealer`
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @revealer.
 */
void g_barbar_reveal_label_set_label(BarBarRevealLabel *revealer,
                                     GtkLabel *label) {
  g_return_if_fail(BARBAR_IS_REVEAL_LABEL(revealer));
  g_return_if_fail(label == NULL || revealer->label == label);

  if (revealer->label == label)
    return;

  g_clear_pointer(&revealer->label, gtk_widget_unparent);

  if (label) {
    gtk_widget_set_parent(GTK_WIDGET(label), GTK_WIDGET(revealer));
    // gtk_widget_set_child_visible(child, revealer->current_pos != 0.0);
    revealer->label = label;
  }

  g_object_notify_by_pspec(G_OBJECT(revealer), properties[PROP_LABEL]);
}

static void g_barbar_reveal_label_set_property(GObject *object,
                                               guint property_id,
                                               const GValue *value,
                                               GParamSpec *pspec) {
  BarBarRevealLabel *label = BARBAR_REVEAL_LABEL(object);

  switch (property_id) {
  case PROP_LABEL:
    g_barbar_reveal_label_set_label(label, g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_reveal_label_get_property(GObject *object,
                                               guint property_id, GValue *value,
                                               GParamSpec *pspec) {
  BarBarRevealLabel *label = BARBAR_REVEAL_LABEL(object);

  switch (property_id) {
  case PROP_LABEL:
    g_value_set_object(value, label->label);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void expand(GtkEventControllerMotion *self, gdouble x, gdouble y,
                   gpointer user_data) {
  BarBarRevealLabel *label = BARBAR_REVEAL_LABEL(user_data);

  // label
}

void collapse(GtkEventControllerMotion *self, gpointer user_data) {
  BarBarRevealLabel *label = BARBAR_REVEAL_LABEL(user_data);
}

static void g_barbar_reveal_label_class_init(BarBarRevealLabelClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  object_class->set_property = g_barbar_reveal_label_set_property;
  object_class->get_property = g_barbar_reveal_label_get_property;

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}

static void g_barbar_reveal_label_init(BarBarRevealLabel *self) {
  self->hover = gtk_event_controller_motion_new();
  g_signal_connect(self->hover, "enter", G_CALLBACK(expand), self);
  g_signal_connect(self->hover, "leave", G_CALLBACK(collapse), self);
}
