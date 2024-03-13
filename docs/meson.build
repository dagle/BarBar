gidocgen_common_args = [
  '--quiet',
  '--no-namespace-dir',
]

expand_content_md_files = [
  # 'overview.md',
]

gidocgen_common_args = [
  '--quiet',
  '--no-namespace-dir',
]

docs_dir = barbar_datadir / 'doc'

toml_conf = configuration_data()
toml_conf.set('version', meson.project_version())

gidocgen = find_program('gi-docgen', required: get_option('doc'), native: true)

doc_dependencies = [ 'GObject-2.0', 'Gio-2.0', 'Gtk-4.0' ]

if get_option('doc')
  barbar_toml = configure_file(
    input: 'barbar.toml.in',
    output: 'barbar.toml',
    configuration: toml_conf,
    install: true,
    install_dir: docs_dir / 'barbar',
  )
  custom_target('barbar-doc',
    input: [ barbar_toml, barbar_gir[0] ],
    output: barbar_package,
    command: [
      gidocgen,
      'generate',
      gidocgen_common_args,
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../src'),
      '--config=@INPUT0@',
      '--output-dir=@OUTPUT@',
      '--content-dir=@0@'.format(meson.current_build_dir()),
      '--content-dir=@0@'.format(meson.current_source_dir()),
      '@INPUT1@',
      ],
    depend_files: [ expand_content_md_files ],
    build_by_default: true,
    install: true,
    install_dir: docs_dir,
    )
endif
