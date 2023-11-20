#include "dwl/barbar-dwl-tags.h"
#include "dwl/barbar-dwl-service.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

struct _BarBarDwlTag {
  GtkWidget parent_instance;

  BarBarDwlService *service;

  GtkWidget *buttons[9];
};

enum {
  PROP_0,

  PROP_TAGNUMS,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarDwlTag, g_barbar_dwl_tag, GTK_TYPE_WIDGET)

static GParamSpec *river_tags_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_tag_constructed(GObject *object);
static void default_clicked_handler(BarBarDwlTag *dwl, guint tag,
                                    gpointer user_data);

static void g_barbar_dwl_tag_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {}

static void g_barbar_dwl_tag_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {}

static guint click_signal;

static void g_barbar_dwl_tag_class_init(BarBarDwlTagClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_tag_set_property;
  gobject_class->get_property = g_barbar_dwl_tag_get_property;
  gobject_class->constructed = g_barbar_dwl_tag_constructed;
  river_tags_props[PROP_TAGNUMS] =
      g_param_spec_uint("tagnums", NULL, NULL, 0, 9, 9, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    river_tags_props);

  click_signal = g_signal_new_class_handler(
      "clicked", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
      G_CALLBACK(default_clicked_handler), NULL, NULL, NULL, G_TYPE_NONE, 1,
      G_TYPE_UINT);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "river-tag");
}

static void clicked(GtkButton *self, gpointer user_data) {
  BarBarDwlTag *dwl = BARBAR_DWL_TAG(user_data);

  guint tag;
  for (int i = 0; i < 9; i++) {
    if (self == GTK_BUTTON(dwl->buttons[i])) {
      tag = i;
    }
  }
  tag = 1 << tag;

  g_signal_emit(dwl, click_signal, 0, tag);
}

static void default_clicked_handler(BarBarDwlTag *dwl, guint tag,
                                    gpointer user_data) {
  char buf[4];

  snprintf(buf, 4, "%d", tag);

  // zriver_control_v1_add_argument(river->control, "set-focused-tags");
  // zriver_control_v1_add_argument(river->control, buf);
  // callback = zriver_control_v1_run_command(river->control, river->seat);
  // zriver_command_callback_v1_add_listener(callback,
  // &command_callback_listener,
  //                                         NULL);
}

void g_dwl_listen_cb(BarBarDwlService *service, gpointer dwl_status,
                     gpointer data) {
  struct dwl_status *status = (struct dwl_status *)dwl_status;
  printf("dwl data\n");
}

static void g_barbar_dwl_tag_init(BarBarDwlTag *self) {}
static void g_barbar_dwl_tag_constructed(GObject *object) {
  GtkWidget *btn;
  BarBarDwlTag *dwl = BARBAR_DWL_TAG(object);
  char str[2];
  for (uint32_t i = 0; i < 9; i++) {
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label(str);
    // gtk_widget_set_name(btn, "tags");
    gtk_widget_set_parent(btn, GTK_WIDGET(dwl));
    // g_signal_connect(btn, "clicked", G_CALLBACK(clicked), dwl);
    dwl->buttons[i] = btn;
  }
}

void g_barbar_dwl_tag_start(BarBarDwlTag *dwl) {
  dwl->service = g_barbar_dwl_service_new();
  g_signal_connect(dwl->service, "listener", G_CALLBACK(g_dwl_listen_cb), dwl);
}
