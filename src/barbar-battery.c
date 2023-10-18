#include "barbar-battery.h"
#include <libupower-glib/upower.h>
#include <stdio.h>

struct _BarBarBattery {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  char *device;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

// use upower
G_DEFINE_TYPE(BarBarBattery, g_barbar_battery, G_TYPE_OBJECT)

static GParamSpec *battery_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_battery_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
}

static void g_barbar_battery_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
}

static void g_barbar_battery_class_init(BarBarBatteryClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_battery_set_property;
  gobject_class->get_property = g_barbar_battery_get_property;
  battery_props[PROP_DEVICE] = g_param_spec_string(
      "device", NULL, NULL, NULL, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, battery_props);
}

static void g_barbar_battery_init(BarBarBattery *self) {
}

void g_barbar_battery_update(BarBarBattery *battery) {
	UpClient *client = up_client_new ();
	GDBusConnection *login1_connection;
	GError *error;
	guint login1_id;

    UpDevice *dev = up_client_get_display_device(client);
	char *str = up_device_to_text(dev);
	printf("UpDevice: %s\n", str);
	g_object_unref(dev);
	g_object_unref(client);
	// GPtrArray* newDevices = up_client_get_devices2(client);
	// for (guint i = 0; i < newDevices->len; i++) {
	// 	UpDevice* device = (UpDevice*)g_ptr_array_index(newDevices, i);
	// 	if (device && G_IS_OBJECT(device)) {
	// 		
	// 	}
	// }
	
	// TODO: add this

	// login1_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	// if (!login1_connection) {
		// throw std::runtime_error("Unable to connect to the SYSTEM Bus!...");
	// } else {
		// login1_id = g_dbus_connection_signal_subscribe(
		// 		login1_connection, "org.freedesktop.login1", "org.freedesktop.login1.Manager",
		// 		"PrepareForSleep", "/org/freedesktop/login1", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
		// 		prepareForSleep_cb, this, NULL);
	// }

	// event_box_.signal_button_press_event().connect(sigc::mem_fun(*this, &UPower::handleToggle));

	// g_signal_connect(client, "device-added", G_CALLBACK(deviceAdded_cb), this);
	// g_signal_connect(client, "device-removed", G_CALLBACK(deviceRemoved_cb), this);
}
