project(
  'barbar', 'c',
  version: '1.0',
  license: 'MIT',
  meson_version: '>= 1.0.0',
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
json = dependency('json-glib-1.0')
udev = dependency('libudev')
module = dependency('gmodule-export-2.0')
upower_glib = dependency('upower-glib', required: get_option('upower_glib'))
libgtop = dependency('libgtop-2.0', required: get_option('libgtop'))

libgweather = dependency('gweather4')
libgeoclue = dependency('libgeoclue-2.0')
# geocode = dependency('geocode-glib-2.0')

libwireplumber = dependency('wireplumber-0.5', required: get_option('wireplumber'))
playerctl = dependency('playerctl', version : ['>=2.0.0'], required: get_option('mpris'))
wayland_client = dependency('wayland-client')
template_glib = dependency('template-glib-1.0')

gtk = dependency('gtk4')
# gtk = dependency('gtk4', version : '>=4.14.0')

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
  # libmpdclient,
  giounix,
  udev,
  json,
  upower_glib,
  libgtop,
  libwireplumber,
  playerctl,
  template_glib,
  libgweather,
  libgeoclue,
  # geocode,
  m_dep,
  module,
]

LIBS = [glib, gio, gobject]
# wayland_client = dependency('wayland-client')
# wayland_cursor = dependency('wayland-cursor')
wayland_protos = dependency('wayland-protocols')
# gtkmm = dependency('gtkmm-3.0', version : ['>=3.22.0'])
# dbusmenu_gtk = dependency('dbusmenu-gtk3-0.4', required: get_option('dbusmenu-gtk'))
# giounix = dependency('gio-unix-2.0', required: (get_option('dbusmenu-gtk').enabled() or
                                                # get_option('logind').enabled() or
                                                # get_option('upower_glib').enabled() or
                                                # get_option('mpris').enabled()))
# jsoncpp = dependency('jsoncpp', version : ['>=1.9.2'], fallback : ['jsoncpp', 'jsoncpp_dep'])
# sigcpp = dependency('sigc++-2.0')
# libinotify = dependency('libinotify', required: false)
# libepoll = dependency('epoll-shim', required: false)
# libinput = dependency('libinput', required: get_option('libinput'))
# libnl = dependency('libnl-3.0', required: get_option('libnl'))
# libnlgen = dependency('libnl-genl-3.0', required: get_option('libnl'))
# upower_glib = dependency('upower-glib', required: get_option('upower_glib'))
# playerctl = dependency('playerctl', version : ['>=2.0.0'], required: get_option('mpris'))
# libpulse = dependency('libpulse', required: get_option('pulseaudio'))
# libudev = dependency('libudev', required: get_option('libudev'))
# libevdev = dependency('libevdev', required: get_option('libevdev'))
# libmpdclient = dependency('libmpdclient', required: get_option('mpd'))
# xkbregistry = dependency('xkbregistry')
# libjack = dependency('jack', required: get_option('jack'))
# libwireplumber = dependency('wireplumber-0.4', required: get_option('wireplumber'))
#

# configure_file(input: 'config.h.meson',
#   output: 'config.h',
#   configuration : conf_data
# )

# subdir('protocol')
subdir('src')
# subdir('docs')
#



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
