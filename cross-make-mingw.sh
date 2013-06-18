#!/bin/sh
CFLAGS="-Iboost" make USE_CURSES=0 ARCH=x86 PLATFORM=mingw32 BOOST_TEST_LIBS="boost/libboost_unit_test_framework.dll.a" CXX=i486-mingw32-g++ $*
