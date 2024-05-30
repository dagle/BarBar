#include "barbar-github-activity.h"
#include "gtk/gtk.h"
#include "widgets/barbar-activity-graph.h"
#include <glib.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <stdio.h>

// TODO: make it possible sort on weeks, without ruining the activitygraph

/**
 * BarBarGitHubActivity:
 *
 * An activity graph, often used to display github activity
 */
struct _BarBarGithubActivity {
  GtkWidget parent_instance;

  char *user_name;
  char *auth_token;
  gboolean native_colors;
  SoupSession *session;
  GtkWidget *activity_graph;

  guint interval;
  guint source_id;
};

enum ContributionLevel {
  NONE,
  FIRST_QUARTILE,
  SECOND_QUARTILE,
  THIRD_QUARTILE,
  FOURTH_QUARTILE,
};

int parse_contrubiton_level(const char *str) {
  if (!g_strcmp0("FIRST_QUARTILE", str)) {
    return FIRST_QUARTILE;
  }
  if (!g_strcmp0("SECOND_QUARTILE", str)) {
    return SECOND_QUARTILE;
  }
  if (!g_strcmp0("THIRD_QUARTILE", str)) {
    return THIRD_QUARTILE;
  }
  if (!g_strcmp0("FOURTH_QUARTILE", str)) {
    return FOURTH_QUARTILE;
  }

  return NONE;
}

// this should inherit the ActivityGraph class or buildable
enum {
  PROP_0,

  PROP_INTERVAL,
  PROP_NATIVE_COLORS,
  PROP_USER_NAME,
  PROP_AUTH_TOKEN,

  NUM_PROPERTIES,
};

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};
static void
g_barbar_github_activity_buildable_interface_init(GtkBuildableIface *iface);

static GtkBuildableIface *parent_buildable_iface;

G_DEFINE_TYPE_WITH_CODE(
    BarBarGithubActivity, g_barbar_github_activity, GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,
                          g_barbar_github_activity_buildable_interface_init))

static void g_barbar_github_activity_tag_add_child(GtkBuildable *buildable,
                                                   GtkBuilder *builder,
                                                   GObject *child,
                                                   const char *type) {
  g_return_if_fail(GTK_IS_WIDGET(child));

  BarBarGithubActivity *self = BARBAR_GITHUB_ACTIVITY(buildable);

  if (GTK_IS_WIDGET(child)) {
    self->activity_graph = GTK_WIDGET(child);
  } else {
    parent_buildable_iface->add_child(buildable, builder, child, type);
  }
}

static void
g_barbar_github_activity_buildable_interface_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_github_activity_tag_add_child;
}

static void g_barbar_github_activity_set_interval(BarBarGithubActivity *hub,
                                                  guint interval) {
  g_return_if_fail(BARBAR_IS_GITHUB_ACTIVITY(hub));

  if (hub->interval == interval) {
    return;
  }

  hub->interval = interval;

  g_object_notify_by_pspec(G_OBJECT(hub), properties[PROP_INTERVAL]);
}

static void
g_barbar_github_activity_set_native_colors(BarBarGithubActivity *hub,
                                           gboolean native) {
  g_return_if_fail(BARBAR_IS_GITHUB_ACTIVITY(hub));

  if (hub->native_colors == native) {
    return;
  }

  hub->native_colors = native;

  g_object_notify_by_pspec(G_OBJECT(hub), properties[PROP_NATIVE_COLORS]);
}

static void g_barbar_github_activity_set_user_name(BarBarGithubActivity *hub,
                                                   const char *user_name) {
  g_return_if_fail(BARBAR_IS_GITHUB_ACTIVITY(hub));

  if (!g_strcmp0(hub->user_name, user_name)) {
    return;
  }

  hub->user_name = g_strdup(user_name);

  g_object_notify_by_pspec(G_OBJECT(hub), properties[PROP_USER_NAME]);
}

static void g_barbar_github_activity_set_auth_token(BarBarGithubActivity *hub,
                                                    const char *auth_token) {
  g_return_if_fail(BARBAR_IS_GITHUB_ACTIVITY(hub));

  if (!g_strcmp0(hub->auth_token, auth_token)) {
    return;
  }

  hub->auth_token = g_strdup(auth_token);

  g_object_notify_by_pspec(G_OBJECT(hub), properties[PROP_AUTH_TOKEN]);
}

