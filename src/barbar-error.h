#include <glib.h>

typedef enum {
  BARBAR_ERROR_COMPOSITOR,
  BARBAR_ERROR_BAD_VALUE,
  BARBAR_ERROR_BAD_DWL_IPC,
  BARBAR_ERROR_BAD_SWAY_IPC,
  BARBAR_ERROR_BAD_NIRI_IPC,
  BARBAR_ERROR_BAD_HYPRLAND_IPC,
  BARBAR_ERROR_MPRIS,
  BARBAR_ERROR_NO_INTERFACE,
} BarBarError;

#define BARBAR_ERROR (g_barbar_error_quark())
GQuark g_barbar_error_quark(void);

/* int my_error_get_parse_error_id(GError *error); */
/* const char *my_error_get_bad_request_details(GError *error); */
