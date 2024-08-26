#include "barbar-playbutton.h"
#include "barbar-enum.h"
// #include <glib/gi18n-lib.h>

/**
 * BarBarPlayButton:
 *
 * A play button that toggles between play and pause.
 */
struct _BarBarPlayButton {
  GtkButton parent_instance;
  BarBarMprisPlaybackStatus status;
  char *str;
};

enum {
  PROP_0,

  PROP_STATUS,

  NUM_PROPERTIES,
};

static GParamSpec *button_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarPlayButton, g_barbar_play_button, GTK_TYPE_BUTTON);

void g_barbar_play_button_set_status(BarBarPlayButton *button,
                                     BarBarMprisPlaybackStatus status) {
  g_return_if_fail(BARBAR_IS_PLAY_BUTTON(button));
  const char *icon_name;

  if (button->status == status)
    return;

  button->status = status;

  if (status == BARBAR_PLAYBACK_STATUS_PLAYING) {
    icon_name = "media-playback-pause-symbolic";
    // tooltip_text = C_("media controls tooltip", "Play");
  } else {
    icon_name = "media-playback-start-symbolic";
    // tooltip_text = C_("media controls tooltip", "Stop");
  }

  gtk_button_set_icon_name(GTK_BUTTON(button), icon_name);
  // gtk_widget_set_tooltip_text(GTK_BUTTON(button), tooltip_text);

  g_object_notify_by_pspec(G_OBJECT(button), button_props[PROP_STATUS]);
}

static void g_barbar_play_button_set_property(GObject *object,
                                              guint property_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  BarBarPlayButton *play = BARBAR_PLAY_BUTTON(object);

  switch (property_id) {
  case PROP_STATUS:
    g_barbar_play_button_set_status(play, g_value_get_enum(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void clicked(BarBarPlayButton *button, gpointer data) {
  BarBarMprisPlaybackStatus next;
  if (button->status == BARBAR_PLAYBACK_STATUS_PLAYING) {
    next = BARBAR_PLAYBACK_STATUS_PAUSED;
  } else {
    next = BARBAR_PLAYBACK_STATUS_PLAYING;
  }
  g_barbar_play_button_set_status(button, next);
}

static void g_barbar_play_button_get_property(GObject *object,
                                              guint property_id, GValue *value,
                                              GParamSpec *pspec) {

  BarBarPlayButton *play = BARBAR_PLAY_BUTTON(object);
  switch (property_id) {
  case PROP_STATUS:
    g_value_set_enum(value, play->status);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_play_button_class_init(BarBarPlayButtonClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  gobject_class->set_property = g_barbar_play_button_set_property;
  gobject_class->get_property = g_barbar_play_button_get_property;

  /**
   * BarBarBar:status:
   *
   * [enum@BarBarPlayback.status] status of the player connected to the button
   */
  button_props[PROP_STATUS] =
      g_param_spec_enum("status", NULL, NULL, BARBAR_TYPE_PLAYBACK_STATUS,
                        BARBAR_PLAYBACK_STATUS_STOPPED,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
}

static void g_barbar_play_button_init(BarBarPlayButton *self) {
  g_signal_connect(self, "clicked", G_CALLBACK(clicked), NULL);
}
