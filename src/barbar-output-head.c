#include "barbar-output-head.h"
#include "gdk/gdk.h"
#include "gdk/wayland/gdkwayland.h"
#include "glib-object.h"
#include "glib.h"
#include "wlr-output-management-unstable-v1-client-protocol.h"
#include <stdint.h>
#include <stdio.h>
#include <wayland-client-protocol.h>

struct head_mode {
  struct zwlr_output_mode_v1 *mode;
  BarBarOutputHead *parent;
  int32_t width;
  int32_t height;
  int32_t refresh;
};

/**
 * BarBarOutputHead:
 * A wayland output (e.g. a computer screen)
 *
 */
struct _BarBarOutputHead {
  GObject parent_instance;
  struct zwlr_output_head_v1 *head;
  struct head_mode *current;
  char *name;
  char *description;
  gint32 height;
  gint32 width;
  gboolean enabled;
  gint32 pos_y;
  gint32 pos_x;
  gint32 transform;
  gint32 scale;
  char *make;
  char *model;
  char *serial_number;
  GList *modes;
  gboolean sync;
};

static struct head_mode *
g_barbar_insert_mode(BarBarOutputHead *head, struct zwlr_output_mode_v1 *mode) {
  struct head_mode *head_mode = malloc(sizeof(struct head_mode));
  head->modes = g_list_append(head->modes, head_mode);
  head_mode->parent = head;

  head_mode->mode = mode;

  return head_mode;
}

static struct head_mode *g_barbar_find_mode(BarBarOutputHead *head,
                                            struct zwlr_output_mode_v1 *mode) {

  for (GList *list = head->modes; list; list = list->next) {
    struct head_mode *head_mode = list->data;
    if (head_mode->mode == mode) {
      return head_mode;
    }
  }
  return NULL;
}

static void g_barbar_free_mode(void *data) {
  struct head_mode *head_mode = data;
  g_clear_pointer(&head_mode->mode, zwlr_output_mode_v1_destroy);
  g_free(head_mode);
}

enum {
  PROP_0,

  PROP_HEAD,
  PROP_NAME,
  PROP_DESCRIPTION,
  PROP_RES_HEIGHT,
  PROP_RES_WIDTH,
  PROP_REFRESH,
  PROP_HEIGHT,
  PROP_WIDTH,
  PROP_ENABLED,
  PROP_POS_X,
  PROP_POS_Y,
  PROP_TRANSFORM,
  PROP_SCALE,
  PROP_MAKE,
  PROP_MODEL,
  PROP_SERAL_NUMBER,
  PROP_ADAPT_SYNC,

  NUM_PROPERTIES,
};

enum {
  NUM_SIGNALS,
};

static guint signals[NUM_SIGNALS];

static GParamSpec *props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarOutputHead, g_barbar_output_head, G_TYPE_OBJECT)

static void g_barbar_output_head_set_head(BarBarOutputHead *self,
                                          struct zwlr_output_head_v1 *head) {
  g_return_if_fail(BARBAR_IS_OUTPUT_HEAD(self));

  if (self->head == head) {
    return;
  }

  free(self->head);
  self->head = head;
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_HEAD]);
}

/**
 * g_barbar_output_head_get_name:
 *
 * Gets the name of the output
 *
 * Returns: (transfer none): A string
 */
const char *g_barbar_output_head_get_name(BarBarOutputHead *head) {
  return head->name;
}

/**
 * g_barbar_output_head_get_resolution_height:
 *
 * Returns: the resolution height of the head
 */
gint32 g_barbar_output_head_get_resolution_height(BarBarOutputHead *head) {
  if (!head->current) {
    return -1;
  }
  return head->current->height;
}

/**
 * g_barbar_output_head_get_resolution_width:
 *
 * Returns: the resolution width of the head
 */
gint32 g_barbar_output_head_get_resolution_width(BarBarOutputHead *head) {
  if (!head->current) {
    return -1;
  }
  return head->current->width;
}
/**
 * g_barbar_output_head_get_refresh:
 *
 * Returns: the refresh rate of the head
 */
gint32 g_barbar_output_head_get_refresh(BarBarOutputHead *head) {
  if (!head->current) {
    return -1;
  }
  return head->current->refresh;
}

/**
 * g_barbar_output_head_get_height:
 *
 * Returns: the height of the head
 */
gint32 g_barbar_output_head_get_height(BarBarOutputHead *head) {
  return head->height;
}

