#include "barbar.h"
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include "sensors/barbar-clock.h"
#include "sensors/barbar-sensor.h"
#include "status-notifier.h"
#include "widgets/barbar-label.h"

static gboolean reg_host(StatusNotifierWatcher *obj,
                         GDBusMethodInvocation *invocation,
                         const gchar *service, gpointer user_data) {
  const gchar *sender = g_dbus_method_invocation_get_sender(invocation);
  g_print("HOST: %s - %s\n", service, sender);
  return TRUE;
}
static gboolean reg_item(StatusNotifierWatcher *obj,
                         GDBusMethodInvocation *invocation,
                         const gchar *service, gpointer user_data) {
  g_print("ITEM\n");
  return TRUE;
}

void g_barbar_status_watcher_bus_acquired_handler2(GDBusConnection *connection,
                                                   const gchar *name,
                                                   gpointer user_data) {
  GError *error = NULL;

  StatusNotifierWatcher *watcher;
  watcher = status_notifier_watcher_skeleton_new();

  g_signal_connect_swapped(watcher, "handle-register-host",
                           G_CALLBACK(reg_host), NULL);
  g_signal_connect_swapped(watcher, "handle-register-item",
                           G_CALLBACK(reg_item), NULL);
  g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(watcher),
                                   connection, "/StatusNotifierWatcher",
                                   &error);
}

G_MODULE_EXPORT char *barbar_strdup_printf(GtkWidget *label, const char *format,
                                           ...) {
  gchar *buffer;
  va_list args;

  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);

  return buffer;
}

static void activate(GtkApplication *app, void *data) {
  GtkBuilderScope *scope;
  GtkBuilder *builder;

  scope = gtk_builder_cscope_new();
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope), label_update);
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope),
                                  barbar_strdup_printf);

  barbar_default_style_provider("barbar/style.css");

  builder = barbar_default_builder("barbar/config.ui", NULL);

  GSList *list = gtk_builder_get_objects(builder);

  for (GSList *it = list; it; it = it->next) {
    GObject *object = it->data;

    if (GTK_IS_WINDOW(object)) {
      GtkWindow *window = GTK_WINDOW(object);
      gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));
      gtk_window_present(window);
    } else if (BARBAR_IS_SENSOR(object)) {
      // TODO: We shouldn't leak the sensors.
      g_object_ref(object);
      BarBarSensor *sensor = BARBAR_SENSOR(object);
      g_barbar_sensor_start(sensor);
    }
  }
  g_slist_free(list);
  g_object_unref(builder);
}

int run(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("com.github.barbar", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}

int main(int argc, char **argv) {
  gtk_init();
  g_barbar_init();
  run(argc, argv);
}
