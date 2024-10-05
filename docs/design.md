Title: Design
Slug: Design

This document describes the general design of the barbar library and binary.
The barbar binary should be easy to follow, examples for different
languages should reimplement most (if not all) of what the barbar binary does.
But some examples might be some what rudimentary in their designs.

The barbar binary doesn't do much. All it does is loading the ui file, then
add all windows, start all sensors and keep track of them (so they don't get
freed to early).

The library is split into 2 parts, widgets and sensors.

# Widgets

There are in general 2 kinds of widgets in barbar. Raw widgets and sensor-less widgets.

## Raw widgets

Raw widgets does nothing on it's own. Some examples of this are buttons,
a graph or a meter. To use these widgets you need something that produces a
value. Either you bind a value from a sensor or reference the sensor.

You can do this multiple ways, but what you would typically do:

```blueprint
// Here we define a bar to display how much we use of a sensor
Gtk.LevelBar {
  // here we tell the bar where to get the data from, we want the percentage
  value: bind mem.percent; 
  orientation: vertical; // this is another property
  inverted: true;
}

// and now we define a sensor
// we need to name to the sensor to be able to reference it.
// here we create a Mem sensor with name mem.
BarBar.Mem mem {
  interval: 10000; // set the memory sensor to update every 10 second.
}
```

This way, if you don't like how the data is displayed, you can change
to a different widget, since it's decoupled.

For other widgets, it takes a references to a sensor.

```blueprint
# TODO: this is wrong
BarBar.MprisControl {
  child: mpris; // we take a reference to the child.
}

BarBar.Mpris mpris {
}
```

For some sensors it it makes to define multiple sensors of the same kind but
with different names and parameters.

## Sensor-less widgets

Sensor-less widgets are widgets that doesn't use a sensor and can't be
controlled that way. For some widgets it doesn't make sense to decouple
the data from the widget.

An example of this is the river-tag

```blueprint
// All you need to do is to add this bar and it should work
BarBar.RiverTag {
}
```

# Sensors

Sensors is just an object, that is configured with parameters and that can
start on it's own. Likely sensors will just be an Gio.Initable (or a
subclass of that interface) in the future. If you define your sensors in a
.ui file, you need to hold a reference to the sensors. The barbar program does
this by default, if you write your own, you need to do this, because gtk wont.
