#include "barbar-dwl-service.h"
#include "barbar-error.h"
#include "glib-object.h"
#include "glib.h"
#include <gio/gio.h>
#include <gio/gunixinputstream.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/**
 * BarBarDwlService:
 * A background service for dwl. For BarBarDwlService to work
 * you need to pipe dwl into barbar.
 *
 *
 */
struct _BarBarDwlService {
  BarBarSensor parent_instance;

  // path to the data, if null, it will read stdin.
  char *file_path;
  GFileMonitor *monitor;
  GDataInputStream *input;
};

typedef enum {
  DWL_STARTED,
  DWL_TAGS,
  DWL_LAYOUT,
  DWL_TITLE,
  DWL_APPID,
  DWL_FULLSCREEN,
  DWL_FLOATING,
  DWL_SELMON,
} g_barbar_dwl_message_kind;

enum {
  TAGS,
  LAYOUT,
  TITLE,
  APPID,
  FULLSCREEN,
  FLOATING,
  SELMON,
  NUM_SIGNALS,
};

static guint dwl_service_signals[NUM_SIGNALS];

static void parse_line(BarBarDwlService *service, char *line, GError **error) {
  g_barbar_dwl_message_kind kind;
  uint32_t num;

  if (!line) {
    return;
  }

  char *save_pointer = NULL;
  char *string = strtok_r(line, " ", &save_pointer);
  char *output_name;

  if (!string) {
    g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_DWL_IPC,
                "empty line from ipc");
    return;
  }

  output_name = string;

  string = strtok_r(NULL, " ", &save_pointer);

  if (!string) {
    g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_DWL_IPC,
                "No kind paramenter");
    return;
  }

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
    g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_DWL_IPC,
                "%s not a valid kind", string);
    return;
  }

  string = strtok_r(NULL, " ", &save_pointer);

  switch (kind) {
  case DWL_TITLE: {
    char *title = string;
    g_signal_emit(service, dwl_service_signals[TITLE], 0, output_name, title);
    break;
  }
  case DWL_APPID: {
    char *appid = string;
    g_signal_emit(service, dwl_service_signals[APPID], 0, output_name, appid);
    break;
  }
  case DWL_FULLSCREEN: {
    gboolean fullscreen = g_strcmp0(string, "1") == 0;
    g_signal_emit(service, dwl_service_signals[FULLSCREEN], 0, output_name,
                  fullscreen);
    break;
  }
  case DWL_FLOATING: {
    gboolean floating = g_strcmp0(string, "1") == 0;
    g_signal_emit(service, dwl_service_signals[FLOATING], 0, output_name,
                  floating);
    break;
  }
  case DWL_SELMON: {
    gboolean selmon = g_strcmp0(string, "1") == 0;
    g_signal_emit(service, dwl_service_signals[SELMON], 0, output_name, selmon);
    break;
  }
  case DWL_LAYOUT: {
    char *layout = string;
    g_signal_emit(service, dwl_service_signals[LAYOUT], 0, output_name, layout);
    break;
  }
  case DWL_TAGS: {
    int id = 0;
    uint32_t occupied, selected, client_tags, urgent;
    while (string != NULL) {
      char *endptr;
      num = strtoul(string, &endptr, 10);

      if (string == endptr) {
        g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_DWL_IPC,
                    "Tag argument number %d: isn't a number %s", id, string);
        return;
      }
      if (id == 0) {
        occupied = num;
      } else if (id == 1) {
        selected = num;
      } else if (id == 2) {
        client_tags = num;
      } else if (id == 3) {
        urgent = num;
      }
      string = strtok_r(NULL, " ", &save_pointer);
      id++;
    }
    if (id != 4) {
      g_set_error(error, BARBAR_ERROR, BARBAR_ERROR_BAD_DWL_IPC,
                  "Bad number of argumentst to tags");
      return;
    }
    g_signal_emit(service, dwl_service_signals[TAGS], 0, output_name, occupied,
                  selected, client_tags, urgent);
    break;
  }
  }
}