/**
 * g_barbar_output_head_get_width:
 *
 * Returns: the width of the head
 */
gint32 g_barbar_output_head_get_width(BarBarOutputHead *head) {
  return head->width;
}

/**
 * g_barbar_output_head_get_pos_y:
 *
 * Returns: the vertical position of the output
 */
gint32 g_barbar_output_head_get_pos_y(BarBarOutputHead *head) {
  return head->pos_y;
}

/**
 * g_barbar_output_head_get_pos_x:
 *
 * Returns: the horizontal position of the output
 */
gint32 g_barbar_output_head_get_pos_x(BarBarOutputHead *head) {
  return head->pos_x;
}

/**
 * g_barbar_output_head_get_transform:
 *
 * Returns: the output transform
 */
gint32 g_barbar_output_head_get_transform(BarBarOutputHead *head) {
  return head->transform;
}

/**
 * g_barbar_output_head_get_scale:
 *
 * Gets the name of the output
 *
 * Returns: get the scaling of the output
 */
gint32 g_barbar_output_head_get_scale(BarBarOutputHead *head) {
  return head->scale;
}

/**
 * g_barbar_output_head_get_sync:
 *
 * Describes whether adaptive sync is currently enabled for
 * the head or not. Adaptive sync is also known as Variable Refresh
 * Rate or VRR.
 *
 * Returns: %TRUE if sync is enabled
 */
gboolean g_barbar_output_head_get_sync(BarBarOutputHead *head) {
  return head->sync;
}

/**
 * g_barbar_output_head_get_enabled:
 *
 * Weither or not the screen has been mapped by the wayland
 * compositor.
 *
 * Returns: %TRUE if the output is enabled
 */
gboolean g_barbar_output_head_get_enabled(BarBarOutputHead *head) {
  return head->enabled;
}

/**
 * g_barbar_output_head_get_description:
 *
 * Gets the name of the output
 *
 * Returns: (transfer none): A string
 */
const char *g_barbar_output_head_get_description(BarBarOutputHead *head) {
  return head->description;
}

/**
 * g_barbar_output_head_get_make:
 *
 * Gets the name of the output
 *
 * Returns: (transfer none): A string
 */
const char *g_barbar_output_head_get_make(BarBarOutputHead *head) {
  return head->make;
}

/**
 * g_barbar_output_head_get_model:
 *
 * Gets the name of the output
 *
 * Returns: (transfer none): A string
 */
const char *g_barbar_output_head_get_model(BarBarOutputHead *head) {
  return head->model;
}

/**
 * g_barbar_output_head_serial_get_number:
 *
 * Gets the name of the output
 *
 * Returns: (transfer none): A string
 */
const char *g_barbar_output_head_serial_get_number(BarBarOutputHead *head) {
  return head->serial_number;
}

