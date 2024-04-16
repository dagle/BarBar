#include "barbar-mpris.h"
#include "barbar-mpris-player.h"
#include <gio/gio.h>
#include <gmodule.h>
#include <stdio.h>

/**
 * BarBarMpris:
 *
 * A mpris sensor.
 *
 * It can be used to show the current playing medium or
 * to controll it.
 */
struct _BarBarMpris {
  BarBarSensor parent_instance;

  GDBusProxy *session_proxy;
  GDBusProxy *system_proxy;

  char **player_interest;
  GList *players;
  BarBarMprisPlayer *current_player;

  gboolean intrest_prio;
};

enum {
  PROP_0,

  PROP_CURRENT_PLAYER,
  PROP_PLAYER_INTEREST,
  PROP_INTEREST_PRIO,

  NUM_PROPERTIES,
};

// enum { STATE_UPDATE, LAST_SIGNAL };

G_DEFINE_TYPE(BarBarMpris, g_barbar_mpris, BARBAR_TYPE_SENSOR);

static GParamSpec *mpris_props[NUM_PROPERTIES] = {
    NULL,
};
// static guint mpris_signals[LAST_SIGNAL] = {0};

static void g_barbar_mpris_start(BarBarSensor *sensor);

void g_barbar_mpris_set_player_interest(BarBarMpris *mpris,
                                        const char **players) {
  char **tmp;
  g_return_if_fail(BARBAR_IS_MPRIS(mpris));

  tmp = mpris->player_interest;
  mpris->player_interest = g_strdupv((char **)players);
  g_strfreev(tmp);

  g_object_notify_by_pspec(G_OBJECT(mpris), mpris_props[PROP_PLAYER_INTEREST]);
}

void g_barbar_mpris_set_current_player(BarBarMpris *mpris,
                                       BarBarMprisPlayer *current_player) {
  g_return_if_fail(BARBAR_IS_MPRIS(mpris));

  if (mpris->current_player) {
    g_object_unref(mpris->current_player);
  }
  mpris->current_player = g_object_ref(current_player);

  g_object_notify_by_pspec(G_OBJECT(mpris), mpris_props[PROP_CURRENT_PLAYER]);
}

void g_barbar_mpris_set_interest_prio(BarBarMpris *mpris, gboolean prio) {
  g_return_if_fail(BARBAR_IS_MPRIS(mpris));

  mpris->intrest_prio = prio;

  g_object_notify_by_pspec(G_OBJECT(mpris), mpris_props[PROP_INTEREST_PRIO]);
}

