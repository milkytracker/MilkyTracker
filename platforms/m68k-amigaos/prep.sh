#!/bin/bash

# exit when any command fails
set -e

#set compiler params
export TARGET='m68k-amigaos'
export SYSROOT=/opt/$TARGET
export M68K_CPU="-m68040 -mhard-float"
export M68K_COMMON="-s -ffast-math -fomit-frame-pointer -noixemul"
export M68K_CFLAGS="${CFLAGS} ${M68K_CPU} ${M68K_COMMON}"
export M68K_CXXFLAGS="${CXXFLAGS} ${M68K_CPU} ${M68K_COMMON}"
export CURPATH="${PWD}"
export SUBMODULES="${CURPATH}/src/submodules"

# ZLIB
rm -rf ${SUBMODULES}/zlib/build
mkdir -p ${SUBMODULES}/zlib/build
cd ${SUBMODULES}/zlib/build
cmake .. -DCMAKE_INSTALL_PREFIX=${SYSROOT} -DM68K_CPU=68040 -DM68K_FPU=hard -DM68K_COMMON="${M68K_COMMON} -O3 -fno-exceptions -w -DBIG_ENDIAN -DAMIGA -fpermissive -std=c++14"
cmake --build . --config Release --target install -- -j$(getconf _NPROCESSORS_ONLN)
cd ${SUBMODULES}

# SDL1.2
rm -rf SDL
git clone https://github.com/AmigaPorts/SDL.git SDL
cd SDL
git checkout SDL-1.2-AmigaOS3
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${SYSROOT} -DM68K_CPU=68040 -DM68K_FPU=hard -DM68K_COMMON="${M68K_COMMON} -O3 -fno-exceptions -w -DBIG_ENDIAN -DAMIGA -fpermissive -std=c++14"
cmake --build . --config Release --target install -- -j$(getconf _NPROCESSORS_ONLN)
cd ${SUBMODULES}

# Zziplib
cd ${SUBMODULES}/zziplib
CFLAGS="${M68K_CFLAGS}" CXXFLAGS="${M68K_CXXFLAGS}" ./configure --disable-sdltest --disable-shared --enable-static --host=${TARGET} --prefix=${SYSROOT}
make -j$(getconf _NPROCESSORS_ONLN)
make install
cd ${SUBMODULES}

# lhasa
cd ${SUBMODULES}/lhasa
./autogen.sh --host=${TARGET}
CFLAGS="${M68K_CFLAGS}" CXXFLAGS="${M68K_CXXFLAGS}" ./configure --disable-sdltest --disable-shared --enable-static --host=${TARGET} --prefix=${SYSROOT}
make -j$(getconf _NPROCESSORS_ONLN)
make install
cd ${SUBMODULES}

cd ${CURPATH}
