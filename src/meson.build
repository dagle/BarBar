libbarbar_src = files(
  'barbar.c',
  'barbar-bar.c',
  'barbar-background.c',
  'barbar-util.c',
  'barbar-error.c',
  # 'barbar-disk.c',
  # 'barbar-mpd.c',

  # 'widgets/barbar-rotary.c',

  'sensors/barbar-battery.c',
  'sensors/barbar-sensor.c',
  'sensors/barbar-clock.c',
  'sensors/barbar-cpu.c',
  'sensors/barbar-mem.c',
  'sensors/barbar-network.c',
  'sensors/barbar-backlight.c',
  'sensors/barbar-cmd.c',
  'sensors/barbar-temperature.c',
  'sensors/barbar-mpris.c',
  'sensors/barbar-inhibitor.c',
  'sensors/barbar-sensorcontext.c',
  'sensors/barbar-wireplumber.c',
  'sensors/barbar-keyboard.c',
  'sensors/barbar-pomodoro.c',
  'sensors/barbar-power-profiles.c',
  'sensors/barbar-systemd-units.c',

  'river/barbar-river-layout.c',
  'river/barbar-river-tags.c',
  'river/barbar-river-view.c',
  'river/barbar-river-mode.c',

  'widgets/barbar-label.c',
  'widgets/barbar-sensorwidget.c',
  'widgets/barbar-rotary.c',
  'widgets/barbar-graph.c',

  # 'river/barbar-river-mode.c',
  # 'sway/barbar-sway-ipc.c',
  # 'sway/barbar-sway-workspace.c',
  # 'sway/barbar-sway-window.c',
  'hyprland/barbar-hyprland-ipc.c',
  'hyprland/barbar-hyprland-workspace.c',
  # 'dwl/barbar-dwl-service.c',
  # 'dwl/barbar-dwl-tags.c',
  'sensors/barbar-weather.c',

  'tray/barbar-watcher.c',
  'tray/barbar-tray-item.c',
  'tray/barbar-dbusmenu.c',
)

libbarbar_headers = [
  'barbar.h',
  'barbar-bar.h',
  'barbar-background.h',
  'barbar-util.h',
  'barbar-error.h',
  # 'barbar-disk.h',
  # 'barbar-mpd.h',
  #
  # 'widgets/barbar-rotary.c',

  'sensors/barbar-battery.h',
  'sensors/barbar-sensor.h',
  'sensors/barbar-clock.h',
  'sensors/barbar-cpu.h',
  'sensors/barbar-mem.h',
  'sensors/barbar-network.h',
  'sensors/barbar-backlight.h',
  'sensors/barbar-cmd.h',
  'sensors/barbar-temperature.h',
  'sensors/barbar-inhibitor.h',
  'sensors/barbar-sensorcontext.h',
  'sensors/barbar-wireplumber.h',
  'sensors/barbar-keyboard.h',
  'sensors/barbar-pomodoro.h',
  'sensors/barbar-power-profiles.h',
  'sensors/barbar-systemd-units.h',

  'sensors/barbar-mpris.h',

  'river/barbar-river-layout.h',
  'river/barbar-river-tags.h',
  'river/barbar-river-view.h',
  'river/barbar-river-mode.h',

  'widgets/barbar-label.h',
  'widgets/barbar-sensorwidget.h',
  'widgets/barbar-rotary.h',
  'widgets/barbar-graph.h',
  # 'river/barbar-river-mode.h',
  # 'sway/barbar-sway-ipc.h',
  # 'sway/barbar-sway-workspace.h',
  # 'sway/barbar-sway-window.h',
  # 'hyprland/barbar-hyprland-ipc.h',
  'hyprland/barbar-hyprland-workspace.h',
  # 'dwl/barbar-dwl-service.h',
  # 'dwl/barbar-dwl-tags.h',
  'sensors/barbar-weather.h',
  # 'barbar-inhibitor.h',

  'tray/barbar-watcher.h',
  # 'tray/barbar-tray-item.h',
  # 'tray/barbar-dbusmenu.h',
]

wl_mod = import('unstable-wayland')

wl_protocol_dir = wayland_protos.get_variable('pkgdatadir')

wayland_prots = [
	wl_protocol_dir / 'unstable/idle-inhibit/idle-inhibit-unstable-v1.xml',
  '../protocol/river-status-unstable-v1.xml',
  '../protocol/river-control-unstable-v1.xml',
]

wayland_src = []

foreach w : wayland_prots
  wayland_src += wl_mod.scan_xml(w)
endforeach

power_profile_names = {
  'org.freedesktop.UPower.PowerProfiles': '/org/freedesktop/UPower/PowerProfiles',
  # 'net.hadess.PowerProfiles': '/net/hadess/PowerProfiles',
}

foreach name, path: power_profile_names
  config = {
    'dbus_name': name,
    'dbus_iface': name,
    'dbus_path': path,
  }

  power_profile_xml = configure_file(
    input: '../protocol/power-profiles-daemon.dbus.xml.in',
    output: name + '.xml',
    configuration: config,
  )
endforeach

status_notifier_prots = [
  '../protocol/dbus-menu.xml',
  '../protocol/dbus-status-notifier-watcher.xml',
  '../protocol/dbus-status-notifier-item.xml',
]

gnome = import('gnome')

status_notifier_src = gnome.gdbus_codegen('status-notifier',
  sources: status_notifier_prots,
  namespace : 'StatusNotifier',
)

power_profiles_src = gnome.gdbus_codegen('power-profiles-daemon',
  sources: power_profile_xml,
  namespace: 'PowerProfiles',
  interface_prefix: 'org.freedesktop.UPower',
)

# content_files += gnome.gdbus_codegen(
#   meson.project_name(),
#   sources: dbus_xml['org.freedesktop.UPower.PowerProfiles'],
#   interface_prefix: 'org.freedesktop.UPower',
#   namespace: 'PowerProfiles',
#   docbook: 'docs',
#   build_by_default: true,
# )

libbarbar = library(barbar_package, libbarbar_src, wayland_src, status_notifier_src, power_profiles_src,
  dependencies : deps, install: true)

pkg = import('pkgconfig')

pkg.generate(libbarbar,
  name: meson.project_name(),
  filebase: barbar_package,
  subdirs: barbar_package,
  description: 'A bar library for writing status bars',
  requires: LIBS,
  )

if get_option('introspection')
  gir_args = [
    '--accept-unprefixed',
    ]

  barbar_gir = gnome.generate_gir(
    libbarbar,
    sources: libbarbar_src + libbarbar_headers,
    export_packages: [barbar_package],
    header: 'barbar/barbar.h',
    namespace: 'BarBar',
    nsversion: '1.0',
    symbol_prefix: ['g_barbar', 'barbar'],
    identifier_prefix : ['BarBar'],
    includes: [ 'GObject-2.0', 'Gio-2.0', 'Gtk-4.0', 'Playerctl-2.0'],
    dependencies: deps,
    extra_args: gir_args,
    fatal_warnings: true,
    install: true,
  )
endif

if get_option('vala')
  gnome.generate_vapi(barbar_package,
    sources: barbar_gir[0],
    packages: [ 'gio-2.0', 'gtk4'],
    install: true)
endif

executable('barbar',
  libbarbar_src + 'main.c', wayland_src, status_notifier_src, power_profiles_src,
  dependencies: deps,
  link_args: ['-Wl,--export-dynamic'],
  install: true
)

