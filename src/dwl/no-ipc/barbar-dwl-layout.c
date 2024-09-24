#include "barbar-dwl-layout.h"
#include "barbar-dwl-service.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarDwlLayout:
 * A widget to display the layout for the associated screen.
 *
 * See `BarBarDwlService` for how BarBarDwlLayout fetches information
 */
struct _BarBarDwlLayout {
  GtkWidget parent_instance;

  char *output_name;
  BarBarDwlService *service;

  GtkWidget *label;
};

enum {
  PROP_0,

  PROP_OUTPUT,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarDwlLayout, g_barbar_dwl_layout, GTK_TYPE_WIDGET)

static GParamSpec *dwl_layout_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_layout_start(GtkWidget *widget);

static void g_barbar_dwl_layout_set_output(BarBarDwlLayout *dwl,
                                           const gchar *output) {
  g_return_if_fail(BARBAR_IS_DWL_LAYOUT(dwl));

  g_free(dwl->output_name);

  dwl->output_name = g_strdup(output);
  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_layout_props[PROP_OUTPUT]);
}

static void g_barbar_dwl_layout_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {
  BarBarDwlLayout *dwl = BARBAR_DWL_LAYOUT(object);
  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_dwl_layout_set_output(dwl, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_layout_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec) {
  BarBarDwlLayout *dwl = BARBAR_DWL_LAYOUT(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_layout_finalize(GObject *object) {
  BarBarDwlLayout *dwl = BARBAR_DWL_LAYOUT(object);

  g_free(dwl->output_name);
  g_clear_object(&dwl->service);
  G_OBJECT_CLASS(g_barbar_dwl_layout_parent_class)->finalize(object);
}

static void g_barbar_dwl_layout_class_init(BarBarDwlLayoutClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_layout_set_property;
  gobject_class->get_property = g_barbar_dwl_layout_get_property;
  gobject_class->finalize = g_barbar_dwl_layout_finalize;

  widget_class->root = g_barbar_dwl_layout_start;

  dwl_layout_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, "WL-1", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dwl_layout_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "dwl-layout");
}

static void g_dwl_listen_cb(BarBarDwlService *service, char *output_name,
                            char *layout, gpointer data) {
  BarBarDwlLayout *dwl = BARBAR_DWL_LAYOUT(data);

  if (!g_strcmp0(dwl->output_name, output_name)) {
    gtk_label_set_text(GTK_LABEL(dwl->label), layout);
  }
}

static void g_barbar_dwl_layout_init(BarBarDwlLayout *self) {
  self->label = gtk_label_new("");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

static void g_barbar_dwl_layout_start(GtkWidget *widget) {
  BarBarDwlLayout *dwl = BARBAR_DWL_LAYOUT(widget);

  GTK_WIDGET_CLASS(g_barbar_dwl_layout_parent_class)->root(widget);

  dwl->service = g_barbar_dwl_service_new(NULL);
  g_barbar_sensor_start(BARBAR_SENSOR(dwl->service));
  g_signal_connect(dwl->service, "layout", G_CALLBACK(g_dwl_listen_cb), dwl);
}
