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

# Under Windows, build sources from Git submodules if present
if(WIN32)
    set(LHASA_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/lhasa)
    set(ZLIB_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/zlib)
    set(ZZIPLIB_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/src/submodules/zziplib)

    if(EXISTS ${LHASA_SUBMODULE_DIR}/lib/public/lhasa.h)
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

    if(EXISTS ${ZLIB_SUBMODULE_DIR}/zlib.h)
        set(ZLIB_FOUND ON)
        set(ZLIB_INCLUDE_DIRS ${ZLIB_SUBMODULE_DIR})
        set(ZLIB_SOURCES
            ${ZLIB_SUBMODULE_DIR}/adler32.c
            ${ZLIB_SUBMODULE_DIR}/compress.c
            ${ZLIB_SUBMODULE_DIR}/crc32.c
            ${ZLIB_SUBMODULE_DIR}/deflate.c
            ${ZLIB_SUBMODULE_DIR}/gzclose.c
            ${ZLIB_SUBMODULE_DIR}/gzlib.c
            ${ZLIB_SUBMODULE_DIR}/gzread.c
            ${ZLIB_SUBMODULE_DIR}/gzwrite.c
            ${ZLIB_SUBMODULE_DIR}/infback.c
            ${ZLIB_SUBMODULE_DIR}/inffast.c
            ${ZLIB_SUBMODULE_DIR}/inflate.c
            ${ZLIB_SUBMODULE_DIR}/inftrees.c
            ${ZLIB_SUBMODULE_DIR}/trees.c
            ${ZLIB_SUBMODULE_DIR}/uncompr.c
            ${ZLIB_SUBMODULE_DIR}/zutil.c
        )
        set(ZLIB_HEADERS
            ${ZLIB_SUBMODULE_DIR}/crc32.h
            ${ZLIB_SUBMODULE_DIR}/deflate.h
            ${ZLIB_SUBMODULE_DIR}/gzguts.h
            ${ZLIB_SUBMODULE_DIR}/inffast.h
            ${ZLIB_SUBMODULE_DIR}/inffixed.h
            ${ZLIB_SUBMODULE_DIR}/inflate.h
            ${ZLIB_SUBMODULE_DIR}/inftrees.h
            ${ZLIB_SUBMODULE_DIR}/trees.h
            ${ZLIB_SUBMODULE_DIR}/zlib.h
            ${ZLIB_SUBMODULE_DIR}/zutil.h
        )
        source_group(zlib FILES ${ZLIB_SOURCES} ${ZLIB_HEADERS})
        list(APPEND SOURCES ${ZLIB_SOURCES})
        list(APPEND HEADERS ${ZLIB_HEADERS})
    endif()

    if(EXISTS ${ZZIPLIB_SUBMODULE_DIR}/zzip/zzip.h)
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
        message("ZIP decompressor disabled (zlib unvailable)")
    else()
        message("ZIP decompressor disabled (zziplib unvailable)")
    endif()
endif()

# Define this library as an object library; the objects will not be archived
add_library(compression OBJECT ${SOURCES} ${HEADERS})

include_directories(
    ${PROJECT_SOURCE_DIR}/src/milkyplay
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
)
