#include "barbar-pomodoro.h"
#include <gtk/gtk.h>

/**
 * BarBarPomodoro:
 *
 * A sensor for using the pomodoro focus techinque.
 *
 */
struct _BarBarPomodoro {
  BarBarSensor parent_instance;

  guint length;
  guint rest;
  guint current;

  gboolean working;

  guint source_id;
};

enum {
  PROP_0,

  PROP_CURRENT,

  PROP_LENGTH,
  PROP_REST,

  PROP_WORK,
  PROP_RUNNING,

  // PROP_RESET,

  NUM_PROPERTIES,
};

enum {
  TICK,
  MODE_CHANGED,
  NUM_SIGNALS,
};

G_DEFINE_TYPE(BarBarPomodoro, g_barbar_pomodoro, BARBAR_TYPE_SENSOR);

static GParamSpec *pomodoro_props[NUM_PROPERTIES] = {
    NULL,
};

static guint pomodoro_signals[NUM_SIGNALS];

static void g_barbar_pomodoro_start(BarBarSensor *sensor);
static void g_barbar_pomodoro_constructed(GObject *object);

static void g_barbar_pomodoro_set_length(BarBarPomodoro *pomodoro,
                                         guint length) {
  g_return_if_fail(BARBAR_IS_POMODORO(pomodoro));

  if (pomodoro->length == length) {
    return;
  }

  pomodoro->length = length;

  g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_LENGTH]);
}

static void g_barbar_pomodoro_set_rest(BarBarPomodoro *pomodoro, guint rest) {
  g_return_if_fail(BARBAR_IS_POMODORO(pomodoro));

  if (pomodoro->rest == rest) {
    return;
  }

  pomodoro->rest = rest;

  g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_REST]);
}

static void g_barbar_pomodoro_set_current(BarBarPomodoro *pomodoro,
                                          guint current) {
  g_return_if_fail(BARBAR_IS_POMODORO(pomodoro));

  if (pomodoro->current == current) {
    return;
  }

  pomodoro->current = current;

  g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_CURRENT]);
}
static gboolean g_barbar_pomodoro_tick(gpointer data) {
  BarBarPomodoro *pomodoro = BARBAR_POMODORO(data);

  // If we are at the end of a session, we change modes
  if (!pomodoro->current) {
    if (pomodoro->working) {
      pomodoro->working = FALSE;
      pomodoro->current = pomodoro->rest;
      g_signal_emit(pomodoro, pomodoro_signals[MODE_CHANGED], 0, FALSE);
    } else {
      pomodoro->working = TRUE;
      pomodoro->current = pomodoro->length;
      g_signal_emit(pomodoro, pomodoro_signals[MODE_CHANGED], 0, TRUE);
    }
    g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_REST]);
  } else {
    pomodoro->current--;
  }
  g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_CURRENT]);

  return G_SOURCE_CONTINUE;
}
static void g_barbar_pomodoro_set_work(BarBarPomodoro *pomodoro,
                                       gboolean working) {
  g_return_if_fail(BARBAR_IS_POMODORO(pomodoro));

  if (pomodoro->working == working) {
    return;
  }

  pomodoro->working = working;
  if (pomodoro->source_id > 0) {
    if (working) {
      pomodoro->current = pomodoro->length;
    } else {
      pomodoro->current = pomodoro->rest;
    }
    g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_CURRENT]);
  }
  g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_WORK]);
}

static void g_barbar_pomodoro_set_running(BarBarPomodoro *pomodoro,
                                          gboolean run) {
  g_return_if_fail(BARBAR_IS_POMODORO(pomodoro));

  if ((pomodoro->source_id > 0 && run) || (pomodoro->source_id <= 0 && !run)) {
    return;
  }

  if (run) {
    pomodoro->source_id =
        g_timeout_add_full(0, 1000, g_barbar_pomodoro_tick, pomodoro, NULL);
  } else {
    g_source_remove(pomodoro->source_id);
    pomodoro->source_id = 0;
  }

  g_object_notify_by_pspec(G_OBJECT(pomodoro), pomodoro_props[PROP_RUNNING]);
}

static void g_barbar_pomodoro_set_reset(BarBarPomodoro *pomodoro) {
  g_return_if_fail(BARBAR_IS_POMODORO(pomodoro));

  if (pomodoro->source_id > 0) {
    g_source_remove(pomodoro->source_id);
  }

  pomodoro->current = pomodoro->length;
  pomodoro->working = TRUE;
  pomodoro->source_id = 0;
}

