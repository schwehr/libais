#!/usr/bin/env python

from setuptools import setup, find_packages, Extension

version = file('VERSION').readline().strip()

ais_module = Extension('ais',
                    sources = ['ais_py.cpp',
                               'ais.cpp',
                               'ais1_2_3.cpp', # Class A position
                               'ais4_11.cpp', # Basestation report, '=' - UTC time response
                               'ais5.cpp', # Static data report / ship name
                               'ais6.cpp', # Addressed binary message (ABM)
                               'ais7_13.cpp',
                               'ais8.cpp', # Broadcast binary message (BBM)
                               'ais8_001_22.cpp', # Area notice
                               'ais9.cpp',
                               'ais10.cpp', # :
                               # 11 See 4 - ;
                               'ais12.cpp', # <
                               # 13 See 7 - =
                               'ais14.cpp', # >
                               'ais15.cpp', # ?
                               'ais16.cpp', # @
                               'ais17.cpp', # A
                               'ais18.cpp', # B
                               'ais19.cpp', # C
                               'ais20.cpp', # D
                               'ais21.cpp', # E
                               'ais22.cpp', # F
                               'ais23.cpp', # G
                               'ais24.cpp', # H
                               'ais25.cpp', # I - single slot binary message
                               'ais26.cpp', # J - Multi-slot binary message with comm-state
                               'ais27.cpp', # K - Long-range position
                               # 'ais28.cpp', # L - Not yet defined
                               ])

setup (name = 'libais',
       version = version,
       description = 'Automatic Identification System decoding - ship tracking',
       author='Kurt Schwehr',
       author_email='schwehr@gmail.com',
       url='https://github.com/schwehr/libais',
       license='LGPL v3+',
       ext_modules = [ais_module],
       classifiers=[
            'License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)',
            'Topic :: System :: Networking',
            'Development Status :: 4 - Beta',
            'Intended Audience :: Science/Research',
            'Topic :: Communications',
            'Topic :: Scientific/Engineering :: Information Analysis',
            'Topic :: Scientific/Engineering :: Interface Engine/Protocol Translator',
            'Topic :: Scientific/Engineering :: GIS',
    ],
       )
