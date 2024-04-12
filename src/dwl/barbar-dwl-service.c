#include "dwl/barbar-dwl-service.h"
#include "barbar-error.h"
#include "dwl/barbar-dwl-status.h"
#include <gio/gio.h>
#include <gio/gunixinputstream.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

struct _BarBarDwlService {
  GObject parent_instance;

  BarBarDwlStatus *status;
  GDataInputStream *input;
  GInputStream *stream;

  // path to the data, if null, iit will read stdin.
  char *file_path;
  GFileMonitor *monitor;
  GFileInputStream *file_input_stream;
  GFile *file;

  // messages comes in chunks of 7 lines.
  uint32_t counter;
};

// example output
// WL-1 tags 0 1 0 0
// WL-1 layout []=
// WL-1 title
// WL-1 appid
// WL-1 fullscreen
// WL-1 floating
// WL-1 selmon 1

typedef enum {
  DWL_TAGS,
  DWL_LAYOUT,
  DWL_TITLE,
  DWL_APPID,
  DWL_FULLSCREEN,
  DWL_FLOATING,
  DWL_SELMON,
} g_barbar_dwl_message_kind;

static void parse_line(char *line, BarBarDwlStatus *status, GError **error) {
  g_barbar_dwl_message_kind kind;
  uint32_t num;

  if (!line) {
    return;
  }

  char *save_pointer = NULL;
  char *string = strtok_r(line, " ", &save_pointer);
  gssize id = 0;
  while (string != NULL) {

    if (id == 0) {
      status->output_name = g_strdup(string);
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
        g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_VALUE,
                    "%s not a valid kind", string);
        g_free(status->output_name);
        g_free(status);
        return;
      }
    } else {
      switch (kind) {
      case DWL_TITLE:
        status->title = g_strdup(string);
        break;
      case DWL_APPID:
        status->appid = g_strdup(string);
        break;
      case DWL_FULLSCREEN:
        status->fullscreen = g_strcmp0(string, "0") != 0;
        break;
      case DWL_FLOATING:
        status->floating = g_strcmp0(string, "0") != 0;
        break;
      case DWL_SELMON:
        status->selmon = g_strcmp0(string, "0") != 0;
        break;
      case DWL_LAYOUT:
        status->appid = g_strdup(string);
        break;
      case DWL_TAGS:
        num = strtoul(string, NULL, 10);
        if (id == 2) {
          status->occupied = num;
        } else if (id == 3) {
          status->selected = num;
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

G_DEFINE_TYPE(BarBarDwlService, g_barbar_dwl_service, G_TYPE_OBJECT)

enum {
  PROP_0,

  PROP_FILEPATH,

  NUM_PROPERTIES,
};

static GParamSpec *dwl_service_props[NUM_PROPERTIES] = {
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

static void g_barbar_dwl_service_pipe_reader(GObject *object, GAsyncResult *res,
                                             gpointer data) {
  GDataInputStream *input = G_DATA_INPUT_STREAM(object);
  BarBarDwlService *service = BARBAR_DWL_SERVICE(data);
  GError *error = NULL;
  gsize length;
  gchar *line;

  line = g_data_input_stream_read_line_finish(input, res, &length, &error);

  if (error) {
    g_printerr("Failed to read input stream from dwl: %s", error->message);
    return;
  }
  // is this function defensive enough?

  if (!service->status) {
    service->status = g_barbar_dwl_status_new();
  }

  parse_line(line, service->status, &error);

  if (error) {
    g_printerr("dwl parse input: %s", error->message);
    return;
  }

  service->counter++;

  if (service->counter == 6) {
    g_signal_emit(service, listener, 0, service->status);

    service->counter = 0;
    g_clear_pointer(&service->status, g_barbar_dwl_status_unref);
  }

  // We need to wait until new data
  g_data_input_stream_read_line_async(input, G_PRIORITY_DEFAULT, NULL,
                                      g_barbar_dwl_service_pipe_reader, data);
}

static void g_barbar_dwl_service_set_pipe(BarBarDwlService *self,
                                          const char *file_path) {
  g_return_if_fail(BARBAR_IS_DWL_SERVICE(self));

  if (!g_strcmp0(self->file_path, file_path)) {
    return;
  }

  g_free(self->file_path);
  self->file_path = g_strdup(file_path);

  g_object_notify_by_pspec(G_OBJECT(self), dwl_service_props[PROP_FILEPATH]);
}

static void g_barbar_rotary_set_property(GObject *object, guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec) {
  BarBarDwlService *service = BARBAR_DWL_SERVICE(object);
  switch (property_id) {
  case PROP_FILEPATH:
    g_barbar_dwl_service_set_pipe(service, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_rotary_get_property(GObject *object, guint property_id,
                                         GValue *value, GParamSpec *pspec) {
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_service_file_data(GObject *object, GAsyncResult *res,
                                           gpointer data) {
  GDataInputStream *input = G_DATA_INPUT_STREAM(object);
  BarBarDwlService *service = BARBAR_DWL_SERVICE(data);
  GError *error = NULL;
  gsize length;
  gchar *line;

  line = g_data_input_stream_read_line_finish(input, res, &length, &error);

  if (error) {
    g_printerr("Failed to read input stream from dwl: %s", error->message);
    return;
  }

  if (line) {
    // is this function defensive enough?

    if (!service->status) {
      service->status = g_barbar_dwl_status_new();
    }

    parse_line(line, service->status, &error);

    if (error) {
      g_printerr("dwl parse input: %s", error->message);
      g_error_free(error);
      return;
    }

    service->counter++;

    if (service->counter == 6) {
      g_signal_emit(service, listener, 0, service->status);

      service->counter = 0;
      g_clear_pointer(&service->status, g_barbar_dwl_status_unref);
    }

    g_data_input_stream_read_line_async(input, G_PRIORITY_DEFAULT, NULL,
                                        g_barbar_dwl_service_file_data, data);
  }
}

static void g_barbar_dwl_service_file_changed(GFileMonitor *self, GFile *file,
                                              GFile *other_file,
                                              GFileMonitorEvent event_type,
                                              gpointer data) {
  BarBarDwlService *service = BARBAR_DWL_SERVICE(data);
  gsize length;
  GError *error = NULL;

  // g_data_input_stream_read_line_async(service->input, G_PRIORITY_DEFAULT,
  // NULL,
  //                                     g_barbar_dwl_service_file_data, data);
  // if (event_type == G_FILE_MONITOR_EVENT_CHANGED) {
  char *line;

  while ((line = g_data_input_stream_read_line(service->input, &length, NULL,
                                               &error))) {
    if (error) {
      g_printerr("Failed to read input stream from dwl: %s", error->message);
      g_error_free(error);
      return;
    }

    if (!service->status) {
      service->status = g_barbar_dwl_status_new();
    }

    parse_line(line, service->status, &error);

    service->counter++;

    if (service->counter == 6) {
      g_signal_emit(service, listener, 0, service->status);

      service->counter = 0;
      g_clear_pointer(&service->status, g_barbar_dwl_status_unref);
    }
  }
  // }
}

static void g_barbar_dwl_service_readfile(GObject *object, GAsyncResult *res,
                                          gpointer data) {
  GDataInputStream *input = G_DATA_INPUT_STREAM(object);
  BarBarDwlService *service = BARBAR_DWL_SERVICE(data);
  GError *error = NULL;
  gsize length;
  gchar *line;

  line = g_data_input_stream_read_line_finish(input, res, &length, &error);

  if (error) {
    g_printerr("Failed to read input stream from dwl: %s", error->message);
    return;
  }

  if (line) {
    // is this function defensive enough?

    if (!service->status) {
      service->status = g_barbar_dwl_status_new();
    }

    parse_line(line, service->status, &error);

    if (error) {
      g_printerr("dwl parse input: %s", error->message);
      g_error_free(error);
      return;
    }

    service->counter++;

    if (service->counter == 6) {
      g_signal_emit(service, listener, 0, service->status);

      service->counter = 0;
      g_clear_pointer(&service->status, g_barbar_dwl_status_unref);
    }

    g_data_input_stream_read_line_async(input, G_PRIORITY_DEFAULT, NULL,
                                        g_barbar_dwl_service_readfile, data);
  } else {
    g_signal_connect(service->monitor, "changed",
                     G_CALLBACK(g_barbar_dwl_service_file_changed), service);
  }
}

static void g_barbar_dwl_service_constructed(GObject *self) {
  BarBarDwlService *service = BARBAR_DWL_SERVICE(self);
  GError *error = NULL;

  if (service->file_path) {
    service->file = g_file_new_for_path(service->file_path);
    service->file_input_stream = g_file_read(service->file, NULL, &error);

    if (service->file_input_stream == NULL) {
      g_printerr("Error opening file: %s\n", error->message);
      g_error_free(error);
      // cleanup
      return;
    }
    service->stream = G_INPUT_STREAM(service->file_input_stream);
    service->monitor =
        g_file_monitor_file(service->file, G_FILE_MONITOR_NONE, NULL, &error);

    if (error) {
      g_printerr("Error opening file: %s\n", error->message);
      g_error_free(error);
      // cleanup
      return;
    }

    service->input = g_data_input_stream_new(service->stream);
    g_data_input_stream_read_line_async(service->input, G_PRIORITY_DEFAULT,
                                        NULL, g_barbar_dwl_service_readfile,
                                        service);
    // g_barbar_dwl_service_readfile(service);
  } else {
    // or we have piped from stdin
    service->stream = g_unix_input_stream_new(STDIN_FILENO, FALSE);
    service->input = g_data_input_stream_new(service->stream);

    g_data_input_stream_read_line_async(service->input, G_PRIORITY_DEFAULT,
                                        NULL, g_barbar_dwl_service_pipe_reader,
                                        service);
  }
}

static void g_barbar_dwl_service_class_init(BarBarDwlServiceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_rotary_set_property;
  gobject_class->get_property = g_barbar_rotary_get_property;

  gobject_class->constructor = g_barbar_dwl_service_constructor;
  gobject_class->constructed = g_barbar_dwl_service_constructed;

  dwl_service_props[PROP_FILEPATH] = g_param_spec_string(
      "file_path", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dwl_service_props);

  listener =
      g_signal_new("listener", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 1, BARBAR_TYPE_DWL_STATUS);
}

static void g_barbar_dwl_service_init(BarBarDwlService *self) {}

BarBarDwlService *g_barbar_dwl_service_new(const char *file_path) {
  BarBarDwlService *dwl =
      g_object_new(BARBAR_TYPE_DWL_SERVICE, "file_path", file_path, NULL);

  return dwl;
}
