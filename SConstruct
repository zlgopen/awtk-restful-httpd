import os
import scripts.app_helper as app

helper = app.Helper(ARGUMENTS);
helper.set_dll_def('src/httpd.def').set_libs(['httpd']).call(DefaultEnvironment)

SConscriptFiles = ['src/SConscript', 'demos/SConscript', 'tests/SConscript']
helper.SConscript(SConscriptFiles)
