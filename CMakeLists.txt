#
#  CMakeLists.txt
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

cmake_minimum_required(VERSION 2.8.5)
project(MilkyTracker)

# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)

# Force SDL if requested
option(FORCESDL "Force SDL instead of native" OFF)
if(FORCESDL)
    unset(APPLE)
    unset(WIN32)
    add_definitions(-D__FORCE_SDL_AUDIO__)
    # Frameworks not supported by findSDL2.cmake
    set(CMAKE_FIND_FRAMEWORK NEVER)
endif()

# Lowercase project name for binaries and packaging
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# Additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# Version number in format X.YY.ZZ
set(VER_X  1)
set(VER_YY 02)
set(VER_ZZ 00)
set(VER_FULL "${VER_X}.${VER_YY}.${VER_ZZ}")

# Generate version header from the above
configure_file(
    ${PROJECT_SOURCE_DIR}/src/tracker/version.h.in
    ${PROJECT_BINARY_DIR}/src/tracker/version.h
)

# Packaging
if(APPLE)
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME} ${VER_FULL}")
    set(
        CPACK_DMG_DS_STORE_SETUP_SCRIPT
        ${PROJECT_SOURCE_DIR}/resources/packaging/osx/DMGSetup.scpt
    )
    set(
        CPACK_DMG_BACKGROUND_IMAGE
        ${PROJECT_SOURCE_DIR}/resources/packaging/osx/DMGBackground.tif
    )
elseif(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()
set(CPACK_PACKAGE_NAME ${PROJECT_NAME_LOWER})
set(CPACK_PACKAGE_VENDOR "MilkyTracker Team")
set(CPACK_PACKAGE_CONTACT "support@milkytracker.org")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${VER_FULL}")
set(CPACK_PACKAGE_VERSION_MAJOR ${VER_X})
set(CPACK_PACKAGE_VERSION_MINOR ${VER_YY})
set(CPACK_PACKAGE_VERSION_PATCH ${VER_ZZ})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FastTracker II compatible music tracker")
include(CPack)

if(APPLE)
    # Warn if deployment target isn't set to Lion
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET MATCHES "10.7")
        message(WARNING
            "Your deployment target is either unset or not set to \"10.7\", "
            "which means that the binaries produced may not run on earlier "
            "versions of macOS.\n"
            "Please re-run CMake with '-DCMAKE_OSX_DEPLOYMENT_TARGET=10.7' "
            "or change the variable in the CMake GUI to target Lion and newer."
        )
    endif()

    # Set variables for generating the Info.plist file
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${VER_FULL}")
    set(MACOSX_BUNDLE_EXECUTABLE ${PROJECT_NAME})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.Titan.MilkyTracker")
    set(MACOSX_BUNDLE_NSMAIN_NIB_FILE "Application")
    set(MACOSX_BUNDLE_ICON_FILE "carton")
    set(MACOSX_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VER_FULL}")

    # Carbon only required for HIToolbox/Events.h (virtual keycodes)
    find_library(CARBON_LIBRARY Carbon)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(CORE_AUDIO_LIBRARY CoreAudio)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(CORE_MIDI_LIBRARY CoreMIDI)
    find_library(CORE_VIDEO_LIBRARY CoreVideo)
    find_library(OPENGL_LIBRARY OpenGL)

    # OS X MIDI support requires no external libraries
    message(STATUS "Enabled MIDI support (Core MIDI)")
    add_subdirectory(src/midi)
elseif(WIN32)
    # Visual C++ Compiler options
    if(MSVC)
        # Warn if platform toolset may not be targetting Windows XP upwards
        if(NOT CMAKE_VS_PLATFORM_TOOLSET MATCHES "xp")
            message(WARNING
                "Your currently-selected platform toolset may generate "
                "executables which are incompatible with Windows XP.\n"
                "Please set your toolset to be one of v110_xp, v120_xp or "
                "v140_xp for VS2012, VS2013, and VS2015 respectively.\n"
                "You can do so with the '-T' argument to CMake, or by entering "
                "it in the CMake GUI."
            )
        endif()

        # Suppress secure string function warnings
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)

        # Enable parallel compilation
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

        # Enable static linkage of the Microsoft Visual C/C++ Runtime
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MTd")
        set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MTd"
        )
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
    endif()

    # Prevent Windows.h from clashing with the Standard Template Library so we
    # can use std::min/std::max (see https://support.microsoft.com/kb/143208)
    add_definitions(-DNOMINMAX)

    # Windows MIDI support requires no external libraries
    message(STATUS "Enabled MIDI support (WinMM)")
    add_subdirectory(src/midi)
else()
    # Workaround for SDL bug #3295, which we might hit if the distro's SDL2
    # package includes the sdl2-config.cmake file
    # https://bugzilla.libsdl.org/show_bug.cgi?id=3295
    cmake_policy(SET CMP0004 OLD)

    find_package(SDL2 2 REQUIRED)
endif()

# Prefer static linkage under OS X for libraries located with find_package()
if(APPLE)
    set(SUFFIXES_ORIG ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

# Under macOS and Windows we use Git Submodules to locate the decompression libs
if(UNIX)
    # zlib is provided by Linux and macOS
    find_package(ZLIB)

    # We can't use Brew for these due to OSX target version mismatch
    # (we're targeting an older version of OSX to increase compatibility)
    if(NOT APPLE)
      find_package(LHASA)
      find_package(ZZIPLIB)
    endif()

    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        find_package(ALSA)
        find_package(JACK)

        # Linux MIDI support requires ALSA and RtMidi
        if(ALSA_FOUND)
            find_package(RTMIDI 2.1.0)
            if(RTMIDI_FOUND)
                message(STATUS "Enabled MIDI support (ALSA/RtMidi)")
                add_subdirectory(src/midi)
            else()
                message("MIDI support disabled (RtMidi unavailable)")
            endif()
        else()
            message("MIDI support disabled (ALSA unavailable)")
        endif()
    endif()
endif()

# Restore library suffixes
if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${SUFFIXES_ORIG})
endif()

add_subdirectory(docs)
add_subdirectory(resources/music)
add_subdirectory(src/compression)
add_subdirectory(src/fx)
add_subdirectory(src/milkyplay)
add_subdirectory(src/ppui)
add_subdirectory(src/tracker)
