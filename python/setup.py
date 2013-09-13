#!/usr/bin/env python

from distutils.core import setup, Extension

#################################################################

NAME         = "sdd"
VERSION      = "0.1"
AUTHORS      = "Alexandre Hamez"
EMAIL        = "alexandre.hamez@isae.fr"
DESCRIPTION  = """A package to use SDDs."""
URL          = "https://github.com/ahamez/libsdd"
LICENSE      = "BSD License"
PLATFORM     = "Any"

#################################################################

dynamic_sdd = \
Extension(  name='_sdd',
            sources=['sdd.cc'],
            include_dirs=["./"],#, "/usr/local/boost-1.54/clang/include"],
            language='c++',
            libraries=['boost_python'])

#################################################################

setup(  name=NAME,
        version=VERSION,
        long_description=DESCRIPTION,
        url=URL,
        license=LICENSE,
        author=AUTHORS,
        platforms=PLATFORM,
        author_email=EMAIL,
        py_modules=['sdd'],
        ext_modules=[dynamic_sdd]
        )

#################################################################
