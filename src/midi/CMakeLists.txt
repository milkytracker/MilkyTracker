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

set(HEADERS
    MidiTools.h
)

# Add platform-specific sources and include paths
if(APPLE)
    set(
        SOURCES
        osx/MidiReceiver_CoreMIDI.mm
    )
    list(
        APPEND HEADERS
        osx/MidiReceiver_CoreMIDI.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
    )
elseif(WIN32)
    set(
        SOURCES
        win32/LongMsg.cpp
        win32/MidiInDevice.cpp
        win32/MidiReceiver_win32.cpp
        win32/ShortMsg.cpp
    )
    list(
        APPEND HEADERS
        win32/LongMsg.h
        win32/MIDIInDevice.h
        win32/MIDIMsg.h
        win32/MidiReceiver_win32.h
        win32/ShortMsg.h
        win32/midi.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/win32
    )
else()
    set(
        SOURCES
        posix/MidiReceiver_pthread.cpp
    )
    list(
        APPEND HEADERS
        posix/MidiReceiver_pthread.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
        ${RTMIDI_INCLUDE_DIR}
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src/midi
    ${PROJECT_SOURCE_DIR}/src/milkyplay
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface
    ${PROJECT_SOURCE_DIR}/src/tracker
)

add_library(midi ${SOURCES} ${HEADERS})

if(APPLE)
    target_link_libraries(midi ${CORE_MIDI_LIBRARY})
elseif(WIN32)
    target_link_libraries(midi winmm)
else()
    target_link_libraries(midi ${RTMIDI_LIBRARIES})
endif()
