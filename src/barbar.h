#ifndef __BARBAR_H__
#define __BARBAR_H__

#include "barbar-background.h"
#include "barbar-bar.h"
#include "barbar-util.h"
#include "sensors/barbar-backlight.h"
#include "sensors/barbar-battery.h"
#include "sensors/barbar-clock.h"
#include "sensors/barbar-cmd.h"
#include "sensors/barbar-cpu.h"
#include "sensors/barbar-disk.h"
#include "sensors/barbar-inhibitor.h"
#include "sensors/barbar-keyboard.h"
#include "sensors/barbar-mem.h"
#include "sensors/barbar-mpris.h"
#include "sensors/barbar-network.h"
#include "sensors/barbar-pomodoro.h"
#include "sensors/barbar-power-profiles.h"
#include "sensors/barbar-sensor.h"
#include "sensors/barbar-sensorcontext.h"
#include "sensors/barbar-systemd-units.h"
#include "sensors/barbar-temperature.h"
#include "sensors/barbar-weather.h"
#include "sensors/barbar-wireplumber.h"

#include "river/barbar-river-layout.h"
#include "river/barbar-river-mode.h"
#include "river/barbar-river-tags.h"
#include "river/barbar-river-view.h"

#include "hyprland/barbar-hyprland-workspace.h"

#include "widgets/barbar-graph.h"
#include "widgets/barbar-label.h"
#include "widgets/barbar-rotary.h"
#include "widgets/barbar-sensorwidget.h"

#include "tray/barbar-dbusmenu.h"
#include "tray/barbar-tray-item.h"
#include "tray/barbar-watcher.h"

void g_barbar_init(void);

#endif /* __BARBAR_H__ */
