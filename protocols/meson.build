wayland_protos= dependency(
	'wayland-protocols',
	version: '>=1.43',
	fallback: 'wayland-protocols',
	default_options: ['tests=false'],
)

wayland_protocol_dir = wayland_protos.get_variable('pkgdatadir')

protocols = [
	wayland_protocol_dir / 'stable/xdg-shell/xdg-shell.xml',
]


# wayland.xml generation
wayland_scanner = dependency('wayland-scanner', native: true)
wayland_scanner_datadir = wayland_scanner.get_variable('pkgdatadir')

wayland_xml = wayland_scanner_datadir / 'wayland.xml'
wayland_protocol = custom_target(
	wayland_xml.underscorify(),
	input: wayland_xml,
	install: true,
	install_dir: [false, join_paths(get_option('includedir'), 'hyprland/protocols')],
	output: ['@BASENAME@.cpp', '@BASENAME@.hpp'],
	command: [hyprwayland_scanner, '--wayland-enums', '@INPUT@', '@OUTDIR@'],
)

lib_server_protos = static_library(
	'server_protos',
	wayland_protocol,
)

server_protos = declare_dependency(
	link_with: lib_server_protos,
	sources: wayland_protocol,
)
