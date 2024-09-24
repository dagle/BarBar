project(
  'barbar', 'c',
  version: '1.0',
  meson_version: '>= 1.0.0',
  license: 'LGPL-2.1-or-later'
  # default_options: [
  # ],
)

barbar_prefix = get_option('prefix')
barbar_datadir = join_paths(barbar_prefix, get_option('datadir'))
barbar_applicationsdir = join_paths(barbar_datadir, 'applications')
barbar_icondir = join_paths(barbar_datadir, 'icons/hicolor/scalable/apps/')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

barbar_package = 'barbar-1.0'
conf_data = configuration_data()

thread_dep = dependency('threads')
glib =  dependency('glib-2.0', version: '>=2.68')
gobject = dependency('gobject-2.0')
gio = dependency('gio-2.0')
giounix = dependency('gio-unix-2.0')
soup = dependency('libsoup-3.0')
json = dependency('json-glib-1.0')
udev = dependency('libudev')
module = dependency('gmodule-export-2.0')
upower_glib = dependency('upower-glib', required: get_option('upower_glib'))
libgtop = dependency('libgtop-2.0', required: get_option('libgtop'))
systray = dependency('statusnotifier-systray-gtk4')

libgweather = dependency('gweather4')
libgeoclue = dependency('libgeoclue-2.0')

libwireplumber = dependency('wireplumber-0.5', required: get_option('wireplumber'))
playerctl = dependency('playerctl', version : ['>=2.0.0'], required: get_option('mpris'))
wayland_client = dependency('wayland-client')
template_glib = dependency('template-glib-1.0')
# xkbcommon = dependency('xkbcommon')

gtk = dependency('gtk4')

gtk_layer_shell_dep = dependency('gtk4-layer-shell',
        required: true,
        fallback : ['gtk4-layer-shell', 'gtk_layer_shell'])

deps = [
  gtk_layer_shell_dep,
  glib,
  gobject,
  gio,
  gtk,
  wayland_client,
  thread_dep,
  giounix,
  udev,
  json,
  upower_glib,
  libgtop,
  systray,
  libwireplumber,
  playerctl,
  template_glib,
  libgweather,
  libgeoclue,
  soup,
  # xkbcommon,
  m_dep,
  module,
]

LIBS = [glib, gio, gobject, gtk]
wayland_protos = dependency('wayland-protocols')
subdir('src')

if get_option('docs')
  # subdir('docs')
endif

install_data('barbar.desktop', install_dir: barbar_applicationsdir)

install_data('barbar.svg', install_dir: barbar_icondir)


summary({'Install Prefix': get_option('prefix'),
        'Compiler': cc.get_id(),
        'Documentation': get_option('gtk_doc'),
        'Profiling enabled': false, #TODO
        'Coverage enabled': false, #TODO
        'GObject introspection': get_option('introspection'),
        'Vala bindings': get_option('vala'),
        }, section: 'Configuration')
