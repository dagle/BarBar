#include "barbar-step-bar.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "widgets/barbar-level-bar.h"

/**
 * BarBarStepBar:
 *
 * StepBar is just like a normal bar but normalize
 * does some normalization. The bar is either colored or not,
 * if not the continuous is set. Meaning that if the bar is
 * set at 25%, the bar will either be colored 0% or 25%, nothing
 * inbetween.
 *
 */

struct _BarBarStepBar {
  GtkWindow parent_instance;

  double value;
  double procentage;
  double treshold;
  GtkLevelBarMode mode;

  GtkWidget *bar;
};

enum {
  PROP_VALUE = 1,
  PROP_PERCENTAGE,
  PROP_BAR,
  PROP_MODE,
  LAST_PROPERTY,
};

static GParamSpec *step_bar_props[LAST_PROPERTY] = {
    NULL,
};

G_DEFINE_TYPE(BarBarStepBar, g_barbar_step_bar, GTK_TYPE_WIDGET)

static void g_barbar_step_bar_get_property(GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec) {
  BarBarStepBar *bar = BARBAR_STEP_BAR(object);

  switch (property_id) {
  case PROP_VALUE:
    g_value_set_double(value, bar->value);
    break;
  case PROP_PERCENTAGE:
    g_value_set_double(value, bar->procentage);
    break;
  case PROP_MODE:
    g_value_set_enum(value, bar->mode);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static inline double treshold(double upper, double lower, double percentage) {
  return lower + (upper - lower) * percentage;
}

static void g_barbar_step_bar_set_value_internal(BarBarStepBar *self,
                                                 double value) {

  self->value = value;

  if (GTK_IS_LEVEL_BAR(self->bar)) {
    GtkLevelBar *bar = GTK_LEVEL_BAR(self->bar);
    double upper = gtk_level_bar_get_max_value(bar);
    double lower = gtk_level_bar_get_min_value(bar);
    double t = treshold(upper, lower, self->procentage);

    if (value >= t) {
      gtk_level_bar_set_value(bar, t);
    } else {
      gtk_level_bar_set_value(bar, 0);
    }
  } else if (BARBAR_IS_LEVEL_BAR(self->bar)) {
  }

  g_object_notify_by_pspec(G_OBJECT(self), step_bar_props[PROP_VALUE]);
}

/**
 * g_barbar_step_bar_set_value_set_value:
 * @self: a `BarBarStepBar`
 * @value: a value
 *
 * Sets the value of the `GtkLevelBar`.
 */
void g_barbar_step_bar_set_value(BarBarStepBar *self, double value) {
  g_return_if_fail(BARBAR_IS_STEP_BAR(self));

  if (value == self->value) {
    return;
  }

  g_barbar_step_bar_set_value_internal(self, value);
}

/**
 * g_barbar_step_bar_set_mode:
 * @self: a `BarBarStepBar`
 * @mode: a `GtkLevelBarMode`
 *
 * Sets the `mode` of the `BarBarStepBar`.
 */
void g_barbar_step_bar_set_mode(BarBarStepBar *self, GtkLevelBarMode mode) {
  g_return_if_fail(BARBAR_IS_LEVEL_BAR(self));

  if (self->mode == mode)
    return;

  self->mode = mode;

  g_barbar_step_bar_set_value_internal(self, self->value);
  g_object_notify_by_pspec(G_OBJECT(self), step_bar_props[PROP_MODE]);
}

/**
 * g_barbar_step_bar_set_percentage:
 * @self: a `BarBarStepBar`
 * @percentage: a percentage
 *
 * Sets the `percentage` of the `BarBarLevelBar`.
 */
void g_barbar_step_bar_set_percentage(BarBarStepBar *self, double percentage) {
  g_return_if_fail(BARBAR_IS_STEP_BAR(self));

  if (self->procentage == percentage) {
    return;
  }

  self->procentage = percentage;

  g_barbar_step_bar_set_value_internal(self, self->value);
  g_object_notify_by_pspec(G_OBJECT(self), step_bar_props[PROP_PERCENTAGE]);
}

/**
 * g_barbar_step_bar_set_bar:
 * @self: a `BarBarStepBar`
 * @mode: a `GtkLevelBarMode`
 *
 * Sets the `mode` of the `BarBarLevelBar`.
 */
void g_barbar_step_bar_set_bar(BarBarStepBar *self, GtkWidget *widget) {
  g_return_if_fail(BARBAR_IS_STEP_BAR(self));

  if (self->bar == widget) {
    return;
  }

  g_return_if_fail(BARBAR_IS_LEVEL_BAR(widget) || GTK_IS_LEVEL_BAR(widget));

  if (self->bar) {
    gtk_widget_unparent(self->bar);
  }

  self->bar = widget;

  if (self->bar) {
    gtk_widget_set_parent(self->bar, GTK_WIDGET(self));
  }

  g_barbar_step_bar_set_value_internal(self, self->value);
  g_object_notify_by_pspec(G_OBJECT(self), step_bar_props[PROP_BAR]);
}

static void g_barbar_step_bar_set_property(GObject *object, guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec) {
  BarBarStepBar *bar = BARBAR_STEP_BAR(object);

  switch (property_id) {
  case PROP_VALUE:
    g_barbar_step_bar_set_value(bar, g_value_get_double(value));
    break;
  case PROP_PERCENTAGE:
    g_barbar_step_bar_set_percentage(bar, g_value_get_double(value));
    break;
  case PROP_BAR:
    g_barbar_step_bar_set_bar(bar, g_value_get_object(value));
    break;
  case PROP_MODE:
    g_barbar_step_bar_set_mode(bar, g_value_get_enum(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_step_bar_class_init(BarBarStepBarClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_step_bar_set_property;
  gobject_class->get_property = g_barbar_step_bar_get_property;

  /**
   * BarBarStepBar:value:
   *
   * The current value, used to set the bar.
   */
  step_bar_props[PROP_VALUE] = g_param_spec_double(
      "value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarStepBar:percentage:
   *
   * The percentage to normalize the value
   */
  step_bar_props[PROP_PERCENTAGE] = g_param_spec_double(
      "percentage", NULL, NULL, 0, 1, 1,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarStepBar:bar:
   *
   * The internal bar
   */
  step_bar_props[PROP_BAR] = g_param_spec_object(
      "bar", NULL, NULL, GTK_TYPE_LEVEL_BAR,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarStepBar:mode:
   *
   * If the last colored bar should be colored full or depend on the value.
   * Example: say we have 4 bars with the levels: 0, 0.25, 0.5, 0.75 and 1.0
   * If the value is 0.85 and we are i continuous mode, then we fill the first 3
   * to thex max and the last one to 0.85%, if in descrete mode we will just
   * round the value and fill the bars fully.
   */
  step_bar_props[PROP_MODE] = g_param_spec_enum(
      "mode", NULL, NULL, GTK_TYPE_LEVEL_BAR_MODE,
      GTK_LEVEL_BAR_MODE_CONTINUOUS,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties(gobject_class, LAST_PROPERTY,
                                    step_bar_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "step-bar");
}

static void g_barbar_step_bar_init(BarBarStepBar *class) {}

GtkWidget *g_barbar_step_bar_new(void) {
  return g_object_new(BARBAR_TYPE_STEP_BAR, NULL);
}
