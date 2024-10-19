#include "barbar-cava.h"
#include "cava/config.h"
#include "cava/input/common.h"
#include "cava/output/common.h"

struct _BarBarCava {
  GtkWidget parent_instance;

  gint bars; // maybe not
  gboolean auto_sens;
  gboolean stereo;
  gdouble noise_reduction;
  gint framerate;
  BarBarCavaInput input;
  gchar *audio_source;
  gboolean active;
  gint channels;
  gint low_cutoff;
  gint high_cutoff;
  gint samplerate;

  GArray *values;
};

enum {
  PROP_0,

  PROP_BARS,
  PROP_AUTO_SENSE,
  PROP_STEREO,
  PROP_NOISE_REDUCTION,
  PROP_FRAME_RATE,
  PROP_INPUT,
  PROP_SOURCE,
  PROP_CHANNELS,
  PROP_LOW_CUTOFF,
  PROP_HIGH_CUTOFF,
  PROP_SAMPLERATE,

  PROP_VALUES,

  NUM_PROPERTIES,
};

typedef struct {
  struct cava_plan plan;
  struct config_params cfg;
  struct audio_data audio_data;
  struct audio_raw audio_raw;
  // ptr input_src;

  // gboolean constructed;
  // GThread* input_thread;
  // guint timer_id;

} BarBarCavaPrivate;

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarCava, g_barbar_cava, GTK_TYPE_WIDGET)

G_DEFINE_ENUM_TYPE(BarBarCavaInput, g_barbar_cava_input,
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_FIFO, "fifo"),
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_PORTAUDIO,
                                       "portaudio"),
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_PIPEWIRE, "pipewire"),
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_ALSA, "alsa"),
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_PULSE, "pulse"),
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_SNDIO, "sndio"),
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_SHMEM, "shmem"),
                   G_DEFINE_ENUM_VALUE(BARBAR_CAVA_INPUT_WINSCAP, "winscap"));

static void g_barbar_cava_tick(BarBarCava *cava) {
  BarBarCavaPrivate *priv = g_barbar_cava_get_instance_private(cava);

  // pthread_mutex_lock(&priv->audio_data.lock);
  cava_execute(priv->audio_data.cava_in, priv->audio_data.samples_counter,
               priv->audio_raw.cava_out, &priv->plan);
  // if (priv->audio_data.samples_counter > 0) priv->audio_data.samples_counter
  // = 0; pthread_mutex_unlock(&priv->audio_data.lock);

  // g_array_remove_range(self->values, 0, priv->audio_raw.number_of_bars);
  // g_array_insert_vals(self->values, 0, priv->audio_raw.cava_out,
  // priv->audio_raw.number_of_bars);

  g_object_notify_by_pspec(G_OBJECT(cava), properties[PROP_VALUES]);
}

static void g_barbar_graph_start(GtkWidget *widget);

static void g_barbar_graph_set_stroke_width(BarBarCava *self, float stroke) {
  // g_return_if_fail(BARBAR_IS(self));
  //
  // if (self->stroke_size == stroke) {
  //   return;
  // }
  //
  // self->stroke_size = stroke;
  // self->stroke = gsk_stroke_new(stroke);
}

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
  // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_cava_set_property;
  gobject_class->get_property = g_barbar_cava_get_property;

  // gobject_class->dispose = g_barbar_cava_dispose;

  // widget_class->root = g_barbar_graph_start;

  /**
   * BarBarGraph:discrete:
   *
   * If a discrete graph should be drawn, produces a bar like graph.
   */
  properties[PROP_BARS] = g_param_spec_boolean(
      "bars", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_AUTO_SENSE] = g_param_spec_boolean(
      "auto-sense", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_STEREO] = g_param_spec_boolean(
      "stereo", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_NOISE_REDUCTION] = g_param_spec_boolean(
      "bars", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_FRAME_RATE] = g_param_spec_boolean(
      "bars", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_INPUTE] = g_param_spec_boolean(
      "bars", NULL, NULL, FALSE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
  gtk_widget_class_set_css_name(widget_class, "graph");
}

static gboolean g_barbar_graph_update(gpointer data) {

  BarBarGraph *self = BARBAR_GRAPH(data);

  push_update(self, self->current);

  gtk_widget_queue_draw(GTK_WIDGET(self));

  return G_SOURCE_CONTINUE;
}

static void g_barbar_graph_init(BarBarGraph *self) {
  gtk_widget_get_color(GTK_WIDGET(self), &self->color);
  self->queue = g_queue_new();
  g_queue_init(self->queue);

  self->current = 0.0;
  self->min_value = 0.0;
  self->max_value = 1.0;
  self->interval = 1000;

  push_update(self, self->current);
}

static void g_barbar_graph_start(GtkWidget *widget) {
  GTK_WIDGET_CLASS(g_barbar_graph_parent_class)->root(widget);

  BarBarGraph *self = BARBAR_GRAPH(widget);

  self->tick_cb =
      g_timeout_add_full(0, self->interval, g_barbar_graph_update, self, NULL);
}

GtkWidget *g_barbar_graph_new(void) {
  return g_object_new(BARBAR_TYPE_GRAPH, NULL);
}
