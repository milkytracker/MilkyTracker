#
#  src/ppui/osinterface/CMakeLists.txt
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

set(
    SOURCES
    PPPathFactory.cpp
)

set(
    HEADERS
    PPMessageBox.h
    PPModalDialog.h
    PPOpenPanel.h
    PPPathFactory.h
    PPQuitSaveAlert.h
    PPSavePanel.h
    PPSystem.h
)

# Add platform-specific sources and include paths
if(APPLE)
    list(APPEND SOURCES
        cocoa/PPMessageBox_COCOA.mm
        cocoa/PPOpenPanel_COCOA.mm
        cocoa/PPQuitSaveAlert_COCOA.mm
        cocoa/PPSavePanel_COCOA.mm
        posix/PPMutex.cpp
        posix/PPPath_POSIX.cpp
        posix/PPSystem_POSIX.cpp
    )
    list(APPEND HEADERS
        posix/PPMutex.h
        posix/PPPath_POSIX.h
        posix/PPSystemString_POSIX.h
        posix/PPSystem_POSIX.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
    )
elseif(WIN32)
    list(APPEND SOURCES
        win32/PPMessageBox_WIN32.cpp
        win32/PPMutex.cpp
        win32/PPOpenPanel_WIN32.cpp
        win32/PPPath_WIN32.cpp
        win32/PPQuitSaveAlert_WIN32.cpp
        win32/PPSavePanel_WIN32.cpp
        win32/PPSystem_WIN32.cpp
        win32/WaitWindow_WIN32.cpp
    )
    list(APPEND HEADERS
        win32/PPMutex.h
        win32/PPPath_WIN32.h
        win32/PPSystemString_WIN32.h
        win32/PPSystem_WIN32.h
        win32/WaitWindow_WIN32.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/milkyplay
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/win32
    )
else()
    list(APPEND SOURCES
        posix/PPPath_POSIX.cpp
        posix/PPSystem_POSIX.cpp
        sdl/PPMessageBox_SDL.cpp
        sdl/PPMutex.cpp
        sdl/PPOpenPanel_SDL.cpp
        sdl/PPQuitSaveAlert_SDL.cpp
        sdl/PPSavePanel_SDL.cpp
        sdl/SDL_ModalLoop.cpp
    )
    list(APPEND HEADERS
        posix/PPMutex.h
        posix/PPPath_POSIX.h
        posix/PPSystemString_POSIX.h
        posix/PPSystem_POSIX.h
        sdl/PPMutex.h
        sdl/SDL_ModalLoop.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
        ${SDL2_INCLUDE_DIRS}
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface
)

add_library(osinterface ${SOURCES} ${HEADERS})
