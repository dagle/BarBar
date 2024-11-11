#include "barbar-output-manager.h"
#include "barbar-output-head.h"
#include "gdk/gdk.h"
#include "gdk/wayland/gdkwayland.h"
#include "glib-object.h"
#include "wlr-output-management-unstable-v1-client-protocol.h"
#include <wayland-client-protocol.h>

/**
 * BarBarOutputManager:
 *
 * Manage your outputs
 */
struct _BarBarOutputManager {
  GObject parent_instance;
  struct zwlr_output_manager_v1 *manager;
};

enum {
  PROP_0,

  NUM_PROPERTIES,
};

enum {
  NEW_HEAD,
  DONE,
  FINISHED,
  NUM_SIGNALS,
};

static guint signals[NUM_SIGNALS];

static GParamSpec *props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarOutputManager, g_barbar_output_manager, G_TYPE_OBJECT)

static void
g_barbar_output_manager_class_init(BarBarOutputManagerClass *class) {

  /**
   * BarBarOutputManager::new-head:
   * @manager: this manager
   * @id: the id of the new head
   *
   * A new head has been created
   */
  signals[NEW_HEAD] =
      g_signal_new("new-head",                             /* signal_name */
                   BARBAR_TYPE_OUTPUT_MANAGER,             /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   1,                                      /* n_params */
                   G_TYPE_UINT);
  /**
   * BarBarOutputManager::done:
   * @manager: this manager
   * @serial: current configuration serial
   *
   * This event is sent after all information has been sent after binding to
   * the output manager object and after any subsequent changes. This
   * applies to child head and mode objects as well. In other words, this event
   * is sent whenever a head or mode is created or destroyed and whenever one of
   * their properties has been changed. Not all state is re-sent each time
   * the current configuration changes: only the actual changes are sent.
   *
   * This allows changes to the output configuration to be seen as
   * atomic, even if they happen via multiple events.
   *
   * A serial is sent to be used in a future create_configuration
   * request.
   */
  signals[DONE] =
      g_signal_new("done",                                 /* signal_name */
                   BARBAR_TYPE_OUTPUT_MANAGER,             /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   1,                                      /* n_params */
                   G_TYPE_UINT);

  /**
   * BarBarOutputManager::finished:
   * @manager: this manager
   *
   * the output manager is finished
   */
  signals[FINISHED] =
      g_signal_new("finished",                             /* signal_name */
                   BARBAR_TYPE_OUTPUT_MANAGER,             /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_output_manager_init(BarBarOutputManager *self) {}

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  BarBarOutputManager *self = BARBAR_OUTPUT_MANAGER(data);
  // BarBarRiverTagPrivate *private =
  //     g_barbar_river_tag_get_instance_private(river);
  //
  if (strcmp(interface, zwlr_output_manager_v1_interface.name) == 0) {
    self->manager = wl_registry_bind(
        registry, name, &zwlr_output_manager_v1_interface, version);
  }
}

static void registry_handle_global_remove(void *_data,
                                          struct wl_registry *_registry,
                                          uint32_t _name) {
  (void)_data;
  (void)_registry;
  (void)_name;
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void head(void *data,
                 struct zwlr_output_manager_v1 *zwlr_output_manager_v1,
                 struct zwlr_output_head_v1 *head) {
  BarBarOutputManager *manager = BARBAR_OUTPUT_MANAGER(data);

  BarBarOutputHead *h = BARBAR_OUTPUT_HEAD(g_barbar_output_head_new(head));

  g_signal_emit(manager, signals[NEW_HEAD], 0, h);
  g_barbar_output_run(h);
}

static void done(void *data,
                 struct zwlr_output_manager_v1 *zwlr_output_manager_v1,
                 uint32_t serial) {
  BarBarOutputManager *manager = BARBAR_OUTPUT_MANAGER(data);

  g_signal_emit(manager, signals[DONE], 0, serial);
}

static void finished(void *data,
                     struct zwlr_output_manager_v1 *zwlr_output_manager_v1) {
  BarBarOutputManager *manager = BARBAR_OUTPUT_MANAGER(data);

  g_signal_emit(manager, signals[FINISHED], 0);
}

struct zwlr_output_manager_v1_listener manager_listner = {
    .head = head,
    .done = done,
    .finished = finished,
};

void g_barbar_output_manager_run(BarBarOutputManager *self) {
  GdkDisplay *gdk_display;
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;

  gdk_display = gdk_display_get_default();
  wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, self);

  wl_display_roundtrip(wl_display);

  if (!self->manager) {
    return;
  }

  zwlr_output_manager_v1_add_listener(self->manager, &manager_listner, self);
  wl_display_roundtrip(wl_display);

  // zwlr_output_head_v1_add_listener(self->manager, &output_listner, self);
}

/**
 * g_barbar_output_manager_new:
 *
 * Creates a new `OutputManager`.
 *
 * Returns: (transfer full): a new `OutputManager`.
 */
GObject *g_barbar_output_manager_new(void) {
  return g_object_new(BARBAR_TYPE_OUTPUT_MANAGER, NULL);
}
