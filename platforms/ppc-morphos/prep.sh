#!/bin/bash

# exit when any command fails
set -e

#set compiler params
export TARGET='ppc-morphos'
export SYSROOT=/opt/$TARGET
export PPC_CPU="-mhard-float"
export PPC_COMMON="-s -ffast-math -fomit-frame-pointer -noixemul"
export PPC_CFLAGS="${CFLAGS} ${PPC_CPU} ${PPC_COMMON}"
export PPC_CXXFLAGS="${CXXFLAGS} ${PPC_CPU} ${PPC_COMMON}"
export CURPATH="${PWD}"
export SUBMODULES="${CURPATH}/src/submodules"

# ZLIB
rm -rf ${SUBMODULES}/zlib/build
mkdir -p ${SUBMODULES}/zlib/build
cd ${SUBMODULES}/zlib/build
cmake .. -DCMAKE_INSTALL_PREFIX=${SYSROOT} -DPPC_COMMON="${PPC_COMMON} -O3 -fno-exceptions -w -DBIG_ENDIAN -DAMIGA -fpermissive -std=c++14"
cmake --build . --config Release --target install -- -j$(getconf _NPROCESSORS_ONLN)
cd ${SUBMODULES}

# SDL1.2
rm -rf powersdl_sdk*
wget http://aminet.net/dev/misc/powersdl_sdk.lha -O powersdl_sdk.lha
lha -x powersdl_sdk.lha
mkdir -p ${SYSROOT}/usr
cp -fvr powersdl_sdk/Developer/usr/local/* ${SYSROOT}/usr/
cd ${SUBMODULES}

# lhasa
cd ${SUBMODULES}/lhasa
./autogen.sh --host=${TARGET}
CFLAGS="${PPC_CFLAGS}" CXXFLAGS="${PPC_CXXFLAGS}" ./configure --disable-sdltest --disable-shared --enable-static --host=${TARGET} --prefix=${SYSROOT}
make -j$(getconf _NPROCESSORS_ONLN)
make install
cd ${SUBMODULES}

cd ${CURPATH}
