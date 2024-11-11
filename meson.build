project(
  'barbar', 'c',
  version: '0.1',
  meson_version: '>= 1.0.0',
  license: 'LGPL-2.1-or-later'
  # default_options: [
  # ],
)

barbar_prefix = get_option('prefix')
barbar_datadir = join_paths(barbar_prefix, get_option('datadir'))

pkgdatadir =  barbar_datadir / meson.project_name()

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
libgtop = dependency('libgtop-2.0')
pipewire_dep = dependency('libpipewire-0.3', 
  required: get_option('pipewire'))

libgweather = dependency('gweather4')
libgeoclue = dependency('libgeoclue-2.0')

libwireplumber = dependency('wireplumber-0.5', required: get_option('wireplumber'))
wayland_client = dependency('wayland-client')
template_glib = dependency('template-glib-1.0')
fftw3 = dependency('fftw3', required: get_option('cava'))
# xkbcommon = dependency('xkbcommon')

# defaults to etc
sysconfdir = get_option('sysconfdir')

gtk = dependency('gtk4')

gtk_layer_shell_dep = dependency('gtk4-layer-shell-0',
        required: true,
        fallback : ['gtk4-layer-shell', 'gtk4-layer-shell-0'])

cava = dependency('cava',
                  version : '>=0.10.3',
                  required: get_option('cava'),
                  fallback : ['cava', 'cava_dep'])

systray = dependency('statusnotifier-systray-gtk4',
  required: get_option('systray'),
  fallback: ['statusnotifier-systray-gtk4'])

scdoc = dependency('scdoc', version: '>=1.9.2', native: true, required: get_option('man-page'))
if scdoc.found()
	scdoc_prog = find_program(scdoc.get_variable(pkgconfig: 'scdoc'), native: true)
	mandir = get_option('mandir')
	man_files = [
    'barbar.5.scd',
	]
	foreach filename : man_files
		topic = filename.split('.')[-3].split('/')[-1]
		section = filename.split('.')[-2]
		output = '@0@.@1@'.format(topic, section)

		custom_target(
			output,
			input: filename,
			output: output,
			command: scdoc_prog,
			install: true,
			feed: true,
			capture: true,
			install_dir: '@0@/man@1@'.format(mandir, section)
		)
	endforeach
endif

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
  libwireplumber,
  template_glib,
  libgweather,
  libgeoclue,
  soup,
  pipewire_dep,
  cava,
  fftw3,
  # xkbcommon,
  m_dep,
  module,
]


# like deps but only needed for the binary
bin_deps = [
  systray,
]

LIBS = [glib, gio, gobject, gtk]
wayland_protos = dependency('wayland-protocols')
subdir('src')

if get_option('docs')
  subdir('docs')
endif

install_data('barbar.desktop', install_dir: barbar_applicationsdir)

install_data('barbar.svg', install_dir: barbar_icondir)

install_data('style.css', install_dir: sysconfdir / 'xdg'/ meson.project_name())


gnome.compile_resources('barbar',
  'barbar.gresource.xml',
  gresource_bundle: true,
  install: true,
  install_dir: pkgdatadir,
)

summary({'Install Prefix': get_option('prefix'),
        'Compiler': cc.get_id(),
        'Documentation': get_option('gtk_doc'),
        'Profiling enabled': false, #TODO
        'Coverage enabled': false, #TODO
        'GObject introspection': get_option('introspection'),
        'Vala bindings': get_option('vala'),
        }, section: 'Configuration')
