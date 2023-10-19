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

	// BarBarBar *bar = g_barbar_bar_new ();
	// int status = g_barbar_run(bar, argc, argv);
 //    return status;
	return 0;
}
