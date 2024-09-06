Can we create a auto-manager, that works that configure a device and presents it self
as a device? This would be great for batteries, mpris etc since we could just read the device directly.

Maybe: Fan/fan-speeds, UPS, entropy, hddtemp, hwmon, i2c, machine info, gpu (nvidia, amd, intel), location/timezone something?, users logged in

-  [x] How to change a widget on click? To have 2 alternating widgets and on click we change.

Finish up:
- [ ] Go over all memory allocation and make sure everything is freed correctly
- [x] All the percentage bullshit. Just make a print function. print_props should be
the base of that function.

- [-] Tray
-- [ ] Being able to display multiple trays by having a tray service
-- [ ] Make menus feel nicer

- [x] River-tags, make it buildable, so you can use widget

- [ ] Hover and shit
-  [-] Tooltip

- A lib to do system info remotely, to go with libgtop?

Bar-graph

# conky layout

- [-] make a conky layout, it's currently way to hard to make a conky setup

# cheat sheet example

# libgtop
- [ ] rewrite it to a more modern version

# Desktop file

- [x] Create an icon
- [x] Add installing the desktop file to meson


# Default css

- [ ] Default css
# A generic interface to a list in sensors?

# Mpris
- [-] Write your own playerctl that better fits your needs
- [-] be able to listen to multiple players
- [-] send/create updates correctly

# modules

- [x] game-mode
- [x] power-profiles
- [x] systemd-units

# keyboard
- [ ] layout
- [ ] name?
- [ ] specify keyboard if multiple
- [ ] update device dynamically

# weather / location
- [-] get it to work
- [-] document how to do it.
-- Use an agent

# Calendar
-- Create sensors or something to fetch the data to the calendar
- [ ] Neorg

# Inhibitor
- [ ] being able to specify a screen
- [ ] listen to events

# Transition Layout Manager

- [ ] Investigate this

## diagram

## rotary

## Select-value widget

## value bottom?

## bar-widget

# documentation
- [x] Generate documentation for the gi stuff
- [x] Document functions and properties
- [-] Create tutorials (with documentation) for different languages: gjs, lua and python
- [-] Create README.md
- [ ] Create wiki?

# other
- [ ] Make ci stuff