static void g_barbar_output_head_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarOutputHead *head = BARBAR_OUTPUT_HEAD(object);

  switch (property_id) {
  case PROP_HEAD:
    g_barbar_output_head_set_head(head, g_value_get_pointer(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_output_head_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {
  BarBarOutputHead *head = BARBAR_OUTPUT_HEAD(object);
  switch (property_id) {
  case PROP_NAME:
    g_value_set_string(value, head->name);
    break;
  case PROP_DESCRIPTION:
    g_value_set_string(value, head->description);
    break;
  case PROP_HEIGHT:
    g_value_set_uint(value, head->height);
    break;
  case PROP_WIDTH:
    g_value_set_uint(value, head->width);
    break;
  case PROP_ENABLED:
    g_value_set_boolean(value, head->enabled);
    break;
  case PROP_POS_X:
    g_value_set_uint(value, head->pos_x);
    break;
  case PROP_POS_Y:
    g_value_set_uint(value, head->pos_y);
    break;
  case PROP_RES_HEIGHT: {
    g_value_set_uint(value, head->current->height);
    break;
  }
  case PROP_RES_WIDTH: {
    g_value_set_uint(value, head->current->width);
    break;
  }
  case PROP_REFRESH: {
    g_value_set_uint(value, head->current->refresh);
    break;
  }
  case PROP_TRANSFORM:
    g_value_set_uint(value, head->transform);
    break;
  case PROP_SCALE:
    g_value_set_uint(value, head->scale);
    break;
  case PROP_MAKE:
    g_value_set_string(value, head->make);
    break;
  case PROP_MODEL:
    g_value_set_string(value, head->model);
    break;
  case PROP_SERAL_NUMBER:
    g_value_set_string(value, head->serial_number);
    break;
  case PROP_ADAPT_SYNC:
    g_value_set_boolean(value, head->sync);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_output_head_finalize(GObject *object) {
  BarBarOutputHead *head = BARBAR_OUTPUT_HEAD(object);

  // struct zwlr_output_head_v1 *head;
  g_free(head->name);
  g_free(head->description);
  g_free(head->make);
  g_free(head->model);
  g_free(head->serial_number);

  g_clear_pointer(&head->head, zwlr_output_head_v1_destroy);
  g_list_free_full(head->modes, g_barbar_free_mode);

  // g_clear_pointer(&head->current, zwlr_output_mode_v1_destroy);

  G_OBJECT_CLASS(g_barbar_output_head_parent_class)->finalize(object);
}

static void g_barbar_output_head_class_init(BarBarOutputHeadClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_output_head_set_property;
  gobject_class->get_property = g_barbar_output_head_get_property;
  gobject_class->finalize = g_barbar_output_head_finalize;

  /**
   * BarBackOutputHead:head:
   *
   * The wayland head
   */
  props[PROP_HEAD] = g_param_spec_pointer(
      "head", NULL, NULL, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BarBackOutputHead:name:
   *
   * The name of the head.
   * The naming convention is compositor defined, but limited to alphanumeric
   * characters and dashes (-). Each name is unique among all wlr_output_head
   * objects, but if a wlr_output_head object is destroyed the same name may
   * be reused later. The names will also remain consistent across sessions
   * with the same hardware and software configuration.

   * Examples of names include 'HDMI-A-1', 'WL-1', 'X11-1', etc. However, do
   * not assume that the name is a reflection of an underlying DRM
   * connector, X11 connection, etc.

   */
  props[PROP_NAME] =
      g_param_spec_string("name", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
  * BarBackOutputHead:description:
  *
  * The description is a UTF-8 string with no convention defined for its
  * contents. Examples might include 'Foocorp 11" Display' or 'Virtual X11
  * output via :1'. However, do not assume that the name is a reflection of
  * the make, model, serial of the underlying DRM connector or the display
  * name of the underlying X11 connection, etc.

  * If the compositor implements xdg-output and this head is enabled,
  * the xdg_output.description must report the same description.
  */
  props[PROP_DESCRIPTION] =
      g_param_spec_string("description", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:height:
   *
   * The height of the device, not applicable if the device doesn't have a
   * physical size (e.g. is not a projector or a virtual device).
   */
  props[PROP_HEIGHT] = g_param_spec_uint("height", NULL, NULL, 0, G_MAXUINT, 0,
                                         G_PARAM_READABLE);

  /**
   * BarBackOutputHead:width:
   *
   * The width of the device, not applicable if the device doesn't have a
   * physical size (is a projector or a virtual device).
   */
  props[PROP_WIDTH] =
      g_param_spec_uint("width", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:enabled:
   * If the screen is enabled (is mapped)
   */
  props[PROP_ENABLED] =
      g_param_spec_boolean("enabled", NULL, NULL, FALSE, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:resolution-height:
   * The resolution height of the output. The output might be scaled.
   */
  props[PROP_RES_HEIGHT] = g_param_spec_uint("resolution-height", NULL, NULL, 0,
                                             G_MAXUINT, 0, G_PARAM_READABLE);
  /**
   * BarBackOutputHead:resolution-width:
   * The resolution width of the output. The output might be scaled.
   */
  props[PROP_RES_WIDTH] = g_param_spec_uint("resolution-width", NULL, NULL, 0,
                                            G_MAXUINT, 0, G_PARAM_READABLE);
  /**
   * BarBackOutputHead:refresh:
   * The refresh ratio of the output.
   */
  props[PROP_REFRESH] = g_param_spec_uint("refresh", NULL, NULL, 0, G_MAXUINT,
                                          0, G_PARAM_READABLE);
  /**
   * BarBackOutputHead:pos-x:
   * The x position of the head in the global compositor
   * space. Only applicable if the output is enabled.
   */
  props[PROP_POS_X] =
      g_param_spec_uint("pos-x", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:pos-y:
   * The y position of the head in the global compositor
   * space. Only applicable if the output is enabled.
   */
  props[PROP_POS_Y] =
      g_param_spec_uint("pos-y", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:transform:
   * Transformation currently applied to the head.
   * Only applicable if the output is enabled.
   */
  props[PROP_TRANSFORM] = g_param_spec_uint("transform", NULL, NULL, 0,
                                            G_MAXUINT, 0, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:scale:
   * The scale of the head in the global compositor
   * space. Only applicable if the output is enabled.
   */
  props[PROP_SCALE] =
      g_param_spec_uint("scale", NULL, NULL, 0, G_MAXUINT, 0, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:make:
   * The manufacturer of the head.

   * Together with the model and serial_number events the purpose is to
   * allow clients to recognize heads from previous sessions and for example
   * load head-specific configurations back.

   * It is not guaranteed this will be ever sent. A reason for that
   * can be that the compositor does not have information about the make of
   * the head or the definition of a make is not sensible in the current
   * setup, for example in a virtual session. Clients can still try to
   * identify the head by available information from other events but should
   * be aware that there is an increased risk of false positives.

   * It is not recommended to display the make string in UI to users. For
   * that the string provided by the description event should be preferred.
   */
  props[PROP_MAKE] =
      g_param_spec_string("make", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:model:
   *
   * Describes the model of the head.

   * Together with the make and serial_number events the purpose is to
   * allow clients to recognize heads from previous sessions and for example
   * load head-specific configurations back.

   * It is not guaranteed this event will be ever sent. A reason for that
   * can be that the compositor does not have information about the model of
   * the head or the definition of a model is not sensible in the current
   * setup, for example in a virtual session. Clients can still try to
   * identify the head by available information from other events but should
   * be aware that there is an increased risk of false positives.

   * It is not recommended to display the model string in UI to users. For
   * that the string provided by the description event should be preferred.
   */
  props[PROP_MODEL] =
      g_param_spec_string("model", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:serial-number:
   *
   * Describes the serial number of the head.

   * Together with the make and model events the purpose is to allow clients
   * to recognize heads from previous sessions and for example load head-
   * specific configurations back.

   * It is not guaranteed this event will be ever sent. A reason for that
   * can be that the compositor does not have information about the serial
   * number of the head or the definition of a serial number is not sensible
   * in the current setup. Clients can still try to identify the head by
   * available information from other events but should be aware that there
   * is an increased risk of false positives.

   * It is not recommended to display the serial_number string in UI to
   * users. For that the string provided by the description event should be
   * preferred.
   */
  props[PROP_SERAL_NUMBER] =
      g_param_spec_string("serial-number", NULL, NULL, NULL, G_PARAM_READABLE);

  /**
   * BarBackOutputHead:adaptive-sync:
   *
   * Describes whether adaptive sync is currently enabled for
   * the head or not. Adaptive sync is also known as Variable Refresh
   * Rate or VRR.
   */
  props[PROP_ADAPT_SYNC] = g_param_spec_boolean("adaptive-sync", NULL, NULL,
                                                FALSE, G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, props);
}

static void g_barbar_output_head_init(BarBarOutputHead *self) {}

static void name(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1,
                 const char *name) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (g_set_str(&self->name, name)) {
    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_NAME]);
  }
}

static void description(void *data,
                        struct zwlr_output_head_v1 *zwlr_output_head_v1,
                        const char *description) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (g_set_str(&self->description, description)) {
    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_DESCRIPTION]);
  }
}

static void physical_size(void *data,
                          struct zwlr_output_head_v1 *zwlr_output_head_v1,
                          int32_t width, int32_t height) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  self->width = width;
  self->height = height;
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_WIDTH]);
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_HEIGHT]);
}

static void size(void *data, struct zwlr_output_mode_v1 *zwlr_output_mode_v1,
                 int32_t width, int32_t height) {
  struct head_mode *self = data;

  self->width = width;
  self->height = height;
}

static void refresh(void *data, struct zwlr_output_mode_v1 *zwlr_output_mode_v1,
                    int32_t refresh) {
  struct head_mode *self = data;

  self->refresh = refresh;
}

static void preferred(void *data,
                      struct zwlr_output_mode_v1 *zwlr_output_mode_v1) {}

static void mode_finished(void *data,
                          struct zwlr_output_mode_v1 *zwlr_output_mode_v1) {
  struct head_mode *mode = data;
  BarBarOutputHead *head = mode->parent;

  head->modes = g_list_remove(head->modes, mode);
  g_barbar_free_mode(mode);

  // GList *list;
  // gboolean found = FALSE;
  // struct head_mode *head_mode;
  // for (list = head->modes; list; list = list->next) {
  //   head_mode = list->data;
  //   if (head_mode->mode == zwlr_output_mode_v1) {
  //     break;
  //     found = TRUE;
  //   }
  // }
  //
  // if (!found) {
  //   return;
  // }
  // head->modes = g_list_remove_link(head->modes, list);
  // g_barbar_free_mode(head_mode);
}

const struct zwlr_output_mode_v1_listener mode_listner = {

    .size = size,
    .refresh = refresh,
    .preferred = preferred,
    .finished = mode_finished,
};

static void mode(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1,
                 struct zwlr_output_mode_v1 *mode) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);
  struct head_mode *head_mode;

  head_mode = g_barbar_insert_mode(self, mode);
  zwlr_output_mode_v1_add_listener(mode, &mode_listner, head_mode);
}

static void enabled(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1,
                    int32_t enabled) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (self->enabled == enabled) {
    return;
  }

  self->enabled = enabled;
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_ENABLED]);
}

