wl_mod = import('unstable-wayland')
gnome = import('gnome')
pkg = import('pkgconfig')

libbarbar_src = [
  'barbar.c',
  'barbar-bar.c',
  'barbar-background.c',
  'barbar-util.c',
  'barbar-timespan.c',
  'barbar-error.c',
  'barbar-enum.c',

  'sensors/barbar-sensor.c',
  'sensors/barbar-interval-sensor.c',
  'sensors/barbar-battery.c',
  'sensors/barbar-clock.c',
  'sensors/barbar-cpu.c',
  'sensors/barbar-mem.c',
  'sensors/barbar-swap.c',
  'sensors/barbar-network.c',
  'sensors/barbar-backlight.c',
  'sensors/barbar-cmd.c',
  'sensors/barbar-temperature.c',
  'sensors/barbar-inhibitor.c',
  'sensors/barbar-sensorcontext.c',
  'sensors/barbar-wireplumber.c',
  'sensors/barbar-keyboard.c',
  'sensors/barbar-pomodoro.c',
  'sensors/barbar-power-profiles.c',
  'sensors/barbar-systemd-units.c',
  'sensors/barbar-gamemode.c',
  'sensors/barbar-weather.c',
  'sensors/barbar-filesystem.c',
  'sensors/barbar-uptime.c',
  'sensors/barbar-disk.c',

  # 'sensors/barbar-mpris.c',
  'sensors/mpris/barbar-mpris-player.c',

  'river/barbar-river-layout.c',
  'river/barbar-river-tags.c',
  'river/barbar-river-view.c',
  'river/barbar-river-mode.c',

  'sway/barbar-sway-ipc.c',
  'sway/barbar-sway-subscribe.c',
  'sway/barbar-sway-workspace.c',
  'sway/barbar-sway-language.c',
  'sway/barbar-sway-window.c',
  'sway/barbar-sway-mode.c',

  'niri/barbar-niri-ipc.c',
  'niri/barbar-niri-subscribe.c',
  'niri/barbar-niri-language.c',
  'niri/barbar-niri-workspace.c',
  'niri/barbar-niri-window.c',

  'hyprland/barbar-hyprland-ipc.c',
  'hyprland/barbar-hyprland-service.c',
  'hyprland/barbar-hyprland-workspace.c',
  'hyprland/barbar-hyprland-window.c',

  'widgets/barbar-label.c',
  'widgets/barbar-sensorwidget.c',
  'widgets/barbar-rotary.c',
  'widgets/barbar-graph.c',
  'widgets/barbar-valueicon.c',
  'widgets/barbar-processes.c',
  'widgets/barbar-activity-graph.c',
  'widgets/barbar-box.c',
  'widgets/barbar-eventswitcher.c',
  'widgets/barbar-playbutton.c',

  'widgets/barbar-github-activity.c',
]

libbarbar_headers = [
  'barbar.h',
  'barbar-bar.h',
  'barbar-background.h',
  'barbar-util.h',
  'barbar-timespan.h',
  'barbar-error.h',
  'barbar-enum.h',

  'sensors/barbar-sensor.h',
  'sensors/barbar-interval-sensor.h',
  'sensors/barbar-battery.h',
  'sensors/barbar-clock.h',
  'sensors/barbar-cpu.h',
  'sensors/barbar-mem.h',
  'sensors/barbar-swap.h',
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
  'sensors/barbar-gamemode.h',
  'sensors/barbar-weather.h',
  'sensors/barbar-filesystem.h',
  'sensors/barbar-uptime.h',
  'sensors/barbar-disk.h',

  # 'sensors/barbar-mpris.h',
  'sensors/mpris/barbar-mpris-player.h',

  'river/barbar-river-layout.h',
  'river/barbar-river-tags.h',
  'river/barbar-river-view.h',
  'river/barbar-river-mode.h',

  'widgets/barbar-label.h',
  'widgets/barbar-sensorwidget.h',
  'widgets/barbar-rotary.h',
  'widgets/barbar-graph.h',
  'widgets/barbar-processes.h',
  'widgets/barbar-activity-graph.h',
  'widgets/barbar-box.h',
  'widgets/barbar-valueicon.h',
  'widgets/barbar-eventswitcher.h',
  'widgets/barbar-playbutton.h',

  'widgets/barbar-github-activity.h',

  'hyprland/barbar-hyprland-ipc.h',
  'hyprland/barbar-hyprland-service.h',
  'hyprland/barbar-hyprland-workspace.h',
  'hyprland/barbar-hyprland-window.h',

  'sway/barbar-sway-ipc.h',
  'sway/barbar-sway-subscribe.h',
  'sway/barbar-sway-workspace.h',
  'sway/barbar-sway-language.h',
  'sway/barbar-sway-window.h',
  'sway/barbar-sway-mode.h',

  'niri/barbar-niri-ipc.h',
  'niri/barbar-niri-subscribe.h',
  'niri/barbar-niri-language.h',
  'niri/barbar-niri-workspace.h',
  'niri/barbar-niri-window.h',

]

