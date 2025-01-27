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

add_library(milkyplay STATIC
    # Sources
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

    # Headers
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

target_include_directories(milkyplay
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        # Include the CMake-generated version header from the build directory
        # (version string required when saving modules)
        ${PROJECT_BINARY_DIR}/src/tracker
)

# Add platform-specific sources, include paths, definitions and link libraries
if(APPLE)
    target_sources(milkyplay
        PRIVATE
            # Sources
            drivers/osx/AudioDriver_COREAUDIO.mm

            # Headers
            drivers/osx/AudioDriver_COREAUDIO.h
    )
    target_include_directories(milkyplay
        PRIVATE
            drivers/osx
    )
    target_link_libraries(milkyplay
        PRIVATE
            ${COCOA_LIBRARY}
            ${CORE_AUDIO_LIBRARY}
            ${CORE_FOUNDATION_LIBRARY}
    )
    message(STATUS "Enabled Core Audio support")
elseif(WIN32)
    # Basic WaveOut support
    target_sources(milkyplay
        PRIVATE
            # Sources
            drivers/windows/AudioDriver_MMSYSTEM.cpp

            # Headers
            drivers/windows/AudioDriver_MMSYSTEM.h
    )

    target_include_directories(milkyplay
        PRIVATE
            drivers/windows
    )

    message(STATUS "Enabled WaveOut support")

    # Build sources from RtAudio Git submodule if present
    set(RTAUDIO_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/rtaudio)
    if(EXISTS ${RTAUDIO_SUBMODULE_DIR}/CMakeLists.txt)
        message(STATUS "Enabled RtAudio support")

        set(RTAUDIO_BUILD_SHARED_LIBS OFF)
        set(RTAUDIO_BUILD_TESTING OFF)
        add_subdirectory(${RTAUDIO_SUBMODULE_DIR} ${CMAKE_BINARY_DIR}/src/submodules/rtaudio EXCLUDE_FROM_ALL)

        target_sources(milkyplay
            PRIVATE
                # Sources
                drivers/generic/AudioDriver_RTAUDIO.cpp
                drivers/generic/RtAudio4Impl.cpp

                # Headers
                drivers/generic/AudioDriver_RTAUDIO.h
        )

        target_include_directories(milkyplay
            PRIVATE
                drivers/generic
                ${RTAUDIO_SUBMODULE_DIR}
        )

        target_link_libraries(milkyplay PRIVATE rtaudio)

        # Place targets into IDE subfolder
        set_target_properties(
            rtaudio
            uninstall
            Continuous
            Experimental
            Nightly
            NightlyMemoryCheck
            PROPERTIES FOLDER "Externals/RtAudio"
        )
    else()
        target_compile_definitions(milkyplay PRIVATE -D__SKIPRTAUDIO__)
        message("RtAudio support disabled (RtAudio unavailable)")
    endif()
    target_link_libraries(milkyplay PRIVATE winmm dsound)
elseif(HAIKU)
    target_sources(milkyplay
        PRIVATE
            # Sources
            drivers/haiku/AudioDriver_Haiku.cpp

            # Headers
            drivers/haiku/AudioDriver_Haiku.h
    )
    target_include_directories(milkyplay
        PRIVATE
            drivers/haiku
    )
    target_link_libraries(milkyplay
        PRIVATE
            media
    )
    message(STATUS "Enabled Haiku Audio support")
else()
    target_compile_definitions(milkyplay PRIVATE -DDRIVER_UNIX)

    if(ALSA_FOUND)
        target_sources(milkyplay PRIVATE
            # Sources
            drivers/alsa/AudioDriver_ALSA.cpp

            # Headers
            drivers/alsa/AudioDriver_ALSA.h
        )
        target_compile_definitions(milkyplay PUBLIC -DHAVE_LIBASOUND)
        target_include_directories(milkyplay
            PUBLIC
                ${ALSA_INCLUDE_DIRS}
            PRIVATE
                drivers/alsa
        )
        target_link_libraries(milkyplay PUBLIC ${ALSA_LIBRARIES})
        message(STATUS "Enabled ALSA support")
    endif()

    if(JACK_FOUND)
        target_sources(milkyplay PRIVATE
            # Sources
            drivers/jack/AudioDriver_JACK.cpp

            # Headers
            drivers/jack/AudioDriver_JACK.h
        )
        target_compile_definitions(milkyplay PRIVATE -DHAVE_JACK_JACK_H)
        target_include_directories(milkyplay PRIVATE
            drivers/jack
            ${JACK_INCLUDE_DIRS}
        )
        target_link_libraries(milkyplay PUBLIC ${CMAKE_DL_LIBS})
        message(STATUS "Enabled JACK support")
    endif()

	if(PIPEWIRE_FOUND)
		target_sources(milkyplay PRIVATE
			# Source
			drivers/pipewire/AudioDriver_Pipewire.cpp

			# Header
			drivers/pipewire/AudioDriver_Pipewire.h
		)
		target_compile_definitions(milkyplay PRIVATE -DHAVE_PIPEWIRE)
		target_include_directories(milkyplay PRIVATE
			drivers/pipewire
			${PIPEWIRE_INCLUDE_DIRS}
		)
		target_link_libraries(milkyplay PUBLIC ${PIPEWIRE_LIBRARIES})
		message(STATUS "Enabled Pipewire support")
	endif()

    if(SDL2_FOUND)
        target_sources(milkyplay PRIVATE
            # Sources
            drivers/sdl/AudioDriver_SDL.cpp

            # Headers
            drivers/sdl/AudioDriver_SDL.h
        )
        target_include_directories(milkyplay PRIVATE
            drivers/sdl
            ${SDL2_INCLUDE_DIRS}
        )

        if(TARGET SDL2::SDL2)
            # If the distro built SDL2 with CMake, we can just link to an exported target
            target_link_libraries(milkyplay PUBLIC SDL2::SDL2)
        else()
            # Otherwise we need to do things the old-fashioned way for compatibility
            target_include_directories(milkyplay PUBLIC ${SDL2_INCLUDE_DIRS})
            target_link_libraries(milkyplay PUBLIC ${SDL2_LIBRARIES})
        endif()
        message(STATUS "Enabled SDL2 support")
    endif()
endif()
