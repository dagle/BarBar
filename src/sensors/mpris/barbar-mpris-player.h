/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Per Odlund
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

/**
 * BarBarMprisPlaybackStatus:
 * @BARBAR_PLAYBACK_STATUS_PLAYING: the player is playing.
 * @BARBAR_PLAYBACK_STATUS_PAUSED: the player is paused.
 * @BARBAR_PLAYBACK_STATUS_STOPPED: the player is stopped.
 *
 * Describs the playback status of a mpris player
 */
typedef enum {
  BARBAR_PLAYBACK_STATUS_PLAYING,
  BARBAR_PLAYBACK_STATUS_PAUSED,
  BARBAR_PLAYBACK_STATUS_STOPPED,
} BarBarMprisPlaybackStatus;

#define BARBAR_TYPE_PLAYBACK_STATUS (g_barbar_playback_status_get_type())

/**
 * BarBarMprisLoopStatus:
 * @BARBAR_PLAYBACK_LOOP_STATUS_NONE: no loop
 * @BARBAR_PLAYBACK_LOOP_STATUS_TRACK: loop the current track
 * @BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST: loop the playlist
 *
 * Describs the loop status of a mpris player
 */
typedef enum {
  BARBAR_PLAYBACK_LOOP_STATUS_NONE,
  BARBAR_PLAYBACK_LOOP_STATUS_TRACK,
  BARBAR_PLAYBACK_LOOK_STATUS_PLAYLIST,
} BarBarMprisLoopStatus;

#define BARBAR_TYPE_LOOP_STATUS (g_barbar_playback_status_get_type())

#define BARBAR_TYPE_MPRIS_PLAYER (g_barbar_mpris_player_get_type())
G_DECLARE_FINAL_TYPE(BarBarMprisPlayer, g_barbar_mpris_player, BARBAR,
                     MPRIS_PLAYER, GObject)

G_END_DECLS
