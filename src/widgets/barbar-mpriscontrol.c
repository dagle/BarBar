/*
 * Copyright © 2024 Per Odlund
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Benjamin Otte <otte@gnome.org>
 */

#include "barbar-mpriscontrol.h"
#include "gtk/gtkcssprovider.h"
#include "sensors/barbar-mpris2.h"

/**
 * BarBarMprisControls:
 *
 * `BarBarMprisControls` is a widget to interact with mpris
 *
 */

struct _BarBarMprisControls {
  GtkWidget parent_instance;

  BarBarMpris *mpris;
  GtkCssProvider *provider;

  GtkAdjustment *time_adjustment;
  GtkAdjustment *volume_adjustment;
  GtkWidget *box;
  GtkWidget *play_button;

  GtkWidget *stop_button;
  GtkWidget *next_button;
  GtkWidget *prev_button;

  GtkWidget *time_box;
  GtkWidget *time_label;
  GtkWidget *seek_scale;
  GtkWidget *duration_label;

  GtkWidget *volume_button;
};

enum {
  PROP_0,
  PROP_MPRIS,

  NUM_PROPERTIES
};

G_DEFINE_TYPE(BarBarMprisControls, g_barbar_mpris_controls, GTK_TYPE_WIDGET)

static GParamSpec *properties[NUM_PROPERTIES] = {
    NULL,
};

static void time_adjustment_changed(GtkAdjustment *adjustment,
                                    BarBarMprisControls *controls) {
  if (controls->mpris == NULL)
    return;

  /* We just updated the adjustment and it's correct now */
  if (gtk_adjustment_get_value(adjustment) ==
      (double)g_barbar_mpris_get_position(controls->mpris) / G_USEC_PER_SEC)
    return;

  g_barbar_mpris_seek(controls->mpris,
                      gtk_adjustment_get_value(adjustment) * G_USEC_PER_SEC +
                          0.5);
}

static void volume_adjustment_changed(GtkAdjustment *adjustment,
                                      BarBarMprisControls *controls) {
  if (controls->mpris == NULL)
    return;

  /* We just updated the adjustment and it's correct now */
  if (gtk_adjustment_get_value(adjustment) ==
      g_barbar_mpris_get_volume(controls->mpris))
    return;

  // g_barbar_mpris_set_muted(controls->mpris,
  //                          gtk_adjustment_get_value(adjustment) == 0.0);
  g_barbar_mpris_set_volume(controls->mpris,
                            gtk_adjustment_get_value(adjustment));
}

static void play_button_clicked(GtkWidget *button,
                                BarBarMprisControls *controls) {
  if (controls->mpris == NULL)
    return;

  g_barbar_mpris_set_play_pause(controls->mpris);
}

static void gtk_media_controls_measure(GtkWidget *widget,
                                       GtkOrientation orientation, int for_size,
                                       int *minimum, int *natural,
                                       int *minimum_baseline,
                                       int *natural_baseline) {
  BarBarMprisControls *controls = BARBAR_MPRIS_CONTROLS(widget);

  gtk_widget_measure(controls->box, orientation, for_size, minimum, natural,
                     minimum_baseline, natural_baseline);
}

static void gtk_media_controls_size_allocate(GtkWidget *widget, int width,
                                             int height, int baseline) {
  BarBarMprisControls *controls = BARBAR_MPRIS_CONTROLS(widget);

  gtk_widget_size_allocate(controls->box, &(GtkAllocation){0, 0, width, height},
                           baseline);
}

static void gtk_media_controls_dispose(GObject *object) {
  BarBarMprisControls *controls = BARBAR_MPRIS_CONTROLS(object);

  g_barbar_mpris_controls_set_media_stream(controls, NULL);

  gtk_widget_dispose_template(GTK_WIDGET(object), GTK_TYPE_MEDIA_CONTROLS);

  G_OBJECT_CLASS(g_barbar_mpris_controls_parent_class)->dispose(object);
}

