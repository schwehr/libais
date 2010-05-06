from distutils.core import setup, Extension

ais_module = Extension('ais',
                    sources = ['ais_py.cpp',
                               'ais.cpp',
                               'ais1_2_3.cpp',
                               'ais4_11.cpp',
                               'ais5.cpp',
                               # ais6
                               'ais7_13.cpp',
                               'ais14.cpp',
                               'ais18.cpp',
                               'ais19.cpp',
                               ])

setup (name = 'ais',
       version = '0.1',
       description = ' Automatic Identification System',
       ext_modules = [ais_module])
