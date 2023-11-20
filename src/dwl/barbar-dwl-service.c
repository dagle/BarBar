#include "dwl/barbar-dwl-service.h"
#include <gio/gio.h>
#include <gio/gunixinputstream.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

struct dwl_status {
  char *output_name;

  char *title;
  char *appid;
  char *layout;
  uint32_t occupied;
  uint32_t tags;
  uint32_t client_tags;
  uint32_t urgent;
  gboolean fullscreen;
  gboolean floating;
  gboolean selmon;
};

struct _BarBarDwlService {
  GObject parent_instance;

  struct dwl_status status;
  GDataInputStream *input;

  // messages comes in chunks of 7 lines.
  uint32_t counter;
};

typedef enum {
  DWL_TITLE,
  DWL_APPID,
  DWL_FULLSCREEN,
  DWL_FLOATING,
  DWL_SELMON,
  DWL_TAGS,
  DWL_LAYOUT,
} g_barbar_dwl_message_kind;

// TODO: Add Error to this
void parse_line(char *line, struct dwl_status *status) {
  g_barbar_dwl_message_kind kind;
  uint32_t num;

  char *save_pointer = NULL;
  char *string = strtok_r(line, " ", &save_pointer);
  gssize id = 0;
  while (string != NULL) {

    if (id == 0) {
      status->output_name = strdup(string);
    } else if (id == 1) {
      if (strcmp(string, "title") == 0) {
        kind = DWL_TITLE;
      } else if (strcmp(string, "appid") == 0) {
        kind = DWL_APPID;
      } else if (strcmp(string, "fullscreen") == 0) {
        kind = DWL_FULLSCREEN;
      } else if (strcmp(string, "floating") == 0) {
        kind = DWL_FLOATING;
      } else if (strcmp(string, "selmon") == 0) {
        kind = DWL_SELMON;
      } else if (strcmp(string, "tags") == 0) {
        kind = DWL_TAGS;
      } else if (strcmp(string, "layout") == 0) {
        kind = DWL_LAYOUT;
      } else {
        // TODO: Error do error
        // g_free(status->output_name);
        // g_free(status);
        return;
      }
    } else {
      // TODO: these null check shouldn't be needed to be done?
      switch (kind) {
      case DWL_TITLE:
        if (string) {
          status->title = strdup(string);
        }
        break;
      case DWL_APPID:
        if (string) {
          status->appid = strdup(string);
        }
        break;
      case DWL_FULLSCREEN:
        status->fullscreen = strcmp(string, "0") != 0;
        break;
      case DWL_FLOATING:
        status->fullscreen = strcmp(string, "0") != 0;
        break;
      case DWL_SELMON:
        status->fullscreen = strcmp(string, "0") != 0;
        break;
      case DWL_LAYOUT:
        if (string) {
          status->appid = strdup(string);
        }
        break;
      case DWL_TAGS:
        // Lets print this
        num = strtoul(string, NULL, 10);
        if (id == 2) {
          status->occupied = num;
        } else if (id == 3) {
          status->tags = num;
        } else if (id == 4) {
          status->client_tags = num;
        } else if (id == 5) {
          status->urgent = num;
        }

        break;
      }
    }
    string = strtok_r(NULL, " ", &save_pointer);
    id++;
  }
}

void g_barbar_dwl_status_free(struct dwl_status *status) {
  g_free(status->output_name);
  g_free(status->appid);
  g_free(status->title);
  g_free(status->layout);
  g_free(status);
}

G_DEFINE_TYPE(BarBarDwlService, g_barbar_dwl_service, G_TYPE_OBJECT)

enum {
  PROP_0,

  NUM_PROPERTIES,
};

static GParamSpec *river_tags_props[NUM_PROPERTIES] = {
    NULL,
};

static guint listener;