conf = configuration_data()
conf.set('version', '@0@'.format(meson.project_version()))
conf.set('LOCALEDIR', get_option('prefix') / get_option('localedir'))
conf.set('PKGDATADIR', pkgdatadir)
if systray.found()
  conf.set('SYSTRAY', true)
endif

config_rs = configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: conf
)
wl_protocol_dir = wayland_protos.get_variable('pkgdatadir')

wayland_src = []

wayland_prots = [
	wl_protocol_dir / 'unstable/idle-inhibit/idle-inhibit-unstable-v1.xml',
	wl_protocol_dir / 'unstable/xdg-output/xdg-output-unstable-v1.xml',
  '../protocol/river-status-unstable-v1.xml',
  '../protocol/river-control-unstable-v1.xml',
]

foreach w : wayland_prots
  wayland_src += wl_mod.scan_xml(w)
endforeach

dwl_opt = get_option('dwl-method')

if dwl_opt in ['both', 'no-ipc']
  dwl_no_ipc_c = [
    'dwl/no-ipc/barbar-dwl-service.c',
    'dwl/no-ipc/barbar-dwl-tags.c',
    'dwl/no-ipc/barbar-dwl-layout.c',
    'dwl/no-ipc/barbar-dwl-title.c',
  ]

  libbarbar_src += dwl_no_ipc_c

  dwl_no_ipc_h = [
    'dwl/no-ipc/barbar-dwl-service.h',
    'dwl/no-ipc/barbar-dwl-tags.h',
    'dwl/no-ipc/barbar-dwl-layout.h',
    'dwl/no-ipc/barbar-dwl-title.h',
  ]

  libbarbar_headers += dwl_no_ipc_h
endif

if dwl_opt in ['both', 'ipc']
dwl_ipc_c = [
  'dwl/ipc/barbar-dwl-tags-ipc.c',
  'dwl/ipc/barbar-dwl-layout-ipc.c',
  'dwl/ipc/barbar-dwl-title-ipc.c',
]
  libbarbar_src += dwl_ipc_c

dwl_ipc_h = [
  'dwl/ipc/barbar-dwl-tags-ipc.h',
  'dwl/ipc/barbar-dwl-layout-ipc.h',
  'dwl/ipc/barbar-dwl-title-ipc.h',
]
  libbarbar_headers += dwl_ipc_h
  wayland_src += wl_mod.scan_xml('../protocol/dwl-ipc-unstable-v2.xml')
endif


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

mpris_prots = ['../protocol/mpris-dbus-interface.xml']


status_notifier_src = gnome.gdbus_codegen('status-notifier',
  sources: status_notifier_prots,
  namespace : 'StatusNotifier',
)

power_profiles_src = gnome.gdbus_codegen('power-profiles-daemon',
  sources: power_profile_xml,
  namespace: 'PowerProfiles',
  interface_prefix: 'org.freedesktop.UPower',
)

mpris_src = gnome.gdbus_codegen('mpris',
  sources: mpris_prots,
  namespace: 'Mpris',
)

libbarbar = library(barbar_package, libbarbar_src, wayland_src,
  status_notifier_src, power_profiles_src, mpris_src,
  dependencies : deps, install: true)

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
    includes: [ 'GObject-2.0', 'Gio-2.0', 'Gtk-4.0', 'Gtk4LayerShell-1.0', 'Json-1.0' ],
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
  'main.c',
  dependencies: deps + bin_deps,
  link_with: libbarbar,
  link_args: ['-Wl,--export-dynamic'],
  install: true
)

