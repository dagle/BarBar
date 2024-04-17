#include "sensors/mpris/barbar-mpris-player.h"
#include "mpris.h"
#include "sensors/mpris/barbar-mpris-constants.h"
#include <gio/gio.h>

GType g_barbar_playback_status_get_type(void) {

  static gsize barbar_playback_status_type;
  if (g_once_init_enter(&barbar_playback_status_type)) {

    static const GEnumValue pattern_types[] = {
        {BARBAR_PLAYBACK_STATUS_PLAYING, "BARBAR_PLAYBACK_STATUS_PLAYING",
         "playing"},
        {BARBAR_PLAYBACK_STATUS_PAUSED, "BARBAR_PLAYBACK_STATUS_PAUSED",
         "paused"},
        {BARBAR_PLAYBACK_STATUS_STOPPED, "BARBAR_PLAYBACK_STATUS_STOPPED",
         "stopped"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarBarPlaybackStatus", pattern_types);
    g_once_init_leave(&barbar_playback_status_type, type);
  }
  return barbar_playback_status_type;
}

GType g_barbar_loop_status_get_type(void) {

  static gsize barbar_loop_status_type;
  if (g_once_init_enter(&barbar_loop_status_type)) {

    static const GEnumValue pattern_types[] = {
        {BARBAR_PLAYBACK_LOOP_STATUS_NONE, "BARBAR_PLAYBACK_LOOP_STATUS_NONE",
         "none"},
        {BARBAR_PLAYBACK_LOOP_STATUS_TRACK, "BARBAR_PLAYBACK_LOOP_STATUS_TRACK",
         "track"},
        {BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST,
         "BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST", "playlist"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarBarLoopStatus", pattern_types);
    g_once_init_leave(&barbar_loop_status_type, type);
  }
  return barbar_loop_status_type;
}

struct _BarBarMprisPlayer {
  MprisOrgMprisMediaPlayer2Player *proxy;
  gchar *name;
  gchar *instance;
  gchar *bus_name;
  GBusType bus_type;

  gint64 length;
  gchar *title;
  gchar *artist;

  BarBarMprisPlaybackStatus playback;
  BarBarMprisLoopStatus loop;
  gint64 position;
  double volume;
};

enum {
  PROP_0,

  PROP_PLAYER_NAME,
  PROP_PLAYER_INSTANCE,
  PROP_BUS_TYPE,
  PROP_PLAYBACK_STATUS,
  PROP_LOOP_STATUS,
  PROP_SHUFFLE,
  PROP_VOLUME,
  PROP_METADATA,
  PROP_POSITION,
  PROP_LENGTH,
  PROP_TITLE,
  PROP_ARTIST, // artist can be anything, not just the real artist. Can also be
               // null

  PROP_CAN_CONTROL,
  PROP_CAN_PLAY,
  PROP_CAN_PAUSE,
  PROP_CAN_SEEK,
  PROP_CAN_GO_NEXT,
  PROP_CAN_GO_PREV,

  N_PROPERTIES
};

enum {
  METADATA,
  // SEEKED,
  EXIT,
  LAST_SIGNAL
};

static GParamSpec *mpris_player_props[N_PROPERTIES] = {
    NULL,
};

static guint connection_signals[LAST_SIGNAL] = {0};

static gboolean playerctl_player_initable_init(GInitable *initable,
                                               GCancellable *cancellable,
                                               GError **err);

G_DEFINE_TYPE(BarBarMprisPlayer, g_barbar_mpris_player, G_TYPE_OBJECT);

static void
g_barbar_mpris_player_set_playback_status(BarBarMprisPlayer *player,
                                          BarBarMprisPlaybackStatus playback) {

  g_return_if_fail(BARBAR_IS_MPRIS_PLAYER(player));

  if (player->playback == playback) {
    return;
  }
  // TODO: upstream shit

  player->playback = playback;
  g_object_notify_by_pspec(G_OBJECT(player),
                           mpris_player_props[PROP_PLAYBACK_STATUS]);
}
static void g_barbar_mpris_player_set_loop_status(BarBarMprisPlayer *player,
                                                  BarBarMprisLoopStatus loop) {

  g_return_if_fail(BARBAR_IS_MPRIS_PLAYER(player));

  if (player->loop == loop) {
    return;
  }
  // TODO: upstream shit

  player->loop = loop;
  g_object_notify_by_pspec(G_OBJECT(player),
                           mpris_player_props[PROP_LOOP_STATUS]);
}

static void g_barbar_mpris_player_set_volume(BarBarMprisPlayer *player,
                                             double volume) {
  g_return_if_fail(BARBAR_IS_MPRIS_PLAYER(player));

  if (player->volume == volume) {
    return;
  }
  // TODO: upstream shit

  player->volume = volume;
  g_object_notify_by_pspec(G_OBJECT(player), mpris_player_props[PROP_VOLUME]);
}

static void g_barbar_mpris_player_set_position(BarBarMprisPlayer *player,
                                               guint64 pos) {
  g_return_if_fail(BARBAR_IS_MPRIS_PLAYER(player));

  if (player->position == pos) {
    return;
  }
  // TODO: upstream shit

  player->position = pos;
  g_object_notify_by_pspec(G_OBJECT(player), mpris_player_props[PROP_POSITION]);
}

static void g_barbar_mpris_player_set_property(GObject *object,
                                               guint property_id,
                                               const GValue *value,
                                               GParamSpec *pspec) {
  BarBarMprisPlayer *player = BARBAR_MPRIS_PLAYER(object);

  switch (property_id) {
  case PROP_PLAYER_NAME: {
    player->name = g_strdup(g_value_get_string(value));
    break;
  }
  case PROP_PLAYER_INSTANCE: {
    player->instance = g_strdup(g_value_get_string(value));
    break;
  }
  case PROP_BUS_TYPE: {
    GBusType type = g_value_get_enum(value);
    player->bus_type = type;
    break;
  }
  case PROP_PLAYBACK_STATUS:
    g_barbar_mpris_player_set_playback_status(player, g_value_get_enum(value));
    break;
  case PROP_LOOP_STATUS:
    g_barbar_mpris_player_set_loop_status(player, g_value_get_enum(value));
    break;
  case PROP_VOLUME:
    g_barbar_mpris_player_set_volume(player, g_value_get_double(value));
    break;
  case PROP_POSITION:
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

#define get_bool_value(func)                                                   \
  do {                                                                         \
    if (!player->proxy) {                                                      \
      g_value_set_boolean(value, FALSE);                                       \
      return;                                                                  \
    }                                                                          \
    g_value_set_boolean(value, func(player->proxy));                           \
  } while (0)

static void g_barbar_mpris_player_get_property(GObject *object,
                                               guint property_id, GValue *value,
                                               GParamSpec *pspec) {
  BarBarMprisPlayer *player = BARBAR_MPRIS_PLAYER(object);

  switch (property_id) {
  case PROP_PLAYER_NAME:
    g_value_set_string(value, player->name);
    break;
  case PROP_PLAYER_INSTANCE:
    g_value_set_string(value, player->instance);
    break;
  case PROP_BUS_TYPE:
    g_value_set_enum(value, player->bus_type);
    break;
  case PROP_PLAYBACK_STATUS:
    g_value_set_enum(value, player->playback);
    break;
  case PROP_LOOP_STATUS:
    g_value_set_enum(value, player->loop);
    break;
  case PROP_VOLUME:
    g_value_set_double(value, player->volume);
    break;
  case PROP_METADATA:
    // g_value_set_double(value, player->volume);
    break;
  case PROP_POSITION:
    g_value_set_uint64(value, player->position);
    break;
  case PROP_TITLE:
    g_value_set_string(value, player->title);
    break;
  case PROP_ARTIST:
    g_value_set_string(value, player->artist);
    break;
  case PROP_CAN_CONTROL:
    get_bool_value(mpris_org_mpris_media_player2_player_get_can_control);
    break;
  case PROP_CAN_PLAY:
    get_bool_value(mpris_org_mpris_media_player2_player_get_can_play);
    break;
  case PROP_CAN_PAUSE:
    get_bool_value(mpris_org_mpris_media_player2_player_get_can_pause);
    break;
  case PROP_CAN_SEEK:
    get_bool_value(mpris_org_mpris_media_player2_player_get_can_seek);
    break;
  case PROP_CAN_GO_NEXT:
    get_bool_value(mpris_org_mpris_media_player2_player_get_can_go_next);
    break;
  case PROP_CAN_GO_PREV:
    get_bool_value(mpris_org_mpris_media_player2_player_get_can_go_previous);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mpris_player_class_init(BarBarMprisPlayerClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  gobject_class->set_property = g_barbar_mpris_player_set_property;
  gobject_class->get_property = g_barbar_mpris_player_get_property;

  mpris_player_props[PROP_PLAYER_NAME] = g_param_spec_string(
      "player-name", "Player name", "Name of the player", NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_PLAYER_INSTANCE] = g_param_spec_string(
      "player-instance", "Player instance", "The player instance", NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_BUS_TYPE] = g_param_spec_enum(
      "g-bus-type", NULL, NULL, BARBAR_TYPE_PLAYBACK_STATUS, G_BUS_TYPE_NONE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_PLAYBACK_STATUS] = g_param_spec_enum(
      "playback-status", NULL, NULL, BARBAR_TYPE_PLAYBACK_STATUS,
      BARBAR_PLAYBACK_STATUS_STOPPED,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_LOOP_STATUS] = g_param_spec_enum(
      "loop-status", "Loop status", "The players loop status",
      BARBAR_TYPE_LOOP_STATUS, BARBAR_PLAYBACK_LOOP_STATUS_NONE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_SHUFFLE] = g_param_spec_boolean(
      "shuffle", NULL, NULL, FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_VOLUME] =
      g_param_spec_double("volume", NULL, NULL, 0, 100, 0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_METADATA] = g_param_spec_boolean(
      "metadata", "metadata", "extra data about the current track", FALSE,
      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_LENGTH] =
      g_param_spec_uint64("length", "length", "length metadata", 0, G_MAXUINT64,
                          0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_TITLE] =
      g_param_spec_string("title", "title", "title metadata", NULL,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_ARTIST] =
      g_param_spec_string("artist", "artist", "artist metadata", NULL,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  // Some boolean value
  mpris_player_props[PROP_CAN_CONTROL] = g_param_spec_boolean(
      "can-control", "Can control", "If the player can be controlled", FALSE,
      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_CAN_PLAY] = g_param_spec_boolean(
      "can-play", "Can play", "If the player can start playing", FALSE,
      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_CAN_PAUSE] = g_param_spec_boolean(
      "can-pause", "Can pause", "If the player can be paused", FALSE,
      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_CAN_SEEK] = g_param_spec_boolean(
      "can-seek", "Can seek", "If the player can seek the current medium",
      FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_CAN_GO_NEXT] = g_param_spec_boolean(
      "can-go-next", "Can go next", "If the player can go to the next", FALSE,
      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  mpris_player_props[PROP_CAN_GO_PREV] = g_param_spec_boolean(
      "can-go-prev", "Can go prev", "If the player can go to the previous",
      FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
}

static void g_barbar_mpris_player_init(BarBarMprisPlayer *self) {}

static void g_barbar_mpris_player_initial_values(BarBarMprisPlayer *player) {
  // g_debug("initializing player: %s", player->priv->instance);
  // player->priv->cached_position =
  //     org_mpris_media_player2_player_get_position(player->priv->proxy);
  // clock_gettime(CLOCK_MONOTONIC, &player->priv->cached_position_monotonic);
  //
  // const gchar *playback_status_str =
  //     org_mpris_media_player2_player_get_playback_status(player->priv->proxy);
  //
  // PlayerctlPlaybackStatus status = 0;
  // if (pctl_parse_playback_status(playback_status_str, &status)) {
  //   player->priv->cached_status = status;
  // }
}

static void g_barbar_seeked(GDBusProxy *_proxy, gint64 position,
                            gpointer user_data) {
  BarBarMprisPlayer *player = BARBAR_MPRIS_PLAYER(user_data);
  player->position = position;
}

static void on_some_property_notify(GObject *proxy, GParamSpec *pspec,
                                    gpointer user_data) {

  BarBarMprisPlayer *player = BARBAR_MPRIS_PLAYER(user_data);
}

static gboolean playerctl_player_initable_init(GInitable *initable,
                                               GCancellable *cancellable,
                                               GError **error) {
  GError *err = NULL;
  BarBarMprisPlayer *player = BARBAR_MPRIS_PLAYER(initable);

  if (!player->instance) {
    // g_set_error(GError **err, GQuark domain, gint code, const gchar *format,
    // ...)
    return FALSE;
  }

  // we need to have the bus_type defined
  if (player->bus_type < 1) {
    // g_set_error(GError **err, GQuark domain, gint code, const gchar *format,
    // ...)
    return FALSE;
  }

  gchar *bus_name = g_strdup_printf(MPRIS_PREFIX "%s", player->instance);

  player->proxy = mpris_org_mpris_media_player2_player_proxy_new_for_bus_sync(
      player->bus_type, G_DBUS_PROXY_FLAGS_NONE, bus_name,
      "/org/mpris/MediaPlayer2", NULL, &err);
  if (err) {
    g_propagate_error(error, err);
    return FALSE;
  }

  g_barbar_mpris_player_initial_values(player);
  g_signal_connect(player->proxy, "g-properties-changed",
                   G_CALLBACK(playerctl_player_properties_changed_callback),
                   player);

  // g_signal_connect(player->proxy, "g-properties-changed",
  //                  G_CALLBACK(playerctl_player_properties_changed_callback),
  //                  player);
  //
  g_signal_connect(player->proxy, "seeked", G_CALLBACK(g_barbar_seeked),
                   player);
  //
  // g_signal_connect(player->proxy, "notify::g-name-owner",
  //                  G_CALLBACK(playerctl_player_name_owner_changed_callback),
  //                  player);

  // player->priv->initted = TRUE;
  return TRUE;
}

BarBarMprisPlayer *g_barbar_mpris_player_new(char *player_name, GBusType type,
                                             GError **error) {
  GError *err = NULL;
  BarBarMprisPlayer *player;

  player =
      g_initable_new(BARBAR_TYPE_MPRIS_PLAYER, NULL, &err, "player-instance",
                     player_name, "g-bus-type", type, NULL);

  if (err != NULL) {
    g_propagate_error(error, err);
    return NULL;
  }

  return player;
}
