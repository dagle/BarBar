#ifndef __BARBAR_H__
#define __BARBAR_H__

#include "barbar-background.h"
#include "barbar-bar.h"
#include "sensors/barbar-backlight.h"
#include "sensors/barbar-battery.h"
#include "sensors/barbar-clock.h"
#include "sensors/barbar-cmd.h"
#include "sensors/barbar-cpu.h"
#include "sensors/barbar-disk.h"
#include "sensors/barbar-inhibitor.h"
#include "sensors/barbar-mem.h"
#include "sensors/barbar-mpris.h"
#include "sensors/barbar-network.h"
#include "sensors/barbar-sensor.h"
#include "sensors/barbar-temperature.h"
#include "sensors/barbar-wireplumber.h"

#include "river/barbar-river-tags.h"
#include "river/barbar-river-view.h"

#include "hyprland/barbar-hyprland-workspace.h"

#include "widgets/barbar-label.h"

void g_barbar_init(void);

#endif /* __BARBAR_H__ */
