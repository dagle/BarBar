#include "barbar.h"
#include "barbar-output-manager.h"
#include "widgets/barbar-cava.h"
#include <glibtop.h>

void g_barbar_init(void) {
  glibtop_init();
  g_barbar_bar_get_type();
  g_barbar_background_get_type();
  g_barbar_position_get_type();
  g_barbar_playback_status_get_type();
  g_barbar_loop_status_get_type();
  g_barbar_sensor_get_type();
  g_barbar_interval_sensor_get_type();
  g_barbar_sensor_context_get_type();
  g_barbar_clock_get_type();
  g_barbar_mem_get_type();
  g_barbar_swap_get_type();
  g_barbar_cpu_get_type();
  g_barbar_backlight_get_type();
  g_barbar_battery_get_type();
  g_barbar_inhibitor_get_type();
  g_barbar_keyboard_get_type();
  g_barbar_pomodoro_get_type();
  g_barbar_power_profile_get_type();
  g_barbar_bus_type_get_type();
  g_barbar_systemd_units_get_type();
  g_barbar_game_mode_get_type();
  g_barbar_weather_get_type();
  g_barbar_filesystem_get_type();
  g_barbar_uptime_get_type();
  g_barbar_disk_get_type();

  g_barbar_rotary_get_type();
  g_barbar_graph_get_type();
  g_barbar_bar_graph_get_type();
  g_barbar_interval_graph_get_type();
  g_barbar_box_get_type();
  g_barbar_activity_graph_get_type();
  g_barbar_label_get_type();
  g_barbar_sensor_widget_get_type();
  g_barbar_value_icon_get_type();
  g_barbar_cpu_processes_get_type();
  g_barbar_event_switcher_get_type();
  g_barbar_play_button_get_type();
  g_barbar_cava_get_type();

  g_barbar_github_activity_get_type();

  g_barbar_wireplumber_get_type();
  g_barbar_mpris_player_get_type();
  g_barbar_temperature_get_type();
  g_barbar_network_get_type();
  g_barbar_cmd_get_type();

  g_barbar_river_tag_get_type();
  g_barbar_river_view_get_type();
  g_barbar_river_layout_get_type();
  g_barbar_river_mode_get_type();

  g_barbar_sway_subscribe_get_type();
  g_barbar_sway_language_get_type();
  g_barbar_sway_workspace_get_type();
  g_barbar_sway_window_get_type();
  g_barbar_sway_mode_get_type();

  g_barbar_niri_subscribe_get_type();
  g_barbar_niri_language_get_type();
  g_barbar_niri_workspace_get_type();
  g_barbar_niri_window_get_type();

  g_barbar_hyprland_service_get_type();
  g_barbar_hyprland_workspace_get_type();
  g_barbar_hyprland_window_get_type();

  g_barbar_dwl_service_get_type();
  g_barbar_dwl_tags_get_type();
  g_barbar_dwl_title_get_type();
  g_barbar_dwl_layout_get_type();

  g_barbar_dwl_tags_ipc_get_type();
  g_barbar_dwl_title_ipc_get_type();
  g_barbar_dwl_layout_ipc_get_type();

  g_barbar_output_manager_get_type();
  g_barbar_output_head_get_type();

  // g_barbar_dbus_menu_get_type();
  // g_barbar_status_watcher_get_type();
  // g_barbar_tray_item_get_type();
  // g_barbar_tray_get_type();
}
