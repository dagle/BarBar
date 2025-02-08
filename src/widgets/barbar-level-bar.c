#include "barbar-level-bar.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"

#include <math.h>
#include <stdlib.h>

enum {
  PROP_VALUE = 1,
  NUM_PROPERTIES,
};

enum { SIGNAL_OFFSET_CHANGED, NUM_SIGNALS };

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

/**
 * BarBarLevelBar:
 *
 * A level bar that works like the gtk level bar but with some additional
 * features. It adds features like shrink history (history between a high and a
 * low value)
 *
 */
struct _BarBarLevelBar {
  GtkWidget parent_instance;

  GtkWidget *overlay;
  GtkLevelBar *bar;

  guint inverted : 1;

  guint interval;
  guint source_id;

  // start_point
};

static GtkBuildableIface *parent_buildable_iface;

static void g_barbar_level_bar_buildable_init(GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE(
    BarBarLevelBar, g_barbar_level_bar, GTK_TYPE_WIDGET,
    // G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
    // G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE_RANGE, NULL)
    G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                          g_barbar_level_bar_buildable_init))

static void g_barbar_level_bar_add_child(GtkBuildable *buildable,
                                         GtkBuilder *builder, GObject *child,
                                         const char *type) {
  g_return_if_fail(GTK_IS_WIDGET(child));

  BarBarLevelBar *self = BARBAR_LEVEL_BAR(buildable);

  if (g_strcmp0(type, "bar") == 0) {
    g_barbar_level_bar_set_bar(self, GTK_LEVEL_BAR(child));
  } else {
    parent_buildable_iface->add_child(buildable, builder, child, type);
  }
}

static void g_barbar_level_bar_buildable_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_level_bar_add_child;
}

void g_barbar_level_bar_set_bar(BarBarLevelBar *self, GtkLevelBar *bar) {
  g_return_if_fail(BARBAR_IS_LEVEL_BAR(self));

  if (self->bar == bar) {
    return;
  }
  // do something

  g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VALUE]);
}

static void g_barbar_level_bar_class_init(BarBarLevelBarClass *class) {}

static void g_barbar_level_bar_init(BarBarLevelBar *class) {}

static gboolean g_barbar_level_bar_update(gpointer data) {
  BarBarLevelBar *self = BARBAR_LEVEL_BAR(data);

  // if (self->value >= self->historic) {
  //   return G_SOURCE_CONTINUE;
  // }

  // self->historic

  return G_SOURCE_CONTINUE;
}

static void g_barbar_level_bar_start(GtkWidget *widget) {
  gboolean ret;
  BarBarLevelBar *self = BARBAR_LEVEL_BAR(widget);

  if (self->source_id > 0) {
    g_source_remove(self->source_id);
  }

  self->source_id = g_timeout_add_full(0, self->interval,
                                       g_barbar_level_bar_update, self, NULL);
}
