#include <gtk4-layer-shell.h>
#include <gtk/gtk.h>
#include "barbar-bar.h"
#include "barbar-battery.h"
#include "barbar-clock.h"
#include "barbar-cpu.h"
#include "barbar-disk.h"
#include "barbar-mpd.h"

int
main (int argc, char **argv)
{
	// BarBarBar *bar = g_barbar_bar_new ();
	BarBarDisk *disk = g_object_new(BARBAR_TYPE_DISK, "path", "/", NULL);
	g_barbar_disk_update(disk);

	BarBarMpd *mpd = g_object_new(BARBAR_TYPE_MPD, NULL);
	g_barbar_mpd_update(mpd);

	BarBarBattery *bat = g_object_new(BARBAR_TYPE_BATTERY, NULL);
	g_barbar_battery_update(bat);

	BarBarClock *clock = g_object_new(BARBAR_TYPE_CLOCK, NULL);
	g_barbar_clock_update(clock);

	BarBarCpu *cpu = g_object_new(BARBAR_TYPE_CPU, NULL);
	g_barbar_cpu_update(cpu);
	// int status = g_barbar_run(bar, argc, argv);
    // return status;
    return 0;
}