static GObject *
g_barbar_dwl_service_constructor(GType type, guint n_construct_properties,
                                 GObjectConstructParam *construct_properties) {
  GObject *obj;
  GObjectClass *parent_class;
  static GObject *__BarBarDwlService_instance = NULL;
  static GMutex __BarBarDwlService_singleton;
  static volatile gsize __BarBarDwlService_once = 0;
  BarBarDwlService *self;

  // Initializes mutex properly by using g_once_init_enter()
  if (g_once_init_enter(&__BarBarDwlService_once) == TRUE) {
    g_mutex_init(&__BarBarDwlService_singleton);
    g_once_init_leave(&__BarBarDwlService_once, 42);
  }
  // Locks mutex and checks if instance is already available
  g_mutex_lock(&__BarBarDwlService_singleton);
  if (__BarBarDwlService_instance != NULL) {
    // Unlocks mutex
    g_mutex_unlock(&__BarBarDwlService_singleton);
    // Adds one more reference to put it on the same count as generated vala
    // code for any subsequent access
    g_object_ref(__BarBarDwlService_instance);
    // Returns same instance each time
    return __BarBarDwlService_instance;
  }
  // If instance is not yet available, then new one is created normally
  parent_class = G_OBJECT_CLASS(g_barbar_dwl_service_parent_class);
  obj = parent_class->constructor(type, n_construct_properties,
                                  construct_properties);

  self = G_TYPE_CHECK_INSTANCE_CAST(obj, BARBAR_TYPE_DWL_SERVICE,
                                    BarBarDwlService);
  // Assigns newly created object as class instance
  __BarBarDwlService_instance = obj;
  // Adds weak pointer for instance in case if singleton will need recreation
  // when Vala drops it
  g_object_add_weak_pointer(__BarBarDwlService_instance,
                            (gpointer)&__BarBarDwlService_instance);
  // Unlocks mutex
  g_mutex_unlock(&__BarBarDwlService_singleton);
  return obj;
}

static void g_barbar_dwl_service_reader(GObject *object, GAsyncResult *res,
                                        gpointer data) {
  GDataInputStream *input = G_DATA_INPUT_STREAM(object);
  BarBarDwlService *service = BARBAR_DWL_SERVICE(data);
  GError *error = NULL;
  gsize length;
  gchar *line;

  line = g_data_input_stream_read_line_finish(input, res, &length, &error);

  parse_line(line, &service->status);

  service->counter++;

  if (service->counter == 6) {
    g_signal_emit(service, listener, 0, &service->status);

    service->counter = 0;
    g_free(service->status.title);
    g_free(service->status.appid);
    g_free(service->status.layout);

    memset(&service->status, 0, sizeof(struct dwl_status));
  }

  g_data_input_stream_read_line_async(input, G_PRIORITY_DEFAULT, NULL,
                                      g_barbar_dwl_service_reader, data);
}

static void g_barbar_dwl_service_constructed(GObject *self) {
  BarBarDwlService *service = BARBAR_DWL_SERVICE(self);
  GInputStream *input;

  input = g_unix_input_stream_new(STDIN_FILENO, FALSE);
  service->input = g_data_input_stream_new(input);

  g_data_input_stream_read_line_async(service->input, G_PRIORITY_DEFAULT, NULL,
                                      g_barbar_dwl_service_reader, service);
}

static void g_barbar_dwl_service_class_init(BarBarDwlServiceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  // gobject_class->set_property = g_barbar_dwl_tag_set_property;
  // gobject_class->get_property = g_barbar_dwl_tag_get_property;
  gobject_class->constructor = g_barbar_dwl_service_constructor;
  gobject_class->constructed = g_barbar_dwl_service_constructed;

  //   river_tags_props[PROP_TAGNUMS] =
  //     g_param_spec_uint("tagnums", NULL, NULL, 0, 9, 9, G_PARAM_READWRITE);
  // g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
  //                                   river_tags_props);

  listener =
      g_signal_new("listener", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void g_barbar_dwl_service_init(BarBarDwlService *self) {}

BarBarDwlService *g_barbar_dwl_service_new(void) {
  BarBarDwlService *dwl = g_object_new(BARBAR_TYPE_DWL_SERVICE, NULL);

  return dwl;
}
