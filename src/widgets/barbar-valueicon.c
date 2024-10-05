#include "barbar-valueicon.h"

/**
 * BarBarValueIcon:
 *
 * ValueIcon is a set of icons that change bepending on the
 * value. Think of it like what scalebutton does but without the scale.
 */
struct _BarBarValueIcon {
  GtkWidget parent_instance;

  double upper;
  double lower;
  double value;

  GtkWidget *image;

  char **icon_list;
};

G_DEFINE_TYPE(BarBarValueIcon, g_barbar_value_icon, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_ICONS,
  PROP_UPPER,
  PROP_LOWER,
  PROP_VALUE,

  NUM_PROPERTIES,
};

static GParamSpec *icons_props[NUM_PROPERTIES] = {
    NULL,
};

static void barbar_value_icon_constructed(GObject *object);

static void g_barbar_value_icon_update_icon(BarBarValueIcon *icons) {
  guint num_icons;
  const char *name;
  double value;

  if (!icons->icon_list || !icons->icon_list[0] ||
      icons->icon_list[0][0] == '\0') {
    gtk_image_set_from_icon_name(GTK_IMAGE(icons->image), "image-missing");
    return;
  }

  num_icons = g_strv_length(icons->icon_list);

  /* The 1-icon special case */
  if (num_icons == 1) {
    gtk_image_set_from_icon_name(GTK_IMAGE(icons->image), icons->icon_list[0]);
    return;
  }

  value = icons->value;

  /* The 2-icons special case */
  if (num_icons == 2) {
    double limit;

    limit = (icons->upper - icons->lower) / 2 + icons->lower;
    if (value < limit)
      name = icons->icon_list[0];
    else
      name = icons->icon_list[1];

    gtk_image_set_from_icon_name(GTK_IMAGE(icons->image), name);
    return;
  }

  /* With 3 or more icons */
  if (value == icons->lower) {
    name = icons->icon_list[0];
  } else if (value == icons->upper) {
    name = icons->icon_list[1];
  } else {
    double step;
    guint i;

    step = (icons->upper - icons->lower) / (num_icons - 2);
    i = (guint)((value - icons->lower) / step) + 2;
    g_assert(i < num_icons);
    name = icons->icon_list[i];
  }

  gtk_image_set_from_icon_name(GTK_IMAGE(icons->image), name);
}

void g_barbar_value_icon_set_icons(BarBarValueIcon *icons, const char **names) {
  char **tmp;
  g_return_if_fail(BARBAR_IS_VALUE_ICON(icons));

  tmp = icons->icon_list;
  icons->icon_list = g_strdupv((char **)names);
  g_strfreev(tmp);

  g_barbar_value_icon_update_icon(icons);
  g_object_notify_by_pspec(G_OBJECT(icons), icons_props[PROP_ICONS]);
}

/**
 * g_barbar_value_icon_set_upper:
 * @icons: a `BarBarValueIcon`
 * @upper: upper treshold
 *
 * Sets the upper value which control when the value will swap
 */
void g_barbar_value_icon_set_upper(BarBarValueIcon *icons, double upper) {
  g_return_if_fail(BARBAR_IS_VALUE_ICON(icons));
  if (icons->upper == upper) {
    return;
  }

  icons->upper = upper;
  g_object_notify_by_pspec(G_OBJECT(icons), icons_props[PROP_UPPER]);
}

/**
 * g_barbar_value_icon_set_lower:
 * @icons: a `BarBarValueIcon`
 * @lower: lower treshold
 *
 * Sets the lower value which control when the value will swap
 */
void g_barbar_value_icon_set_lower(BarBarValueIcon *icons, double lower) {
  g_return_if_fail(BARBAR_IS_VALUE_ICON(icons));
  if (icons->lower == lower) {
    return;
  }

  icons->lower = lower;

  g_object_notify_by_pspec(G_OBJECT(icons), icons_props[PROP_LOWER]);
}

/**
 * g_barbar_value_icon_set_value:
 * @icons: a `BarBarValueIcon`
 * @value: current value treshold
 *
 * Sets the current value
 */
void g_barbar_value_icon_set_value(BarBarValueIcon *icons, double value) {
  g_return_if_fail(BARBAR_IS_VALUE_ICON(icons));
  if (icons->value == value) {
    return;
  }

  icons->value = value;
  // calculate the current icon

  g_object_notify_by_pspec(G_OBJECT(icons), icons_props[PROP_VALUE]);
}

