#include "dwl/barbar-dwl-status.h"

G_DEFINE_BOXED_TYPE(BarBarDwlStatus, g_barbar_dwl_status,
                    g_barbar_dwl_status_ref, g_barbar_dwl_status_unref)

BarBarDwlStatus *g_barbar_dwl_status_new(void) {
  BarBarDwlStatus *status = g_malloc0(sizeof(BarBarDwlStatus));

  g_atomic_ref_count_init(&status->ref_count);

  return status;
}

BarBarDwlStatus *g_barbar_dwl_status_ref(BarBarDwlStatus *status) {
  g_return_val_if_fail(status, NULL);

  g_atomic_ref_count_inc(&status->ref_count);

  return status;
}

void g_barbar_dwl_status_unref(BarBarDwlStatus *status) {
  g_return_if_fail(status);

  if (!g_atomic_ref_count_dec(&status->ref_count)) {
    g_free(status->output_name);
    g_free(status->title);
    g_free(status->appid);
    g_free(status->layout);
    g_free(status);
  }
}

const char *g_barbar_dwl_status_get_output_name(BarBarDwlStatus *status) {
  return status->output_name;
}

const char *g_barbar_dwl_status_get_title(BarBarDwlStatus *status) {
  return status->title;
}
const char *g_barbar_dwl_status_get_appid(BarBarDwlStatus *status) {
  return status->appid;
}
const char *g_barbar_dwl_status_get_layout(BarBarDwlStatus *status) {
  return status->layout;
}

guint32 g_barbar_dwl_status_get_occupide(BarBarDwlStatus *status) {
  return status->occupied;
}
guint32 g_barbar_dwl_status_get_selected(BarBarDwlStatus *status) {
  return status->selected;
}
guint32 g_barbar_dwl_status_get_client_tags(BarBarDwlStatus *status) {
  return status->client_tags;
}
guint32 g_barbar_dwl_status_get_urgent(BarBarDwlStatus *status) {
  return status->urgent;
}

gboolean g_barbar_dwl_status_get_fullscreen(BarBarDwlStatus *status) {
  return status->fullscreen;
}
gboolean g_barbar_dwl_status_get_float(BarBarDwlStatus *status) {
  return status->floating;
}
gboolean g_barbar_dwl_status_get_selmon(BarBarDwlStatus *status) {
  return status->selmon;
}
