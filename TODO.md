Can we create a auto-manager, that works that configure a device and presents it self
as a device? This would be great for batteries, mpris etc since we could just read the device directly.

Maybe: Fan/fan-speeds, UPS, entropy, hddtemp, hwmon, i2c, machine info, gpu (nvidia, amd, intel), location/timezone something?, users logged in

Bar-graph

# conky layout

- [ ] make a conky layout

# libgtop
- [ ] add device name or identifier to disks
- [ ] add support for darwin, freebsd, openbsd and bsd
-- https://man.openbsd.org/sysctl.2#HW_DISKSTATS
-- https://man.freebsd.org/cgi/man.cgi?query=devstat&apropos=0&sektion=9&manpath=FreeBSD+7.2-RELEASE&format=html
# Desktop file

- [x] Create an icon
- [x] Add installing the desktop file to meson


# Default css

- [ ] Default css
# A generic interface to a list in sensors?
# Mpris

- [x] create a format function
- [ ] be able to listen to multiple players
- [ ] send/create updates correctly

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

# Transition Layout Manager

- [ ] Investigate this

# widgets
## mediacontrol
- [ ] An initial widget
- [ ] Make the widget respect size.

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
