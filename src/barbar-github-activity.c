#include "barbar-github-activity.h"
#include <glib.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <stdio.h>

/**
 * BarBarGitHubActivity:
 *
 * An activity graph, often used to display github activity
 */
struct _BarBarGithubActivity {
  GtkWidget parent_instance;

  char *user_name;
  char *auth_token;
  GtkWidget *activity_graph;
};

// this should inherit the ActivityGraph class or buildable
enum {
  PROP_0,

  PROP_INTERVAL,
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

  // if (g_strcmp0(type, "tag") == 0) {
  // g_barbar_river_tag_add_button(self, GTK_WIDGET(child));
  // }
}

static void
g_barbar_github_activity_buildable_interface_init(GtkBuildableIface *iface) {
  parent_buildable_iface = g_type_interface_peek_parent(iface);
  iface->add_child = g_barbar_github_activity_tag_add_child;
}

static void on_response(SoupSession *session, GAsyncResult *res,
                        gpointer user_data);

static int fetch_data(BarBarGithubActivity *activity) {
  // Initialize libsoup
  // GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  SoupSession *session = soup_session_new();
  SoupMessage *msg;
  SoupMessageHeaders *headers;

  const char *url = "https://api.github.com/graphql";

  // GraphQL query
  const char *query =
      "query($userName:String!) { user(login: $userName) { "
      "contributionsCollection { contributionCalendar { totalContributions "
      "weeks { contributionDays { contributionCount date } } } } } }";

  // Construct the JSON payload
  gchar *payload = g_strdup_printf(
      "{\"query\": \"%s\", \"variables\": {\"userName\": \"%s\"}}", query,
      activity->user_name);

  // Create the message
  msg = soup_message_new(SOUP_METHOD_POST, url);
  soup_message_set_request_body_from_bytes(
      msg, "application/json", g_bytes_new(payload, strlen(payload)));
  headers = soup_message_get_request_headers(msg);

  // Set the headers
  soup_message_headers_append(
      headers, "Authorization",
      g_strdup_printf("Bearer %s", activity->auth_token));
  soup_message_headers_append(headers, "Content-Type", "application/json");
  soup_message_headers_append(headers, "User-Agent",
                              "Mozilla/5.0 (Windows NT 10.0; Win64; x64; "
                              "rv:126.0) Gecko/20100101 Firefox/126.0");

  // Send the message asynchronously
  soup_session_send_and_read_async(session, msg, 0, NULL,
                                   (GAsyncReadyCallback)on_response, NULL);

  // Run the main loop
  // g_main_loop_run(loop);

  // Clean up
  // g_object_unref(session);
  // g_main_loop_unref(loop);
  g_object_unref(msg);
  g_free(payload);

  return 0;
}

static void on_response(SoupSession *session, GAsyncResult *res,
                        gpointer user_data) {
  JsonParser *parser;
  GError *error = NULL;
  gboolean ret;
  GBytes *bytes = soup_session_send_and_read_finish(session, res, &error);

  if (error) {
    g_printerr("Error: %s\n", error->message);
    g_error_free(error);
  } else {
    // SoupMessageBody *body = soup_message_get_response_body(msg);
    // GBytes *response_data = soup_message_body_flatten(body);
    gsize size;
    const gchar *data = g_bytes_get_data(bytes, &size);
    parser = json_parser_new();
    ret = json_parser_load_from_data(parser, data, size, &error);
    // g_print("%.*s\n", (int)size, data);
    if (!ret) {
      g_printerr("Failed to parse json: %s\n", error->message);
      return;
    }
    JsonReader *reader = json_reader_new(json_parser_get_root(parser));
    json_reader_read_member(reader, "data");
    json_reader_read_member(reader, "user");
    json_reader_read_member(reader, "contributionsCollection");
    json_reader_read_member(reader, "contributionCalendar");
    json_reader_read_member(reader, "totalContributions");
    int identifier = json_reader_get_int_value(reader);
    json_reader_end_member(reader);
    json_reader_read_member(reader, "weeks");
    gint i = json_reader_count_elements(reader);
    for (int j = 0; j < i; j++) {
      json_reader_read_element(reader, j);
      json_reader_read_member(reader, "contributionDays");

      gint f = json_reader_count_elements(reader);
      for (int m = 0; m < f; m++) {
        json_reader_read_element(reader, m);
        json_reader_read_member(reader, "contributionCount");
        int num = json_reader_get_int_value(reader);
        printf("contribs: %d\n", num);
        json_reader_end_member(reader);
        json_reader_end_element(reader);
      }
      json_reader_end_member(reader);
      json_reader_end_element(reader);
    }

    json_reader_end_member(reader);
    json_reader_end_member(reader);
    json_reader_end_member(reader);
    json_reader_end_member(reader);
    json_reader_end_member(reader);
    printf("totalContributions: %d\n", identifier);
  }
  g_bytes_unref(bytes);
}

static void
g_barbar_github_activity_class_init(BarBarGithubActivityClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  /**
   * BarBarGithubActivity:interval:
   *
   * How often we should pull info from github
   */
  properties[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      3600 * 1000, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

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
  properties[PROP_USER_NAME] = g_param_spec_string(
      "auth-token", NULL, NULL, NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
}

static void g_barbar_github_activity_init(BarBarGithubActivity *self) {}
