Can we create a auto-manager, that works that configure a device and presents it self
as a device? This would be great for batteries, mpris etc since we could just read the device directly.

- [ ] Install icons

- Add https://wayland.app/protocols/xdg-output-unstable-v1 so we can restrict stuff
to one screen

- Figure out a way to make dynamic properties (or something like them) that work at runtime

-  [x] How to change a widget on click? To have 2 alternating widgets and on click we change.

- [-] Create helper functions for beings and such
-- [-] Create constructor functions for every type.

- [ ] value icon -> value image
-- should be a buildable that you add bounds to, with a similar C interface.

Finish up:
- [ ] Go over all memory allocation and make sure everything is freed correctly

- [ ] Hover and shit
-  [-] Tooltip

- [ ] rewrite it to a more modern version
- A lib to do system info remotely, to go with libgtop? LATER
Maybe: Fan/fan-speeds, UPS, entropy, hddtemp, hwmon, i2c, machine info, gpu (nvidia, amd, intel), location/timezone something?, users logged in

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

# documentation
- [x] Generate documentation for the gi stuff
- [x] Document functions and properties
- [-] Create tutorials (with documentation) for different languages: gjs, lua and python
- [-] Create README.md
- [ ] Create wiki?

# other
- [ ] Make ci stuff
