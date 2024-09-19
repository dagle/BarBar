#include <barbar-enum.h>

GType g_barbar_position_get_type(void) {

  static gsize barbar_bar_role_type;
  if (g_once_init_enter(&barbar_bar_role_type)) {

    static GEnumValue pattern_types[] = {
        {BARBAR_POS_TOP, "BARBAR_POS_TOP", "top"},
        {BARBAR_POS_BOTTOM, "BARBAR_POS_BOTTOM", "bot"},
        {BARBAR_POS_LEFT, "BARBAR_POS_LEFT", "left"},
        {BARBAR_POS_RIGHT, "BARBAR_POS_RIGHT", "right"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarBarPosition", pattern_types);
    g_once_init_leave(&barbar_bar_role_type, type);
  }
  return barbar_bar_role_type;
}

GType g_barbar_playback_status_get_type(void) {

  static gsize barbar_playback_status_type;
  if (g_once_init_enter(&barbar_playback_status_type)) {

    static const GEnumValue pattern_types[] = {
        {BARBAR_PLAYBACK_STATUS_PLAYING, "BARBAR_PLAYBACK_STATUS_PLAYING",
         "Playing"},
        {BARBAR_PLAYBACK_STATUS_PAUSED, "BARBAR_PLAYBACK_STATUS_PAUSED",
         "Paused"},
        {BARBAR_PLAYBACK_STATUS_STOPPED, "BARBAR_PLAYBACK_STATUS_STOPPED",
         "Stopped"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarBarPlaybackStatus", pattern_types);
    g_once_init_leave(&barbar_playback_status_type, type);
  }
  return barbar_playback_status_type;
}

const char *g_barbar_playback_status_nick(BarBarMprisPlaybackStatus playback) {
  switch (playback) {
  case BARBAR_PLAYBACK_STATUS_PLAYING:
    return "Playing";
  case BARBAR_PLAYBACK_STATUS_PAUSED:
    return "Paused";
  case BARBAR_PLAYBACK_STATUS_STOPPED:
    return "Stopped";
  }
  return NULL;
}

gboolean g_barbar_playback_status_enum(const char *loop,
                                       BarBarMprisPlaybackStatus *ret) {
  if (!g_strcmp0("Playing", loop)) {
    *ret = BARBAR_PLAYBACK_STATUS_PLAYING;
    return TRUE;
  }
  if (!g_strcmp0("Paused", loop)) {
    *ret = BARBAR_PLAYBACK_STATUS_PAUSED;
    return TRUE;
  }
  if (!g_strcmp0("Stopped", loop)) {
    *ret = BARBAR_PLAYBACK_STATUS_STOPPED;
    return TRUE;
  }
  return FALSE;
}

GType g_barbar_loop_status_get_type(void) {

  static gsize barbar_loop_status_type;
  if (g_once_init_enter(&barbar_loop_status_type)) {

    static const GEnumValue pattern_types[] = {
        {BARBAR_PLAYBACK_LOOP_STATUS_NONE, "BARBAR_PLAYBACK_LOOP_STATUS_NONE",
         "None"},
        {BARBAR_PLAYBACK_LOOP_STATUS_TRACK, "BARBAR_PLAYBACK_LOOP_STATUS_TRACK",
         "Track"},
        {BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST,
         "BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST", "Playlist"},
        {0, NULL, NULL},
    };

    GType type = 0;
    type = g_enum_register_static("BarBarBarLoopStatus", pattern_types);
    g_once_init_leave(&barbar_loop_status_type, type);
  }
  return barbar_loop_status_type;
}

const char *g_barbar_loop_status_nick(BarBarMprisLoopStatus loop) {
  switch (loop) {
  case BARBAR_PLAYBACK_LOOP_STATUS_NONE:
    return "None";
  case BARBAR_PLAYBACK_LOOP_STATUS_TRACK:
    return "Track";
  case BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST:
    return "Playlist";
  }
  return NULL;
}

gboolean g_barbar_loop_status_enum(const char *loop,
                                   BarBarMprisLoopStatus *ret) {
  if (!g_strcmp0("None", loop)) {
    *ret = BARBAR_PLAYBACK_LOOP_STATUS_NONE;
    return TRUE;
  }
  if (!g_strcmp0("Track", loop)) {
    *ret = BARBAR_PLAYBACK_LOOP_STATUS_TRACK;
    return TRUE;
  }
  if (!g_strcmp0("Playlist", loop)) {
    *ret = BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST;
    return TRUE;
  }
  return FALSE;
}