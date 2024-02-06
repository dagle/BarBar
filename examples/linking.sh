#!/bin/sh

# The dynamic linker needs to load libgtk4-layer-shell.so before libwayland-client.so.
# There are multiple ways to do this. One (and the easiest) is to do the LD_PRELOAD hack.
# Another way is to do the preloading to your language. If you use the barbar binary,
# the linking is done correctly (or it's a bug).
# See: https://github.com/wmww/gtk4-layer-shell/blob/main/linking.md
# The location of libgtk4-layer-shell.so might depend on your distro
#
# Here is an example using javascipt
LD_PRELOAD=/usr/lib/libgtk4-layer-shell.so gjs -m ./bar.js
