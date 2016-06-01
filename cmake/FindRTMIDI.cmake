#
#  cmake/FindRTMIDI.cmake
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

# - Try to find RtMidi
# Once done this will define
#  RTMIDI_FOUND - System has RtMidi
#  RTMIDI_INCLUDE_DIRS - The RtMidi include directories
#  RTMIDI_LIBRARIES - The libraries needed to use RtMidi
#  RTMIDI_DEFINITIONS - Compiler switches required for using RtMidi
#  RTMIDI_VERSION_STRING - The version of RtMidin

find_package(PkgConfig QUIET)
pkg_check_modules(PC_RTMIDI QUIET rtmidi)
set(RTMIDI_DEFINITIONS ${PC_RTMIDI_CFLAGS_OTHER})

find_path(
    RTMIDI_INCLUDE_DIR RtMidi.h
    HINTS ${PC_RTMIDI_INCLUDEDIR} ${PC_RTMIDI_INCLUDE_DIRS}
    PATH_SUFFIXES rtmidi
)

find_library(
    RTMIDI_LIBRARY NAMES rtmidi
    HINTS ${PC_RTMIDI_LIBDIR} ${PC_RTMIDI_LIBRARY_DIRS}
)

# Get version from pkg-config if possible, else scrape it from the header
if(PC_RTMIDI_VERSION)
    set(RTMIDI_VERSION_STRING ${PC_RTMIDI_VERSION})
elseif(RTMIDI_INCLUDE_DIR AND EXISTS "${RTMIDI_INCLUDE_DIR}/RtMidi.h")
    file(
        STRINGS "${RTMIDI_INCLUDE_DIR}/RtMidi.h" RTMIDI_VERSION_LINE
        REGEX "^// RtMidi: Version .*$"
    )
    string(
        REGEX REPLACE "^.*Version (.*)$" "\\1" RTMIDI_VERSION_STRING
        ${RTMIDI_VERSION_LINE}
    )
    unset(RTMIDI_VERSION_LINE)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    RTMIDI
    REQUIRED_VARS RTMIDI_LIBRARY RTMIDI_INCLUDE_DIR
    VERSION_VAR RTMIDI_VERSION_STRING
)

mark_as_advanced(RTMIDI_INCLUDE_DIR RTMIDI_LIBRARY)

if(RTMIDI_FOUND)
    set(RTMIDI_LIBRARIES ${RTMIDI_LIBRARY})
    set(RTMIDI_INCLUDE_DIRS ${RTMIDI_INCLUDE_DIR})
endif()
