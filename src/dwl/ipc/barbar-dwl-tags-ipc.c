#include "barbar-dwl-tags-ipc.h"
#include <gdk/wayland/gdkwayland.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

/**
 * BarBarDwlTagsIpc:
 * A widget to display the title for current application for the
 * associated screen.
 */
struct _BarBarDwlTagsIpc {
  GtkWidget parent_instance;

  char *output_name;
  uint nums;

  GtkWidget *buttons[32];
};

enum {
  PROP_0,

  PROP_OUTPUT,
  PROP_TAGNUMS,

  NUM_PROPERTIES,
};

static GtkBuildableIface *parent_buildable_iface;

static void
g_barbar_dwl_tags_ipc_buildable_interface_init(GtkBuildableIface *iface);
static void g_barbar_dwl_tags_add_button(BarBarDwlTagsIpc *self,
                                         GtkWidget *child);
static void g_barbar_dwl_tag_root(GtkWidget *widget);
static void default_clicked_handler(BarBarDwlTagsIpc *dwl, guint tag,
                                    gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(
    BarBarDwlTagsIpc, g_barbar_dwl_tags_ipc, GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                          g_barbar_dwl_tags_ipc_buildable_interface_init))

static GParamSpec *dwl_tags_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_dwl_tags_add_child(GtkBuildable *buildable,
                                        GtkBuilder *builder, GObject *child,
                                        const char *type) {
  g_return_if_fail(GTK_IS_WIDGET(child));

  BarBarDwlTagsIpc *self = BARBAR_DWL_TAGS_IPC(buildable);

  if (g_strcmp0(type, "tag") == 0) {
    g_barbar_dwl_tags_add_button(self, GTK_WIDGET(child));
  }
}

static void
g_barbar_dwl_tags_ipc_buildable_interface_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_dwl_tags_add_child;
}

static void g_barbar_dwl_tags_set_output(BarBarDwlTagsIpc *dwl,
                                         const gchar *output) {
  g_return_if_fail(BARBAR_IS_DWL_TAGS_IPC(dwl));

  g_free(dwl->output_name);

  dwl->output_name = g_strdup(output);
  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_tags_props[PROP_OUTPUT]);
}

static void g_barbar_dwl_tags_set_tagnums(BarBarDwlTagsIpc *dwl,
                                          guint tagnums) {
  g_return_if_fail(BARBAR_IS_DWL_TAGS_IPC(dwl));

  if (dwl->nums == tagnums) {
    return;
  }

  dwl->nums = tagnums;

  g_object_notify_by_pspec(G_OBJECT(dwl), dwl_tags_props[PROP_TAGNUMS]);
}