static void gtk_media_controls_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  BarBarMprisControls *controls = BARBAR_MPRIS_CONTROLS(object);

  switch (property_id) {
  case PROP_MPRIS:
    g_value_set_object(value, controls->mpris);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void gtk_media_controls_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  BarBarMprisControls *controls = BARBAR_MPRIS_CONTROLS(object);

  switch (property_id) {
  case PROP_MPRIS:
    g_barbar_mpris_controls_set_media_stream(controls,
                                             g_value_get_object(value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void
g_barbar_mpris_controls_class_init(BarBarMprisControlsClass *klass) {
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  widget_class->measure = gtk_media_controls_measure;
  widget_class->size_allocate = gtk_media_controls_size_allocate;

  gobject_class->dispose = gtk_media_controls_dispose;
  gobject_class->get_property = gtk_media_controls_get_property;
  gobject_class->set_property = gtk_media_controls_set_property;

  /**
   * GtkMediaControls:media-stream: (attributes
   * org.gtk.Property.get=gtk_media_controls_get_media_stream
   * org.gtk.Property.set=gtk_media_controls_set_media_stream)
   *
   * The media-stream managed by this object or %NULL if none.
   */
  properties[PROP_MPRIS] = g_param_spec_object(
      "media-stream", NULL, NULL, GTK_TYPE_MEDIA_STREAM,
      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class, "/org/gtk/libgtk/ui/gtkmediacontrols.ui");
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       time_adjustment);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       volume_adjustment);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls, box);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       play_button);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       time_box);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       time_label);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       seek_scale);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       duration_label);
  gtk_widget_class_bind_template_child(widget_class, BarBarMprisControls,
                                       volume_button);

  gtk_widget_class_bind_template_callback(widget_class, play_button_clicked);
  gtk_widget_class_bind_template_callback(widget_class,
                                          time_adjustment_changed);
  gtk_widget_class_bind_template_callback(widget_class,
                                          volume_adjustment_changed);

  // gtk_widget_class_set_css_name(widget_class, I_("controls"));
  gtk_widget_class_set_css_name(widget_class, "controls");
}

static void g_barbar_mpris_controls_init(BarBarMprisControls *controls) {
  gtk_widget_init_template(GTK_WIDGET(controls));
}

/**
 * gtk_media_controls_new:
 * @stream: (nullable) (transfer none): a `GtkMediaStream` to manage
 *
 * Creates a new `GtkMediaControls` managing the @stream passed to it.
 *
 * Returns: a new `GtkMediaControls`
 */
GtkWidget *gtk_media_controls_new(GtkMediaStream *stream) {
  return g_object_new(GTK_TYPE_MEDIA_CONTROLS, "media-stream", stream, NULL);
}

/**
 * gtk_media_controls_get_media_stream: (attributes
 * org.gtk.Method.get_property=media-stream)
 * @controls: a `GtkMediaControls`
 *
 * Gets the media stream managed by @controls or %NULL if none.
 *
 * Returns: (nullable) (transfer none): The media stream managed by @controls
 */
BarBarMpris *g_barbar_mpris_controls_get_mpris(BarBarMprisControls *controls) {
  g_return_val_if_fail(GTK_IS_MEDIA_CONTROLS(controls), NULL);

  return controls->mpris;
}

static void update_timestamp(BarBarMprisControls *controls) {
  gint64 timestamp, duration;
  char *time_string;

  if (controls->mpris) {
    timestamp = g_barbar_mpris_get_position(controls->mpris);
    duration = g_barbar_mpris_get_duration(controls->mpris);
  } else {
    timestamp = 0;
    duration = 0;
  }

  // time_string = totem_time_to_string(timestamp, FALSE, FALSE);
  gtk_label_set_text(GTK_LABEL(controls->time_label), time_string);
  g_free(time_string);

  if (duration > 0) {
    // time_string = totem_time_to_string(
    //     duration > timestamp ? duration - timestamp : 0, TRUE, FALSE);
    gtk_label_set_text(GTK_LABEL(controls->duration_label), time_string);
    g_free(time_string);

    gtk_adjustment_set_value(controls->time_adjustment,
                             (double)timestamp / G_USEC_PER_SEC);
  }
}

static void update_duration(BarBarMprisControls *controls) {
  gint64 timestamp, duration;
  char *time_string;

  if (controls->mpris) {
    timestamp = g_barbar_mpris_get_position(controls->mpris);
    duration = g_barbar_mpris_get_duration(controls->mpris);
  } else {
    timestamp = 0;
    duration = 0;
  }

  // time_string = totem_time_to_string(
  //     duration > timestamp ? duration - timestamp : 0, TRUE, FALSE);
  gtk_label_set_text(GTK_LABEL(controls->duration_label), time_string);
  gtk_widget_set_visible(controls->duration_label, duration > 0);
  g_free(time_string);

  gtk_adjustment_set_upper(
      controls->time_adjustment,
      gtk_adjustment_get_page_size(controls->time_adjustment) +
          (double)duration / G_USEC_PER_SEC);
  gtk_adjustment_set_value(controls->time_adjustment,
                           (double)timestamp / G_USEC_PER_SEC);
}

