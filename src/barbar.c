#include "barbar.h"

void g_barbar_init(void) {
  // lets init like this now
  g_barbar_bar_get_type();
  g_barbar_background_get_type();
  g_barbar_position_get_type();
  g_barbar_sensor_get_type();
  barbar_sensor_context_get_type();
  g_barbar_clock_get_type();
  g_barbar_mem_get_type();
  g_barbar_cpu_get_type();
  g_barbar_backlight_get_type();
  g_barbar_battery_get_type();
  g_barbar_inhibitor_get_type();
  g_barbar_keyboard_get_type();
  g_barbar_pomodoro_get_type();
  g_barbar_power_profile_get_type();
  g_barbar_bus_type_get_type();
  g_barbar_systemd_units_get_type();
  g_barbar_rotary_get_type();
  g_barbar_graph_get_type();
  g_barbar_weather_get_type();

  g_barbar_wireplumber_get_type();
  g_barbar_mpris_get_type();
  g_barbar_label_get_type();
  g_barbar_sensor_widget_get_type();
  g_barbar_temperature_get_type();
  g_barbar_network_get_type();
  g_barbar_river_tag_get_type();
  g_barbar_river_view_get_type();
  g_barbar_river_layout_get_type();
  g_barbar_river_mode_get_type();
  g_barbar_cmd_get_type();
  g_barbar_hyprland_workspace_get_type();
  g_barbar_hyprland_window_get_type();

  g_barbar_dbus_menu_get_type();
  g_barbar_status_watcher_get_type();
  g_barbar_tray_item_get_type();
}
