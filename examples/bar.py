from ctypes import CDLL
CDLL('libgtk4-layer-shell.so')

import gi
gi.require_version("Gtk", "4.0")
gi.require_version("BarBar", "1.0")

from gi.repository import Gtk
from gi.repository import BarBar
from gi.repository import GLib

Gtk.init()
BarBar.init()

manager = []
ui = GLib.get_user_config_dir() + '/barbar/config.ui'

def on_activate(app):
    builder = Gtk.Builder()

    builder.add_from_file(ui)

    list = builder.get_objects()

    for obj in list: 
        if isinstance(obj, Gtk.Window):
            app.add_window(obj)
            obj.present()
        elif isinstance(obj, BarBar.Sensor):
            obj.start()
            manager.append(obj)

app = Gtk.Application(application_id='com.github.barbar')
app.connect('activate', on_activate)
app.run(None)
