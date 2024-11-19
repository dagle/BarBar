#include "barbar-cava.h"
#include "cava/config.h"
#include "cava/input/common.h"
#include "cava/output/common.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "widgets/barbar-graph.h"

#include <pipewire/impl-metadata.h>
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/type-info.h>
#include <spa/param/latency-utils.h>
#include <stdint.h>

#define SAMPLE_RATE 44100

struct _BarBarCava {
  BarBarGraph parent_instance;

  struct pw_main_loop *loop;
  struct pw_stream *stream;
  struct spa_audio_info format;

  gboolean auto_sens;
  gboolean stereo;
  gdouble noise_reduction;
  gint framerate;
  gchar *audio_source;
  gboolean active;
  gint channels;
  gint low_cutoff;
  gint high_cutoff;
  gint samplerate;

  // GArray *values;
};

enum {
  PROP_0,

  PROP_AUTO_SENSE,
  PROP_STEREO,
  PROP_NOISE_REDUCTION,
  PROP_FRAME_RATE,

  PROP_INPUT,
  PROP_SOURCE,
  PROP_CHANNELS,
  PROP_LOW_CUTOFF,
  PROP_HIGH_CUTOFF,

  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarCava, g_barbar_cava, BARBAR_TYPE_GRAPH)

static void g_barbar_cava_start(GtkWidget *widget);

