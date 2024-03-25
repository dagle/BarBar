#include "barbar-keyboard.h"
#include <gtk/gtk.h>

/**
 * BarBarKeyboard:
 * A sensor to read state of the keyboard
 */
struct _BarBarKeyboard {
  BarBarSensor parent_instance;

  GdkDevice *device;
};

enum {
  PROP_0,

  PROP_CAPSLOCK,
  PROP_NUMLOCK,
  PROP_SCROLLLOCK,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarKeyboard, g_barbar_keyboard, BARBAR_TYPE_SENSOR);

static GParamSpec *keyboard_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_keyboard_start(BarBarSensor *sensor);

static void g_barbar_keyboard_set_property(GObject *object, guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec) {}

static void g_barbar_keyboard_get_property(GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec) {
  BarBarKeyboard *keyboard = BARBAR_KEYBOARD(object);

  if (!keyboard->device) {
    g_value_set_boolean(value, FALSE);
    return;
  }

  switch (property_id) {
  case PROP_CAPSLOCK:
    g_value_set_boolean(value,
                        gdk_device_get_caps_lock_state(keyboard->device));
    break;
  case PROP_NUMLOCK:
    g_value_set_boolean(value, gdk_device_get_num_lock_state(keyboard->device));
    break;
  case PROP_SCROLLLOCK:
    g_value_set_boolean(value,
                        gdk_device_get_scroll_lock_state(keyboard->device));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_keyboard_class_init(BarBarKeyboardClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_keyboard_set_property;
  gobject_class->get_property = g_barbar_keyboard_get_property;
  sensor->start = g_barbar_keyboard_start;

  /**
   * BarBarKeyboard:caps-lock:
   *
   * The name of the player we want to watch
   *
   */
  keyboard_props[PROP_CAPSLOCK] =
      g_param_spec_boolean("caps-lock", NULL, NULL, FALSE, G_PARAM_READABLE);

  /**
   * BarBarKeyboard:num-lock:
   *
   * The name of the player we want to watch
   *
   */
  keyboard_props[PROP_NUMLOCK] =
      g_param_spec_boolean("num-lock", NULL, NULL, FALSE, G_PARAM_READABLE);

  /**
   * BarBarKeyboard:scroll-lock:
   *
   * The name of the player we want to watch
   *
   */
  keyboard_props[PROP_SCROLLLOCK] =
      g_param_spec_boolean("scroll-lock", NULL, NULL, FALSE, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    keyboard_props);
}

static void g_barbar_keyboard_init(BarBarKeyboard *class) {}

// TODO: can update the device dynamically?
static void g_barbar_keyboard_start(BarBarSensor *sensor) {
  BarBarKeyboard *keyboard = BARBAR_KEYBOARD(sensor);

  GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());

  keyboard->device = gdk_seat_get_keyboard(seat);
}
