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
    if(EXISTS ${PROJECT_SOURCE_DIR}/src/compression/lhasa/lib/public/lhasa.h)
        set(LHASA_FOUND ON)
        set(LHASA_INCLUDE_DIRS lhasa/lib lhasa/lib/public)
        set(LHASA_SOURCES
            lhasa/lib/crc16.c
            lhasa/lib/ext_header.c
            lhasa/lib/lh1_decoder.c
            lhasa/lib/lh5_decoder.c
            lhasa/lib/lh6_decoder.c
            lhasa/lib/lh7_decoder.c
            lhasa/lib/lha_arch_unix.c
            lhasa/lib/lha_arch_win32.c
            lhasa/lib/lha_basic_reader.c
            lhasa/lib/lha_decoder.c
            lhasa/lib/lha_endian.c
            lhasa/lib/lha_file_header.c
            lhasa/lib/lha_input_stream.c
            lhasa/lib/lha_reader.c
            lhasa/lib/lhx_decoder.c
            lhasa/lib/lz5_decoder.c
            lhasa/lib/lzs_decoder.c
            lhasa/lib/macbinary.c
            lhasa/lib/null_decoder.c
            lhasa/lib/pm1_decoder.c
            lhasa/lib/pm2_decoder.c
        )
        set(LHASA_HEADERS
            lhasa/lib/crc16.h
            lhasa/lib/ext_header.h
            lhasa/lib/lha_arch.h
            lhasa/lib/lha_basic_reader.h
            lhasa/lib/lha_decoder.h
            lhasa/lib/lha_endian.h
            lhasa/lib/lha_file_header.h
            lhasa/lib/lha_input_stream.h
            lhasa/lib/macbinary.h
            lhasa/lib/public/lha_decoder.h
            lhasa/lib/public/lha_file_header.h
            lhasa/lib/public/lha_input_stream.h
            lhasa/lib/public/lha_reader.h
            lhasa/lib/public/lhasa.h
        )
        source_group(lhasa FILES ${LHASA_SOURCES} ${LHASA_HEADERS})
        list(APPEND SOURCES ${LHASA_SOURCES})
        list(APPEND HEADERS ${LHASA_HEADERS})
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/src/compression/zlib/generic/zlib.h)
        set(ZLIB_FOUND ON)
        set(ZLIB_INCLUDE_DIRS zlib/generic)
        set(ZLIB_SOURCES
            zlib/generic/adler32.c
            zlib/generic/compress.c
            zlib/generic/crc32.c
            zlib/generic/deflate.c
            zlib/generic/gzclose.c
            zlib/generic/gzlib.c
            zlib/generic/gzread.c
            zlib/generic/gzwrite.c
            zlib/generic/infback.c
            zlib/generic/inffast.c
            zlib/generic/inflate.c
            zlib/generic/inftrees.c
            zlib/generic/trees.c
            zlib/generic/uncompr.c
            zlib/generic/zutil.c
        )
        set(ZLIB_HEADERS
            zlib/generic/crc32.h
            zlib/generic/deflate.h
            zlib/generic/gzguts.h
            zlib/generic/inffast.h
            zlib/generic/inffixed.h
            zlib/generic/inflate.h
            zlib/generic/inftrees.h
            zlib/generic/trees.h
            zlib/generic/zlib.h
            zlib/generic/zutil.h
        )
        source_group(zlib FILES ${ZLIB_SOURCES} ${ZLIB_HEADERS})
        list(APPEND SOURCES ${ZLIB_SOURCES})
        list(APPEND HEADERS ${ZLIB_HEADERS})
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/src/compression/zziplib/generic/zzip/zzip.h)
        set(ZZIPLIB_FOUND ON)
        set(ZZIPLIB_INCLUDE_DIRS zziplib/generic)
        set(ZZIPLIB_SOURCES
            zziplib/generic/zzip/dir.c
            zziplib/generic/zzip/err.c
            zziplib/generic/zzip/fetch.c
            zziplib/generic/zzip/file.c
            zziplib/generic/zzip/fseeko.c
            zziplib/generic/zzip/info.c
            zziplib/generic/zzip/memdisk.c
            zziplib/generic/zzip/mmapped.c
            zziplib/generic/zzip/plugin.c
            zziplib/generic/zzip/stat.c
            zziplib/generic/zzip/write.c
            zziplib/generic/zzip/zip.c
        )
        set(ZZIPLIB_HEADERS
            zziplib/generic/zzip/__debug.h
            zziplib/generic/zzip/__dirent.h
            zziplib/generic/zzip/__fnmatch.h
            zziplib/generic/zzip/__hints.h
            zziplib/generic/zzip/__mmap.h
            zziplib/generic/zzip/_msvc.h
            zziplib/generic/zzip/autoconf.h
            zziplib/generic/zzip/conf.h
            zziplib/generic/zzip/fetch.h
            zziplib/generic/zzip/file.h
            zziplib/generic/zzip/format.h
            zziplib/generic/zzip/fseeko.h
            zziplib/generic/zzip/info.h
            zziplib/generic/zzip/lib.h
            zziplib/generic/zzip/memdisk.h
            zziplib/generic/zzip/mmapped.h
            zziplib/generic/zzip/plugin.h
            zziplib/generic/zzip/stdint.h
            zziplib/generic/zzip/types.h
            zziplib/generic/zzip/write.h
            zziplib/generic/zzip/zzip.h
            zziplib/generic/zzip/zzip32.h
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
