#pragma once

#include "barbar-background.h"
#include "barbar-bar.h"
#include "barbar-enum.h"
#include "barbar-util.h"
#include "sensors/barbar-backlight.h"
#include "sensors/barbar-battery.h"
#include "sensors/barbar-clock.h"
#include "sensors/barbar-cmd.h"
#include "sensors/barbar-cpu.h"
#include "sensors/barbar-disk.h"
#include "sensors/barbar-filesystem.h"
#include "sensors/barbar-gamemode.h"
#include "sensors/barbar-inhibitor.h"
#include "sensors/barbar-interval-sensor.h"
#include "sensors/barbar-keyboard.h"
#include "sensors/barbar-mem.h"
#include "sensors/barbar-network.h"
#include "sensors/barbar-pomodoro.h"
#include "sensors/barbar-power-profiles.h"
#include "sensors/barbar-sensor.h"
#include "sensors/barbar-sensorcontext.h"
#include "sensors/barbar-swap.h"
#include "sensors/barbar-systemd-units.h"
#include "sensors/barbar-temperature.h"
#include "sensors/barbar-uptime.h"
#include "sensors/barbar-weather.h"
#include "sensors/barbar-wireplumber.h"
#include "sensors/mpris/barbar-mpris-player.h"

#include "river/barbar-river-layout.h"
#include "river/barbar-river-mode.h"
#include "river/barbar-river-tags.h"
#include "river/barbar-river-view.h"

#include "hyprland/barbar-hyprland-service.h"
#include "hyprland/barbar-hyprland-window.h"
#include "hyprland/barbar-hyprland-workspace.h"

#include "sway/barbar-sway-language.h"
#include "sway/barbar-sway-mode.h"
#include "sway/barbar-sway-subscribe.h"
#include "sway/barbar-sway-window.h"
#include "sway/barbar-sway-workspace.h"

#include "niri/barbar-niri-ipc.h"
#include "niri/barbar-niri-language.h"
#include "niri/barbar-niri-subscribe.h"
#include "niri/barbar-niri-window.h"
#include "niri/barbar-niri-workspace.h"

#include "dwl/no-ipc/barbar-dwl-layout.h"
#include "dwl/no-ipc/barbar-dwl-service.h"
#include "dwl/no-ipc/barbar-dwl-tags.h"
#include "dwl/no-ipc/barbar-dwl-title.h"

#include "dwl/ipc/barbar-dwl-layout-ipc.h"
#include "dwl/ipc/barbar-dwl-tags-ipc.h"
#include "dwl/ipc/barbar-dwl-title-ipc.h"

#include "widgets/barbar-activity-graph.h"
#include "widgets/barbar-box.h"
#include "widgets/barbar-cava.h"
#include "widgets/barbar-eventswitcher.h"
#include "widgets/barbar-graph.h"
#include "widgets/barbar-interval-graph.h"
#include "widgets/barbar-label.h"
#include "widgets/barbar-playbutton.h"
#include "widgets/barbar-processes.h"
#include "widgets/barbar-rotary.h"
#include "widgets/barbar-sensorwidget.h"
#include "widgets/barbar-valueicon.h"

#include "barbar-output-head.h"
#include "barbar-output-manager.h"
#include "widgets/barbar-github-activity.h"

/*#include "tray/barbar-dbusmenu.h"*/
/*#include "tray/barbar-tray-item.h"*/
/*#include "tray/barbar-tray.h"*/
/*#include "tray/barbar-watcher.h"*/

void g_barbar_init(void);
