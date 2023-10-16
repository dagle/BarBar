#include <gtk4-layer-shell.h>
#include <gtk/gtk.h>
#include "barbar-bar.h"

int
main (int argc, char **argv)
{
	BarBarBar *bar = g_barbar_bar_new ();
	int status = g_barbar_run(bar, argc, argv);
    return status;
}
