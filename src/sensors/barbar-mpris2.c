#include "barbar-mpris2.h"
#include <playerctl/playerctl.h>
#include <stdio.h>

struct _BarBarMpris {
  BarBarSensor parent_instance;

  PlayerctlPlayerManager *manager;
  PlayerctlPlayer *player;

  char *player_name;
};

enum {
  PROP_0,

  PROP_PLAYER,

  // PROP_SONG,
  // PROP_DURATION,
  // PROP_LENGTH,

  NUM_PROPERTIES,
};

enum {
  STATE_UPDATE,
};

G_DEFINE_TYPE(BarBarMpris, g_barbar_mpris, BARBAR_TYPE_SENSOR)

static GParamSpec *mpris_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_mpris_start(BarBarSensor *sensor);

void g_barbar_mpris_set_player(BarBarMpris *mpris, const char *player) {
  g_return_if_fail(BARBAR_IS_MPRIS(mpris));

  if (mpris->player_name && !strcmp(mpris->player_name, player)) {
    return;
  }

  g_free(mpris->player_name);
  mpris->player_name = g_strdup(player);

  g_object_notify_by_pspec(G_OBJECT(mpris), mpris_props[PROP_PLAYER]);
}

static void g_barbar_mpris_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarMpris *mpris = BARBAR_MPRIS(object);

  switch (property_id) {
  case PROP_PLAYER:
    g_barbar_mpris_set_player(mpris, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mpris_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {}

static void g_barbar_mpris_class_init(BarBarMprisClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_mpris_set_property;
  gobject_class->get_property = g_barbar_mpris_get_property;
  sensor->start = g_barbar_mpris_start;

  /**
   * BarBarMpris:player
   *
   * The name of the player we want to watch
   *
   */
  mpris_props[PROP_PLAYER] =
      g_param_spec_string("player", NULL, NULL, "mpd", G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, mpris_props);
}

static void g_barbar_mpris_init(BarBarMpris *self) {}

static void g_barbar_mpris_on_play(PlayerctlPlayer *player, gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);
  g_print("Playing\n");
}

static void g_barbar_mpris_on_stop(PlayerctlPlayer *player, gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);
  g_print("Stopped\n");
}

static void g_barbar_mpris_on_pause(PlayerctlPlayer *player, gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);
  g_print("Paused\n");
}

static void g_barbar_mpris_on_metadata(PlayerctlPlayer *player,
                                       GVariant *metadata, gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);
  g_print("metadata\n");
}

static void g_barbar_mpris_connect_events(PlayerctlPlayer *player,
                                          BarBarMpris *mpris) {}

PlayerctlPlayer *g_barbar_mpris_get_player(BarBarMpris *mpris) {
  return mpris->player;
}

gchar *g_barbar_mpris_format_player(BarBarMpris *mpris, const char *format) {
  return NULL;
}

static void g_barbar_mpris_player_update(PlayerctlPlayer *player,
                                         gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);

  g_signal_emit(mpris, STATE_UPDATE, 0, NULL);
}

static void g_barbar_mpris_player_vanished(PlayerctlPlayerManager *manager,
                                           PlayerctlPlayerName *player_name,
                                           gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);

  if (!mpris->player || strcmp(player_name->name, mpris->player_name)) {
    return;
  }

  g_clear_pointer(&mpris->player, g_object_unref);
}

static void g_barbar_mpris_player_appeared(PlayerctlPlayerManager *manager,
                                           PlayerctlPlayerName *player_name,
                                           gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);
  PlayerctlPlayer *player = NULL;
  GError *err = NULL;

  if (strcmp(player_name->name, mpris->player_name)) {
    return;
  }

  player = playerctl_player_new_from_name(player_name, &err);

  if (!player || err) {
    fprintf(stderr, "Unable to read file: %s\n", err->message);
    g_error_free(err);
    return;
  }
  g_signal_connect(player, "notify::metadata",
                   G_CALLBACK(g_barbar_mpris_player_update), data);
}

// gboolean g_barbar_mpris_get_playing(BarBarMpris *mpris) { return 0; }
//
// gboolean g_barbar_mpris_is_seekable(BarBarMpris *mpris) { return 0; }
//
// gint g_barbar_mpris_get_position(BarBarMpris *mpris) { return 0; }
// gint g_barbar_mpris_get_duration(BarBarMpris *mpris) { return 0; }
//
// void g_barbar_mpris_seek(BarBarMpris *mpris, gint64 pos) { return; }
//
// double g_barbar_mpris_get_volume(BarBarMpris *mpris) { return 0; }
// // gboolean g_barbar_mpris_get_muted(BarBarMpris *mpris) { return 0; }
//
// void gtk_media_stream_set_muted(BarBarMpris *mpris, double volume) {}
//
// void g_barbar_mpris_set_muted(BarBarMpris *mpris, gboolean mute) {}
//
// void g_barbar_mpris_set_play_pause(BarBarMpris *mpris) {}
//
// void g_barbar_mpris_next(BarBarMpris *mpris) { return; }
// void g_barbar_mpris_prev(BarBarMpris *mpris) { return; }

static void g_barbar_mpris_start(BarBarSensor *sensor) {
  BarBarMpris *mpris = BARBAR_MPRIS(sensor);
  GError *error = NULL;
  PlayerctlPlayerManager *manager = playerctl_player_manager_new(&error);

  g_return_if_fail(mpris);

  g_object_connect(manager, "signal::name-appeared",
                   G_CALLBACK(g_barbar_mpris_player_appeared), mpris, NULL);
  g_object_connect(manager, "signal::name-vanished",
                   G_CALLBACK(g_barbar_mpris_player_vanished), mpris, NULL);

  // g_signal_connect(manager, "name-appeared",
  // G_CALLBACK(g_barbar_mpris_player_appeared), mpris);
  // g_signal_connect(manager, "name-vanished",
  // G_CALLBACK(g_barbar_mpris_player_vanished), mpris);

  GList *players = playerctl_list_players(&error);
  if (error) {
    printf("unable to list players: %s", error->message);
    g_error_free(error);
    return;
  }

  for (; players != NULL; players = players->next) {
    PlayerctlPlayerName *pn = players->data;
    g_barbar_mpris_player_appeared(manager, pn, mpris);
  }
}