static void g_barbar_mpris_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  BarBarMpris *mpris = BARBAR_MPRIS(object);

  switch (property_id) {
  case PROP_PLAYER_INTEREST:
    g_barbar_mpris_set_player_interest(mpris,
                                       (const char **)g_value_get_boxed(value));
    break;
  case PROP_INTEREST_PRIO:
    g_barbar_mpris_set_interest_prio(mpris, g_value_get_boolean(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_mpris_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {
  BarBarMpris *mpris = BARBAR_MPRIS(object);

  switch (property_id) {
  case PROP_CURRENT_PLAYER:
    g_value_set_object(value, mpris->current_player);
    break;
  // case PROP_PLAYER_INTEREST:
  //   g_value_set_string(value, mpris->player_name);
  //   break;
  case PROP_INTEREST_PRIO:
    g_value_set_boolean(value, mpris->intrest_prio);
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
   * BarBarMpris:player-interest:
   *
   * A list of players we are interested of. %NULL equal all players.
   *
   */
  mpris_props[PROP_PLAYER_INTEREST] =
      g_param_spec_boxed("player-interest", NULL, NULL, G_TYPE_STRV,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarMpris:current-player:
   *
   * The current active player.
   *
   */
  mpris_props[PROP_CURRENT_PLAYER] = g_param_spec_object(
      "current-player", NULL, NULL, PLAYERCTL_TYPE_PLAYER,
      G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * BarBarMpris:interest-prio:
   *
   * If the prio should be based on the prio list, fale by default.
   * If false or player-interest is null. Then the order they
   * started playing is used.
   *
   */
  mpris_props[PROP_INTEREST_PRIO] =
      g_param_spec_boolean("interest-prio", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  // /**
  //  * BarBarMpris::state-update:
  //  * @sensor: This sensor
  //  *
  //  * Emit that an update has happened. This means that we want to refetch
  //  * the player.
  //  */
  // mpris_signals[STATE_UPDATE] =
  //     g_signal_new("state-update",                         /* signal_name */
  //                  BARBAR_TYPE_MPRIS,                      /* itype */
  //                  G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
  //                  0,                                      /* class_offset */
  //                  NULL,                                   /* accumulator */
  //                  NULL,                                   /* accu_data */
  //                  NULL,                                   /* c_marshaller */
  //                  G_TYPE_NONE,                            /* return_type */
  //                  0                                       /* n_params */
  //     );

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, mpris_props);
}

static void g_barbar_mpris_init(BarBarMpris *self) {}

// static void g_barbar_mpris_on_play(PlayerctlPlayer *player, gpointer data) {
//   BarBarMpris *mpris = BARBAR_MPRIS(data);
//   g_print("Playing\n");
// }
//
// static void g_barbar_mpris_on_stop(PlayerctlPlayer *player, gpointer data) {
//   BarBarMpris *mpris = BARBAR_MPRIS(data);
//   g_print("Stopped\n");
// }
//
// static void g_barbar_mpris_on_pause(PlayerctlPlayer *player, gpointer data) {
//   BarBarMpris *mpris = BARBAR_MPRIS(data);
//   g_print("Paused\n");
// }
//
// static void g_barbar_mpris_on_metadata(PlayerctlPlayer *player,
//                                        GVariant *metadata, gpointer data) {
//   BarBarMpris *mpris = BARBAR_MPRIS(data);
//   g_print("metadata\n");
// }
//
static void g_barbar_mpris_connect_events(PlayerctlPlayer *player,
                                          BarBarMpris *mpris) {}

/**
 * g_barbar_mpris_get_current_player:
 *
 * Get the current player.
 *
 * Returns: (transfer none): The current PlayerctlPlayer
 */
BarBarMprisPlayer *g_barbar_mpris_get_current_player(BarBarMpris *mpris) {
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

// static gchar *pctl_print_gvariant(GVariant *value) {
//   GString *printed = g_string_new("");
//   if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING_ARRAY)) {
//     gsize prop_count;
//     const gchar **prop_strv = g_variant_get_strv(value, &prop_count);
//
//     for (gsize i = 0; i < prop_count; i += 1) {
//       g_string_append(printed, prop_strv[i]);
//
//       if (i != prop_count - 1) {
//         g_string_append(printed, ", ");
//       }
//     }
//
//     g_free(prop_strv);
//   } else if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING)) {
//     g_string_append(printed, g_variant_get_string(value, NULL));
//   } else {
//     printed = g_variant_print_string(value, printed, FALSE);
//   }
//
//   return g_string_free(printed, FALSE);
// }

#define PLAYERCTLD_BUS_NAME "org.mpris.MediaPlayer2.playerctld"
#define MPRIS_PREFIX "org.mpris.MediaPlayer2."

GList *pctl_list_player_names_on_bus(GDBusProxy *proxy, GError **error) {
  GError *err = NULL;
  GList *players = NULL;

  GVariant *reply = g_dbus_proxy_call_sync(
      proxy, "ListNames", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &err);

  if (err != NULL) {
    g_propagate_error(error, err);
    g_object_unref(proxy);
    return NULL;
  }

  GVariant *reply_child = g_variant_get_child_value(reply, 0);
  gsize reply_count;
  const gchar **names = g_variant_get_strv(reply_child, &reply_count);

  size_t offset = strlen(MPRIS_PREFIX);
  for (gsize i = 0; i < reply_count; i += 1) {
    if (g_str_has_prefix(names[i], MPRIS_PREFIX)) {
      // Create a player

      // PlayerctlPlayerName *player_name = pctl_player_name_new(
      //     names[i] + offset, pctl_bus_type_to_source(bus_type));
      // players = g_list_append(players, player_name);
    }
  }

  g_variant_unref(reply);
  g_variant_unref(reply_child);
  g_free(names);

  return players;
}

void g_barbar_mpris_list_players(BarBarMpris *mpris, GError **error) {
  GError *err = NULL;

  GList *session_players =
      pctl_list_player_names_on_bus(mpris->session_proxy, &err);
  if (err != NULL) {
    g_propagate_error(error, err);
    return;
  }

  GList *system_players =
      pctl_list_player_names_on_bus(mpris->system_proxy, &err);
  if (err != NULL) {
    g_propagate_error(error, err);
    return;
  }

  mpris->players = g_list_concat(session_players, system_players);

  return;
}

static inline gboolean vanished(const char *new, const char *prev) {
  return strlen(new) == 0 && strlen(prev) != 0;
}

static inline gboolean appeared(const char *new, const char *prev) {
  return strlen(prev) == 0 && strlen(new) != 0;
}

static gchar *get_player_name(const gchar *bus_name) {
  const size_t prefix_len = strlen(MPRIS_PREFIX);

  if (bus_name == NULL || !g_str_has_prefix(bus_name, MPRIS_PREFIX) ||
      strlen(bus_name) <= prefix_len) {
    return NULL;
  }

  return g_strdup(bus_name + prefix_len);
}

static void dbus_name_owner_changed_callback(GDBusProxy *proxy,
                                             gchar *sender_name,
                                             gchar *signal_name,
                                             GVariant *parameters,
                                             gpointer *data) {
  BarBarMpris *mpris = BARBAR_MPRIS(data);

  if (g_strcmp0(signal_name, "NameOwnerChanged") != 0) {
    return;
  }

  if (!g_variant_is_of_type(parameters, G_VARIANT_TYPE("(sss)"))) {
    g_debug("Got unknown parameters on org.freedesktop.DBus "
            "NameOwnerChange signal: %s",
            g_variant_get_type_string(parameters));
    return;
  }

  GVariant *name_variant = g_variant_get_child_value(parameters, 0);
  const gchar *bus_name = g_variant_get_string(name_variant, NULL);
  gchar *player_id = get_player_name(bus_name);

  if (player_id == NULL) {
    g_debug("Player id is null");
    g_variant_unref(name_variant);
    return;
  }

  GBusType bus_type = 0;
  if (proxy == mpris->session_proxy) {
    bus_type = G_BUS_TYPE_SESSION;
  } else if (proxy == mpris->system_proxy) {
    bus_type = G_BUS_TYPE_SYSTEM;
  } else {
    g_error("got unknown proxy in callback");
    g_variant_unref(name_variant);
    return;
  }

  GVariant *previous_owner_variant = g_variant_get_child_value(parameters, 1);
  const gchar *previous_owner =
      g_variant_get_string(previous_owner_variant, NULL);

  GVariant *new_owner_variant = g_variant_get_child_value(parameters, 2);
  const gchar *new_owner = g_variant_get_string(new_owner_variant, NULL);

  GList *player_entry = NULL;
  if (vanished(new_owner, previous_owner)) {
    // the name has vanished
    //
    // player_entry = pctl_player_name_find(manager->priv->player_names,
    // player_id,
    //                                      pctl_bus_type_to_source(bus_type));
    // if (player_entry != NULL) {
    //   PlayerctlPlayerName *player_name = player_entry->data;
    //   manager->priv->player_names =
    //       g_list_remove_link(manager->priv->player_names, player_entry);
    //   manager_remove_managed_player_by_name(manager, player_name);
    //   g_debug("player name vanished: %s", player_name->instance);
    //   g_signal_emit(manager, connection_signals[NAME_VANISHED], 0,
    //   player_name); pctl_player_name_list_destroy(player_entry);
    // }
  } else if (appeared(new_owner, previous_owner)) {
    // the name has appeared
    // player_entry = pctl_player_name_find(manager->priv->players, player_id,
    //                                      pctl_bus_type_to_source(bus_type));
    // if (player_entry == NULL) {
    //   PlayerctlPlayerName *player_name =
    //       pctl_player_name_new(player_id, pctl_bus_type_to_source(bus_type));
    //
    //   manager->priv->player_names =
    //       g_list_prepend(manager->priv->player_names, player_name);
    //   g_debug("player name appeared: %s", player_name->instance);
    //   g_signal_emit(manager, connection_signals[NAME_APPEARED], 0,
    //   player_name);
    // }
  }

  g_free(player_id);
  g_variant_unref(name_variant);
  g_variant_unref(previous_owner_variant);
  g_variant_unref(new_owner_variant);
}

static gboolean g_barbar_manager_initable_init(GInitable *initable,
                                               GCancellable *cancellable,
                                               GError **error) {
  GError *err = NULL;
  BarBarMpris *mpris = BARBAR_MPRIS(initable);

  // if (manager->priv->initted) {
  //   return TRUE;
  // }
  //
  mpris->session_proxy = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, "org.freedesktop.DBus",
      "/org/freedesktop/DBus", "org.freedesktop.DBus", NULL, &err);

  if (err != NULL) {
    if (err->domain == G_IO_ERROR && err->code == G_IO_ERROR_NOT_FOUND) {
      g_clear_error(&err);
    } else {
      g_propagate_error(error, err);
      return FALSE;
    }
  }
  mpris->system_proxy = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL, "org.freedesktop.DBus",
      "/org/freedesktop/DBus", "org.freedesktop.DBus", NULL, &err);

  if (err != NULL) {
    if (err->domain == G_IO_ERROR && err->code == G_IO_ERROR_NOT_FOUND) {
      g_clear_error(&err);
    } else {
      g_propagate_error(error, err);
      return FALSE;
    }
  }

  g_barbar_mpris_list_players(mpris, &err);
  if (err != NULL) {
    g_propagate_error(error, err);
    return FALSE;
  }

  if (mpris->session_proxy) {
    g_signal_connect(G_DBUS_PROXY(mpris->session_proxy), "g-signal",
                     G_CALLBACK(dbus_name_owner_changed_callback), mpris);
  }

  if (mpris->system_proxy) {
    g_signal_connect(G_DBUS_PROXY(mpris->system_proxy), "g-signal",
                     G_CALLBACK(dbus_name_owner_changed_callback), mpris);
  }

  return TRUE;
}

// static void g_barbar_mpris_player_appeared(PlayerctlPlayerManager *manager,
//                                            PlayerctlPlayerName *player_name,
//                                            gpointer data) {
//   BarBarMpris *mpris = BARBAR_MPRIS(data);
//   GError *err = NULL;
//
//   // g_object_notify_by_pspec(G_OBJECT(player),
//   mpris_props[PROP_PLAYER_NAME]); if (strcmp(player_name->name,
//   mpris->player_name)) {
//     return;
//   }
//
//   mpris->current_player = playerctl_player_new_from_name(player_name, &err);
//
//   if (!mpris->current_player || err) {
//     g_printerr("mpris: unable to create player: %s\n", err->message);
//     g_error_free(err);
//     return;
//   }
//   g_signal_connect(mpris->current_player, "metadata",
//                    G_CALLBACK(g_barbar_mpris_player_update), data);
// }

static void g_barbar_mpris_start(BarBarSensor *sensor) {
  GError *err = NULL;
  BarBarMpris *mpris = BARBAR_MPRIS(sensor);

  // if (manager->priv->initted) {
  //   return TRUE;
  // }
  //
  mpris->session_proxy = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, "org.freedesktop.DBus",
      "/org/freedesktop/DBus", "org.freedesktop.DBus", NULL, &err);

  if (err != NULL) {
    if (err->domain == G_IO_ERROR && err->code == G_IO_ERROR_NOT_FOUND) {
      g_clear_error(&err);
    } else {
      g_printerr("Failed to setup session bus: %s", err->message);
      // g_propagate_error(error, err);
      return;
    }
  }
  mpris->system_proxy = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL, "org.freedesktop.DBus",
      "/org/freedesktop/DBus", "org.freedesktop.DBus", NULL, &err);

  if (err != NULL) {
    if (err->domain == G_IO_ERROR && err->code == G_IO_ERROR_NOT_FOUND) {
      g_clear_error(&err);
    } else {
      g_printerr("Failed to setup system bus: %s", err->message);
      // g_propagate_error(error, err);
      return;
    }
  }

  g_barbar_mpris_list_players(mpris, &err);
  if (err != NULL) {
    // g_propagate_error(error, err);
    return;
  }

  if (mpris->session_proxy) {
    g_signal_connect(G_DBUS_PROXY(mpris->session_proxy), "g-signal",
                     G_CALLBACK(dbus_name_owner_changed_callback), mpris);
  }

  if (mpris->system_proxy) {
    g_signal_connect(G_DBUS_PROXY(mpris->system_proxy), "g-signal",
                     G_CALLBACK(dbus_name_owner_changed_callback), mpris);
  }
}