static void current_mode(void *data,
                         struct zwlr_output_head_v1 *zwlr_output_head_v1,
                         struct zwlr_output_mode_v1 *mode) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);
  self->current = g_barbar_find_mode(self, mode);

  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_RES_WIDTH]);
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_RES_HEIGHT]);
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_REFRESH]);
}

static void position(void *data,
                     struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t x,
                     int32_t y) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  self->pos_x = x;
  self->pos_y = y;
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_POS_Y]);
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_POS_X]);
  //
}

static void transform(void *data,
                      struct zwlr_output_head_v1 *zwlr_output_head_v1,
                      int32_t transform) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (self->transform == transform) {
    return;
  }

  self->transform = transform;
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_TRANSFORM]);
  //
}

static void scale(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1,
                  wl_fixed_t scale) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (self->scale == scale) {
    return;
  }

  self->scale = scale;
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_SCALE]);
  //
}

static void head_finished(void *data,
                          struct zwlr_output_head_v1 *zwlr_output_head_v1) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);
  g_clear_pointer(&self->head, zwlr_output_head_v1_destroy);
}

static void make(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1,
                 const char *make) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (g_set_str(&self->make, make)) {
    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_MAKE]);
  }
}

static void model(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1,
                  const char *model) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (g_set_str(&self->model, model)) {
    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_MODEL]);
  }
  //
}

