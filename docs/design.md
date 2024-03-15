Title: design
Slug: design

This is the general design of the barbar library and binary. The barbar binary
should be easy to follow and the examples for different languages should implement
most (if not all) of what the binary does, but might be some what rudimentary in it's design.

# Widgets
There are in general 2 kinds of widgets in barbar. Raw widgets and sensor-less widgets.

## Raw widgets
Raw widgets does nothing on it's own. Some examples of this buttons, a graph or a meter.
To use these widgets you need to reference these. Either you bind a value from a sensor
or reference the sensor.

You can do this multiple ways, but what you would typically do:

--- code blueprint

// Here we define a bar to display how much we use of a sensor
Gtk.LevelBar {
  value: bind mem.percent; // we tell the bar where to get the data from
  orientation: vertical; // this is property
  inverted: true;
}

// and here we define a sensor
BarBar.Mem mem /* this mem is the name of the instance */{
  interval: 10000; // set the memory sensor to update every 10 second.
}

--- end code

This way, if you don't like how the data is displayed, you can change
to a different widget, since it's decoupled.

For other widgets, it takes a references to a sensor.

--- code blueprint

BarBar.MprisControl {
  child: mpris; // we take a reference to the child. 
}

BarBar.Mpris mpris {
}

--- end code

For some sensors it it makes to define multiple sensors of the same kind but with different
names and parameters.

## Sensor-less widgets
Sensor-less widgets are widgets that doesn't use a sensor and can't be controlled that way.
For some widgets it doesn't make sense to decouple the data from the widget.

An example of this is the river-tag 

--- code blueprint

// All you need to do is to add this bar and it should work
BarBar.RiverTag {
}

--- end code

# Sensors
Sensors is just an object, that is configured with parameters and that can start on it's own. Likely sensors will just be an Gio.Initable (or a subclass of that interface) in the future.
If you define your sensors in a .ui file, you need to hold a reference to the sensors. The barbar program does this by default, if you write your own, you need to do this, because gtk wont.