G_DEFINE_TYPE(BarBarDwlService, g_barbar_dwl_service, BARBAR_TYPE_SENSOR)

enum {
  PROP_0,

  PROP_FILEPATH,

  NUM_PROPERTIES,
};

static GParamSpec *dwl_service_props[NUM_PROPERTIES] = {
    NULL,
};

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

  parse_line(service, line, &error);

  if (error) {
    g_printerr("dwl parse input: %s", error->message);
    g_error_free(error);
    return;
  }
  g_data_input_stream_read_line_async(input, G_PRIORITY_DEFAULT, NULL,
                                      g_barbar_dwl_service_pipe_reader, data);
}

static void g_barbar_dwl_service_set_pipe(BarBarDwlService *self,
                                          const char *file_path) {
  g_return_if_fail(BARBAR_IS_DWL_SERVICE(self));

  if (g_set_str(&self->file_path, file_path)) {
    g_object_notify_by_pspec(G_OBJECT(self), dwl_service_props[PROP_FILEPATH]);
  }
}

static void g_barbar_dwl_service_set_property(GObject *object,
                                              guint property_id,
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

static void g_barbar_dwl_service_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_dwl_service_read_file(BarBarDwlService *service) {
  GError *error = NULL;
  gsize length;
  char *line;

  while ((line = g_data_input_stream_read_line(service->input, &length, NULL,
                                               &error))) {
    if (error) {
      g_printerr("Failed to read input stream from dwl: %s", error->message);
      g_error_free(error);
      return;
    }

    parse_line(service, line, &error);

    if (error) {
      g_printerr("dwl parse input: %s", error->message);
      g_error_free(error);
      return;
    }
  }
}

static void g_barbar_dwl_service_file_changed(GFileMonitor *self, GFile *file,
                                              GFile *other_file,
                                              GFileMonitorEvent event_type,
                                              gpointer data) {
  BarBarDwlService *service = BARBAR_DWL_SERVICE(data);
  g_barbar_dwl_service_read_file(service);
}

static void g_barbar_dwl_service_start(BarBarSensor *sensor) {

  static GMutex __BarBarDwlService_started;
  static volatile gsize __BarBarDwlService_once = 0;
  static gboolean started = FALSE;

  if (g_once_init_enter(&__BarBarDwlService_once) == TRUE) {
    g_mutex_init(&__BarBarDwlService_started);
    g_once_init_leave(&__BarBarDwlService_once, 42);
  }
  g_mutex_lock(&__BarBarDwlService_started);
  if (started) {
    g_mutex_unlock(&__BarBarDwlService_started);
    return;
  }
  started = TRUE;
  g_mutex_unlock(&__BarBarDwlService_started);

  BarBarDwlService *service = BARBAR_DWL_SERVICE(sensor);
  GError *error = NULL;

  if (service->file_path) {
    GFileType ft;
    GFileInputStream *file_input_stream = NULL;
    GFile *file = NULL;

    file = g_file_new_for_path(service->file_path);
    ft = g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL);

    file_input_stream = g_file_read(file, NULL, &error);

    if (file_input_stream == NULL) {
      g_printerr("Error opening file: %s\n", error->message);
      g_error_free(error);
      g_clear_object(&file);
      return;
    }
    service->input = g_data_input_stream_new(G_INPUT_STREAM(file_input_stream));
    g_clear_object(&file_input_stream);

    // if we have a fifo, we can read it like a pipe
    if (ft & G_FILE_TYPE_SPECIAL) {
      g_data_input_stream_read_line_async(
          service->input, G_PRIORITY_DEFAULT, NULL,
          g_barbar_dwl_service_pipe_reader, service);
    } else {
      service->monitor =
          g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, &error);

      if (error) {
        g_printerr("Error opening file: %s\n", error->message);
        g_error_free(error);
        g_clear_object(&file);
        return;
      }
      g_barbar_dwl_service_read_file(service);

      g_signal_connect(service->monitor, "changed",
                       G_CALLBACK(g_barbar_dwl_service_file_changed), service);
    }
    g_clear_object(&file);
  } else {
    // or we have piped from stdin
    GInputStream *stream;
    stream = g_unix_input_stream_new(STDIN_FILENO, FALSE);
    service->input = g_data_input_stream_new(stream);

    g_clear_object(&stream);

    g_data_input_stream_read_line_async(service->input, G_PRIORITY_DEFAULT,
                                        NULL, g_barbar_dwl_service_pipe_reader,
                                        service);
  }
}