static void serial_number(void *data,
                          struct zwlr_output_head_v1 *zwlr_output_head_v1,
                          const char *serial_number) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (g_set_str(&self->serial_number, serial_number)) {
    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_SERAL_NUMBER]);
  }
}

static void adaptive_sync(void *data,
                          struct zwlr_output_head_v1 *zwlr_output_head_v1,
                          uint32_t state) {
  BarBarOutputHead *self = BARBAR_OUTPUT_HEAD(data);

  if (self->sync == state) {
    return;
  }

  self->sync = state;
  g_object_notify_by_pspec(G_OBJECT(self), props[PROP_ADAPT_SYNC]);
}

static struct zwlr_output_head_v1_listener listner = {
    .name = name,
    .description = description,
    .physical_size = physical_size,
    .mode = mode,
    .enabled = enabled,
    .current_mode = current_mode,
    .position = position,
    .transform = transform,
    .scale = scale,
    .finished = head_finished,
    .make = make,
    .model = model,
    .serial_number = serial_number,
    .adaptive_sync = adaptive_sync,
};

/**
 * g_barbar_output_run:
 *
 * Starts the output. Will likely happen on construction in future.
 *
 */
void g_barbar_output_head_run(BarBarOutputHead *self) {
  GdkDisplay *gdk_display;
  struct wl_display *wl_display;

  gdk_display = gdk_display_get_default();
  wl_display = gdk_wayland_display_get_wl_display(gdk_display);

  zwlr_output_head_v1_add_listener(self->head, &listner, self);

  wl_display_roundtrip(wl_display);
}

/**
 * g_barbar_output_head_new:
 *
 * Creates a new `OutputHead`.
 *
 * Returns: (transfer full): a new `OutputHead`.
 */
GObject *g_barbar_output_head_new(struct zwlr_output_head_v1 *head) {
  BarBarOutputHead *self =
      g_object_new(BARBAR_TYPE_OUTPUT_HEAD, "head", head, NULL);

  return G_OBJECT(self);
}
