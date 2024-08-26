#include <barbar-util.h>
#include <gtk4-layer-shell.h>

/**
 * g_barbar_get_parent_layer_window:
 * @widget: A pointer to a #GtkWidget
 *
 * Returns: (nullable) (transfer none): a refrence to the layered window
 */
GtkWindow *g_barbar_get_parent_layer_window(GtkWidget *widget) {
  g_return_val_if_fail(widget, NULL);

  GtkWidget *parent = widget;

  while ((parent = gtk_widget_get_parent(parent))) {
    if (GTK_IS_WINDOW(parent)) {
      GtkWindow *window = GTK_WINDOW(parent);
      if (gtk_layer_is_layer_window(window)) {
        return window;
      }
    }
  }

  return NULL;
}

/**
 * g_barbar_time_to_string:
 * @secs: The amount of seconds
 * @remaining: If we should print the time as negative
 * @force_hour: if should print hour even if it's 0
 *
 * Returns: (transfer full): A string for the time
 */
char *g_barbar_time_to_string(gint64 secs, gboolean remaining,
                              gboolean force_hour) {
  int sec, min, hour, _time;

  // _time = (int)(secs / G_USEC_PER_SEC);
  _time = secs;
  if (remaining)
    _time++;

  sec = _time % 60;
  _time = _time - sec;
  min = (_time % (60 * 60)) / 60;
  _time = _time - (min * 60);
  hour = _time / (60 * 60);

  if (hour > 0 || force_hour) {
    if (!remaining) {
      return g_strdup_printf("%d:%02d:%02d", hour, min, sec);
    } else {
      return g_strdup_printf("-%d:%02d:%02d", hour, min, sec);
    }
  }

  if (remaining) {
    return g_strdup_printf("-%d:%02d", min, sec);
  }
  return g_strdup_printf("%d:%02d", min, sec);
}

/**
 * g_barbar_default_style_provider:
 * @path: path relative in config to find config
 *
 * Load the css style in config
 *
 */
void g_barbar_default_style_provider(const char *path) {
  GtkCssProvider *css_provider;
  GdkDisplay *display;
  char *style_path;

  css_provider = gtk_css_provider_new();
  style_path = g_strdup_printf("%s/%s", g_get_user_config_dir(), path);
  gtk_css_provider_load_from_path(css_provider, style_path);
  g_free(style_path);

  display = gdk_display_get_default();

  gtk_style_context_add_provider_for_display(display,
                                             GTK_STYLE_PROVIDER(css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  g_object_unref(css_provider);
}

/**
 * g_barbar_default_builder:
 * @path: path relative in config to find config
 * @err: Return location for an error
 *
 * Load the builder in config
 *
 * Returns: (transfer full): returns a builder
 */
GtkBuilder *g_barbar_default_builder(const char *path, GError **err) {
  GtkBuilder *builder;
  char *ui_path;

  builder = gtk_builder_new();

  ui_path = g_strdup_printf("%s/%s", g_get_user_config_dir(), path);
  gtk_builder_add_from_file(builder, ui_path, err);
  g_free(ui_path);

  return builder;
}

/**
 * barbar_default_blueprint:
 * @err: Return location for an error
 *
 * Load the builder from config
 *
 * Returns: (transfer full): returns a builder
 */
GtkBuilder *barbar_default_blueprint(const char *path, GError *err) {
  // TODO:
  return NULL;
}

G_MODULE_EXPORT void custom_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  GString *str = g_string_new("");

  while (*format != '\0') {
    const char *begin = strchr(format, '%');
    if (begin == NULL) {
      g_string_append(str, format);
      break;
    }

    if (begin != format) {
      g_string_append_len(str, format, begin - format);
    }

    begin++; // Move past '%'
    switch (*begin) {
    case 'd':
      g_string_append_printf(str, "%d", va_arg(args, int));
      break;
    case 'f':
      g_string_append_printf(str, "%f", va_arg(args, double));
      break;
    case 's':
      g_string_append_printf(str, "%s", va_arg(args, char *));
      break;
    default:
      putchar('%');    // Print '%' character if no matching specifier found
      putchar(*begin); // Print the character following '%'
      break;
    }

    begin++;
    format = begin;
  }

  va_end(args);
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

// delete this?
GString *print_props(const gchar *format, GObject *object) {
  if (format == NULL) {
    return NULL;
  }

  GString *str = g_string_new("");

  while (TRUE) {
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
      num_format = FMT_DEFAULT;
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
      // print_variant(str, value, sperator, length);
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
  // printf("%s\n", str->str);
  return str;
}
