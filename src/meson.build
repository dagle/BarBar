libbarbar_src = files(
  'barbar.c',
  'barbar-bar.c',
  'barbar-util.c',
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
  # 'sensors/barbar-wireplumber.c',
  'sensors/barbar-mpris.c',


  'river/barbar-river-tags.c',
  'river/barbar-river-view.c',

  # 'river/barbar-river-layout.c',
  # 'river/barbar-river-mode.c',
  # 'sway/barbar-sway-ipc.c',
  # 'sway/barbar-sway-workspace.c',
  # 'sway/barbar-sway-window.c',
  'hyprland/barbar-hyprland-ipc.c',
  'hyprland/barbar-hyprland-workspace.c',
  # 'dwl/barbar-dwl-service.c',
  # 'dwl/barbar-dwl-tags.c',
  # 'barbar-temperature.c',
  # 'barbar-weather.c',
  # 'barbar-inhibitor.c',
  # 'tray/barbar-watcher.c',
)

libbarbar_headers = [
  'barbar.h',
  'barbar-bar.h',
  'barbar-util.h',
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
  # 'sensors/barbar-wireplumber.h',

  # 'sensors/barbar-wireplumber.h',
  'sensors/barbar-mpris.h',

  'river/barbar-river-tags.h',
  'river/barbar-river-view.h',

  # 'river/barbar-river-layout.h',
  # 'river/barbar-river-mode.h',
  # 'sway/barbar-sway-ipc.h',
  # 'sway/barbar-sway-workspace.h',
  # 'sway/barbar-sway-window.h',
  'hyprland/barbar-hyprland-ipc.h',
  'hyprland/barbar-hyprland-workspace.h',
  # 'dwl/barbar-dwl-service.h',
  # 'dwl/barbar-dwl-tags.h',
  # 'barbar-temperature.h',
  # 'barbar-weather.h',
  # 'barbar-inhibitor.h',
  # 'tray/barbar-watcher.h',
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

dbus_prots = [
  '../protocol/dbus-menu.xml',
  '../protocol/dbus-status-notifier-watcher.xml',
  '../protocol/dbus-status-notifier-item.xml',
]

gnome = import('gnome')

gdbus_src = gnome.gdbus_codegen('status-notifier',
  sources: dbus_prots,
  namespace : 'StatusNotifier',
)

libbarbar = library(barbar_package, libbarbar_src, wayland_src, gdbus_src,
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

  gmime_gir = gnome.generate_gir(
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
    sources: gmime_gir[0],
    packages: [ 'gio-2.0', 'gtk4'],
    install: true)
endif

executable('barbar',
  libbarbar_src + 'main.c', wayland_src, gdbus_src, 
  dependencies: deps,
  link_args: ['-Wl,--export-dynamic'],
  install: true
)

