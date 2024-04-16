#include "sensors/mpris/barbar-mpris-player.h"
#include <gio/gio.h>

#define MPRIS_PATH "/org/mpris/MediaPlayer2"
#define PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define PLAYER_IFACE "org.mpris.MediaPlayer2.Player"
// #define SET_MEMBER "Set"

struct _BarBarMprisPlayer {
  // OrgMprisMediaPlayer2Player *proxy;
  gchar *player_name;
  gchar *instance;
  gchar *bus_name;
  // PlayerctlSource source;
  GError *init_error;
  gboolean initted;
  // PlayerctlPlaybackStatus cached_status;
  gint64 cached_position;
  gchar *cached_track_id;
  struct timespec cached_position_monotonic;
};

enum {
  PROP_0,

  PROP_PLAYER_NAME,
  PROP_PLAYER_INSTANCE,
  PROP_SOURCE,
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
  PROP_CAN_GO_PREVIOUS,

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

G_DEFINE_TYPE(BarBarMprisPlayer, g_barbar_mpris, G_TYPE_OBJECT);

BarBarMprisPlayer *pctl_player_name_new(char *player_name, GBusType type) {
  return NULL;
}

static void g_barbar_mpris_class_init(BarBarMprisPlayerClass *class) {}
static void g_barbar_mpris_init(BarBarMprisPlayer *self) {}
