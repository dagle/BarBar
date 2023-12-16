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

static gboolean is_variant_number(GParamSpec *pspec) {
  if (G_IS_PARAM_SPEC_INT(pspec) || G_IS_PARAM_SPEC_UINT(pspec)) {
  }

  return FALSE;
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

    if (is_variant_number(param_spec)) {
      // print_number(str, value, param_spec, num_format, digits, decimals);
    } else {
      print_variant(str, value, sperator, length);
      //   const char *s = g_value_get_string(&value);
      //   g_string_append(str, s);
      // } else if (G_IS_PARAM_SPEC_BOOLEAN(param_spec)) {
      //   if (g_value_get_boolean(&value)) {
      //     g_string_append(str, "true");
      //   } else {
      //     g_string_append(str, "false");
      //   }
    }
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

GString *g_variant_print_string(GVariant *value, GString *string,
                                gboolean type_annotate) {
  const gchar *value_type_string = g_variant_get_type_string(value);

  if G_UNLIKELY (string == NULL)
    string = g_string_new(NULL);

  switch (value_type_string[0]) {
  case G_VARIANT_CLASS_MAYBE:
    if (type_annotate)
      g_string_append_printf(string, "@%s ", value_type_string);

    if (g_variant_n_children(value)) {
      const GVariantType *base_type;
      guint i, depth;
      GVariant *element = NULL;

      /* Nested maybes:
       *
       * Consider the case of the type "mmi".  In this case we could
       * write "just just 4", but "4" alone is totally unambiguous,
       * so we try to drop "just" where possible.
       *
       * We have to be careful not to always drop "just", though,
       * since "nothing" needs to be distinguishable from "just
       * nothing".  The case where we need to ensure we keep the
       * "just" is actually exactly the case where we have a nested
       * Nothing.
       *
       * Search for the nested Nothing, to save a lot of recursion if there
       * are multiple levels of maybes.
       */
      for (depth = 0, base_type = g_variant_get_type(value);
           g_variant_type_is_maybe(base_type);
           depth++, base_type = g_variant_type_element(base_type))
        ;

      element = g_variant_ref(value);
      for (i = 0; i < depth && element != NULL; i++) {
        GVariant *new_element = g_variant_n_children(element)
                                    ? g_variant_get_child_value(element, 0)
                                    : NULL;
        g_variant_unref(element);
        element = g_steal_pointer(&new_element);
      }

      if (element == NULL) {
        /* One of the maybes was Nothing, so print out the right number of
         * justs. */
        for (; i > 1; i--)
          g_string_append(string, "just ");
        g_string_append(string, "nothing");
      } else {
        /* There are no Nothings, so print out the child with no prefixes. */
        g_variant_print_string(element, string, FALSE);
      }

      g_clear_pointer(&element, g_variant_unref);
    } else
      g_string_append(string, "nothing");

    break;

  case G_VARIANT_CLASS_ARRAY:
    /* it's an array so the first character of the type string is 'a'
     *
     * if the first two characters are 'ay' then it's a bytestring.
     * under certain conditions we print those as strings.
     */
    if (value_type_string[1] == 'y') {
      const gchar *str;
      gsize size;
      gsize i;

      /* first determine if it is a byte string.
       * that's when there's a single nul character: at the end.
       */
      str = g_variant_get_data(value);
      size = g_variant_get_size(value);

      for (i = 0; i < size; i++)
        if (str[i] == '\0')
          break;

      /* first nul byte is the last byte -> it's a byte string. */
      if (i == size - 1) {
        gchar *escaped = g_strescape(str, NULL);

        /* use double quotes only if a ' is in the string */
        if (strchr(str, '\''))
          g_string_append_printf(string, "b\"%s\"", escaped);
        else
          g_string_append_printf(string, "b'%s'", escaped);

        g_free(escaped);
        break;
      }

      else {
        /* fall through and handle normally... */
      }
    }

    /*
     * if the first two characters are 'a{' then it's an array of
     * dictionary entries (ie: a dictionary) so we print that
     * differently.
     */
    // <property name="format">{apa:32}<property>
    // <property name="format">{apa@,:32}<property>
    if (value_type_string[1] == '{')
    /* dictionary */
    {
      const gchar *comma = "";
      gsize n, i;

      if ((n = g_variant_n_children(value)) == 0) {
        if (type_annotate)
          g_string_append_printf(string, "@%s ", value_type_string);
        g_string_append(string, "{}");
        break;
      }

      g_string_append_c(string, '{');
      for (i = 0; i < n; i++) {
        GVariant *entry, *key, *val;

        g_string_append(string, comma);
        comma = ", ";

        entry = g_variant_get_child_value(value, i);
        key = g_variant_get_child_value(entry, 0);
        val = g_variant_get_child_value(entry, 1);
        g_variant_unref(entry);

        g_variant_print_string(key, string, type_annotate);
        g_variant_unref(key);
        g_string_append(string, ": ");
        g_variant_print_string(val, string, type_annotate);
        g_variant_unref(val);
        type_annotate = FALSE;
      }
      g_string_append_c(string, '}');
    } else
    /* normal (non-dictionary) array */
    {
      const gchar *comma = "";
      gsize n, i;

      if ((n = g_variant_n_children(value)) == 0) {
        if (type_annotate)
          g_string_append_printf(string, "@%s ", value_type_string);
        g_string_append(string, "[]");
        break;
      }

      g_string_append_c(string, '[');
      for (i = 0; i < n; i++) {
        GVariant *element;

        g_string_append(string, comma);
        comma = ", ";

        element = g_variant_get_child_value(value, i);

        g_variant_print_string(element, string, type_annotate);
        g_variant_unref(element);
        type_annotate = FALSE;
      }
      g_string_append_c(string, ']');
    }

    break;

  case G_VARIANT_CLASS_TUPLE: {
    gsize n, i;

    n = g_variant_n_children(value);

    g_string_append_c(string, '(');
    for (i = 0; i < n; i++) {
      GVariant *element;

      element = g_variant_get_child_value(value, i);
      g_variant_print_string(element, string, type_annotate);
      g_string_append(string, ", ");
      g_variant_unref(element);
    }

    /* for >1 item:  remove final ", "
     * for 1 item:   remove final " ", but leave the ","
     * for 0 items:  there is only "(", so remove nothing
     */
    g_string_truncate(string, string->len - (n > 0) - (n > 1));
    g_string_append_c(string, ')');
  } break;

  case G_VARIANT_CLASS_DICT_ENTRY: {
    GVariant *element;

    g_string_append_c(string, '{');

    element = g_variant_get_child_value(value, 0);
    g_variant_print_string(element, string, type_annotate);
    g_variant_unref(element);

    g_string_append(string, ", ");

    element = g_variant_get_child_value(value, 1);
    g_variant_print_string(element, string, type_annotate);
    g_variant_unref(element);

    g_string_append_c(string, '}');
  } break;

  case G_VARIANT_CLASS_VARIANT: {
    GVariant *child = g_variant_get_variant(value);

    /* Always annotate types in nested variants, because they are
     * (by nature) of variable type.
     */
    g_string_append_c(string, '<');
    g_variant_print_string(child, string, TRUE);
    g_string_append_c(string, '>');

    g_variant_unref(child);
  } break;

  case G_VARIANT_CLASS_BOOLEAN:
    if (g_variant_get_boolean(value))
      g_string_append(string, "true");
    else
      g_string_append(string, "false");
    break;

  case G_VARIANT_CLASS_STRING: {
    const gchar *str = g_variant_get_string(value, NULL);
    gunichar quote = strchr(str, '\'') ? '"' : '\'';

    g_string_append_c(string, quote);

    while (*str) {
      gunichar c = g_utf8_get_char(str);

      if (c == quote || c == '\\')
        g_string_append_c(string, '\\');

      if (g_unichar_isprint(c))
        g_string_append_unichar(string, c);

      else {
        g_string_append_c(string, '\\');
        if (c < 0x10000)
          switch (c) {
          case '\a':
            g_string_append_c(string, 'a');
            break;

          case '\b':
            g_string_append_c(string, 'b');
            break;

          case '\f':
            g_string_append_c(string, 'f');
            break;

          case '\n':
            g_string_append_c(string, 'n');
            break;

          case '\r':
            g_string_append_c(string, 'r');
            break;

          case '\t':
            g_string_append_c(string, 't');
            break;

          case '\v':
            g_string_append_c(string, 'v');
            break;

          default:
            g_string_append_printf(string, "u%04x", c);
            break;
          }
        else
          g_string_append_printf(string, "U%08x", c);
      }

      str = g_utf8_next_char(str);
    }

    g_string_append_c(string, quote);
  } break;

  case G_VARIANT_CLASS_BYTE:
    if (type_annotate)
      g_string_append(string, "byte ");
    g_string_append_printf(string, "0x%02x", g_variant_get_byte(value));
    break;

  case G_VARIANT_CLASS_INT16:
    if (type_annotate)
      g_string_append(string, "int16 ");
    g_string_append_printf(string, "%" G_GINT16_FORMAT,
                           g_variant_get_int16(value));
    break;

  case G_VARIANT_CLASS_UINT16:
    if (type_annotate)
      g_string_append(string, "uint16 ");
    g_string_append_printf(string, "%" G_GUINT16_FORMAT,
                           g_variant_get_uint16(value));
    break;

  case G_VARIANT_CLASS_INT32:
    /* Never annotate this type because it is the default for numbers
     * (and this is a *pretty* printer)
     */
    g_string_append_printf(string, "%" G_GINT32_FORMAT,
                           g_variant_get_int32(value));
    break;

  case G_VARIANT_CLASS_HANDLE:
    if (type_annotate)
      g_string_append(string, "handle ");
    g_string_append_printf(string, "%" G_GINT32_FORMAT,
                           g_variant_get_handle(value));
    break;

  case G_VARIANT_CLASS_UINT32:
    if (type_annotate)
      g_string_append(string, "uint32 ");
    g_string_append_printf(string, "%" G_GUINT32_FORMAT,
                           g_variant_get_uint32(value));
    break;

  case G_VARIANT_CLASS_INT64:
    if (type_annotate)
      g_string_append(string, "int64 ");
    g_string_append_printf(string, "%" G_GINT64_FORMAT,
                           g_variant_get_int64(value));
    break;

  case G_VARIANT_CLASS_UINT64:
    if (type_annotate)
      g_string_append(string, "uint64 ");
    g_string_append_printf(string, "%" G_GUINT64_FORMAT,
                           g_variant_get_uint64(value));
    break;

  case G_VARIANT_CLASS_DOUBLE: {
    gchar buffer[100];
    gint i;

    g_ascii_dtostr(buffer, sizeof buffer, g_variant_get_double(value));

    for (i = 0; buffer[i]; i++)
      if (buffer[i] == '.' || buffer[i] == 'e' || buffer[i] == 'n' ||
          buffer[i] == 'N')
        break;

    /* if there is no '.' or 'e' in the float then add one */
    if (buffer[i] == '\0') {
      buffer[i++] = '.';
      buffer[i++] = '0';
      buffer[i++] = '\0';
    }

    g_string_append(string, buffer);
  } break;

  case G_VARIANT_CLASS_OBJECT_PATH:
    if (type_annotate)
      g_string_append(string, "objectpath ");
    g_string_append_printf(string, "\'%s\'", g_variant_get_string(value, NULL));
    break;

  case G_VARIANT_CLASS_SIGNATURE:
    if (type_annotate)
      g_string_append(string, "signature ");
    g_string_append_printf(string, "\'%s\'", g_variant_get_string(value, NULL));
    break;

  default:
    g_assert_not_reached();
  }

  return string;
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
