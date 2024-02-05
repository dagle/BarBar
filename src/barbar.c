#include "barbar.h"

void g_barbar_init(void) {
  // lets init like this now
  g_barbar_bar_get_type();
  g_barbar_background_get_type();
  g_barbar_position_get_type();
  g_barbar_sensor_get_type();
  g_barbar_clock_get_type();
  g_barbar_mem_get_type();
  g_barbar_cpu_get_type();
  g_barbar_backlight_get_type();
  g_barbar_battery_get_type();
  g_barbar_inhibitor_get_type();
  // g_barbar_battery_get_type();
  // g_barbar_wireplumber_get_type();
  g_barbar_mpris_get_type();
  g_barbar_label_get_type();
  g_barbar_temperature_get_type();
  g_barbar_network_get_type();
  g_barbar_river_tag_get_type();
  g_barbar_river_view_get_type();
  g_barbar_cmd_get_type();
  g_barbar_hyprland_workspace_get_type();
}
