project(
  'barbar', 'c',
  version: '0.0.1',
  license: 'MIT',
  meson_version: '>= 1.0.0',
  # default_options: [
  # ],
)

# gmime_version = meson.project_version()
# ver_arr = gmime_version.split('.')

# GMIME_MAJOR_VERSION=ver_arr[0].to_int()
# GMIME_MINOR_VERSION=ver_arr[1].to_int()
# GMIME_MICRO_VERSION=ver_arr[2].to_int()
# GMIME_BINARY_AGE= 100 * GMIME_MINOR_VERSION + GMIME_MICRO_VERSION
# GMIME_INTERFACE_AGE=0

# barbar_package = 'barbar-@0@.0'.format(GMIME_MAJOR_VERSION)
cc = meson.get_compiler('c')

barbar_package = 'barbar-0.0.1'
conf_data = configuration_data()

thread_dep = dependency('threads')
glib =  dependency('glib-2.0', version: '>=2.68')
gobject = dependency('gobject-2.0')
gio = dependency('gio-2.0')
giounix = dependency('gio-unix-2.0')
upower_glib = dependency('upower-glib', required: get_option('upower_glib'))
libgtop = dependency('libgtop-2.0', required: get_option('glibtop'))

gtk = dependency('gtk4')
libmpdclient = dependency('libmpdclient', required: get_option('mpd'))

gtk_layer_shell_dep = dependency('gtk4-layer-shell',
        required: true,
        fallback : ['gtk4-layer-shell', 'gtk_layer_shell'])

deps = [
  thread_dep,
  glib,
  gobject,
  gio,
  gtk,
  gtk_layer_shell_dep,
  libmpdclient,
  giounix,
  upower_glib,
  libgtop,
]

LIBS = [glib, gio, gobject]
# wayland_client = dependency('wayland-client')
# wayland_cursor = dependency('wayland-cursor')
# wayland_protos = dependency('wayland-protocols')
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

subdir('src')

summary({'Install Prefix': get_option('prefix'),
        'Compiler': cc.get_id(),
        'Documentation': get_option('gtk_doc'),
        'Profiling enabled': false, #TODO
        'Coverage enabled': false, #TODO
        # 'PGP/MIME support': get_option('crypto'),
        # 'S/MIME support': get_option('crypto'),
        # 'libidn2 support': libidn2.found(),
        'GObject introspection': get_option('introspection'),
        'Vala bindings': get_option('vala'),
        }, section: 'Configuration')
