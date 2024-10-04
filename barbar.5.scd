barbar(5)

# NAME

barbar - A layered gtk4 shell for wayland

# DESCRIPTION

barbar is a gtk4 lib and a program for creating bars and shell functionallity for
your wayland compositor. It requires suppport for layered shell in your
compositor.

The barbar binary is just a small wrapper around the library that just loads
a ui file from $XDG_CONFIG_HOME/barbar/config.ui that is all.

If you want more functionallity than a config file could do,
you can extend it using a programming language. For examples on
how to do this. https://github.com/dagle/BarBar/tree/main/examples

To style your bar you use css, to customize edit
$XDG_CONFIG_HOME/barbar/style.css. For the default config
https://github.com/dagle/BarBar/blob/main/style.css

# AUTHORS

Per Odlund <per.odlund@gmail.com>
