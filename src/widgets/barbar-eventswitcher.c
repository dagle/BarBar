#include "barbar-eventswitcher.h"
#include "gio/gio.h"
#include "glib-object.h"

/**
 * BarBarEventSwitcher:
 *
 * A stack switcher that uses events
 */
struct _BarBarEventSwitcher {
  GtkWidget parent_instance;

  guint index;
  GtkStack *stack;
};

enum {
  PROP_0,

  PROP_CHILD,
  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

static GtkBuildableIface *parent_buildable_iface;

static void
g_barbarevent_switcher_buildable_interface_init(GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE(
    BarBarEventSwitcher, g_barbar_event_switcher, GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                          g_barbarevent_switcher_buildable_interface_init))

void g_barbar_event_switcher_set_stack(BarBarEventSwitcher *self,
                                       GtkStack *stack) {
  // ref/unref stuff?
  self->stack = GTK_STACK(stack);
  gtk_widget_set_parent(GTK_WIDGET(self->stack), GTK_WIDGET(self));
}

static void g_barbar_event_switcher_add_child(GtkBuildable *buildable,
                                              GtkBuilder *builder,
                                              GObject *child,
                                              const char *type) {
  BarBarEventSwitcher *self = BARBAR_EVENT_SWITCHER(buildable);

  if (GTK_IS_STACK(child)) {
    g_barbar_event_switcher_set_stack(self, GTK_STACK(child));
  } else {
    parent_buildable_iface->add_child(buildable, builder, child, type);
  }
}

static void
g_barbarevent_switcher_buildable_interface_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_event_switcher_add_child;
}
static void g_barbar_event_switcher_set_property(GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_event_switcher_get_property(GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {

  BarBarEventSwitcher *switcher = BARBAR_EVENT_SWITCHER(object);

  switch (property_id) {
  case PROP_CHILD:
    g_value_set_object(value, switcher->stack);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

// static void g_barbar_event_switcher_constructed(GtkWidget *self) {
//   BarBarEventSwitcher *switcher = BARBAR_EVENT_SWITCHER(self);
//   GTK_WIDGET_CLASS(g_barbar_event_switcher_parent_class)->root(self);
// }
//
static void
g_barbar_event_switcher_class_init(BarBarEventSwitcherClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_event_switcher_set_property;
  gobject_class->get_property = g_barbar_event_switcher_get_property;
  // widget_class->root = g_barbar_event_switcher_constructed;

  /**
   * BarBarEventSwitcher:child:
   *
   * The child stack of the eventswitcher.
   */
  properties[PROP_CHILD] = g_param_spec_object(
      "child", NULL, NULL, GTK_TYPE_STACK, G_PARAM_READABLE);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "eventswitcher");
}

static void g_barbar_event_switcher_init(BarBarEventSwitcher *self) {}

void g_barbar_event_switcher_next(BarBarEventSwitcher *switcher, gint n_press,
                                  gdouble x, gdouble y, GtkGestureClick *self) {

  GtkSelectionModel *pages;

  pages = gtk_stack_get_pages(switcher->stack);
  guint num = g_list_model_get_n_items(G_LIST_MODEL(pages));
  switcher->index = (switcher->index + 1) % num;
  gtk_selection_model_select_item(pages, switcher->index, TRUE);
}

void g_barbar_event_switcher_previous(BarBarEventSwitcher *switcher,
                                      gint n_press, gdouble x, gdouble y,
                                      GtkGestureClick *self) {
  GtkSelectionModel *pages;

  pages = gtk_stack_get_pages(switcher->stack);

  if (switcher->index <= 0) {
    guint num = g_list_model_get_n_items(G_LIST_MODEL(pages));
    switcher->index = num - 1;
  } else {
    switcher->index--;
  }
  gtk_selection_model_select_item(pages, switcher->index, TRUE);
}

void g_barbar_event_switcher_select(BarBarEventSwitcher *switcher,
                                    guint position, gint n_press, gdouble x,
                                    gdouble y, GtkGestureClick *self) {
  GtkSelectionModel *pages;
  pages = gtk_stack_get_pages(switcher->stack);

  gtk_selection_model_select_item(pages, switcher->index, TRUE);
}

GtkWidget *g_barbar_event_switcher_new(void) {
  BarBarEventSwitcher *switcher;

  switcher = g_object_new(BARBAR_TYPE_EVENT_SWITCHER, NULL);

  return GTK_WIDGET(switcher);
}
