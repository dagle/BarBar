#include "barbar-mpris.h"
// #include "barbar-mpris2-formater.h"
#include <playerctl/playerctl.h>
#include <stdio.h>

/* TODO: Be able to listen to multiple players and pick the last one used */
struct _BarBarMpris {
  BarBarSensor parent_instance;

  PlayerctlPlayerManager *manager;
  PlayerctlPlayer *current_player;

  // PlayerctlPlayer **players;
  // int num_players;

  char *player_name;
  // char *player_names;
};

enum {
  PROP_0,

  PROP_PLAYER,
  PROP_PLAYER_NAME,

  NUM_PROPERTIES,
};

enum { STATE_UPDATE, LAST_SIGNAL };

G_DEFINE_TYPE(BarBarMpris, g_barbar_mpris, BARBAR_TYPE_SENSOR);

static GParamSpec *mpris_props[NUM_PROPERTIES] = {
    NULL,
};
static guint mpris_signals[LAST_SIGNAL] = {0};

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
  case PROP_PLAYER_NAME:
    g_barbar_mpris_set_player(mpris, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mpris_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {
  BarBarMpris *mpris = BARBAR_MPRIS(object);

  switch (property_id) {
  case PROP_PLAYER:
    g_value_set_object(value, mpris->current_player);
    break;
  case PROP_PLAYER_NAME:
    g_value_set_string(value, mpris->player_name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mpris_class_init(BarBarMprisClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_mpris_set_property;
  gobject_class->get_property = g_barbar_mpris_get_property;
  sensor->start = g_barbar_mpris_start;

  /**
   * BarBarMpris:player-name:
   *
   * The name of the player we want to watch
   *
   */
  mpris_props[PROP_PLAYER_NAME] = g_param_spec_string(
      "player-name", NULL, NULL, "mpd", G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarMpris:current-player:
   *
   * The current active player.
   *
   */
  mpris_props[PROP_PLAYER] = g_param_spec_object(
      "current-player", NULL, NULL, PLAYERCTL_TYPE_PLAYER,
      G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarMpris::state-update:
   * @sensor: This sensor
   *
   * Emit that an update has happened. This means that we want to refetch
   * the player.
   */
  mpris_signals[STATE_UPDATE] =
      g_signal_new("state-update",                         /* signal_name */
                   BARBAR_TYPE_MPRIS,                      /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );

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

/**
 * g_barbar_mpris_get_current_player:
 *
 * Get the current player.
 *
 * Returns: (transfer none): The current PlayerctlPlayer
 */
PlayerctlPlayer *g_barbar_mpris_get_current_player(BarBarMpris *mpris) {
  return mpris->current_player;
}

/**
 * get_current_player_print:
 *
 * Formats a message
 *
 * Returns: (transfer full): A string of the current status
 */
char *get_current_player_print(void) { return strdup("aue"); }

gchar *g_barbar_mpris_format_player(BarBarMpris *mpris, const char *format) {
  return NULL;
}

static gchar *pctl_print_gvariant(GVariant *value) {
  GString *printed = g_string_new("");
  if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING_ARRAY)) {
    gsize prop_count;
    const gchar **prop_strv = g_variant_get_strv(value, &prop_count);

    for (gsize i = 0; i < prop_count; i += 1) {
      g_string_append(printed, prop_strv[i]);

      if (i != prop_count - 1) {
        g_string_append(printed, ", ");
      }
    }

    g_free(prop_strv);
  } else if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING)) {
    g_string_append(printed, g_variant_get_string(value, NULL));
  } else {
    printed = g_variant_print_string(value, printed, FALSE);
  }

  return g_string_free(printed, FALSE);
}

static void g_barbar_mpris_player_update(PlayerctlPlayer *player,
                                         GVariant *metadata, gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);

  // barbar_gvariant_printf("apa {mpris:length}, {xesam:title}")

  // g_signal_emit(mpris, STATE_UPDATE, 0, NULL);
  // GVariant *track_id_variant = g_variant_lookup_value(
  //     metadata, "mpris:trackid", G_VARIANT_TYPE_OBJECT_PATH);

  gchar *track_id = pctl_print_gvariant(metadata);
  printf("trackid: %s\n", track_id);
  // g_variant_unref(track_id_variant);

  g_object_notify_by_pspec(G_OBJECT(player), mpris_props[PROP_PLAYER_NAME]);
}

static void g_barbar_mpris_player_vanished(PlayerctlPlayerManager *manager,
                                           PlayerctlPlayerName *player_name,
                                           gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);

  if (!mpris->current_player || strcmp(player_name->name, mpris->player_name)) {
    return;
  }

  g_clear_pointer(&mpris->current_player, g_object_unref);
}

static void g_barbar_mpris_player_appeared(PlayerctlPlayerManager *manager,
                                           PlayerctlPlayerName *player_name,
                                           gpointer data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);
  GError *err = NULL;

  // g_object_notify_by_pspec(G_OBJECT(player), mpris_props[PROP_PLAYER_NAME]);
  if (strcmp(player_name->name, mpris->player_name)) {
    return;
  }

  mpris->current_player = playerctl_player_new_from_name(player_name, &err);

  if (!mpris->current_player || err) {
    fprintf(stderr, "Unable to read file: %s\n", err->message);
    g_error_free(err);
    return;
  }
  g_signal_connect(mpris->current_player, "metadata",
                   G_CALLBACK(g_barbar_mpris_player_update), data);
}

static void g_barbar_mpris_start(BarBarSensor *sensor) {
  BarBarMpris *mpris = BARBAR_MPRIS(sensor);
  GError *error = NULL;
  PlayerctlPlayerManager *manager = playerctl_player_manager_new(&error);

  g_return_if_fail(mpris);

  g_signal_connect(manager, "name-appeared",
                   G_CALLBACK(g_barbar_mpris_player_appeared), mpris);
  g_signal_connect(manager, "name-vanished",
                   G_CALLBACK(g_barbar_mpris_player_vanished), mpris);

  GList *players = NULL;
  g_object_get(manager, "player-names", &players, NULL);

  for (; players != NULL; players = players->next) {
    PlayerctlPlayerName *pn = players->data;
    g_barbar_mpris_player_appeared(manager, pn, mpris);
  }
}