static void update_playing(BarBarMprisControls *controls) {
  gboolean playing;
  const char *icon_name;
  const char *tooltip_text;

  if (controls->mpris)
    playing = g_barbar_mpris_get_playing(controls->mpris);
  else
    playing = FALSE;

  if (playing) {
    icon_name = "media-playback-pause-symbolic";
    // tooltip_text = C_("media controls tooltip", "Stop");
  } else {
    icon_name = "media-playback-start-symbolic";
    // tooltip_text = C_("media controls tooltip", "Play");
  }

  gtk_button_set_icon_name(GTK_BUTTON(controls->play_button), icon_name);
  gtk_widget_set_tooltip_text(controls->play_button, tooltip_text);
}

static void update_seekable(BarBarMprisControls *controls) {
  gboolean seekable;

  if (controls->mpris)
    seekable = g_barbar_mpris_is_seekable(controls->mpris);
  else
    seekable = FALSE;

  gtk_widget_set_sensitive(controls->seek_scale, seekable);
}

static void update_volume(BarBarMprisControls *controls) {
  double volume;

  if (controls->mpris == NULL)
    volume = 1.0;
  // else if (gtk_media_stream_get_muted(controls->mpris))
  //   volume = 0.0;
  else
    volume = g_barbar_mpris_get_volume(controls->mpris);

  gtk_adjustment_set_value(controls->volume_adjustment, volume);

  gtk_widget_set_sensitive(controls->volume_button,
                           controls->mpris == NULL ||
                               gtk_media_stream_has_audio(controls->mpris));
}

static void update_all(BarBarMprisControls *controls) {
  update_timestamp(controls);
  update_duration(controls);
  update_playing(controls);
  update_seekable(controls);
  update_volume(controls);
}

static void g_barbar_mpris_controls_notify_cb(BarBarMpris *mpris,
                                              GParamSpec *pspec,
                                              BarBarMprisControls *controls) {
  if (g_str_equal(pspec->name, "position"))
    update_timestamp(controls);
  else if (g_str_equal(pspec->name, "duration"))
    update_duration(controls);
  else if (g_str_equal(pspec->name, "playing"))
    update_playing(controls);
  else if (g_str_equal(pspec->name, "seekable"))
    update_seekable(controls);
  else if (g_str_equal(pspec->name, "volume"))
    update_volume(controls);
  // PROP_CAN_GO_NEXT,
  // PROP_CAN_GO_PREVIOUS,
  // else if (g_str_equal(pspec->name, "has-audio"))
  //   update_volume(controls);
}

/**
 * gtk_media_controls_set_media_stream: (attributes
 * org.gtk.Method.set_property=media-stream)
 * @controls: a `GtkMediaControls` widget
 * @stream: (nullable):  a `GtkMediaStream`
 *
 * Sets the stream that is controlled by @controls.
 */
void g_barbar_mpris_controls_set_media_stream(BarBarMprisControls *controls,
                                              BarBarMpris *mpris) {
  g_return_if_fail(BARBAR_IS_MPRIS_CONTROLS(controls));
  g_return_if_fail(mpris == NULL || BARBAR_IS_MPRIS(mpris));

  if (controls->mpris == mpris)
    return;

  if (controls->mpris) {
    g_signal_handlers_disconnect_by_func(
        controls->mpris, g_barbar_mpris_controls_notify_cb, controls);
    g_object_unref(controls->mpris);
    controls->mpris = NULL;
  }

  if (mpris) {
    controls->mpris = g_object_ref(mpris);
    g_signal_connect(controls->mpris, "notify",
                     G_CALLBACK(g_barbar_mpris_controls_notify_cb), controls);
  }

  update_all(controls);
  gtk_widget_set_sensitive(controls->box, mpris != NULL);

  g_object_notify_by_pspec(G_OBJECT(controls), properties[PROP_MPRIS]);
}
