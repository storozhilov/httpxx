import os

AddOption('--prefix',
		dest = 'prefix',
		nargs = 1,
		type = 'string',
		action = 'store',
		help = 'Prefix for installation. Usage: "scons --prefix=<instalation_path> install"')

sconscriptTargets = ['src/SConscript']

SConscript(['src/SConscript', 'test/SConscript'])
#SConscript('src/SConscript')

# Uninstall section
env = Environment()
uninstaller = env.Command('uninstall', None, Delete(env.FindInstalledFiles()))
env.Alias('uninstall', uninstaller)
