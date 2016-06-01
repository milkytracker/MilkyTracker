#
#  cmake/FindJACK.cmake
#
#  Copyright 2016 Dale Whinham
#
#  This file is part of MilkyTracker.
#
#  MilkyTracker is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  MilkyTracker is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with MilkyTracker.  If not, see <http://www.gnu.org/licenses/>.
#

# - Try to find JACK
# Once done this will define
#  JACK_FOUND - System has JACK
#  JACK_INCLUDE_DIRS - The JACK include directories
#  JACK_LIBRARIES - The libraries needed to use JACK
#  JACK_DEFINITIONS - Compiler switches required for using JACK
#  JACK_VERSION_STRING - The version of JACK

find_package(PkgConfig QUIET)
pkg_check_modules(PC_JACK QUIET jack)
set(JACK_DEFINITIONS ${PC_JACK_CFLAGS_OTHER})

find_path(
    JACK_INCLUDE_DIR jack/jack.h
    HINTS ${PC_JACK_INCLUDEDIR} ${PC_JACK_INCLUDE_DIRS}
)

find_library(JACK_LIBRARY NAMES jack
    HINTS ${PC_JACK_LIBDIR} ${PC_JACK_LIBRARY_DIRS}
)

# Get version from pkg-config if possible
if(PC_JACK_VERSION)
    set(JACK_VERSION_STRING ${PC_JACK_VERSION})
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    JACK
    REQUIRED_VARS JACK_LIBRARY JACK_INCLUDE_DIR
    VERSION_VAR JACK_VERSION_STRING
)

mark_as_advanced(JACK_INCLUDE_DIR JACK_LIBRARY)

if(JACK_FOUND)
    set(JACK_LIBRARIES ${JACK_LIBRARY})
    set(JACK_INCLUDE_DIRS ${JACK_INCLUDE_DIR})
endif()
