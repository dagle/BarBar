#include "barbar-util.h"
#include "barbar.h"
#include "config.h"
#include "glib.h"
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#ifdef SYSTRAY
#include <snsystray.h>
#endif
#include <string.h>

#include "sensors/barbar-sensor.h"

GList *sensors;

void on_button_clicked1(GtkButton *button, gpointer user_data) {
  g_print("Button1 Clicked!\n");
}
void on_button_clicked2(GtkButton *button, gpointer user_data) {
  g_print("Button2 Clicked!\n");
}
void on_button_clicked3(GtkButton *button, gpointer user_data) {
  g_print("Button3 Clicked!\n");
}

G_MODULE_EXPORT char *g_barbar_strdup_printf(GtkWidget *label,
                                             const char *format, ...) {
  gchar *buffer;
  va_list args;

  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);

  return buffer;
}

G_MODULE_EXPORT char *barbar_header_static(GtkWidget *label, gpointer data) {
  gchar *buffer;
  printf("what is happening?!\n");

  const char *str = gtk_label_get_label(GTK_LABEL(label));

  int len = strlen(str);
  int widget_width = gtk_widget_get_width(label);
  int count = widget_width - len;

  if (count < 0) {
    printf("widget_width is: %d\n", widget_width);
    buffer = g_strdup_printf("%s", str);
    return buffer;
  }

  gchar *dash_string = g_strnfill(count, '-');
  buffer = g_strdup_printf("%s%s", str, dash_string);

  return buffer;
}

G_MODULE_EXPORT char *barbar_header_dynamic(GtkWidget *label,
                                            const char seperator,
                                            const char *format, ...) {
  gchar *buffer;
  va_list args;

  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);

  return buffer;
}

void g_barbar_toggle_stack(GtkToggleButton *button, gpointer user_data) {
  GtkSelectionModel *pages;
  GtkWidget *stack = gtk_button_get_child(GTK_BUTTON(button));

  pages = gtk_stack_get_pages(GTK_STACK(stack));
  guint idx = gtk_toggle_button_get_active(button) ? 1 : 0;
  gtk_selection_model_select_item(pages, idx, TRUE);
}

static void g_barbar_resources_install(void) {
  GError *error = NULL;
  GResource *resource;

  resource = g_resource_load(PKGDATADIR "/barbar.gresource", &error);
  printf("path: %s\n", PKGDATADIR "/barbar.gresource");
  if (error) {
    g_printerr("Error loading resources: %s\n", error->message);
    return;
  }

  g_resources_register(resource);
}

static void activate(GtkApplication *app, void *data) {
  GtkBuilderScope *scope;
  GtkBuilder *builder;
  g_barbar_resources_install();

  scope = gtk_builder_cscope_new();

  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope),
                                  g_barbar_strdup_printf);
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope),
                                  g_barbar_toggle_stack);
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope),
                                  g_barbar_mpris_player_play);
  gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope),
                                  g_barbar_mpris_player_pause);
  // gtk_builder_cscope_add_callback(GTK_BUILDER_CSCOPE(scope),
  //                                 g_barbar_mpris_player_stop);

  g_barbar_default_style_provider("barbar/style.css");

  builder = g_barbar_default_builder("barbar/config.ui", NULL);

  GSList *list = gtk_builder_get_objects(builder);

  for (GSList *it = list; it; it = it->next) {
    GObject *object = it->data;

    if (GTK_IS_WINDOW(object)) {
      GtkWindow *window = GTK_WINDOW(object);
      gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));
      gtk_window_present(window);
    } else if (BARBAR_IS_SENSOR(object)) {
      g_object_ref(object);
      BarBarSensor *sensor = BARBAR_SENSOR(object);
      g_barbar_sensor_start(sensor);
      sensors = g_list_append(sensors, object);
    }
  }
  g_slist_free(list);
  g_object_unref(builder);
}

void shutdown(GApplication *self, gpointer user_data) {
  g_list_free_full(sensors, g_object_unref);
}

int run(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("com.github.barbar", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect(app, "shutdown", G_CALLBACK(shutdown), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}

int main(int argc, char **argv) {
  gtk_init();
  g_barbar_init();

#ifdef SYSTRAY
  g_type_ensure(SN_TYPE_SYSTRAY);
#endif

  run(argc, argv);
}
