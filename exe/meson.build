
lyra_proj = subproject('lyra')
lyra_dep = lyra_proj.get_variable('lyra_dep')

exe = executable('hwctrl',
	[
		'src/main.cpp'
	],
	dependencies: [
		lib_dep,
		lyra_dep
	]
)
