#include "glib.h"
#include "glibconfig.h"
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
 * g_barbar_print_percent:
 * @ptr: dummy pointer
 * @percent: value we want to format
 * @decimal: number of decimal places.
 * @sign: if we should add the "%" or not
 *
 * Returns: (transfer full): A string of the percent
 */
char *g_barbar_print_percent(gpointer *ptr, double percent, guint decimal,
                             gboolean sign) {
  percent = 100 * percent;
  if (sign) {
    return g_strdup_printf("%.*f%%", decimal, percent);
  } else {
    return g_strdup_printf("%.*f", decimal, percent);
  }
}

static void _g_barbar_print_autosized(GString *str, guint64 bytes,
                                      uint decimals) {
  double dbytes;
  if (bytes >= (1024 * 1024 * 1024)) {
    dbytes = bytes / (1024.0 * 1024 * 1024);
    g_string_append_printf(str, "%.*fGb", decimals, dbytes);
  } else if (bytes >= (1024 * 1024)) {
    dbytes = bytes / (1024.0 * 1024);
    g_string_append_printf(str, "%.*fMb", decimals, dbytes);
  } else if (bytes >= (1024)) {
    dbytes = bytes / (1024.0);
    g_string_append_printf(str, "%.*fkb", decimals, dbytes);
  } else {
    g_string_append_printf(str, "%lub", bytes);
  }
}

/**
 * g_barbar_print_autosized:
 * @bytes: the bytes we want to convert
 * @decimals: how many decimals we want
 *
 * Returns: (transfer full): A string with the bytes formated and unit
 */
char *g_barbar_print_autosized(guint64 bytes, uint decimals) {
  GString *str = g_string_new("");
  _g_barbar_print_autosized(str, bytes, decimals);
  return g_string_free_and_steal(str);
}

/**
 * g_barbar_printf:
 * @ptr: a dummy pointer
 * @format: how to format the string.
 * @...: arguments
 *
 * A version of printf usable in expressions.
 *
 * Returns: (transfer full): A new string.
 */
G_MODULE_EXPORT char *g_barbar_printf(gpointer *ptr, const char *format, ...) {
  gchar *buffer;
  va_list args;

  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);

  return buffer;
}

/**
 * g_barbar_print_bytes:
 * @ptr: a dummy pointer
 * @format: value we want to format
 * @...: variables we want to convert
 *
 * Converts and prints number of bytes.
 *
 * %k converts to kilobytes
 * %m converts to megabytes
 * %t converts to terabytes
 * %a does auto convertion, also includes the unit in the string
 *
 * By default it does binary convertion, so a kilo byte is 1024 bytes (aka a
 * kibibyte) If upper case is specified, we do a SI convertion (a kb is 1000
 * bytes).
 *
 * Returns: (transfer full): A formated string
 */
G_MODULE_EXPORT char *g_barbar_print_bytes(gpointer *ptr, const char *format,
                                           ...) {
  va_list args;
  va_start(args, format);

  GString *str = g_string_new("");

  while (*format != '\0') {
    long int decimals = 2;
    const char *begin = strchr(format, '%');
    if (begin == NULL) {
      g_string_append(str, format);
      break;
    }

    if (begin != format) {
      g_string_append_len(str, format, begin - format);
    }

    begin++; // Move past '%'
    if (*begin == '.') {
      char *endptr;

      begin++;
      decimals = strtol(begin, &endptr, 10);

      if (!endptr) {
        return g_string_free_and_steal(str);
      }
      begin = endptr;
    }

    switch (*begin) {
    case 'k': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / 1024.0;
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 'K': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / 1000.0;
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 'm': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / (1024.0 * 1024);
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 'M': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / (1000.0 * 1000);
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 'g': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / (1024.0 * 1024 * 1024);
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 'G': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / (1000.0 * 1000 * 1000);
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 't': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / (1024.0 * 1024 * 1024);
      d = d / 1024;
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 'T': {
      guint64 i = va_arg(args, guint64);
      double d;
      d = i / (1000.0 * 1000 * 1000);
      d = d / 1000;
      g_string_append_printf(str, "%.*f", (int)decimals, d);
      break;
    }
    case 'z': {
      break;
    }
    case 'a': {
      guint64 i = va_arg(args, guint64);
      _g_barbar_print_autosized(str, i, decimals);
      break;
    }
    case 'A': {
      const int unit;
      int i = va_arg(args, int);
      if (i >= 1000000000) {
      }
      break;
    }
    default:
      putchar('%');    // Print '%' character if no matching specifier found
      putchar(*begin); // Print the character following '%'
      break;
    }

    begin++;
    format = begin;
  }

  va_end(args);
  return g_string_free_and_steal(str);
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
GtkBuilder *g_barbar_default_blueprint(const char *path, GError *err) {
  // TODO:
  return NULL;
}

G_MODULE_EXPORT void g_barbar_custom_printf(const char *format, ...) {
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
