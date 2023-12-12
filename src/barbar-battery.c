#include "barbar-battery.h"
#include <libupower-glib/upower.h>
#include <stdio.h>

struct _BarBarBattery {
  GtkWidget parent_instance;

  GtkWidget *label;
  guint interval;

  char *device;
  guint source_id;
  UpClient *client;
  UpDevice *dev;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

#define DEFAULT_INTERVAL 1000

// use upower
G_DEFINE_TYPE(BarBarBattery, g_barbar_battery, GTK_TYPE_WIDGET)

static GParamSpec *battery_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_battery_constructed(GObject *self);

static void g_barbar_battery_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {}

static void g_barbar_battery_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {}

static void g_barbar_battery_class_init(BarBarBatteryClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_battery_set_property;
  gobject_class->get_property = g_barbar_battery_get_property;
  gobject_class->constructed = g_barbar_battery_constructed;
  battery_props[PROP_DEVICE] =
      g_param_spec_string("device", NULL, NULL, NULL, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    battery_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "battery");
}

static void g_barbar_battery_constructed(GObject *self) {
  // BarBarBattery *battery = BARBAR_BATTERY(self);
}

void g_barbar_battery_start(BarBarBattery *mem, gpointer data);

static void g_barbar_battery_init(BarBarBattery *self) {
  self->label = gtk_label_new("");
  self->interval = DEFAULT_INTERVAL;

  gtk_widget_set_parent(self->label, GTK_WIDGET(self));

  g_signal_connect(self, "map", G_CALLBACK(g_barbar_battery_start), NULL);
}

static void g_barbar_battery_up(UpDevice *dev, BarBarBattery *battery) {

  double d;
  g_object_get(dev, "percentage", &d, NULL);

  gchar *str = g_strdup_printf("%.0f%%", d);
  gtk_label_set_label(GTK_LABEL(battery->label), str);
  g_free(str);
}

static void g_barbar_battery_update(GObject *object, GParamSpec *pspec,
                                    gpointer data) {

  BarBarBattery *battery = BARBAR_BATTERY(data);

  g_barbar_battery_up(UP_DEVICE(object), battery);

  // GPtrArray* newDevices = up_client_get_devices2(client);
  // for (guint i = 0; i < newDevices->len; i++) {
  // 	UpDevice* device = (UpDevice*)g_ptr_array_index(newDevices, i);
  // 	if (device && G_IS_OBJECT(device)) {
  //
  // 	}
  // }

  // login1_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
  // if (!login1_connection) {
  // throw std::runtime_error("Unable to connect to the SYSTEM Bus!...");
  // } else {
  // login1_id = g_dbus_connection_signal_subscribe(
  // 		login1_connection, "org.freedesktop.login1",
  // "org.freedesktop.login1.Manager", 		"PrepareForSleep",
  // "/org/freedesktop/login1", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
  // 		prepareForSleep_cb, this, NULL);
  // }

  // event_box_.signal_button_press_event().connect(sigc::mem_fun(*this,
  // &UPower::handleToggle));

  // g_signal_connect(client, "device-added", G_CALLBACK(deviceAdded_cb), this);
  // g_signal_connect(client, "device-removed", G_CALLBACK(deviceRemoved_cb),
  // this);
}

void g_barbar_battery_setup_device(BarBarBattery *battery) {}

void g_barbar_battery_start(BarBarBattery *battery, gpointer data) {
  battery->client = up_client_new();
  battery->dev = up_client_get_display_device(battery->client);

  g_barbar_battery_up(battery->dev, battery);

  g_signal_connect(battery->dev, "notify", G_CALLBACK(g_barbar_battery_update),
                   battery);
}
