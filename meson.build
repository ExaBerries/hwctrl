project('hwctrl', 'cpp',
	version: '0.1.0',
	meson_version: '>=0.53.2',
	default_options: [
		'cpp_std=c++2a',
		'warning_level=3'
	]
)

cpp_compiler = meson.get_compiler('cpp')

add_project_arguments(cpp_compiler.get_supported_arguments([
	'-Wconversion',
	'-Wsign-conversion',
	'-Wdiv-by-zero',
	'-Wshorten-64-to-32',
	'-Wfloat-equal',
	'-Wcast-align',
	'-Wuseless-cast',
	'-Wshadow',
	'-Wmissing-field-initializers',
	'-Wmissing-noreturn',
	'-Wmissing-declarations',
	'-Wmissing-variable-declarations',
	'-Wmissing-prototypes',
	'-Wstrict-prototypes',
	'-Woverloaded-virtual',
	'-Winline',
	'-Wvla',
	'-Wpointer-arith',
	'-Wempty-body',
	'-Wpragmas',
	'-Wformat-security',
	'-Wformat=2',
	'-Wformat-nonliteral',
	'-Wwrite-strings',
	'-Wnested-externs',
	'-Wendif-labels',
	'-Wunreachable-code',
	'-Wunreachable-code-break',
	'-Waddress',
	'-Wnull-dereference',
	'-Wattributes',
	'-Wextended-offsetof',
	'-Wlanguage-extension-token',
	'-Wswitch-enum',
	'-Wunused-macros',
	'-Wunused-parameter',
	'-Wredundant-decls',
	'-Wheader-guard',
	'-Wmisleading-indentation',
	'-Wlogical-op',
	'-Wduplicated-cond',
	'-Wduplicated-branches',
	'-Wlifetime',
	'-Weffc++'
]), language: 'cpp')

subdir('lib')
subdir('exe')
