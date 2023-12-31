#include "barbar-mpris2.h"
#include <playerctl/playerctl.h>
#include <stdio.h>

struct _BarBarMpris {
  GtkWidget parent_instance;

  GtkWidget *label;

  char *player;
};

enum {
  PROP_0,

  PROP_PLAYER,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarMpris, g_barbar_mpris, GTK_TYPE_WIDGET)

static GParamSpec *mpris_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_mpris_constructed(GObject *object);

void g_barbar_mpris_set_player(BarBarMpris *mpris, const char *player) {
  g_return_if_fail(BARBAR_IS_MPRIS(mpris));

  g_free(mpris->player);
  mpris->player = g_strdup(player);

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
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_mpris_set_property;
  gobject_class->get_property = g_barbar_mpris_get_property;
  gobject_class->constructed = g_barbar_mpris_constructed;
  mpris_props[PROP_PLAYER] =
      g_param_spec_string("player", NULL, NULL, "mpd", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, mpris_props);

  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "mpris");
}

static void g_barbar_mpris_init(BarBarMpris *self) {}

static void g_barbar_mpris_constructed(GObject *object) {
  BarBarMpris *mpris = BARBAR_MPRIS(object);

  G_OBJECT_CLASS(g_barbar_mpris_parent_class)->constructed(object);

  mpris->player = strdup("mpd");
  mpris->label = gtk_label_new("");
  gtk_widget_set_parent(mpris->label, GTK_WIDGET(mpris));
}

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

static void g_barbar_mpris_player_appeared(PlayerctlPlayerManager *manager,
                                           PlayerctlPlayerName *player_name,
                                           gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);
  PlayerctlPlayer *player = NULL;
  GError *err = NULL;
  gchar *artist = NULL;
  gchar *track = NULL;
  gchar *str = NULL;

  if (strcmp(player_name->name, mpris->player)) {
    return;
  }
  player = playerctl_player_new_from_name(player_name, &err);

  if (!player || err) {
    fprintf(stderr, "Unable to read file: %s\n", err->message);
    g_error_free(err);
    return;
  }

  char *player_status = NULL;
  PlayerctlPlaybackStatus player_playback_status =
      PLAYERCTL_PLAYBACK_STATUS_STOPPED;

  artist = playerctl_player_get_artist(player, &err);
  track = playerctl_player_get_title(player, &err);

  // g_object_get(player, "status", &player_status, "playback-status",
  //              &player_playback_status, NULL);
  // gchar *str = g_strdup_printf("status: %s, playback-status: %d",
  // player_status,
  //                              player_playback_status);
  str = g_strdup_printf("%s - %s", artist, track);

  gtk_label_set_label(GTK_LABEL(mpris->label), str);

  g_free(str);
  g_free(track);
  g_free(artist);

  // g_object_connect(player, "signal::play",
  // G_CALLBACK(g_barbar_mpris_on_play),
  //                  mpris, "signal::pause",
  //                  G_CALLBACK(g_barbar_mpris_on_pause), mpris,
  //                  "signal::stop", G_CALLBACK(g_barbar_mpris_on_stop), mpris,
  //                  "signal::metadata",
  //                  G_CALLBACK(g_barbar_mpris_on_metadata), mpris, NULL);
}

static void g_barbar_mpris_player_vanished(PlayerctlPlayerManager *manager,
                                           PlayerctlPlayerName *player_name,
                                           gpointer data) {
  g_print("player: %s\n", player_name->name);
}

void g_barbar_mpris_start(BarBarMpris *mpris) {
  GError *error = NULL;
  PlayerctlPlayerManager *manager = playerctl_player_manager_new(&error);

  g_return_if_fail(mpris);

  g_object_connect(manager, "signal::name-appeared",
                   G_CALLBACK(g_barbar_mpris_player_appeared), mpris, NULL);
  g_object_connect(manager, "signal::name-vanished",
                   G_CALLBACK(g_barbar_mpris_player_vanished), mpris, NULL);

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