static void g_barbar_pomodoro_get_property(GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec) {
  BarBarPomodoro *pomodoro = BARBAR_POMODORO(object);

  switch (property_id) {
  case PROP_LENGTH:
    g_value_set_uint(value, pomodoro->length);
    break;
  case PROP_REST:
    g_value_set_uint(value, pomodoro->rest);
    break;
  case PROP_CURRENT:
    g_value_set_uint(value, pomodoro->current);
    break;
  case PROP_WORK:
    g_value_set_boolean(value, pomodoro->working);
    break;
  case PROP_RUNNING:
    g_value_set_boolean(value, pomodoro->source_id > 0);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_pomodoro_set_property(GObject *object, guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec) {
  BarBarPomodoro *pomodoro = BARBAR_POMODORO(object);

  switch (property_id) {
  case PROP_LENGTH:
    g_barbar_pomodoro_set_length(pomodoro, g_value_get_uint(value));
    break;
  case PROP_REST:
    g_barbar_pomodoro_set_rest(pomodoro, g_value_get_uint(value));
    break;
  case PROP_CURRENT:
    g_barbar_pomodoro_set_current(pomodoro, g_value_get_uint(value));
    break;
  case PROP_WORK:
    g_barbar_pomodoro_set_work(pomodoro, g_value_get_boolean(value));
    break;
  case PROP_RUNNING:
    g_barbar_pomodoro_set_running(pomodoro, g_value_get_boolean(value));
    break;
  // case PROP_RESET:
  //   g_barbar_pomodoro_set_reset(pomodoro);
  //   break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_pomodoro_class_init(BarBarPomodoroClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor = BARBAR_SENSOR_CLASS(class);

  gobject_class->set_property = g_barbar_pomodoro_set_property;
  gobject_class->get_property = g_barbar_pomodoro_get_property;
  gobject_class->constructed = g_barbar_pomodoro_constructed;
  sensor->start = g_barbar_pomodoro_start;

  /**
   * BarBarPomodoro:length:
   *
   * The length of a work session.
   *
   */
  pomodoro_props[PROP_LENGTH] =
      g_param_spec_uint("length", "Length", "How long we work", 0, G_MAXUINT32,
                        25 * 60, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarPomodoro:rest:
   *
   * The lentgth of a rest between session
   *
   */
  pomodoro_props[PROP_REST] =
      g_param_spec_uint("rest", "Rest", "How long we rest", 0, G_MAXUINT32,
                        5 * 60, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarPomodoro:current:
   *
   * The current clock for either work or rest session.
   *
   */
  pomodoro_props[PROP_CURRENT] = g_param_spec_uint(
      "current", "Current", "Current type in the mode", 0, G_MAXUINT32, 25 * 60,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarPomodoro:work:
   *
   * If we are in work or rest mode
   *
   */
  pomodoro_props[PROP_WORK] =
      g_param_spec_boolean("work", "Work", "Are we currently in working", TRUE,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarPomodoro:running:
   *
   * If we are currently running
   *
   */
  pomodoro_props[PROP_RUNNING] =
      g_param_spec_boolean("running", "Running", "Is the clock running", FALSE,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  // /**
  //  * BarBarPomodoro:reset:
  //  *
  //  * Reset the timer
  //  *
  //  */
  // pomodoro_props[PROP_RESET] =
  //     g_param_spec_boolean("reset", "Reset", "Reset the time",
  //     FALSE,
  //                          G_PARAM_WRITEABLE);
  /**
   * BarBarPomodoro::tick:
   * @sensor: This sensor
   *
   * Emit that the clock has ticked. This means that we want to refetch
   * the clock.
   */
  pomodoro_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_POMODORO,                   /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
  /**
   * BarBarPomodoro::mode-change:
   * @sensor: This sensor
   * @working: if we changed to work mode
   *
   * Emit that the clock has ticked. This means that we want to refetch
   * the clock.
   */
  pomodoro_signals[MODE_CHANGED] =
      g_signal_new("mode-change",                          /* signal_name */
                   BARBAR_TYPE_POMODORO,                   /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   1,                                      /* n_params */
                   G_TYPE_BOOLEAN                          /* mode */
      );

  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    pomodoro_props);
}

static void g_barbar_pomodoro_init(BarBarPomodoro *self) {}

static void g_barbar_pomodoro_constructed(GObject *object) {
  BarBarPomodoro *self = BARBAR_POMODORO(object);

  if (self->working) {
    if (self->current > self->length) {
      self->current = self->length;
    }
  } else {
    if (self->current > self->rest) {
      self->current = self->rest;
    }
  }
}

static void g_barbar_pomodoro_start(BarBarSensor *sensor) {}
