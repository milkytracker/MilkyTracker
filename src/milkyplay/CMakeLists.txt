#
#  src/milkyplay/CMakeLists.txt
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
    AudioDriverBase.cpp
    AudioDriverManager.cpp
    AudioDriver_NULL.cpp
    AudioDriver_WAVWriter.cpp
    ChannelMixer.cpp
    ExporterXM.cpp
    LittleEndian.cpp
    Loader669.cpp
    LoaderAMF.cpp
    LoaderAMS.cpp
    LoaderCBA.cpp
    LoaderDBM.cpp
    LoaderDIGI.cpp
    LoaderDSM.cpp
    LoaderDTM.cpp
    LoaderFAR.cpp
    LoaderGDM.cpp
    LoaderIMF.cpp
    LoaderIT.cpp
    LoaderMDL.cpp
    LoaderMOD.cpp
    LoaderMTM.cpp
    LoaderMXM.cpp
    LoaderOKT.cpp
    LoaderPLM.cpp
    LoaderPSM.cpp
    LoaderPTM.cpp
    LoaderS3M.cpp
    LoaderSTM.cpp
    LoaderULT.cpp
    LoaderUNI.cpp
    LoaderXM.cpp
    MasterMixer.cpp
    PlayerBase.cpp
    PlayerFAR.cpp
    PlayerGeneric.cpp
    PlayerIT.cpp
    PlayerSTD.cpp
    ResamplerFactory.cpp
    SampleLoaderAIFF.cpp
    SampleLoaderALL.cpp
    SampleLoaderAbstract.cpp
    SampleLoaderGeneric.cpp
    SampleLoaderIFF.cpp
    SampleLoaderWAV.cpp
    XIInstrument.cpp
    XMFile.cpp
    XModule.cpp
)

set(
    HEADERS
    AudioDriverBase.h
    AudioDriverManager.h
    AudioDriver_COMPENSATE.h
    AudioDriver_NULL.h
    AudioDriver_WAVWriter.h
    ChannelMixer.h
    LittleEndian.h
    Loaders.h
    MasterMixer.h
    MilkyPlay.h
    MilkyPlayCommon.h
    MilkyPlayResults.h
    MilkyPlayTypes.h
    Mixable.h
    PlayerBase.h
    PlayerFAR.h
    PlayerGeneric.h
    PlayerIT.h
    PlayerSTD.h
    ResamplerAmiga.h
    ResamplerCubic.h
    ResamplerFactory.h
    ResamplerFast.h
    ResamplerMacros.h
    ResamplerSinc.h
    SampleLoaderAIFF.h
    SampleLoaderALL.h
    SampleLoaderAbstract.h
    SampleLoaderGeneric.h
    SampleLoaderIFF.h
    SampleLoaderWAV.h
    XIInstrument.h
    XMFile.h
    XModule.h
    computed-blep.h
)

# Add platform-specific sources, include paths and definitions
if(APPLE)
    list(APPEND SOURCES drivers/osx/AudioDriver_COREAUDIO.mm)
    list(APPEND HEADERS drivers/osx/AudioDriver_COREAUDIO.h)
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/milkyplay/drivers/osx
    )
    message(STATUS "Enabled Core Audio support")
