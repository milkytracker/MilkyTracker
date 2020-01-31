#
#  src/midi/CMakeLists.txt
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

add_library(midi STATIC
    # Headers
    MidiTools.h
)

target_include_directories(midi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../milkyplay
        ${CMAKE_CURRENT_SOURCE_DIR}/../ppui
        ${CMAKE_CURRENT_SOURCE_DIR}/../ppui/osinterface
        ${CMAKE_CURRENT_SOURCE_DIR}/../tracker
)

# Add platform-specific sources and include paths
if(APPLE)
    target_sources(midi PRIVATE
        # Sources
        osx/MidiReceiver_CoreMIDI.mm

        # Headers
        osx/MidiReceiver_CoreMIDI.h
    )

    # Enable ARC (automatic reference counting) for OS X build
    target_compile_options(midi PRIVATE -fobjc-arc)

    target_include_directories(midi
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/osx
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../ppui/osinterface/posix
    )

    target_link_libraries(midi PUBLIC ${CORE_MIDI_LIBRARY})
elseif(WIN32)
    target_sources(midi PRIVATE
        # Sources
        win32/LongMsg.cpp
        win32/MidiInDevice.cpp
        win32/MidiReceiver_win32.cpp
        win32/ShortMsg.cpp

        # Headers
        win32/LongMsg.h
        win32/MIDIInDevice.h
        win32/MIDIMsg.h
        win32/MidiReceiver_win32.h
        win32/ShortMsg.h
        win32/midi.h
    )

    target_include_directories(midi
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/win32
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../ppui/osinterface/win32
    )

    target_link_libraries(midi PUBLIC winmm)
else()
    target_sources(midi PRIVATE
        # Sources
        posix/MidiReceiver_pthread.cpp

        # Headers
        posix/MidiReceiver_pthread.h
    )

    target_include_directories(midi
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../ppui/osinterface/posix
            ${RTMIDI_INCLUDE_DIRS}
    )

    target_link_libraries(midi PUBLIC ${RTMIDI_LIBRARIES})
endif()
