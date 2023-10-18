#include "barbar-mpris2.h"
#include <playerctl/playerctl.h>
#include <stdio.h>

struct _BarBarMpris {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  char *player;

  GMainLoop *main_loop;
};

enum {
  PROP_0,

  PROP_DEVICE,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarMpris, g_barbar_mpris, G_TYPE_OBJECT)

static GParamSpec *mpris_props[NUM_PROPERTIES] = {
    NULL,
};

void g_barbar_mpris_set_player(BarBarMpris *mpris, const char *player) {
  g_return_if_fail(BARBAR_IS_MPRIS(mpris));

  g_free(mpris->player);
  mpris->player = g_strdup(player);

  g_object_notify_by_pspec(G_OBJECT(mpris), mpris_props[PROP_DEVICE]);
}

static void g_barbar_mpris_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
  BarBarMpris *mpris = BARBAR_MPRIS(object);

  switch (property_id) {
  case PROP_DEVICE:
    g_barbar_mpris_set_player(mpris, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mpris_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
}

static void g_barbar_mpris_class_init(BarBarMprisClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_mpris_set_property;
  gobject_class->get_property = g_barbar_mpris_get_property;
  mpris_props[PROP_DEVICE] = g_param_spec_string(
      "player", NULL, NULL, "mpd", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, mpris_props);
}

static void g_barbar_mpris_init(BarBarMpris *self) {
}

static void g_barbar_mpris_on_play(PlayerctlPlayer* player, gpointer data) {
	BarBarMpris *mpris = BARBAR_MPRIS(data);
	g_print("Playing\n");
	g_main_loop_quit(mpris->main_loop);
}
static void g_barbar_mpris_on_stop(PlayerctlPlayer* player, gpointer data) {
	BarBarMpris *mpris = BARBAR_MPRIS(data);
	g_print("Stopped\n");
}
static void g_barbar_mpris_on_pause(PlayerctlPlayer* player, gpointer data) {
	BarBarMpris *mpris = BARBAR_MPRIS(data);
	g_print("Paused\n");
	g_main_loop_quit(mpris->main_loop);
}

static void g_barbar_mpris_on_metadata(PlayerctlPlayer* player, GVariant* metadata, gpointer data) {
	BarBarMpris *mpris = BARBAR_MPRIS(data);
	g_print("metadata\n");
	g_main_loop_quit(mpris->main_loop);
}

static void g_barbar_mpris_connect_events(PlayerctlPlayer* player, BarBarMpris *mpris) {

}

static void g_barbar_mpris_player_appeared(PlayerctlPlayerManager* manager, PlayerctlPlayerName* player_name,
                                 gpointer data) {
	g_print("player: %s\n", player_name->name);
}

static void g_barbar_mpris_player_vanished(PlayerctlPlayerManager* manager, PlayerctlPlayerName* player_name,
                                 gpointer data) {
	g_print("player: %s\n", player_name->name);
}


void g_barbar_mpris_update(BarBarMpris *mpris) {
  GError* error = NULL;
  PlayerctlPlayerManager * manager = playerctl_player_manager_new(&error);
  PlayerctlPlayer *player = NULL;

  g_return_if_fail(mpris);

  g_object_connect(manager, "signal::name-appeared", G_CALLBACK(g_barbar_mpris_player_appeared),
		  mpris, NULL);
  g_object_connect(manager, "signal::name-vanished", G_CALLBACK(g_barbar_mpris_player_vanished),
		  mpris, NULL);

  GList* players = playerctl_list_players(&error);
  if (error) {
	  printf("unable to list players: %s", error->message);
	  g_error_free(error);
  }

  for (; players != NULL; players = players->next){
	  PlayerctlPlayerName *name = (PlayerctlPlayerName *)players->data;
	  if (!strcmp(name->name, mpris->player)) {
		  player = playerctl_player_new_from_name(name, &error);
		  break;
	  }
  }
  if (player) {
	  char* player_status = NULL;
	  PlayerctlPlaybackStatus player_playback_status = PLAYERCTL_PLAYBACK_STATUS_STOPPED;

	  g_object_get(player, "status", &player_status, "playback-status", &player_playback_status, NULL);
	  printf("status: %s, playback-status: %d\n", player_status, player_playback_status);

	  g_object_connect(player, "signal::play", G_CALLBACK(g_barbar_mpris_on_play), mpris,
			  "signal::pause", G_CALLBACK(g_barbar_mpris_on_pause), mpris,
			  "signal::stop", G_CALLBACK(g_barbar_mpris_on_stop),mpris,
			  "signal::metadata", G_CALLBACK(g_barbar_mpris_on_metadata), mpris,NULL);
  } else {
  }
}
