libbarbar_src = files(
  'barbar.c',
  'barbar-bar.c',
)

libbarbar_headers = [
  'barbar.h',
  'barbar-bar.h',
]

# version_data = configuration_data()
# version_data.set('GMIME_MAJOR_VERSION', GMIME_MAJOR_VERSION)
# version_data.set('GMIME_MINOR_VERSION', GMIME_MINOR_VERSION)
# version_data.set('GMIME_MICRO_VERSION', GMIME_MICRO_VERSION)
# version_data.set('GMIME_BINARY_AGE', GMIME_BINARY_AGE)
# version_data.set('GMIME_INTERFACE_AGE', GMIME_INTERFACE_AGE)
# configure_file(input: 'gmime-version_meson.h.in',
#   output: 'gmime-version.h',
#   configuration : version_data
# )


executable('testbar',
  libbarbar_src + 'main.c',
  dependencies: deps
)

libbarbar = library(barbar_package, libbarbar_src, dependencies : deps, install: true)

pkg = import('pkgconfig')
gnome = import('gnome')

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
