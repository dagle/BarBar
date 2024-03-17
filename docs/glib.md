Title: Glib
Slug: Glib

This is intended to be a small intro to glib and gtk. It's not intended to be a definitive guide but
more of a starting point/faq to questions getting asked a lot. If you want a deep guide into these
system I would recommend [gtk-docs](https://www.gtk.org/docs/) , [Gtk4-tutorial](https://github.com/ToshioCP/Gtk4-tutorial) and
[Gobject-tutorial](https://github.com/ToshioCP/Gobject-tutorial)

# What is glib/gobject/introspectable/gtk?
## Glib 
[Glib](https://docs.gtk.org/glib/) is the underlying system that use used by these system.
It contains common types, async programming, an eventloop and much more.

You can think of it like a standard library for glib programs (some parts tied to C, others are not).

## GObject
[GObject](https://docs.gtk.org/gobject/) is an object system. It's a way to create
new objects that can be used from multiple programming languages.
GObjects can be created from multiple languages and a gobject created in
one language can be interacted with from other.

This means that you can create new sensors, widgets or etc in the language of your choice and
interact with other widgets like they were written in your native language.

## introspectable
It's a way to say this code is written in a way that others can understand it. An introspectable api
will automatically let you generate bindings for multiple languages. What barbar does is it generates
a gir used for compiled languages and a typelib for interpreted languages.

Most languages can't generate gir and typelibs (but they can create objects and you could use something
like libpeas). For this you would need to use something C, rust or similar.

## gtk
Gtk is a gui library that use all of the technologies above. Barbar uses the gtk4 version of gtk, not only gkt4 but
version 4.14. This is because it includes a new path library used for creating widgets, instead of cairo.

## ui
[.ui](https://docs.gtk.org/gtk4/class.Builder.html) is a format used with the builder api of gtk. 
It's an xml format and lets us create an ui, with relationships between different elements,
making it possible to make a reactive ui. barbar abuses this format and defines sensors, 
not part of the spec. (but we do a hack to make it work)

## blueprint
Since xml isn't the most pleasant to read, I recommend using
[blueprint](https://jwestman.pages.gitlab.gnome.org/blueprint-compiler/). Blueprint is a
format that lets you build barbar ui (or any other gtk project) but with a nicer syntax, lsp support,
type checking and better highlighting (if the this is installed in your editor).
First install the blueprint compiler. Then write a config.blp, look in examples for a starting point.
When you are done with your config, you can compile it to a xml.

## css
A good starting for getting started with css in [gtk4-css](https://docs.gtk.org/gtk4/css-overview.html), 
the old [gtk3-css](https://docs.gtk.org/gtk3/css-overview.html) documentation might 
clearer and has more examples though not everything apply.
