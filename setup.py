#!/usr/bin/env python

import os
import sys

from setuptools import setup, find_packages, Extension

VERSION = open('VERSION').readline().strip()

EXTRA_COMPILE_ARGS = []
if sys.platform in ('darwin', 'linux', 'linux2'):
    EXTRA_COMPILE_ARGS = ['-std=c++11']

AIS_MODULE = Extension(
    '_ais',
    extra_compile_args=EXTRA_COMPILE_ARGS,
    sources=[os.path.join('src/libais', filename) for filename in (
        'ais_py.cpp',
        'ais.cpp',
        'ais1_2_3.cpp', # Class A position
        'ais4_11.cpp', # Basestation report, '=' - UTC time response
        'ais5.cpp', # Static data report / ship name
        'ais6.cpp', # Addressed binary message (ABM)
        'ais7_13.cpp',
        'ais8.cpp', # Broadcast binary message (BBM)
        'ais8_1_22.cpp', # Area notice
        'ais8_1_26.cpp', # Environmental Sensor Report
        'ais8_200.cpp',
        'ais8_366.cpp',
        'ais8_367.cpp',
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
        )

        ])

setup(name='libais',
      version=VERSION,
      description='Automatic Identification System decoding - ship tracking',
      author='Kurt Schwehr',
      author_email='schwehr@gmail.com',
      url='https://github.com/schwehr/libais',
      license='Apache 2.0',
      ext_modules=[AIS_MODULE],
      packages=find_packages(),
      install_requires=[
          'six'
      ],
      extras_require={
          'test': ['gpsd_format']
      },
      classifiers=[
           'License :: OSI Approved :: Apache Software License',
           'Topic :: System :: Networking',
           'Development Status :: 4 - Beta',
           'Intended Audience :: Science/Research',
           'Topic :: Communications',
           'Topic :: Scientific/Engineering :: Information Analysis',
           'Topic :: Scientific/Engineering :: '
           'Interface Engine/Protocol Translator',
           'Topic :: Scientific/Engineering :: GIS',
           ],
      scripts=["bin/aisdecode"],
      test_suite = "test"
      )
