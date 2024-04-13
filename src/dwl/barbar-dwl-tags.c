#include "dwl/barbar-dwl-tags.h"
#include "dwl/barbar-dwl-service.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarDwlTags {
  GtkWidget parent_instance;

  char *output_name;
  BarBarDwlService *service;

  GtkWidget *buttons[9];
};

enum {
  PROP_0,

  PROP_OUTPUT,
  // PROP_TAGNUMS,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarDwlTags, g_barbar_dwl_tags, GTK_TYPE_WIDGET)

static GParamSpec *dwl_tags_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_tag_start(GtkWidget *widget);

static void g_barbar_dwl_tags_set_output(BarBarDwlTags *dwl,
                                         const gchar *output) {
  g_return_if_fail(BARBAR_IS_DWL_TAGS(dwl));

  g_free(dwl->output_name);

  dwl->output_name = g_strdup(output);
  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_tags_props[PROP_OUTPUT]);
}

static void default_clicked_handler(BarBarDwlTags *dwl, guint tag,
                                    gpointer user_data);

static void g_barbar_dwl_tag_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  BarBarDwlTags *dwl = BARBAR_DWL_TAGS(object);
  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_dwl_tags_set_output(dwl, g_value_get_string(value));
    break;
  // case PROP_TAGNUMS:
  // g_barbar_sway_workspace_set_output(sway, g_value_get_string(value));
  // break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_tag_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  BarBarDwlTags *dwl = BARBAR_DWL_TAGS(object);

  switch (property_id) {}
}

static guint click_signal;

static void g_barbar_dwl_tags_class_init(BarBarDwlTagsClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_tag_set_property;
  gobject_class->get_property = g_barbar_dwl_tag_get_property;

  widget_class->root = g_barbar_dwl_tag_start;

  // dwl_tags_props[PROP_TAGNUMS] =
  //     g_param_spec_uint("tagnums", NULL, NULL, 0, 9, 9, G_PARAM_READWRITE);
  dwl_tags_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, "WL-1", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dwl_tags_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "dwl-tag");
}

static void clicked(GtkButton *self, gpointer user_data) {
  BarBarDwlTags *dwl = BARBAR_DWL_TAGS(user_data);

  guint tag;
  for (int i = 0; i < 9; i++) {
    if (self == GTK_BUTTON(dwl->buttons[i])) {
      tag = i;
    }
  }
  tag = 1 << tag;

  g_signal_emit(dwl, click_signal, 0, tag);
}

static void default_clicked_handler(BarBarDwlTags *dwl, guint tag,
                                    gpointer user_data) {
  char buf[4];

  snprintf(buf, 4, "%d", tag);
}

static void handle_occupied(BarBarDwlTags *dwl, uint32_t occupied) {
  for (size_t id = 0; id < 9; ++id) {
    uint32_t mask = 1 << id;

    if (mask & occupied) {
      gtk_widget_add_css_class(dwl->buttons[id], "occupied");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "occupied");
    }
  }
}
static void handle_selected(BarBarDwlTags *dwl, uint32_t selected) {
  for (size_t id = 0; id < 9; ++id) {
    uint32_t mask = 1 << id;

    if (mask & selected) {
      gtk_widget_add_css_class(dwl->buttons[id], "focused");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "focused");
    }
  }
}

static void handle_urgent(BarBarDwlTags *dwl, uint32_t urgent) {
  for (size_t id = 0; id < 9; ++id) {
    uint32_t mask = 1 << id;

    if (mask & urgent) {
      gtk_widget_add_css_class(dwl->buttons[id], "urgent");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "urgent");
    }
  }
}

void g_dwl_listen_cb(BarBarDwlService *service, char *output_name,
                     guint occupied, guint selected, guint urgent,
                     gpointer data) {
  BarBarDwlTags *dwl = BARBAR_DWL_TAGS(data);

  if (!g_strcmp0(dwl->output_name, output_name)) {
    handle_occupied(dwl, occupied);
    handle_selected(dwl, selected);
    handle_urgent(dwl, urgent);
  }
}

static void g_barbar_dwl_tags_init(BarBarDwlTags *self) {
  GtkWidget *btn;
  char str[2];
  for (uint32_t i = 0; i < 9; i++) {
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label(str);
    gtk_widget_set_parent(btn, GTK_WIDGET(self));
    self->buttons[i] = btn;
  }
}

static void g_barbar_dwl_tag_start(GtkWidget *widget) {
  BarBarDwlTags *dwl = BARBAR_DWL_TAGS(widget);

  GTK_WIDGET_CLASS(g_barbar_dwl_tags_parent_class)->root(widget);

  dwl->service = g_barbar_dwl_service_new("/home/dagle/apa.txt");
  g_signal_connect(dwl->service, "tags", G_CALLBACK(g_dwl_listen_cb), dwl);
}
