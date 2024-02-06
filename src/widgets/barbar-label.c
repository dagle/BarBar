/*
 * Copyright © 2024 Per Odlund
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * BarBarLabel:
 *
 * This is a gtk label with some extras. The main function is the
 * the template functionallity to update the code.
 */

#include "barbar-label.h"
#include <tmpl-glib.h>
struct _BarBarLabel {
  GtkWidget parent_instance;

  GtkWidget *child;
  char *templ;
  TmplTemplate *tmpl;
};

G_DEFINE_TYPE(BarBarLabel, g_barbar_label, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_TEMPL,
  PROP_LABEL,
  PROP_CHILD,

  NUM_PROPERTIES,
};

static GParamSpec *label_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_label_set_templ(BarBarLabel *label, const char *templ) {
  g_return_if_fail(BARBAR_IS_LABEL(label));
  GError *error = NULL;

  g_free(label->templ);
  if (!templ) {
    label->templ = NULL;
    g_object_notify_by_pspec(G_OBJECT(label), label_props[PROP_TEMPL]);
    return;
  }

  label->templ = g_strdup(templ);

  label->tmpl = tmpl_template_new(NULL);

  if (!tmpl_template_parse_string(label->tmpl, label->templ, &error)) {
    g_printerr("Label: Error building template: %s\n", error->message);
    g_error_free(error);
  }
}

G_MODULE_EXPORT void label_update(GtkWidget *object, GObject *sensor) {
  const gchar *className = g_type_name(G_TYPE_FROM_INSTANCE(sensor));
  GError *error = NULL;
  BarBarLabel *label = BARBAR_LABEL(object);
  char *str;
  g_autoptr(TmplScope) scope = NULL;

  TmplSymbol *symbol = NULL;
  scope = tmpl_scope_new();

  symbol = tmpl_scope_get(scope, className);
  tmpl_symbol_assign_object(symbol, sensor);

  if (!(str = tmpl_template_expand_string(label->tmpl, scope, &error))) {
    g_printerr("Label: Error expanding the template: %s\n", error->message);
    return;
  }

  gtk_label_set_text(GTK_LABEL(label->child), str);
  g_free(str);
}

static void g_barbar_label_set_label(BarBarLabel *label, const char *text) {
  g_return_if_fail(BARBAR_IS_LABEL(label));
  return;

  g_object_notify_by_pspec(G_OBJECT(label), label_props[PROP_LABEL]);
}

static void g_barbar_label_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarLabel *label = BARBAR_LABEL(object);

  switch (property_id) {
  case PROP_TEMPL:
    g_barbar_label_set_templ(label, g_value_get_string(value));
    break;
  case PROP_LABEL:
    g_barbar_label_set_label(label, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_label_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {

  BarBarLabel *label = BARBAR_LABEL(object);
  switch (property_id) {
  case PROP_CHILD:
    g_value_set_object(value, label->child);
    break;
  case PROP_TEMPL:
    g_value_set_string(value, label->templ);
    break;
  case PROP_LABEL: {
    const char *str = gtk_label_get_text(GTK_LABEL(label->child));
    g_value_set_string(value, str);
    break;
  }
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_label_class_init(BarBarLabelClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_label_set_property;
  gobject_class->get_property = g_barbar_label_get_property;

  /**
   * BarBarLabel:templ:
   *
   * A template string used when a sensors is updated
   */
  label_props[PROP_TEMPL] =
      g_param_spec_string("templ", "Template", "Template string", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarLabel:label:
   *
   * The label string
   */
  label_props[PROP_LABEL] =
      g_param_spec_string("label", "Label", "The formated label", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarLabel:child:
   *
   * Child widget
   */
  label_props[PROP_CHILD] = g_param_spec_object(
      "child", "Child", "Child label", GTK_TYPE_LABEL, G_PARAM_READABLE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, label_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "barbar-label");
}
static void g_barbar_label_init(BarBarLabel *label) {
  GError *error = NULL;
  label->child = gtk_label_new("");

  gtk_widget_set_parent(label->child, GTK_WIDGET(label));
}