static void g_barbar_github_activity_set_property(GObject *object,
                                                  guint property_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec) {

  BarBarGithubActivity *hub = BARBAR_GITHUB_ACTIVITY(object);
  // printf("apa: %d\n", property_id);

  switch (property_id) {
  case PROP_INTERVAL:
    g_barbar_github_activity_set_interval(hub, g_value_get_uint(value));
    break;
  case PROP_NATIVE_COLORS:
    g_barbar_github_activity_set_native_colors(hub, g_value_get_boolean(value));
    break;
  case PROP_USER_NAME:
    g_barbar_github_activity_set_user_name(hub, g_value_get_string(value));
    break;
  case PROP_AUTH_TOKEN:
    g_barbar_github_activity_set_auth_token(hub, g_value_get_string(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_github_activity_get_property(GObject *object,
                                                  guint property_id,
                                                  GValue *value,
                                                  GParamSpec *pspec) {
  BarBarGithubActivity *hub = BARBAR_GITHUB_ACTIVITY(object);

  switch (property_id) {
  case PROP_INTERVAL:
    g_value_set_uint(value, hub->interval);
    break;
  case PROP_USER_NAME:
    g_value_set_string(value, hub->user_name);
    break;
  case PROP_NATIVE_COLORS:
    g_value_set_boolean(value, hub->native_colors);
    break;
  case PROP_AUTH_TOKEN:
    g_value_set_string(value, hub->auth_token);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void on_response(SoupSession *session, GAsyncResult *res,
                        gpointer user_data);

static gboolean fetch_data(gpointer data) {
  BarBarGithubActivity *self = data;
  SoupMessage *msg;
  SoupMessageHeaders *headers;

  const char *url = "https://api.github.com/graphql";

  // make this dynamic, should we be able to hoover etc?
  // const char *query =
  //     "query($userName:String!) { user(login: $userName) { "
  //     "contributionsCollection { contributionCalendar { totalContributions "
  //     "weeks { contributionDays { contributionCount date color "
  //     "contributionLevel } } } } } }";
  const char *query =
      "query($userName:String!) { user(login: $userName) { "
      "contributionsCollection { contributionCalendar { totalContributions "
      "weeks { contributionDays { contributionLevel color } } } } } }";

  gchar *payload = g_strdup_printf(
      "{\"query\": \"%s\", \"variables\": {\"userName\": \"%s\"}}", query,
      self->user_name);

  msg = soup_message_new(SOUP_METHOD_POST, url);
  soup_message_set_request_body_from_bytes(
      msg, "application/json", g_bytes_new(payload, strlen(payload)));
  headers = soup_message_get_request_headers(msg);

  soup_message_headers_append(headers, "Authorization",
                              g_strdup_printf("Bearer %s", self->auth_token));
  soup_message_headers_append(headers, "Content-Type", "application/json");
  soup_message_headers_append(headers, "User-Agent",
                              "Mozilla/5.0 (Windows NT 10.0; Win64; x64; "
                              "rv:126.0) Gecko/20100101 Firefox/126.0");

  soup_session_send_and_read_async(self->session, msg, 0, NULL,
                                   (GAsyncReadyCallback)on_response, self);
  return G_SOURCE_CONTINUE;
}

static void on_response(SoupSession *session, GAsyncResult *res,
                        gpointer user_data) {

  BarBarGithubActivity *hub = BARBAR_GITHUB_ACTIVITY(user_data);

  JsonParser *parser;
  GError *error = NULL;
  gboolean ret;
  GBytes *bytes = soup_session_send_and_read_finish(session, res, &error);

  if (error) {
    g_printerr("Error: %s\n", error->message);
    g_error_free(error);
  } else {
    gsize size;
    const gchar *data = g_bytes_get_data(bytes, &size);

    // printf("data: %.*s\n", (int)size, data);
    parser = json_parser_new();
    ret = json_parser_load_from_data(parser, data, size, &error);
    if (!ret) {
      g_printerr("Failed to parse json: %s\n", error->message);
      g_error_free(error);
      return;
    }
    JsonReader *reader = json_reader_new(json_parser_get_root(parser));
    json_reader_read_member(reader, "data");
    json_reader_read_member(reader, "user");
    json_reader_read_member(reader, "contributionsCollection");
    json_reader_read_member(reader, "contributionCalendar");
    json_reader_read_member(reader, "weeks");
    gint weeks = json_reader_count_elements(reader);
    uint graph_index = g_barbar_activity_graph_get_size(
        BARBAR_ACTIVITY_GRAPH(hub->activity_graph));

    for (int j = weeks - 1; j >= 0; j--) {
      json_reader_read_element(reader, j);
      json_reader_read_member(reader, "contributionDays");

      gint days = json_reader_count_elements(reader);
      for (int m = days - 1; m >= 0; m--) {
        graph_index--;
        json_reader_read_element(reader, m);

        if (hub->native_colors) {
          json_reader_read_member(reader, "color");
          const char *color = json_reader_get_string_value(reader);

          // g_barbar_activity_graph_set_color_linear(
          //     BARBAR_ACTIVITY_GRAPH(hub->activity_graph), graph_index,
          //     level);
          json_reader_end_member(reader);
        } else {
          json_reader_read_member(reader, "contributionLevel");
          const char *str_level = json_reader_get_string_value(reader);
          int level = parse_contrubiton_level(str_level);
          json_reader_end_member(reader);

          g_barbar_activity_graph_set_activity_linear(
              BARBAR_ACTIVITY_GRAPH(hub->activity_graph), graph_index, level);
        }

        json_reader_end_element(reader);
        if (!graph_index) {
          break;
        }
      }
      json_reader_end_member(reader);
      json_reader_end_element(reader);
      if (!graph_index) {
        break;
      }
    }

    json_reader_end_member(reader);
    json_reader_end_member(reader);
    json_reader_end_member(reader);
    json_reader_end_member(reader);
    json_reader_end_member(reader);
    // printf("totalContributions: %d\n", identifier);
  }
  g_bytes_unref(bytes);
}

void g_barbar_github_activity_root(GtkWidget *widget) {
  BarBarGithubActivity *hub = BARBAR_GITHUB_ACTIVITY(widget);

  GTK_WIDGET_CLASS(g_barbar_github_activity_parent_class)->root(widget);

  if (!hub->user_name || !hub->auth_token) {
    g_printerr("Either username or auth-token is unset for github-activity");
    return;
  }

  if (!hub->activity_graph) {
    hub->activity_graph = g_barbar_activity_graph_new(24, 7);
  }
  gtk_widget_set_parent(hub->activity_graph, GTK_WIDGET(hub));

  if (hub->source_id > 0) {
    g_source_remove(hub->source_id);
  }

  fetch_data(hub);

  // hub->source_id = g_timeout_add_full(0, hub->interval, fetch_data, hub,
  // NULL);
}

static void
g_barbar_github_activity_class_init(BarBarGithubActivityClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  widget_class->root = g_barbar_github_activity_root;

  gobject_class->set_property = g_barbar_github_activity_set_property;
  gobject_class->get_property = g_barbar_github_activity_get_property;

  /**
   * BarBarGithubActivity:interval:
   *
   * How often we should pull info from github
   */
  properties[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      3600 * 1000, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarGithubActivity:native-colors:
   *
   * If we should use the colors from github
   */
  properties[PROP_NATIVE_COLORS] =
      g_param_spec_boolean("native-colors", NULL, NULL, FALSE,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarGithubActivity:user-name:
   *
   * Github user-name
   */
  properties[PROP_USER_NAME] = g_param_spec_string(
      "user-name", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarGithubActivity:auth_token:
   *
   * The github token generated for the user. The token needs to have read:user
   * enabled.
   */
  properties[PROP_AUTH_TOKEN] = g_param_spec_string(
      "auth-token", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name(widget_class, "github-activity");
}

static void g_barbar_github_activity_init(BarBarGithubActivity *self) {
  self->session = soup_session_new();
}
