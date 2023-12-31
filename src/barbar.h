#ifndef __BARBAR_H__
#define __BARBAR_H__

#include "sensors/barbar-backlight.h"
#include "sensors/barbar-battery.h"
#include "sensors/barbar-clock.h"
#include "sensors/barbar-cmd.h"
#include "sensors/barbar-cpu.h"
#include "sensors/barbar-disk.h"
#include "sensors/barbar-mem.h"
#include "sensors/barbar-mpris2.h"
#include "sensors/barbar-network.h"
#include "sensors/barbar-sensor.h"
#include "sensors/barbar-temperature.h"

#include "river/barbar-river-tags.h"
#include "river/barbar-river-view.h"
/* #include "sensors/barbar-wireplumber.h" */

void g_barbar_init(void);

#endif /* __BARBAR_H__ */
