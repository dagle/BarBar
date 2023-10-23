#include <gtk4-layer-shell.h>
#include <gtk/gtk.h>
#include "barbar-bar.h"
#include "barbar-battery.h"
#include "barbar-clock.h"
#include "barbar-cpu.h"
#include "barbar-disk.h"
#include "barbar-mpd.h"
#include "barbar-mpris2.h"
#include "barbar-river.h"
#include "barbar-temperature.h"
#include "barbar-wireplumber.h"

#include "status-notifier.h"

static gboolean reg_host(StatusNotifierWatcher* obj, GDBusMethodInvocation* invocation,
                                     const gchar* service, gpointer user_data) {
	const gchar *sender = g_dbus_method_invocation_get_sender(invocation);
	g_print("HOST: %s - %s\n", service, sender);
	return TRUE;
}
static gboolean reg_item(StatusNotifierWatcher* obj, GDBusMethodInvocation* invocation,
                                     const gchar* service, gpointer user_data) {
	g_print("ITEM\n");
	return TRUE;
}

void g_barbar_status_watcher_bus_acquired_handler2 (
  GDBusConnection* connection,
  const gchar* name,
  gpointer user_data
) {
	GError *error = NULL;

	StatusNotifierWatcher *watcher;
	watcher = status_notifier_watcher_skeleton_new();

	g_signal_connect_swapped(watcher, "handle-register-host",
                           G_CALLBACK(reg_host), NULL);
	g_signal_connect_swapped(watcher, "handle-register-item",
                           G_CALLBACK(reg_item), NULL);
	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(watcher), connection, "/StatusNotifierWatcher", &error);
}

int
main (int argc, char **argv)
{
	BarBarDisk *disk = g_object_new(BARBAR_TYPE_DISK, NULL);
	g_barbar_disk_update(disk);

	// BarBarMpd *mpd = g_object_new(BARBAR_TYPE_MPD, NULL);
	// g_barbar_mpd_update(mpd);

	BarBarBattery *bat = g_object_new(BARBAR_TYPE_BATTERY, NULL);
	g_barbar_battery_update(bat);

	BarBarClock *clock = g_object_new(BARBAR_TYPE_CLOCK, "tz", "Europe/Stockholm", NULL);
	g_barbar_clock_update(clock);

	BarBarCpu *cpu = g_object_new(BARBAR_TYPE_CPU, NULL);
	g_barbar_cpu_update(cpu);

	// BarBarWireplumber *wp = g_object_new(BARBAR_TYPE_WIREPLUMBER, NULL);
	// g_barbar_wireplumber_update(wp);

	BarBarMpris *mpris = g_object_new(BARBAR_TYPE_MPRIS, "player", "mpd", NULL);
	g_barbar_mpris_update(mpris);

	// TODO: we can't can't test this one until we actually make things into widgets!

	// BarBarRiver *river = g_object_new(BARBAR_TYPE_RIVER, NULL);
	// g_barbar_river_update(river);

	BarBarTemperature *temp = g_object_new(BARBAR_TYPE_TEMPERATURE, NULL);
	g_barbar_temperature_update(temp);

	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

    g_bus_own_name(G_BUS_TYPE_SESSION,
						   "org.kde.StatusNotifierWatcher",
						   G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT | G_BUS_NAME_OWNER_FLAGS_REPLACE,
						   NULL,
						   g_barbar_status_watcher_bus_acquired_handler2,
						   NULL,
						   NULL,
						   NULL);
	g_main_loop_run(loop);

	// BarBarBar *bar = g_barbar_bar_new ();
	// int status = g_barbar_run(bar, argc, argv);
 //    return status;
	return 0;
}
