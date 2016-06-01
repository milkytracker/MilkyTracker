#
#  cmake/FindLHASA.cmake
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

# - Try to find lhasa
# Once done this will define
#  LHASA_FOUND - System has zziplib
#  LHASA_INCLUDE_DIRS - The zziplib include directories
#  LHASA_LIBRARIES - The libraries needed to use zziplib
#  LHASA_DEFINITIONS - Compiler switches required for using zziplib
#  LHASA_VERSION_STRING - The version of zziplib

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LHASA QUIET liblhasa)
set(LHASA_DEFINITIONS ${PC_LHASA_CFLAGS_OTHER})

find_path(
    LHASA_INCLUDE_DIR lhasa.h
    HINTS ${PC_LHASA_INCLUDEDIR} ${PC_LHASA_INCLUDE_DIRS}
)

find_library(
    LHASA_LIBRARY NAMES lhasa
    HINTS ${PC_LHASA_LIBDIR} ${PC_LHASA_LIBRARY_DIRS}
)

# Get version from pkg-config if possible
set(LHASA_VERSION_STRING ${PC_LHASA_VERSION})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LHASA
    REQUIRED_VARS LHASA_LIBRARY LHASA_INCLUDE_DIR
    VERSION_VAR LHASA_VERSION_STRING)

mark_as_advanced(LHASA_INCLUDE_DIR LHASA_LIBRARY)

if(LHASA_FOUND)
    set(LHASA_LIBRARIES ${LHASA_LIBRARY})
    set(LHASA_INCLUDE_DIRS ${LHASA_INCLUDE_DIR})
endif()
