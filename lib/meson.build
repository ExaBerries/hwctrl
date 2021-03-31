
lib_include = include_directories('include')

lib = library('hwctrl',
	[
		'src/source/spd.cpp',
		'src/util/file.cpp'
	],
	include_directories: [
		lib_include
	]
)

lib_dep = declare_dependency(include_directories: [lib_include], link_with: [lib])
