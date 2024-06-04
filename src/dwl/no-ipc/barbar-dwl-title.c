#include "barbar-dwl-title.h"
#include "barbar-dwl-service.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarDwlTitle:
 * A widget to display the title for current application for the
 * associated screen.
 */
struct _BarBarDwlTitle {
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

G_DEFINE_TYPE(BarBarDwlTitle, g_barbar_dwl_title, GTK_TYPE_WIDGET)

static GParamSpec *dwl_title_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_title_start(GtkWidget *widget);

static void g_barbar_dwl_title_set_output(BarBarDwlTitle *dwl,
                                          const gchar *output) {
  g_return_if_fail(BARBAR_IS_DWL_TITLE(dwl));

  g_free(dwl->output_name);

  dwl->output_name = g_strdup(output);
  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_title_props[PROP_OUTPUT]);
}

static void g_barbar_dwl_title_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarDwlTitle *dwl = BARBAR_DWL_TITLE(object);
  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_dwl_title_set_output(dwl, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_title_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarDwlTitle *dwl = BARBAR_DWL_TITLE(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_title_finalize(GObject *object) {
  BarBarDwlTitle *dwl = BARBAR_DWL_TITLE(object);

  g_free(dwl->output_name);
  g_clear_object(&dwl->service);
  G_OBJECT_CLASS(g_barbar_dwl_title_parent_class)->finalize(object);
}

static void g_barbar_dwl_title_class_init(BarBarDwlTitleClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_title_set_property;
  gobject_class->get_property = g_barbar_dwl_title_get_property;
  gobject_class->finalize = g_barbar_dwl_title_finalize;

  widget_class->root = g_barbar_dwl_title_start;

  /**
   * BarBarDwlTitle:output:
   *
   * What screen we want this be connected to.
   * This is because gtk4 not having support for
   * wl_output interface v4
   */
  dwl_title_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, "WL-1", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dwl_title_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "dwl-title");
}

static void g_dwl_listen_cb(BarBarDwlService *service, char *output_name,
                            char *title, gpointer data) {
  BarBarDwlTitle *dwl = BARBAR_DWL_TITLE(data);

  if (!g_strcmp0(dwl->output_name, output_name)) {
    gtk_label_set_text(GTK_LABEL(dwl->label), title);
  }
}

static void g_barbar_dwl_title_init(BarBarDwlTitle *self) {
  self->label = gtk_label_new("");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));
}

static void g_barbar_dwl_title_start(GtkWidget *widget) {
  BarBarDwlTitle *dwl = BARBAR_DWL_TITLE(widget);

  GTK_WIDGET_CLASS(g_barbar_dwl_title_parent_class)->root(widget);

  dwl->service = g_barbar_dwl_service_new(NULL);
  g_barbar_sensor_start(BARBAR_SENSOR(dwl->service));
  g_signal_connect(dwl->service, "title", G_CALLBACK(g_dwl_listen_cb), dwl);
}
