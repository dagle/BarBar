#include "barbar.h"

void g_barbar_init(void) {
  // lets init like this now
  g_barbar_sensor_get_type();
  g_barbar_clock_get_type();
  g_barbar_mem_get_type();
  g_barbar_cpu_get_type();
  // g_barbar_battery_get_type();
  g_barbar_network_get_type();
  g_barbar_river_tag_get_type();
  g_barbar_river_view_get_type();
}
