#include "barbar-mpd.h"
#include <mpd/client.h>
#include <mpd/tag.h>
#include <stdio.h>

// server_(nullptr),
// port_(config_["port"].isUInt() ? config["port"].asUInt() : 0),
// password_(config_["password"].empty() ? "" : config_["password"].asString()),
// timeout_(config_["timeout"].isUInt() ? config_["timeout"].asUInt() * 1'000 : 30'000),
// connection_(nullptr, &mpd_connection_free),
// status_(nullptr, &mpd_status_free),
// song_(nullptr, &mpd_song_free) {
struct _BarBarMpd {
  GObject parent;

  // TODO:This should be in parent
  char *label;

  char *path;
  struct mpd_connection *connection;
};

enum {
  PROP_0,

  PROP_PATH,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarMpd, g_barbar_mpd, G_TYPE_OBJECT)

static GParamSpec *mpd_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_mpd_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
}

static void g_barbar_mpd_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
}

static void g_barbar_mpd_class_init(BarBarMpdClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_mpd_set_property;
  gobject_class->get_property = g_barbar_mpd_get_property;
  mpd_props[PROP_PATH] = g_param_spec_string(
      "path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, mpd_props);
}

static void g_barbar_mpd_init(BarBarMpd *self) {
}

void g_barbar_mpd_update(BarBarMpd *mpd) {
  struct mpd_connection *connection;

  /// Add this properties
  connection = mpd_connection_new(NULL, 0, 0);
  if (!connection) {
	  return;
  }
  struct mpd_song *song = mpd_run_current_song(connection);

  if (!song) {
	  return;
  }
  const char* artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
  const char* track = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);

  printf("%s - %s\n", artist, track);

  mpd_song_free(song);
  mpd_connection_free(connection);

  // if (!mpd.password) {
  //    int res = mpd_run_password(connection, mpd->password);
  //    if (!res) {
  //        return error;
  //    }
  // }

}
