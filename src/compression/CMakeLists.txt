#
#  src/compression/CMakeLists.txt
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
    # AIFFWriter.m
    Decompressor.cpp
    DecompressorLZX.cpp
    DecompressorPP20.cpp
    DecompressorUMX.cpp
    PP20.cpp
    unlzx.cpp
)

set(
    HEADERS
    # AIFFWriter.h
    Decompressor.h
    DecompressorLZX.h
    DecompressorPP20.h
    DecompressorUMX.h
    PP20.h
    unlzx.h
)

# Under macOS and Windows, build sources from Git submodules if present
if(APPLE OR WIN32)
    set(LHASA_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/lhasa)
    set(ZLIB_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/zlib)
    set(ZZIPLIB_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/zziplib)
endif()

# macOS: Build submodules using their own build systems
if(APPLE)
    include(ExternalProject)

    # Collect compiler flags to pass to the external build systems
    set(EXTERNAL_C_FLAGS ${CMAKE_C_FLAGS})
    set(EXTERNAL_CXX_FLAGS ${CMAKE_CXX_FLAGS})

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(EXTERNAL_C_FLAGS "${EXTERNAL_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
        set(EXTERNAL_CXX_FLAGS "${EXTERNAL_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(EXTERNAL_C_FLAGS
            "${EXTERNAL_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}"
        )
        set(EXTERNAL_CXX_FLAGS
            "${EXTERNAL_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}"
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(EXTERNAL_C_FLAGS
            "${EXTERNAL_C_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO}"
        )
        set(EXTERNAL_CXX_FLAGS
            "${EXTERNAL_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        set(EXTERNAL_C_FLAGS
            "${SEXTERNAL_C_FLAGS} ${CMAKE_C_FLAGS_MINSIZEREL}"
        )
        set(EXTERNAL_CXX_FLAGS
            "${EXTERNAL_CXX_FLAGS} ${CMAKE_CXX_FLAGS_MINSIZEREL}"
        )
    endif()

    # Ensure our libraries match the deployment target of the main project
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        set(V_MIN_FLAG "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        set(EXTERNAL_C_FLAGS "${EXTERNAL_C_FLAGS} ${V_MIN_FLAG}")
        set(EXTERNAL_CXX_FLAGS "${EXTERNAL_CXX_FLAGS} ${V_MIN_FLAG}")
    endif()

    # Trim leading/trailing whitespace
    string(STRIP "${EXTERNAL_C_FLAGS}" EXTERNAL_C_FLAGS)
    string(STRIP "${EXTERNAL_CXX_FLAGS}" EXTERNAL_CXX_FLAGS)

    if(EXISTS ${LHASA_SUBMODULE_DIR}/lib/public/lhasa.h)
        message(STATUS "Found LHASA as a submodule")
        set(LHASA_DESTDIR ${CMAKE_BINARY_DIR}/src/submodules/lhasa)
        set(LHASA_FOUND ON CACHE INTERNAL "LHASA_FOUND")
        set(LHASA_INCLUDE_DIRS
            ${LHASA_DESTDIR}/include/liblhasa-1.0
            CACHE INTERNAL "LHASA_INCLUDE_DIRS"
        )
        set(LHASA_LIBRARIES
            ${LHASA_DESTDIR}/lib/liblhasa.a
            CACHE INTERNAL "LHASA_LIBRARIES"
        )

        ExternalProject_Add(lhasa
            PREFIX ${LHASA_DESTDIR}
            SOURCE_DIR ${LHASA_SUBMODULE_DIR}
            CONFIGURE_COMMAND autoreconf -i &&
            ./configure --prefix=${LHASA_DESTDIR}
            CFLAGS=${EXTERNAL_C_FLAGS} CXXFLAGS=${EXTERNAL_CXX_FLAGS}
            CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
            BUILD_COMMAND make -j
            INSTALL_COMMAND make -j install
            BUILD_IN_SOURCE ON
        )
    endif()

    if(EXISTS ${ZZIPLIB_SUBMODULE_DIR}/zzip/zzip.h)
        message(STATUS "Found ZZIPLIB as a submodule")

        # zziplib relies on xmlto to build its manpages, but if not found, its
        # build system fails at the docs target with an unhelpful error
        find_program(XMLTO xmlto)
        if (NOT XMLTO)
            message(WARNING
                "xmlto could not be found; building zziplib may fail"
            )
        endif()

        set(ZZIPLIB_DESTDIR ${CMAKE_BINARY_DIR}/src/submodules/zziplib)
        set(ZZIPLIB_FOUND ON CACHE INTERNAL "ZZIPLIB_FOUND")
        set(ZZIPLIB_INCLUDE_DIRS
            ${ZZIPLIB_DESTDIR}/include
            CACHE INTERNAL "ZZIPLIB_INCLUDE_DIRS"
        )
        set(ZZIPLIB_LIBRARIES
            ${ZZIPLIB_DESTDIR}/lib/libzzip.a
            CACHE INTERNAL "ZZIPLIB_LIBRARIES"
        )

        ExternalProject_Add(zziplib
            PREFIX ${ZZIPLIB_DESTDIR}
            SOURCE_DIR ${ZZIPLIB_SUBMODULE_DIR}
            CONFIGURE_COMMAND ./configure --prefix=${ZZIPLIB_DESTDIR}
            --without-debug --disable-dependency-tracking
            CFLAGS=${EXTERNAL_C_FLAGS} CXXFLAGS=${EXTERNAL_CXX_FLAGS}
            CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
            BUILD_COMMAND make -j
            INSTALL_COMMAND make -j install
            BUILD_IN_SOURCE ON
        )
    endif()
endif()

# Windows: Build zlib using its own build system, build lhasa/zziplib by
# rolling their sources into the compression library
# FIXME: The latter is a bit of a hack, really
if(WIN32)
    include(ExternalProject)

    if(EXISTS ${ZLIB_SUBMODULE_DIR}/zlib.h)
        message(STATUS "Found ZLIB as a submodule")

        set(ZLIB_DESTDIR ${CMAKE_BINARY_DIR}/src/submodules/zlib)
        set(ZLIB_FOUND ON CACHE INTERNAL "ZLIB_FOUND")
        set(ZLIB_INCLUDE_DIRS
            ${ZLIB_DESTDIR}/include
            CACHE INTERNAL "ZLIB_INCLUDE_DIRS"
        )

        # zlib adds a 'd' suffix to debug builds of the library
        set(ZLIB_LIBRARIES
            debug ${ZLIB_DESTDIR}/lib/zlibstaticd.lib
            optimized ${ZLIB_DESTDIR}/lib/zlibstatic.lib
            CACHE INTERNAL "ZLIB_LIBRARIES"
        )

        ExternalProject_Add(zlib
            PREFIX ${ZLIB_DESTDIR}
            SOURCE_DIR ${ZLIB_SUBMODULE_DIR}
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
            -DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
            -DCMAKE_C_FLAGS_RELWITHDEBINFO=${CMAKE_C_FLAGS_RELWITHDEBINFO}
            -DCMAKE_C_FLAGS_MINSIZEREL=${CMAKE_C_FLAGS_MINSIZEREL}
        )
    endif()

    if(EXISTS ${LHASA_SUBMODULE_DIR}/lib/public/lhasa.h)
        message(STATUS "Found LHASA as a submodule")
        set(LHASA_FOUND ON)
        set(
            LHASA_INCLUDE_DIRS
            ${LHASA_SUBMODULE_DIR}/lib
            ${LHASA_SUBMODULE_DIR}/lib/public
        )
        set(
            LHASA_SOURCES
            ${LHASA_SUBMODULE_DIR}/lib/crc16.c
            ${LHASA_SUBMODULE_DIR}/lib/ext_header.c
            ${LHASA_SUBMODULE_DIR}/lib/lh1_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/lh5_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/lh6_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/lh7_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_arch_unix.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_arch_win32.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_basic_reader.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_endian.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_file_header.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_input_stream.c
            ${LHASA_SUBMODULE_DIR}/lib/lha_reader.c
            ${LHASA_SUBMODULE_DIR}/lib/lhx_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/lz5_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/lzs_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/macbinary.c
            ${LHASA_SUBMODULE_DIR}/lib/null_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/pm1_decoder.c
            ${LHASA_SUBMODULE_DIR}/lib/pm2_decoder.c
        )
        set(LHASA_HEADERS
            ${LHASA_SUBMODULE_DIR}/lib/crc16.h
            ${LHASA_SUBMODULE_DIR}/lib/ext_header.h
            ${LHASA_SUBMODULE_DIR}/lib/lha_arch.h
            ${LHASA_SUBMODULE_DIR}/lib/lha_basic_reader.h
            ${LHASA_SUBMODULE_DIR}/lib/lha_decoder.h
            ${LHASA_SUBMODULE_DIR}/lib/lha_endian.h
            ${LHASA_SUBMODULE_DIR}/lib/lha_file_header.h
            ${LHASA_SUBMODULE_DIR}/lib/lha_input_stream.h
            ${LHASA_SUBMODULE_DIR}/lib/macbinary.h
            ${LHASA_SUBMODULE_DIR}/lib/public/lha_decoder.h
            ${LHASA_SUBMODULE_DIR}/lib/public/lha_file_header.h
            ${LHASA_SUBMODULE_DIR}/lib/public/lha_input_stream.h
            ${LHASA_SUBMODULE_DIR}/lib/public/lha_reader.h
            ${LHASA_SUBMODULE_DIR}/lib/public/lhasa.h
        )
        source_group(lhasa FILES ${LHASA_SOURCES} ${LHASA_HEADERS})
        list(APPEND SOURCES ${LHASA_SOURCES})
        list(APPEND HEADERS ${LHASA_HEADERS})
    endif()

    if(EXISTS ${ZZIPLIB_SUBMODULE_DIR}/zzip/zzip.h)
        message(STATUS "Found ZZIPLIB as a submodule")
        set(ZZIPLIB_FOUND ON)
        set(ZZIPLIB_INCLUDE_DIRS ${ZZIPLIB_SUBMODULE_DIR})
        set(ZZIPLIB_SOURCES
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/dir.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/err.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/fetch.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/file.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/fseeko.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/info.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/memdisk.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/mmapped.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/plugin.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/stat.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/write.c
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/zip.c
        )
        set(ZZIPLIB_HEADERS
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/__debug.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/__dirent.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/__fnmatch.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/__hints.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/__mmap.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/_msvc.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/autoconf.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/conf.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/fetch.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/file.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/format.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/fseeko.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/info.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/lib.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/memdisk.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/mmapped.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/plugin.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/stdint.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/types.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/write.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/zzip.h
            ${ZZIPLIB_SUBMODULE_DIR}/zzip/zzip32.h
        )
        source_group(zziplib FILES ${ZZIPLIB_SOURCES} ${ZZIPLIB_HEADERS})
        list(APPEND SOURCES ${ZZIPLIB_SOURCES})
        list(APPEND HEADERS ${ZZIPLIB_HEADERS})
    endif()
endif()

# Optional decompressors
if(LHASA_FOUND)
    message(STATUS "Enabled LHA decompressor")
    list(APPEND SOURCES DecompressorLHA.cpp)
    list(APPEND HEADERS DecompressorLHA.h)
    include_directories(${LHASA_INCLUDE_DIRS})
else()
    message("LHA decompressor disabled (lhasa unavailable)")
endif()

if(ZLIB_FOUND)
    message(STATUS "Enabled GZIP decompressor")
    list(APPEND SOURCES DecompressorGZIP.cpp)
    list(APPEND HEADERS DecompressorGZIP.h)
    include_directories(${ZLIB_INCLUDE_DIRS})
else()
    message("GZIP decompressor disabled (zlib unvailable)")
endif()

if(ZLIB_FOUND AND ZZIPLIB_FOUND)
    message(STATUS "Enabled ZIP decompressor")
    list(APPEND SOURCES DecompressorZIP.cpp ZipExtractor.cpp zziplib/MyIO.cpp)
    list(APPEND HEADERS DecompressorZIP.h ZipExtractor.h zziplib/MyIO.h)
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/compression/zziplib
        ${ZZIPLIB_INCLUDE_DIRS}
    )
else()
    if(NOT ZLIB_FOUND AND NOT ZZIPLIB_FOUND)
        message("ZIP decompressor disabled (zlib and zziplib unavailble)")
    elseif(ZZIPLIB_FOUND)
        message("ZIP decompressor disabled (zlib unavailable)")
    else()
        message("ZIP decompressor disabled (zziplib unavailable)")
    endif()
endif()

# Define this library as an object library; the objects will not be archived
add_library(compression OBJECT ${SOURCES} ${HEADERS})

# Ensure dependencies are built before this library
if(APPLE)
    if(LHASA_FOUND)
        add_dependencies(compression lhasa)
    endif()
    if(ZZIPLIB_FOUND)
        add_dependencies(compression zziplib)
    endif()
endif()

if(WIN32)
    if(ZLIB_FOUND)
        add_dependencies(compression zlib)
    endif()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src/milkyplay
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
)