static void g_barbar_dwl_tag_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(object);
  switch (property_id) {
  case PROP_OUTPUT:
    g_barbar_dwl_tags_set_output(dwl, g_value_get_string(value));
    break;
  case PROP_TAGNUMS:
    g_barbar_dwl_tags_set_tagnums(dwl, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_tag_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(object);

  switch (property_id) {
  case PROP_TAGNUMS:
    g_value_set_uint(value, dwl->nums);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static guint click_signal;

static void g_barbar_dwl_tags_ipc_class_init(BarBarDwlTagsIpcClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_tag_set_property;
  gobject_class->get_property = g_barbar_dwl_tag_get_property;

  widget_class->root = g_barbar_dwl_tag_root;

  /**
   * BarBarDwlTagsIpc:output:
   *
   * What screen we want this be connected to.
   * This is because gtk4 not having support for
   * wl_output interface v4
   */
  dwl_tags_props[PROP_OUTPUT] = g_param_spec_string(
      "output", NULL, NULL, "WL-1", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBarDwlTagsIpc:tagnums:
   *
   * How many tags we should show
   */
  dwl_tags_props[PROP_TAGNUMS] = g_param_spec_uint(
      "tagnums", NULL, NULL, 0, 32, 9,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  click_signal = g_signal_new("clicked", BARBAR_TYPE_DWL_TAGS_IPC,
                              G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL,
                              NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dwl_tags_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "dwl-tag");
}

static void clicked(GtkButton *self, gpointer user_data) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(user_data);

  guint tag;
  for (int i = 0; i < 9; i++) {
    if (self == GTK_BUTTON(dwl->buttons[i])) {
      tag = i;
    }
  }
  tag = 1 << tag;

  g_signal_emit(dwl, click_signal, 0, tag);
}

static void default_clicked_handler(BarBarDwlTagsIpc *dwl, guint tag,
                                    gpointer user_data) {
  char buf[4];

  snprintf(buf, 4, "%d", tag);
  // TODO: change to the tag on send
}

static void handle_occupied(BarBarDwlTagsIpc *dwl, uint32_t occupied) {
  for (size_t id = 0; id < dwl->nums; ++id) {
    uint32_t mask = 1 << id;

    if (mask & occupied) {
      gtk_widget_add_css_class(dwl->buttons[id], "occupied");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "occupied");
    }
  }
}
static void handle_selected(BarBarDwlTagsIpc *dwl, uint32_t selected) {
  for (size_t id = 0; id < dwl->nums; ++id) {
    uint32_t mask = 1 << id;

    if (mask & selected) {
      gtk_widget_add_css_class(dwl->buttons[id], "focused");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "focused");
    }
  }
}

static void handle_urgent(BarBarDwlTagsIpc *dwl, uint32_t urgent) {
  for (size_t id = 0; id < dwl->nums; ++id) {
    uint32_t mask = 1 << id;

    if (mask & urgent) {
      gtk_widget_add_css_class(dwl->buttons[id], "urgent");
    } else {
      gtk_widget_remove_css_class(dwl->buttons[id], "urgent");
    }
  }
}

// static void g_dwl_listen_cb(BarBarDwlService *service, char *output_name,
//                             guint occupied, guint selected, guint
//                             client_tags, guint urgent, gpointer data) {
//   BarBarDwlTags *dwl = BARBAR_DWL_TAGS(data);
//
//   if (!g_strcmp0(dwl->output_name, output_name)) {
//     handle_occupied(dwl, occupied);
//     handle_selected(dwl, selected);
//     handle_urgent(dwl, urgent);
//   }
// }
static void g_barbar_dwl_tags_add_button(BarBarDwlTagsIpc *self,
                                         GtkWidget *child) {
  GtkWidget *btn;

  uint32_t i = 0;
  for (; i < 32; i++) {
    if (self->buttons[i]) {
      continue;
    }
    btn = gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(btn), child);
    gtk_widget_set_parent(btn, GTK_WIDGET(self));
    g_signal_connect(btn, "clicked", G_CALLBACK(clicked), self);
    self->buttons[i] = btn;
    break;
  }
  i++;
  if (self->nums < i) {
    self->nums = i;
    g_object_notify_by_pspec(G_OBJECT(self), dwl_tags_props[PROP_TAGNUMS]);
  }
}

static void g_barbar_dwl_tags_ipc_init(BarBarDwlTagsIpc *self) {}

static void g_barbar_dwl_tags_defaults(BarBarDwlTagsIpc *self) {
  GtkWidget *btn;
  char str[2];
  for (uint32_t i = 0; i < self->nums; i++) {
    if (self->buttons[i]) {
      continue;
    }
    sprintf(str, "%d", i + 1);
    btn = gtk_button_new_with_label(str);
    gtk_widget_set_parent(btn, GTK_WIDGET(self));
    g_signal_connect(btn, "clicked", G_CALLBACK(clicked), self);
    self->buttons[i] = btn;
  }
}

static void g_barbar_dwl_tag_root(GtkWidget *widget) {
  BarBarDwlTagsIpc *dwl = BARBAR_DWL_TAGS_IPC(widget);

  GTK_WIDGET_CLASS(g_barbar_dwl_tags_ipc_parent_class)->root(widget);
  g_barbar_dwl_tags_defaults(dwl);

  // dwl->service = g_barbar_dwl_service_new(NULL);
  // g_barbar_sensor_start(BARBAR_SENSOR(dwl->service));
  // g_signal_connect(dwl->service, "tags", G_CALLBACK(g_dwl_listen_cb), dwl);
}
