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
mkdir -p ${SUBMODULES}/zlib/build
cd ${SUBMODULES}/zlib/build
cmake .. -DCMAKE_INSTALL_PREFIX=${SYSROOT} -DM68K_CPU=68040 -DM68K_FPU=hard -DM68K_COMMON="${M68K_COMMON} -O3 -fno-exceptions -w -DBIG_ENDIAN -DAMIGA -fpermissive -std=c++14"
cmake --build . --config Release --target install -- -j$(getconf _NPROCESSORS_ONLN)
cd ${SUBMODULES}

# SDL1.2
wget https://github.com/AmigaPorts/libSDL12/archive/master.tar.gz -O SDL-1.2.tar.gz
tar -xvf SDL-1.2.tar.gz
cd libSDL12-master
make PREFX=${SYSROOT} PREF=${SYSROOT} -j$(getconf _NPROCESSORS_ONLN)
cp -fvr libSDL.a ${SYSROOT}/lib
cp -fvr include/* ${SYSROOT}/include/
cd ${SUBMODULES}

# lhasa
#wget https://github.com/SDL-mirror/SDL_mixer/archive/SDL-1.2.tar.gz -O SDL_mixer-SDL-1.2.tar.gz
#tar -xvf SDL_mixer-SDL-1.2.tar.gz
cd ${SUBMODULES}/lhasa
./autogen.sh --host=${TARGET}
CFLAGS="${M68K_CFLAGS}" CXXFLAGS="${M68K_CXXFLAGS}" ./configure --disable-sdltest --disable-shared --enable-static --host=${TARGET} --prefix=${SYSROOT}
make -j$(getconf _NPROCESSORS_ONLN)
make install
cd ${SUBMODULES}

cd ${CURPATH}
