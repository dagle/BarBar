#include "barbar-level-bar.h"

#include <math.h>
#include <stdlib.h>

enum {
  PROP_VALUE = 1,
  PROP_MIN_VALUE,
  PROP_MAX_VALUE,
  PROP_MODE,
  PROP_INVERTED,
  LAST_PROPERTY,
  PROP_ORIENTATION /* overridden */
};

enum { SIGNAL_OFFSET_CHANGED, NUM_SIGNALS };

static GParamSpec *properties[LAST_PROPERTY] = {
    NULL,
};
static guint signals[NUM_SIGNALS] = {
    0,
};

// typedef struct _GtkLevelBarClass   GtkLevelBarClass;

// typedef struct {
//   char *name;
//   double value;
// } GtkLevelBarOffset;

struct _BarBarLevelBar {
  GtkWidget parent_instance;

  GtkOrientation orientation;

  GtkLevelBarMode bar_mode;

  double min_value;
  double max_value;
  double cur_value;

  GList *offsets;

  GtkWidget *trough_widget;
  GtkWidget **block_widget;
  guint n_blocks;

  guint inverted : 1;

  // start_point
};

// struct _GtkLevelBarClass {
//   GtkWidgetClass parent_class;
//
//   void (* offset_changed) (GtkLevelBar *self,
//                            const char *name);
// };

// static void g_barbar_level_bar_set_value_internal(GtkLevelBar *self, double
// value);

static void g_barbar_level_bar_buildable_init(GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE(
    BarBarLevelBar, g_barbar_level_bar, GTK_TYPE_WIDGET,
    // G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
    // G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE_RANGE, NULL)
    G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                          g_barbar_level_bar_buildable_init))

static void g_barbar_level_bar_class_init(BarBarLevelBarClass *class) {}

static void g_barbar_level_bar_init(BarBarLevelBar *class) {}
