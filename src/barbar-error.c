#include "barbar-error.h"

// typedef struct {
//   int _dummy;
//   // char *bad_request_details;
// } BarBarErrorPrivate;
//
// static void g_barbar_error_private_init(BarBarErrorPrivate *priv) {
//   // priv->parse_error_id = -1;
// }
//
// static void g_barbar_error_private_copy(const BarBarErrorPrivate *src_priv,
//                                         BarBarErrorPrivate *dest_priv) {
//   // dest_priv->parse_error_id = src_priv->parse_error_id;
//   // dest_priv->bad_request_details =
//   g_strdup(src_priv->bad_request_details);
// }
//
// static void g_barbar_error_private_clear(BarBarErrorPrivate *priv) {
//   // g_free(priv->bad_request_details);
// }

GQuark g_barbar_error_quark(void) {
  return g_quark_from_static_string("BarBarError");
}

// G_DEFINE_EXTENDED_ERROR(BarBarError, g_barbar_error)

// int g_barbar_error_get_parse_error_id(GError *error) {
//   BarBarErrorPrivate *priv = g_barbar_error_get_private(error);
//   g_return_val_if_fail(priv != NULL, -1);
//   return priv->parse_error_id;
// }
//
// const char *g_barbar_error_get_bad_request_details(GError *error) {
//   BarBarErrorPrivate *priv = g_barbar_error_get_private(error);
//   g_return_val_if_fail(priv != NULL, NULL);
//   g_return_val_if_fail(error->code != BARBAR_ERROR_BAD_REQUEST, NULL);
//   return priv->bad_request_details;
// }
//
// static void g_barbar_error_set_bad_request(GError **error, const char
// *reason,
//                                            int error_id, const char *details)
//                                            {
//   BarBarErrorPrivate *priv;
//   g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_REQUEST,
//               "Invalid request: %s", reason);
//   if (error != NULL && *error != NULL) {
//     priv = g_barbar_error_get_private(*error);
//     g_return_if_fail(priv != NULL);
//     priv->parse_error_id = error_id;
//     priv->bad_request_details = g_strdup(details);
//   }
// }