static void g_barbar_cava_set_property(GObject *object, guint property_id,
                                       const GValue *value, GParamSpec *pspec) {
  BarBarCava *cava = BARBAR_CAVA(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cava_get_property(GObject *object, guint property_id,
                                       GValue *value, GParamSpec *pspec) {
  BarBarCava *cava = BARBAR_CAVA(object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cava_class_init(BarBarCavaClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_cava_set_property;
  gobject_class->get_property = g_barbar_cava_get_property;

  // gobject_class->dispose = g_barbar_cava_dispose;

  widget_class->root = g_barbar_cava_start;

  /**
   * BarBarCava:auto-sense:
   *
   * How many bars we should render
   */
  properties[PROP_AUTO_SENSE] =
      g_param_spec_boolean("auto-sense", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:stero:
   *
   * How many bars we should render
   */
  properties[PROP_STEREO] = g_param_spec_boolean(
      "stereo", NULL, NULL, FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:noice-reduction:
   *
   * How many bars we should render
   */
  properties[PROP_NOISE_REDUCTION] =
      g_param_spec_boolean("noice-reduction", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:framerate:
   *
   * How many bars we should render
   */
  properties[PROP_FRAME_RATE] =
      g_param_spec_boolean("framerate", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:input:
   *
   * How many bars we should render
   */
  properties[PROP_INPUT] = g_param_spec_boolean(
      "input", NULL, NULL, FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:source:
   *
   * How many bars we should render
   */
  properties[PROP_SOURCE] = g_param_spec_boolean(
      "source", NULL, NULL, FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:channels:
   *
   * How many bars we should render
   */
  properties[PROP_CHANNELS] =
      g_param_spec_boolean("channels", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:low-cutoff:
   *
   * How many bars we should render
   */
  properties[PROP_LOW_CUTOFF] =
      g_param_spec_boolean("low-cutoff", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /**
   * BarBarCava:high-cutoff:
   *
   * How many bars we should render
   */
  properties[PROP_HIGH_CUTOFF] =
      g_param_spec_boolean("high-cutoff", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
}

#define BUFFER_SIZE2 1024 // FFT size (must be a power of 2)

static void g_barbar_cava_init(BarBarCava *self) { self->framerate = 44100; }

static void on_process(void *userdata) {
  struct pw_buffer *b;
  struct spa_buffer *buf;
  uint32_t n_samples;
  // int16_t *samples;
  int16_t *data;

  BarBarCava *cava = BARBAR_CAVA(userdata);

  // if (data->cava_audio->terminate == 1)
  //   pw_main_loop_quit(data->loop);
  //
  if ((b = pw_stream_dequeue_buffer(cava->stream)) == NULL) {
    pw_log_warn("out of buffers: %m\n");
    return;
  }

  buf = b->buffer;

  if (!buf->datas || !buf->datas[0].data) {
    pw_stream_queue_buffer(cava->stream, b);
    printf("error:\n");
    return;
  }

  data = buf->datas[0].data;
  n_samples = buf->datas[0].chunk->size / sizeof(int16_t);

  if (n_samples < BUFFER_SIZE2) {
    pw_stream_queue_buffer(cava->stream, b);
    return; // Wait until enough samples are available
  }

  double *input = (double *)fftw_malloc(sizeof(double) * BUFFER_SIZE2);
  fftw_complex *output = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) *
                                                     (BUFFER_SIZE2 / 2 + 1));

  fftw_plan plan =
      fftw_plan_dft_r2c_1d(BUFFER_SIZE2, input, output, FFTW_ESTIMATE);

  for (int i = 0; i < BUFFER_SIZE2; i++) {
    input[i] = data[i] / 32768.0; // Normalize 16-bit PCM to range -1 to 1
  }

  // Execute the FFT
  fftw_execute(plan);

  int num_bands = g_barbar_graph_get_entry_numbers(BARBAR_GRAPH(cava));
  int band_size = (BUFFER_SIZE / 2) /
                  num_bands; // Calculate the number of FFT bins per band

  for (int band = 0; band < num_bands; band++) {

    double sum_magnitude = 0.0;
    int start = band * band_size;
    int end = start + band_size;

    for (int i = start; i < end; i++) {
      double real = output[i][0];
      double imag = output[i][1];
      sum_magnitude += sqrt(real * real + imag * imag);
    }

    g_barbar_graph_push_entry(BARBAR_GRAPH(cava), sum_magnitude);
    // Store average magnitude for the band
    // double *v = head->data;
    // *v = sum_magnitude / band_size;
    // head = head->next;
    // g_queue_push_tail(queue, 5);
  }
  g_barbar_graph_update_path(BARBAR_GRAPH(cava));
  static int i = 0;
  printf("i: %d\n", i);
  i++;

  // g_barbar_graph_set_entries(BARBAR_GRAPH(cava), cava->queue);
  gtk_widget_queue_draw(GTK_WIDGET(cava));

  // Print the 10 frequency band magnitudes
  // printf("Frequency Bands:\n");
  // for (int i = 0; i < num_bands; i++) {
  //   printf("Band %d: Magnitude %.2f\n", i + 1, band_magnitudes[i]);
  // }

  // Cleanup
  fftw_destroy_plan(plan);
  fftw_free(input);
  fftw_free(output);

  // Re-queue the buffer
  pw_stream_queue_buffer(cava->stream, b);
}

static void on_stream_param_changed(void *userdata, uint32_t id,
                                    const struct spa_pod *param) {
  // printf("param changed!\n");
  BarBarCava *cava = BARBAR_CAVA(userdata);

  if (param == NULL || id != SPA_PARAM_Format) {
    return;
  }

  if (spa_format_parse(param, &cava->format.media_type,
                       &cava->format.media_subtype) < 0) {
    return;
  }

  if (cava->format.media_type != SPA_MEDIA_TYPE_audio ||
      cava->format.media_subtype != SPA_MEDIA_SUBTYPE_raw) {
    return;
  }

  if (spa_format_audio_raw_parse(param, &cava->format.info.raw) < 0) {
    return;
  }

  // printf("got video format:\n");
  // printf("  format: %d %d\n", cava->format.info.raw.format,
  //        SPA_AUDIO_FORMAT_S16_LE);
  // spa_debug_type_find_name(spa_type_video_format,
  //                          cava->format.info.raw.format));
  // printf("  size: %dx%d\n", cava->format.info.raw.size.width,
  //        cava->format.info.raw.size.height);
  // printf("  framerate: %d/%d\n", cava->format.info.raw.framerate.num,
  //        cava->format.info.raw.framerate.denom);
}

static const struct pw_stream_events stream_events = {
    PW_VERSION_STREAM_EVENTS,
    .param_changed = on_stream_param_changed,
    .process = on_process,
};

static gboolean on_pipewire_event(GIOChannel *channel, GIOCondition condition,
                                  gpointer data) {
  struct pw_loop *loop = (struct pw_loop *)data;

  // Run PipeWire's loop to process pending events
  pw_loop_iterate(loop, 0); // Non-blocking iteration

  // Return TRUE to keep the event source active, FALSE to remove it
  return TRUE;
}

static void setup_eventloop(BarBarCava *self) {
  struct pw_loop *pw_loop = pw_main_loop_get_loop(self->loop);
  // Get the file descriptor associated with PipeWire's loop
  int pw_fd = pw_loop_get_fd(pw_loop);

  // Create a GIOChannel from the PipeWire file descriptor
  GIOChannel *gio_channel = g_io_channel_unix_new(pw_fd);

  // Add the PipeWire file descriptor to the GLib main loop
  g_io_add_watch(gio_channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_pipewire_event,
                 pw_loop);

  // Cleanup: unreference the GIOChannel (the event source will keep it alive)
  g_io_channel_unref(gio_channel);
}

static void g_barbar_cava_start(GtkWidget *widget) {
  GTK_WIDGET_CLASS(g_barbar_cava_parent_class)->root(widget);
  BarBarCava *self = BARBAR_CAVA(widget);
  struct pw_properties *props;
  const struct spa_pod *params[1];
  pw_init(NULL, NULL);

  // TODO: better buffer size
  uint8_t buffer[BUFFER_SIZE2];
  struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

  uint32_t nom;
  nom = nearbyint((10000 * self->framerate) / 1000000.0);

  self->loop = pw_main_loop_new(NULL);
  if (self->loop == NULL) {
    g_printerr("failed to instantiate main loop");
    return;
    // goto err;
  }

  props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY,
                            "Capture", PW_KEY_MEDIA_ROLE, "Music", NULL);

  struct pw_loop *pw_loop = pw_main_loop_get_loop(self->loop);

  const char *source = "auto";
  if (strcmp(source, "auto") == 0) {
    pw_properties_set(props, PW_KEY_STREAM_CAPTURE_SINK, "true");
  } else {
    pw_properties_set(props, PW_KEY_TARGET_OBJECT, source);
  }

  // printf("%u/%u\n", nom, self->framerate);
  // pw_properties_setf(props, PW_KEY_NODE_LATENCY, "%u/%u", nom,
  // self->framerate);
  // pw_properties_setf(props, PW_KEY_NODE_LATENCY, "1024/48000");

  // pw_properties_setf(props, PW_KEY_NODE_RATE, "2205/4410");
  pw_properties_setf(props, PW_KEY_NODE_RATE,
                     "44100/44100"); // 44100 Hz, one second per update

  // pw_properties_set(props, PW_KEY_NODE_ALWAYS_PROCESS, "true");

  self->stream =
      pw_stream_new_simple(pw_loop, "cava", props, &stream_events, self);

  enum spa_audio_format audio_format = SPA_AUDIO_FORMAT_S16;

  // switch (data.cava_audio->format) {
  // case 8:
  //   audio_format = SPA_AUDIO_FORMAT_S8;
  //   break;
  // case 16:
  //   audio_format = SPA_AUDIO_FORMAT_S16;
  //   break;
  // case 24:
  //   audio_format = SPA_AUDIO_FORMAT_S24;
  //   break;
  // case 32:
  //   audio_format = SPA_AUDIO_FORMAT_S32;
  //   break;
  // };

  params[0] = spa_format_audio_raw_build(
      &b, SPA_PARAM_EnumFormat,
      &SPA_AUDIO_INFO_RAW_INIT(.format = audio_format,
                               .rate = self->framerate));

  pw_stream_connect(self->stream, PW_DIRECTION_INPUT, PW_ID_ANY,
                    PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS |
                        PW_STREAM_FLAG_RT_PROCESS,
                    params, 1);

  setup_eventloop(self);
  //
  /* Context */
  // self->context = pw_context_new(pw_main_loop_get_loop(data->loop), NULL, 0);
  // if (self->context == NULL) {
  //   LOG_ERR("failed to instantiate pipewire context");
  //   goto err;
  // }
  //
  // /* Core */
  // self->core = pw_context_connect(data->context, NULL, 0);
  // if (self->core == NULL) {
  //   LOG_ERR("failed to connect to pipewire");
  //   goto err;
  // }
  // pw_core_add_listener(self->core, &self->core_listener, &self, self);
  //
  // /* Registry */
  // self->registry = pw_core_get_registry(data->core, PW_VERSION_REGISTRY, 0);
  // if (data->registry == NULL) {
  //   LOG_ERR("failed to get core registry");
  //   goto err;
  // }
  // pw_registry_add_listener(data->registry, &data->registry_listener,
  //                          &registry_events, data);
  //
  // /* Sync */
  // data->sync = pw_core_sync(data->core, PW_ID_CORE, data->sync);
  //
  // data->module = module;
  //
  // /* node_events_param_data */
  // data->node_data_sink.data = data;
  // data->node_data_sink.is_sink = true;
  // data->node_data_source.data = data;
  // data->node_data_source.is_sink = false;

  // err:
  //   if (data->registry != NULL)
  //     pw_proxy_destroy((struct pw_proxy *)data->registry);
  //   if (data->core != NULL)
  //     pw_core_disconnect(data->core);
  //   if (data->context != NULL)
  //     pw_context_destroy(data->context);
  //   if (data->loop != NULL)
  //     pw_main_loop_destroy(data->loop);
  //   free(data);
  //   return NULL;
}

GtkWidget *g_barbar_cava_new(void) {
  return g_object_new(BARBAR_TYPE_CAVA, NULL);
}
