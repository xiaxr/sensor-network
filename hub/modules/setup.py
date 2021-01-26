#!/usr/bin/env python

from distutils.core import setup, Extension
import sys

if sys.version_info >= (3,):
    BOOST_LIB = "boost_python3"
else:
    BOOST_LIB = "boost_python"

module_xiaxr_r24_net = Extension(
    "rf24netx", libraries=["rf24network", BOOST_LIB], sources=["pyrf24netx.cpp"]
)

setup(name="rf24netx", version="1.0", ext_modules=[module_xiaxr_r24_net])