elseif(WIN32)
    # Basic WaveOut support
    list(APPEND SOURCES drivers/windows/AudioDriver_MMSYSTEM.cpp)
    list(APPEND HEADERS drivers/windows/AudioDriver_MMSYSTEM.h)
    include_directories(${PROJECT_SOURCE_DIR}/src/milkyplay/drivers/windows)
    message(STATUS "Enabled WaveOut support")

    # Build sources from RtAudio Git submodule if present
    set(RTAUDIO_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/rtaudio)
    if(EXISTS ${RTAUDIO_SUBMODULE_DIR}/RtAudio.cpp)
        message(STATUS "Enabled RtAudio support")
        list(
            APPEND SOURCES
            ${RTAUDIO_SUBMODULE_DIR}/include/asio.cpp
            ${RTAUDIO_SUBMODULE_DIR}/include/asiodrivers.cpp
            ${RTAUDIO_SUBMODULE_DIR}/include/asiolist.cpp
            ${RTAUDIO_SUBMODULE_DIR}/include/iasiothiscallresolver.cpp
            ${RTAUDIO_SUBMODULE_DIR}/RtAudio.cpp
            drivers/generic/AudioDriver_RTAUDIO.cpp
            drivers/generic/RtAudio4Impl.cpp
        )
        list(
            APPEND HEADERS
            ${RTAUDIO_SUBMODULE_DIR}/include/asio.h
            ${RTAUDIO_SUBMODULE_DIR}/include/asiodrivers.h
            ${RTAUDIO_SUBMODULE_DIR}/include/asiodrvr.h
            ${RTAUDIO_SUBMODULE_DIR}/include/asiolist.h
            ${RTAUDIO_SUBMODULE_DIR}/include/asiosys.h
            ${RTAUDIO_SUBMODULE_DIR}/include/dsound.h
            ${RTAUDIO_SUBMODULE_DIR}/include/FunctionDiscoveryKeys_devpkey.h
            ${RTAUDIO_SUBMODULE_DIR}/include/ginclude.h
            ${RTAUDIO_SUBMODULE_DIR}/include/iasiodrv.h
            ${RTAUDIO_SUBMODULE_DIR}/include/iasiothiscallresolver.h
            ${RTAUDIO_SUBMODULE_DIR}/include/soundcard.h
            ${RTAUDIO_SUBMODULE_DIR}/RtAudio.h
            drivers/generic/AudioDriver_RTAUDIO.h
        )
        include_directories(
            ${PROJECT_SOURCE_DIR}/src/milkyplay/drivers/generic
            ${RTAUDIO_SUBMODULE_DIR}
            ${RTAUDIO_SUBMODULE_DIR}/include
        )
        add_definitions(
            -D__WINDOWS_DS__
            -D__WINDOWS_ASIO__
            -D__WINDOWS_WASAPI__
        )
    else()
        add_definitions(-D__SKIPRTAUDIO__)
        message("RtAudio support disabled (RtAudio unavailable)")
    endif()
else()
    add_definitions(-DDRIVER_UNIX)

    if(ALSA_FOUND)
        list(APPEND SOURCES drivers/alsa/AudioDriver_ALSA.cpp)
        list(APPEND HEADERS drivers/alsa/AudioDriver_ALSA.h)
        add_definitions(-DHAVE_LIBASOUND)
        include_directories(
            ${PROJECT_SOURCE_DIR}/src/milkyplay/drivers/alsa
            ${ALSA_INCLUDE_DIRS}
        )
        message(STATUS "Enabled ALSA support")
    endif()

    if(JACK_FOUND)
        list(APPEND SOURCES drivers/jack/AudioDriver_JACK.cpp)
        list(APPEND HEADERS drivers/jack/AudioDriver_JACK.h)
        add_definitions(-DHAVE_JACK_JACK_H)
        include_directories(
            ${PROJECT_SOURCE_DIR}/src/milkyplay/drivers/jack
            ${JACK_INCLUDE_DIRS}
        )
        message(STATUS "Enabled JACK support")
    endif()

    if(SDL2_FOUND)
        list(APPEND SOURCES drivers/sdl/AudioDriver_SDL.cpp)
        list(APPEND HEADERS drivers/sdl/AudioDriver_SDL.h)
        include_directories(
            ${PROJECT_SOURCE_DIR}/src/milkyplay/drivers/sdl
            ${SDL2_INCLUDE_DIRS}
        )
        message(STATUS "Enabled SDL2 support")
    endif()
endif()

add_definitions(-DMILKYTRACKER)

include_directories(
    # Include the CMake-generated version header from the build directory
    # (version string required when saving modules)
    ${PROJECT_BINARY_DIR}/src/tracker
    ${PROJECT_SOURCE_DIR}/src/milkyplay
)

add_library(milkyplay ${SOURCES} ${HEADERS})

if(APPLE)
    target_link_libraries(
        milkyplay
        ${COCOA_LIBRARY}
        ${CORE_AUDIO_LIBRARY}
        ${CORE_FOUNDATION_LIBRARY}
    )
elseif(WIN32)
    target_link_libraries(milkyplay winmm dsound)
else()
    target_link_libraries(milkyplay ${CMAKE_DL_LIBS} ${ALSA_LIBRARIES})
endif()
