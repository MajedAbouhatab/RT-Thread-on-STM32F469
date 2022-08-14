import os
from building import *

cwd     = GetCurrentDir()
CPPPATH = [cwd]
src     = ['main.c','lv_conf.h','lv_port_disp.c','lv_port_indev.c']

group = DefineGroup('Applications', src, depend = [''], CPPPATH = CPPPATH)

for item in os.listdir(cwd):
    if os.path.isfile(os.path.join(cwd, item, 'SConscript')):
        group = group + SConscript(os.path.join(item, 'SConscript'))

Return('group')
