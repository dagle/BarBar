libbarbar_src = files(
  'barbar.c',
  'barbar-bar.c',
  'barbar-disk.c',
  'barbar-mpd.c',
  'barbar-battery.c',
  'barbar-clock.c',
  'barbar-cpu.c',
  'barbar-wireplumber.c',
  'barbar-mpris2.c',
  'barbar-river.c',
  'barbar-temperature.c',
  'tray/barbar-watcher.c',
)

libbarbar_headers = [
  'barbar.h',
  'barbar-bar.h',
  'barbar-disk.h',
  'barbar-mpd.h',
  'barbar-battery.h',
  'barbar-clock.h',
  'barbar-cpu.h',
  'barbar-wireplumber.h',
  'barbar-mpris2.h',
  'barbar-river.h',
  'barbar-temperature.h',
  'tray/barbar-watcher.h',
]

wl_mod = import('unstable-wayland')
river_status = wl_mod.scan_xml('../protocol/river-status-unstable-v1.xml')

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

executable('testbar',
  libbarbar_src + 'main.c', river_status, gdbus_src,
  dependencies: deps
)

libbarbar = library(barbar_package, libbarbar_src, river_status, gdbus_src,
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
    namespace: 'Barbar',
    nsversion: '1.0',
    symbol_prefix: ['g_barbar', 'barbar'],
    identifier_prefix : 'Barbar',
    includes: [ 'GObject-2.0', 'Gio-2.0'],
    dependencies: deps,
    extra_args: gir_args,
    fatal_warnings: true,
    install: true,
  )
endif

if get_option('vala')
  gnome.generate_vapi(barbar_package,
    sources: gmime_gir[0],
    packages: [ 'gio-2.0' ],
    install: true)
endif
