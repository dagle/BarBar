# BarBar the layered gtk4 shell.

Barbar is a gtk4 shell for wayland compositors using the layered-shell protocol. Actually
barbar isn't even that, it's more like a lib for writing a shell.


# Why is named barbar?
Barbar started off as a statusbar, partially inspired by things like waybar and later on ags.
But with the idea to keep things closer to glib/gtk and not to invent new things. Now
that it isn't a bar anymore, the name might change.

# How to configure barbar?
There are 2 main ways to configure barbar.
1. Write a program using barbar as lib.
2. Construct a ui file and place it in config directory.

The easiest is to do number 2. Barbar comes with an example program called (I bet you guessed it).
barbar. barbar looks for a config.ui and a style.css in the $XDG_CONFIG_HOME (defaulting to ~/.config/barbar/) directory and uses that to build an ui. barbar is actually around 100 lines of code and is very easy to replicate.

In the style.css you define your style. Look in example for a good starting point on how to get started.
The config.ui should be defined as 2 part xml file. One part where you setup sensors and one part where you construct widgets (and connect to the sensors). Then when a sensor updates a value, all widgets connected to that widget will be updated.

## Blueprint
Since xml isn't the easiest to read, I recommend using [blueprint](https://jwestman.pages.gitlab.gnome.org/blueprint-compiler/). Blueprint is a format that lets you configure barbar but with a nicer syntax, lsp support, type checking and better highlighting (if the this is installed in your editor). First install the blueprint compiler. Then write a config.blp, look in examples for a starting point.

``` bash
blueprint-compiler compile config.blp > ~/.config/barbar/config.ui
```

# Using barbar as a lib.
Barbar uses glib/gobjects and generates a introspectable api, with a typelib and a gir. This means
multiple languages can use barbar. For more info read [this](https://gi.readthedocs.io/en/latest/) and 
[supported languages](https://gi.readthedocs.io/en/latest/users.html)

Note: lua needs the git version of lgi to work properly.

gtk4 doesn't really support layered-shell, so we need to do a hack and load
libgtk4-layer-shell.so before gtk4. If you intend to use a compiled language,
hopefully the linker does this automatically for you.

The solution for dynamic language is to preload the libgtk4-layer-shell.so,
either by doing what examples/linking.sh does or doing in the programming language
of your choice. More info [here](https://github.com/wmww/gtk4-layer-shell/blob/main/linking.md).

In examples there should be a couple examples on how to get started with different
languages. If you have written an config in another language, I would like include
in thee examples. I would recommend gjs to getting started because it has the best
bindings for glib/gtk.

# Documentation
Most of the documentations is generated from the source
and uses gi-docgen.

For online [docs](TODO)

If you want completion in your editor (more incoming):
typescritp: [ts-for-gir](https://github.com/gjsify/ts-for-gir) , lua: [gir-to-stub](https://github.com/dagle/gir-to-stub)
