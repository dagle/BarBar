#include "barbar-bar.h"
#include "barbar.h"
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include "sensors/barbar-clock.h"
#include "sensors/barbar-sensor.h"
#include "status-notifier.h"

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

// static void activate(GtkApplication *app, void *data) {
//   MyCustomWidget *widget = my_custom_widget_new();
//   BarBarClock *clock = g_object_new(BARBAR_TYPE_CLOCK, NULL);
//
//   GtkWindow *gtk_window = GTK_WINDOW(gtk_application_window_new(app));
//   gtk_window_set_child(gtk_window, widget);
//   gtk_window_present(gtk_window);
// }

// int main(int argc, char *argv[]) {
//   GtkApplication *app = gtk_application_new(
//       "com.github.wmww.gtk4-layer-shell.example",
//       G_APPLICATION_DEFAULT_FLAGS);
//   g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
//   int status = g_application_run(G_APPLICATION(app), argc, argv);
//   g_object_unref(app);
//   return status;
//
//   // MyCustomWidget *widget = my_custom_widget_new();
//   // gtk_widget_show(GTK_WIDGET(widget));
//
//   // Perform your application logic here
// }
G_MODULE_EXPORT char *bepa(GtkWidget *label, char *time) {
  if (time) {
    return strdup(time);
  }
  return NULL;
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

G_MODULE_EXPORT void set_title(GtkWidget *label, GParamSpec *pspec,
                               GtkWidget *clock) {
  char *time;
  g_object_get(clock, "time", &time, NULL);
  gtk_label_set_text(GTK_LABEL(label), time);
  // return g_strdup_printf("%s", time);
}

G_MODULE_EXPORT void update(GtkWidget *label, GtkWidget *clock) {
  const gchar *className = g_type_name(G_TYPE_FROM_INSTANCE(clock));
  printf("update: %s\n", className);
  // printf("update called\n");
  char *time;
  g_object_get(clock, "time", &time, NULL);
  gtk_label_set_text(GTK_LABEL(label), time);
}

static void activate2(GtkApplication *app, void *data) {
  GtkBuilderScope *scope;
  GtkBuilder *builder;
  scope = gtk_builder_cscope_new();
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope), bepa);
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope), set_title);
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope), update);
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope),
                                  barbar_strdup_printf);

  builder = gtk_builder_new();
  // gtk_builder_set_scope(builder, scope);

  // builder =
  // gtk_builder_new_from_file("/home/dagle/.config/barbar/config.ui");
  gtk_builder_add_from_file(builder, "/home/dagle/.config/barbar/config.ui",
                            NULL);
  // gtk_builder_cscope_add_callback_symbol(builder, "bepa", G_CALLBACK(bepa));
  // gtk_builder_cscope_add_callback
  GSList *list = gtk_builder_get_objects(builder);

  for (GSList *it = list; it; it = it->next) {
    GObject *object = it->data;

    if (BARBAR_IS_BAR(object)) {
      GtkWindow *window = GTK_WINDOW(object);
      gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));
      gtk_window_present(window);
    } else if (BARBAR_IS_SENSOR(object)) {
      g_object_ref(object);
      BarBarSensor *sensor = BARBAR_SENSOR(object);
      g_barbar_sensor_start(sensor);
    }
  }
  g_slist_free(list);

  // GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(builder, "window1"));
  // gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));
  g_object_unref(builder);
}

int run(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("com.github.barbar", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate2), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}

int main(int argc, char **argv) {
  // BarBarDisk *disk = g_object_new(BARBAR_TYPE_DISK, NULL);
  // g_barbar_disk_update(disk);
  //
  // // BarBarMpd *mpd = g_object_new(BARBAR_TYPE_MPD, NULL);
  // // g_barbar_mpd_update(mpd);
  //
  // BarBarBattery *bat = g_object_new(BARBAR_TYPE_BATTERY, NULL);
  // g_barbar_battery_update(bat);
  //
  // BarBarClock *clock =
  //     g_object_new(BARBAR_TYPE_CLOCK, "tz", "Europe/Stockholm", NULL);
  // g_barbar_clock_start(clock);
  //
  // BarBarCpu *cpu = g_object_new(BARBAR_TYPE_CPU, NULL);
  // g_barbar_cpu_update(cpu);
  //
  // // BarBarWireplumber *wp = g_object_new(BARBAR_TYPE_WIREPLUMBER, NULL);
  // // g_barbar_wireplumber_update(wp);
  //
  // BarBarMpris *mpris = g_object_new(BARBAR_TYPE_MPRIS, "player", "mpd",
  // NULL); g_barbar_mpris_update(mpris);
  //
  // // TODO: we can't can't test this one until we actually make things into
  // widgets!
  //
  // // BarBarRiver *river = g_object_new(BARBAR_TYPE_RIVER, NULL);
  // // g_barbar_river_update(river);
  //
  // BarBarTemperature *temp = g_object_new(BARBAR_TYPE_TEMPERATURE, NULL);
  // g_barbar_temperature_update(temp);
  //
  // GMainLoop *loop;
  //
  // loop = g_main_loop_new(NULL, FALSE);
  //
  // g_bus_own_name(
  //     G_BUS_TYPE_SESSION, "org.kde.StatusNotifierWatcher",
  //
  //     G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
  //     G_BUS_NAME_OWNER_FLAGS_REPLACE, NULL,
  //
  //     g_barbar_status_watcher_bus_acquired_handler2, NULL, NULL, NULL);
  // g_main_loop_run(loop);
  gtk_init();
  g_barbar_bar_get_type();
  g_barbar_init();
  // GtkWidget *bar = g_barbar_bar_new();
  // g_object_unref(bar);
  run(argc, argv);
  // int status = g_barbar_run(argc, argv, NULL);
  // return status;
}