static void g_barbar_dwl_service_finalize(GObject *object) {
  BarBarDwlService *dwl = BARBAR_DWL_SERVICE(object);

  g_free(dwl->file_path);
  g_clear_object(&dwl->monitor);
  g_clear_object(&dwl->input);
  G_OBJECT_CLASS(g_barbar_dwl_service_parent_class)->finalize(object);
}

static void g_barbar_dwl_service_class_init(BarBarDwlServiceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_dwl_service_set_property;
  gobject_class->get_property = g_barbar_dwl_service_get_property;
  gobject_class->finalize = g_barbar_dwl_service_finalize;

  gobject_class->constructor = g_barbar_dwl_service_constructor;
  sensor_class->start = g_barbar_dwl_service_start;

  dwl_service_props[PROP_FILEPATH] = g_param_spec_string(
      "file_path", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    dwl_service_props);

  /**
   * BarBarDwlService::tags:
   * @service: this object
   * @monitor: name of the monitor
   * @occupied: occupied tags
   * @selected: selected tags
   * @clienttag: if it's occupied
   * @urgent: urgent tags
   *
   * Tag information
   */
  dwl_service_signals[TAGS] =
      g_signal_new("tags", G_TYPE_FROM_CLASS(class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 5, G_TYPE_STRING,
                   G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);
  /**
   * BarBarDwlService::layout:
   * @service: this object
   * @monitor: name of the monitor
   * @layout: String describing the layout
   *
   * Current layout
   */
  dwl_service_signals[LAYOUT] = g_signal_new(
      "layout", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarDwlService::title:
   * @service: this object
   * @monitor: name of the monitor
   * @title: (nullable): String title of the current view
   *
   * Current title
   */
  dwl_service_signals[TITLE] = g_signal_new(
      "title", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarDwlService::appid:
   * @service: this object
   * @monitor: name of the monitor
   * @appid: (nullable): String appid of the current view
   *
   */
  dwl_service_signals[APPID] = g_signal_new(
      "appid", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * BarBarDwlService::fullscreen:
   * @service: this object
   * @monitor: name of the monitor
   * @fullscreen: if we in fullscreen mode
   *
   */
  dwl_service_signals[FULLSCREEN] = g_signal_new(
      "fullscreen", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);

  /**
   * BarBarDwlService::floating:
   * @service: this object
   * @monitor: name of the monitor
   * @floating: if we are in floating mode
   *
   */
  dwl_service_signals[FLOATING] = g_signal_new(
      "floating", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);

  /**
   * BarBarDwlService::selmon:
   * @service: this object
   * @monitor: name of the monitor
   * @selmon: if the monitor is selected
   *
   */
  dwl_service_signals[SELMON] = g_signal_new(
      "selmon", G_TYPE_FROM_CLASS(class),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);
}

static void g_barbar_dwl_service_init(BarBarDwlService *self) {}

BarBarDwlService *g_barbar_dwl_service_new(const char *file_path) {
  BarBarDwlService *dwl =
      g_object_new(BARBAR_TYPE_DWL_SERVICE, "file_path", file_path, NULL);

  return dwl;
}
