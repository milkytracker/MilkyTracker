#
#  cmake/FindZZIPLIB.cmake
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

# - Try to find zziplib
# Once done this will define
#  ZZIPLIB_FOUND - System has zziplib
#  ZZIPLIB_INCLUDE_DIRS - The zziplib include directories
#  ZZIPLIB_LIBRARIES - The libraries needed to use zziplib
#  ZZIPLIB_DEFINITIONS - Compiler switches required for using zziplib
#  ZZIPLIB_VERSION_STRING - The version of zziplib

find_package(PkgConfig QUIET)
pkg_check_modules(PC_ZZIPLIB QUIET zziplib)
set(ZZIPLIB_DEFINITIONS ${PC_ZZIPLIB_CFLAGS_OTHER})

find_path(
    ZZIPLIB_INCLUDE_DIR zzip/zzip.h
    HINTS ${PC_ZZIPLIB_INCLUDEDIR} ${PC_ZZIPLIB_INCLUDE_DIRS}
)

find_library(
    ZZIPLIB_LIBRARY NAMES zzip
    HINTS ${PC_ZZIPLIB_LIBDIR} ${PC_ZZIPLIB_LIBRARY_DIRS}
)

# Get version from pkg-config if possible, else scrape it from the header
if(PC_ZZIPLIB_VERSION)
    set(ZZIPLIB_VERSION_STRING ${PC_ZZIPLIB_VERSION})
elseif(ZZIPLIB_INCLUDE_DIR AND EXISTS "${ZZIPLIB_INCLUDE_DIR}/_config.h")
    file(
        STRINGS "${ZZIPLIB_INCLUDE_DIR}/_config.h" ZZIPLIB_VERSION_LINE
        REGEX "^#define ZZIP_VERSION.*$"
    )
    string(
        REGEX REPLACE "^.*ZZIP_VERSION.*\"([0-9.]+)\".*$" "\\1"
        ZZIPLIB_VERSION_STRING ${ZZIPLIB_VERSION_LINE}
    )
    unset(ZZIPLIB_VERSION_LINE)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    ZZIPLIB
    REQUIRED_VARS ZZIPLIB_LIBRARY ZZIPLIB_INCLUDE_DIR
    VERSION_VAR ZZIPLIB_VERSION_STRING
)

mark_as_advanced(ZZIPLIB_INCLUDE_DIR ZZIPLIB_LIBRARY)

if(ZZIPLIB_FOUND)
    set(ZZIPLIB_LIBRARIES ${ZZIPLIB_LIBRARY})
    set(ZZIPLIB_INCLUDE_DIRS ${ZZIPLIB_INCLUDE_DIR})
endif()