static void g_barbar_value_icon_set_property(GObject *object, guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec) {
  BarBarValueIcon *icons = BARBAR_VALUE_ICON(object);

  switch (property_id) {
  case PROP_UPPER:
    g_barbar_value_icon_set_upper(icons, g_value_get_double(value));
    break;
  case PROP_LOWER:
    g_barbar_value_icon_set_lower(icons, g_value_get_double(value));
    break;
  case PROP_VALUE:
    g_barbar_value_icon_set_value(icons, g_value_get_double(value));
    break;
  case PROP_ICONS:
    g_barbar_value_icon_set_icons(icons,
                                  (const char **)g_value_get_boxed(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_value_icon_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec) {

  BarBarValueIcon *icons = BARBAR_VALUE_ICON(object);

  switch (property_id) {
  case PROP_UPPER:
    g_value_set_double(value, icons->upper);
    break;
  case PROP_LOWER:
    g_value_set_double(value, icons->lower);
    break;
  case PROP_VALUE:
    g_value_set_double(value, icons->value);
    break;
  // case PROP_ICONS:
  //   g_barbar_value_icon_set_icons(icons,
  //                                 (const char **)g_value_get_boxed(value));
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_value_icon_class_init(BarBarValueIconClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_value_icon_set_property;
  gobject_class->get_property = g_barbar_value_icon_get_property;
  gobject_class->constructed = barbar_value_icon_constructed;

  /**
   * BarBarValueIcon:icons:
   *
   * The names of the icons to be used by the value button.
   *
   * The first item in the array will be used in the button
   * when the current value is the lowest value, the second
   * item for the highest value. All the subsequent icons will
   * be used for all the other values, spread evenly over the
   * range of values.
   *
   * If there's only one icon name in the @icons array, it will
   * be used for all the values. If only two icon names are in
   * the @icons array, the first one will be used for the bottom
   * 50% of the scale, and the second one for the top 50%.
   *
   */
  icons_props[PROP_ICONS] = g_param_spec_boxed(
      "icons", NULL, NULL, G_TYPE_STRV, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarValueIcon:upper:
   *
   * Upper value of the value. By befault 1.0.
   */
  icons_props[PROP_UPPER] =
      g_param_spec_double("upper", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarValueIcon:lower:
   *
   * Upper value of the value. By befault 0.0.
   */
  icons_props[PROP_LOWER] =
      g_param_spec_double("lower", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  /**
   * BarBarValueIcon:value:
   *
   * Value reflecting the icon choice
   */
  icons_props[PROP_VALUE] =
      g_param_spec_double("value", NULL, NULL, -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, icons_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "barbar-value-icon");
}

static void barbar_value_icon_constructed(GObject *object) {
  BarBarValueIcon *icons = BARBAR_VALUE_ICON(object);

  G_OBJECT_CLASS(g_barbar_value_icon_parent_class)->constructed(object);

  g_barbar_value_icon_update_icon(icons);
}

static void g_barbar_value_icon_init(BarBarValueIcon *icons) {
  icons->image = gtk_image_new();

  gtk_widget_set_parent(icons->image, GTK_WIDGET(icons));
}

/**
 * g_barbar_value_icon_get_upper:
 * @icons: a `BarBarValueIcon`
 *
 * Returns: the upper value
 */
double g_barbar_value_icon_get_upper(BarBarValueIcon *icons) {
  return icons->upper;
}

/**
 * g_barbar_value_icon_get_lower:
 * @icons: a `BarBarValueIcon`
 *
 * Returns: the lower value
 */
double g_barbar_value_icon_get_lower(BarBarValueIcon *icons) {
  return icons->lower;
}

/**
 * g_barbar_value_icon_get_value:
 * @icons: a `BarBarValueIcon`
 *
 * Returns: the current value
 */
double g_barbar_value_icon_get_value(BarBarValueIcon *icons) {
  return icons->value;
}
/**
 * g_barbar_value_icon_new:
 *
 * Returns: (transfer full): a `BarBarValueIcon`
 */
GtkWidget *g_barbar_value_icon_new(char **icons) {
  return g_object_new(BARBAR_TYPE_VALUE_ICON, "icons", icons);
}
