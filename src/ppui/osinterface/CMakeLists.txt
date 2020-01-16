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

add_library(osinterface STATIC
    # Sources
    PPPathFactory.cpp

    # Headers
    PPMessageBox.h
    PPModalDialog.h
    PPOpenPanel.h
    PPPathFactory.h
    PPQuitSaveAlert.h
    PPSavePanel.h
    PPSystem.h
)

target_include_directories(osinterface
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Add platform-specific sources and include paths
if(APPLE)
    target_sources(osinterface
        PRIVATE
            # Sources
            cocoa/PPMessageBox_COCOA.mm
            cocoa/PPOpenPanel_COCOA.mm
            cocoa/PPQuitSaveAlert_COCOA.mm
            cocoa/PPSavePanel_COCOA.mm
            posix/PPMutex.cpp
            posix/PPPath_POSIX.cpp
            posix/PPSystem_POSIX.cpp

            # Headers
            posix/PPMutex.h
            posix/PPPath_POSIX.h
            posix/PPSystemString_POSIX.h
            posix/PPSystem_POSIX.h
    )
    target_include_directories(osinterface PUBLIC posix)
elseif(WIN32)
    target_sources(osinterface
        PRIVATE
            # Sources
            win32/PPMessageBox_WIN32.cpp
            win32/PPMutex.cpp
            win32/PPOpenPanel_WIN32.cpp
            win32/PPPath_WIN32.cpp
            win32/PPQuitSaveAlert_WIN32.cpp
            win32/PPSavePanel_WIN32.cpp
            win32/PPSystem_WIN32.cpp
            win32/WaitWindow_WIN32.cpp

            # Headers
            win32/PPMutex.h
            win32/PPPath_WIN32.h
            win32/PPSystemString_WIN32.h
            win32/PPSystem_WIN32.h
            win32/WaitWindow_WIN32.h
    )
    target_include_directories(osinterface
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src/milkyplay
        PUBLIC
            win32
    )
else()
    target_sources(osinterface
        PRIVATE
            # Sources
            posix/PPPath_POSIX.cpp
            posix/PPSystem_POSIX.cpp
            sdl/PPMessageBox_SDL.cpp
            sdl/PPMutex.cpp
            sdl/PPOpenPanel_SDL.cpp
            sdl/PPQuitSaveAlert_SDL.cpp
            sdl/PPSavePanel_SDL.cpp
            sdl/SDL_ModalLoop.cpp

            # Headers
            posix/PPMutex.h
            posix/PPPath_POSIX.h
            posix/PPSystemString_POSIX.h
            posix/PPSystem_POSIX.h
            sdl/PPMutex.h
            sdl/SDL_ModalLoop.h
    )
    target_include_directories(osinterface PUBLIC posix)

    if(TARGET SDL2::SDL2)
        # If the distro built SDL2 with CMake, we can just link to an exported target
        target_link_libraries(osinterface PUBLIC SDL2::SDL2)
    else()
        # Otherwise we need to do things the old-fashioned way for compatibility
        target_include_directories(osinterface PUBLIC ${SDL2_INCLUDE_DIRS})
        target_link_libraries(osinterface PUBLIC ${SDL2_LIBRARIES})
    endif()
endif()

if(APPLE)
    # Enable ARC (automatic reference counting) for OS X build
    target_compile_options(osinterface PRIVATE -fobjc-arc)
endif()
