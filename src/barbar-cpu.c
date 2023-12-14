#include "barbar-cpu.h"
#include "barbar-bar.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <math.h>
#include <stdio.h>

struct _BarBarCpu {
  GtkWidget parent_instance;

  GtkWidget *label;

  double prev_total;
  double prev_idle;

  guint interval;

  guint source_id;

  // An array for doubles;
  GArray *states;
};

enum {
  PROP_0,

  PROP_INTERVAL,

  NUM_PROPERTIES,
};

// update every 10 sec
#define DEFAULT_INTERVAL 10000

static GParamSpec *cpu_props[NUM_PROPERTIES] = {
    NULL,
};

G_DEFINE_TYPE(BarBarCpu, g_barbar_cpu, GTK_TYPE_WIDGET)

static void g_barbar_cpu_constructed(GObject *object);

static void g_barbar_cpu_set_interval(BarBarCpu *cpu, guint inteval) {
  g_return_if_fail(BARBAR_IS_CPU(cpu));

  cpu->interval = inteval;

  g_object_notify_by_pspec(G_OBJECT(clock), cpu_props[inteval]);
}

static void g_barbar_cpu_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {

  BarBarCpu *cpu = BARBAR_CPU(object);

  switch (property_id) {
  case PROP_INTERVAL:
    g_barbar_cpu_set_interval(cpu, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_cpu_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  BarBarCpu *cpu = BARBAR_CPU(object);

  switch (property_id) {
  case PROP_INTERVAL:
    g_value_set_uint(value, cpu->interval);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

/**
 * g_barbar_cpu_set_states:
 * @cpu: a #BarBarCpu
 * @states: (array zero-terminated=1): an array of doubles, terminated by a
 *%NULL element
 *
 * Set the the states for the cpu.
 *
 * The states are used to format different levels of load differently
 *
 **/
void g_barbar_cpu_set_states(BarBarCpu *cpu, const double states[]) {
  GArray *garray = g_array_new(FALSE, FALSE, sizeof(int));

  g_array_free(cpu->states, TRUE);

  for (; states; states++) {
    g_array_append_val(garray, states);
  }
  cpu->states = garray;
}

static void g_barbar_cpu_class_init(BarBarCpuClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_cpu_set_property;
  gobject_class->get_property = g_barbar_cpu_get_property;
  gobject_class->constructed = g_barbar_cpu_constructed;
  cpu_props[PROP_INTERVAL] =
      g_param_spec_uint("interval", "Interval", "Interval in milli seconds", 0,
                        G_MAXUINT32, DEFAULT_INTERVAL, G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, cpu_props);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}

void g_barbar_cpu_start(BarBarCpu *cpu, gpointer data);
static void g_barbar_cpu_init(BarBarCpu *self) {
  // BarBarCpu *cpu = BARBAR_CPU(self);

  // g_barbar_cpu_set_interval();
  self->interval = DEFAULT_INTERVAL;

  self->label = gtk_label_new("");
  gtk_widget_set_parent(self->label, GTK_WIDGET(self));

  g_signal_connect(self, "map", G_CALLBACK(g_barbar_cpu_start), NULL);
}

static gboolean is_number(const char *str, int *i) {

  char *endptr;
  long result = strtol(str, &endptr, 10);

  if (*endptr != '\0') {
    return FALSE;
  }

  *i = result;
  return TRUE;
}

GString *print_props(const gchar *format, GObject *object) {
  if (format == NULL) {
    return NULL;
  }

  GString *str = g_string_new("");

  while (true) {
    const char *begin = strchr(format, '{');
    if (begin == NULL) {
      g_string_append(str, format);
      break;
    }

    if (begin != format) {
      g_string_append_len(str, format, begin - format);
    }

    const char *end = strchr(begin, '}');

    if (end == NULL) {
      /* Wasn't actually a tag, copy as-is instead */
      g_string_append_len(str, format, begin - format + 1);
      format = begin + 1;
      continue;
    }

    char property_name_and_arg[end - begin];
    strncpy(property_name_and_arg, begin + 1, end - begin - 1);
    property_name_and_arg[end - begin - 1] = '\0';
    const char *property_name = NULL;
    const char *property_arg = NULL;

    {
      char *saveptr;
      property_name = strtok_r(property_name_and_arg, ":", &saveptr);

      property_arg = strtok_r(NULL, ":", &saveptr);
    }
    enum {
      FMT_DEFAULT,
      FMT_HEX,
      FMT_OCT,
      FMT_PERCENT,
      FMT_KBYTE,
      FMT_MBYTE,
      FMT_GBYTE,
      FMT_KIBYTE,
      FMT_MIBYTE,
      FMT_GIBYTE,
    } num_format = FMT_DEFAULT;

    int digits = 0;
    int decimals = 2;
    gboolean zero_pad;
    char *point = NULL;

    if (property_arg == NULL) {
    } else if (strcmp(property_arg, "hex") == 0) {
      num_format = FMT_HEX;
    } else if (strcmp(property_arg, "oct") == 0) {
      num_format = FMT_OCT;
    } else if (strcmp(property_arg, "%") == 0) {
      num_format = FMT_PERCENT;
    } else if (strcmp(property_arg, "kb") == 0) {
      num_format = FMT_KBYTE;
    } else if (strcmp(property_arg, "mb") == 0) {
      num_format = FMT_MBYTE;
    } else if (strcmp(property_arg, "gb") == 0) {
      num_format = FMT_GBYTE;
    } else if (strcmp(property_arg, "kib") == 0) {
      num_format = FMT_KIBYTE;
    } else if (strcmp(property_arg, "mib") == 0) {
      num_format = FMT_MIBYTE;
    } else if (strcmp(property_arg, "gib") == 0) {
      num_format = FMT_GIBYTE;
    } else if (is_number(property_arg, &digits)) {
      zero_pad = property_arg[0] == '0';
    } // i.e.: "{tag:03}"
    else if ((point = strchr(property_arg, '.')) != NULL) {
      *point = '\0';

      const char *digits_str = property_arg;
      const char *decimals_str = point + 1;

      if (digits_str[0] != '\0') { // guards against i.e. "{tag:.3}"
        if (!is_number(digits_str, &digits)) {
          // LOG_WARN("tag `%s`: invalid field width formatter. Ignoring...",
          //          tag_name);
        }
      }

      if (decimals_str[0] != '\0') { // guards against i.e. "{tag:3.}"
        if (!is_number(decimals_str, &decimals)) {
          // LOG_WARN("tag `%s`: invalid decimals formatter. Ignoring...",
          //          tag_name);
        }
      }
      zero_pad = digits_str[0] == '0';
    } else {
      // LOG_WARN("invalid tag formatter: %s", tag_args[i]);
    }

    GParamSpec *param_spec =
        g_object_class_find_property(G_OBJECT_GET_CLASS(object), property_name);
    // GType type = G_PARAM_SPEC_VALUE_TYPE(param_spec);

    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(param_spec));
    g_object_get_property(object, property_name, &value);
    // g_object_get_property(object, property_name, &value);
    switch (num_format) {
    case FMT_DEFAULT: {
      if (param_spec) {
        if (G_IS_PARAM_SPEC_CHAR(param_spec)) {
        } else if (G_IS_PARAM_SPEC_BOOLEAN(param_spec)) {
        } else if (G_IS_PARAM_SPEC_LONG(param_spec)) {
          // G_IS_PARAM_SPEC_LONG
        } else if (G_IS_PARAM_SPEC_DOUBLE(param_spec)) {
          const char *fmt = zero_pad ? "%0*.*f" : "%*.*f";

          guint num = g_value_get_double(&value);
          g_string_append_printf(str, fmt, digits, decimals, num);
        } else if (G_IS_PARAM_SPEC_UINT(param_spec)) {
          const char *fmt = zero_pad ? "%0*ld" : "%*ld";
          guint num = g_value_get_uint(&value);
          g_string_append_printf(str, fmt, digits, num);

          // gint default_value =
          //     g_value_get_int(g_param_spec_get_default_value(param_spec));
          // guint min_value = G_PARAM_SPEC_UINT(param_spec)->minimum;
          // guint max_value = G_PARAM_SPEC_UINT(param_spec)->maximum;
        } else if (G_IS_PARAM_SPEC_STRING(param_spec)) {
          const char *s = g_value_get_string(&value);
          g_string_append(str, s);
        }
      }
      break;
    }
    case FMT_HEX:
    case FMT_OCT: {
      const char *fmt = num_format == FMT_HEX ? zero_pad ? "%0*lx" : "%*lx"
                        : zero_pad            ? "%0*lo"
                                              : "%*lo";
      guint num = g_value_get_uint(&value);
      g_string_append_printf(str, fmt, digits, num);
      break;
    }
    case FMT_PERCENT: {
      guint num = g_value_get_uint(&value);
      guint min_value = G_PARAM_SPEC_UINT(param_spec)->minimum;
      guint max_value = G_PARAM_SPEC_UINT(param_spec)->maximum;

      const char *fmt = zero_pad ? "%0*lu" : "%*lu";
      g_string_append_printf(str, fmt, digits,
                             (num - min_value) * 100 / (max_value - min_value));
      break;
    }
    case FMT_KBYTE:
    case FMT_MBYTE:
    case FMT_GBYTE:
    case FMT_KIBYTE:
    case FMT_MIBYTE:
    case FMT_GIBYTE: {
      long div = num_format == FMT_KBYTE    ? 1000
                 : num_format == FMT_MBYTE  ? 1000 * 1000
                 : num_format == FMT_GBYTE  ? 1000 * 1000 * 1000
                 : num_format == FMT_KIBYTE ? 1024
                 : num_format == FMT_MIBYTE ? 1024 * 1024
                 : num_format == FMT_GIBYTE ? 1024 * 1024 * 1024
                                            : 1;
      if (G_IS_PARAM_SPEC_DOUBLE(param_spec)) {
        const char *fmt = zero_pad ? "%0*.*f" : "%*.*f";
        double num = g_value_get_double(&value);

        g_string_append_printf(str, fmt, digits, decimals, num / (double)div);
      } else if (G_IS_PARAM_SPEC_UINT(param_spec)) {
        const char *fmt = zero_pad ? "%0*lu" : "%*lu";
        guint num = g_value_get_uint(&value);

        g_string_append_printf(str, fmt, digits, num / div);
      }
      break;
    }
    }
    format = end + 1;
  }
  printf("%s\n", str->str);
  return str;
}

static void g_barbar_cpu_constructed(GObject *object) {}

static gboolean g_barbar_cpu_update(gpointer data) {
  BarBarCpu *self = BARBAR_CPU(data);
  // double load[1];
  //
  print_props("bepa l{interval}", G_OBJECT(self));

  double total, idle, percent;

  glibtop_cpu cpu;

  glibtop_init();

  glibtop_get_cpu(&cpu);

  total = ((unsigned long)cpu.total) ? ((double)cpu.total) : 1.0;
  idle = ((unsigned long)cpu.idle) ? ((double)cpu.idle) : 1.0;

  percent =
      100.0 * (1.0 - (idle - self->prev_idle) / (total - self->prev_total));

  gchar *str = g_strdup_printf("%.0f%%", percent);
  gtk_label_set_label(GTK_LABEL(self->label), str);

  g_free(str);
  // printf("load: %f\n", percent);
  self->prev_idle = idle;
  self->prev_total = total;

  return G_SOURCE_CONTINUE;
}

void g_barbar_cpu_start(BarBarCpu *cpu, gpointer data) {
  if (cpu->source_id > 0) {
    g_source_remove(cpu->source_id);
  }
  g_barbar_cpu_update(cpu);
  g_timeout_add_full(0, cpu->interval, g_barbar_cpu_update, cpu, NULL);
}
