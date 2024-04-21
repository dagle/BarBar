#include "src/hyprland/barbar-hyprland-submap.h"
#include <math.h>
#include <stdio.h>

/**
 * BarBarHyprlandSubmap:
 *
 * A sensor to tell what hyprland mode we are in.
 */
struct _BarBarHyprlandSubmap {
  BarBarSensor parent_instance;

  GSocketConnection *listener;

  char *mode;
};

enum {
  PROP_0,

  PROP_MODE,

  NUM_PROPERTIES,
};

static GParamSpec *hyprland_submap_props[NUM_PROPERTIES] = {
    NULL,
};

staticvoid g_barbar_hyprland_submap_set(BarBarHyprlandSubmap *hypr,
                                        const char *args) {
  g_return_if_fail(BARBAR_IS_HYPERLAND_SUBMAP(hypr));

  g_free(hypr->mode);
  hypr->mode = g_strdup(hypr->mode);

  g_object_notify_by_pspec(G_OBJECT(hypr), hyprland_submap_props[PROP_MODE]);
}

G_DEFINE_TYPE(BarBarHyprlandSubmap, hyprland_submap_props, BARBAR_TYPE_SENSOR)

static void g_barbar_hyprland_submap_start(BarBarSensor *sensor);

static void g_barbar_hyprland_submap_set_property(GObject *object,
                                                  guint property_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec) {

  // BarBarHyprlandSubmap *submap = BARBAR_HYPRLAND_SUBMAP(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_hyprland_submap_get_property(GObject *object,
                                                  guint property_id,
                                                  GValue *value,
                                                  GParamSpec *pspec) {
  BarBarHyprlandSubmap *submap = BARBAR_HYPRLAND_SUBMAP(object);

  switch (property_id) {
  case PROP_MODE:
    g_value_set_double(value, submap->mode);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void
g_barbar_hyprland_submap_class_init(BarBarHyprlandSubmapClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_hyprland_submap_start;

  gobject_class->set_property = g_barbar_hyprland_submap_set_property;
  gobject_class->get_property = g_barbar_hyprland_submap_get_property;

  /**
   * BarBarHyprlandSubmap:mode:
   *
   * In which mode the hyprland is in
   */
  hyprland_submap_props[PROP_MODE] =
      g_param_spec_double("mode", NULL, NULL, 0, 100, 0, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, CPU_NUM_PROPERTIES,
                                    hyprland_submap_props);
}

static void g_barbar_hyprland_submap_init(BarBarHyprlandSubmap *self) {}

static void g_barbar_hyprland_workspace_callback(uint32_t type, char *args,
                                                 gpointer data) {
  BarBarHyprlandWorkspace *hypr = BARBAR_HYPRLAND_WORKSPACE(data);
  if (type == HYPRLAND_SUBMAP) {
    g_barbar_hyprland_submap_set(hypr, args);
  }
}

static void g_barbar_hyprland_submap_update(BarBarSensor *sensor) {
  GError *error = NULL;

  BarBarHyprlandSubmap *submap = BARBAR_HYPRLAND_SUBMAP(object);

  GSocketConnection *ipc = g_barbar_hyprland_ipc_controller(&error);

  if (error) {
    g_printerr("Hyprland submap: Error connecting to the ipc: %s",
               error->message);
    return;
  }

  hypr->listener = g_barbar_hyprland_ipc_listner(
      g_barbar_hyprland_submap_callback, hypr, NULL, &error);
}
