#!/usr/bin/env python

from distutils.core import setup, Extension

module1 = Extension('bgp', define_macros = [('_FILE_OFFSET_BITS', '64'), ('_LARGEFILE_SOURCE', '1')], sources = ['PYGetSurePath.cpp', 'SimpleRoutingTable2.cpp', 'Address.cpp', 'VMEM.cpp', 'VMEMstd.cpp', 'MyUtil.cpp'])

setup (name = 'bgplib',
       version = '1.0',
       description = 'This is a bgp package',
       ext_modules = [module1])
