#!/bin/sh
# Usage:
#   locally: ./build_rpi.sh compile alpine
#            ./build_rpi.sh compile raspbian 
#            and so on
#
#   thru docker/podman:
#            ./build_rpi.sh oci  # this will build all binaries

toolkit(){
  test -d rpi || git clone --depth=1 https://github.com/raspberrypi/tools rpi
  test -d || mkdir build
  export CC=$(pwd)/rpi/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-gcc
  export CPP=$(pwd)/rpi/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-c++
  cd build
  cmake -D CMAKE_C_COMPILER=$CC -D CMAKE_CXX_COMPILER=$CPP .. 
  # 32bit: /rpi/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-gcc   
  # 64bit: /rpi/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
}

compile(){
  echo $1 | grep alpine && {
    apk update
    apk add sdl2-dev cmake build-base
  }
  echo $1 | grep raspbian && {
    apt-get update
    apt-get install cmake libsdl2-dev
  }
  cd /src
  test -d build && rm -rf build 
  mkdir build
  cd build
  cmake ..
  make
}

oci(){
  OCI=$(which docker || which podman)
  which qemu-arm || sudo apt install -y qemu qemu-user-static qemu-user binfmt-support

  alpine(){
    IMAGES="arm64v8/alpine:3.18.2 arm32v7/alpine:3.18.2 arm32v6/alpine:3.18.2"
    $OCI run --rm --privileged multiarch/qemu-user-static:register --reset
    for IMG in $IMAGES; do
      set -x
      $OCI run --rm --privileged docker/binfmt:820fdd95a9972a5308930a2bdfb8573dd4447ad3 # enable arm
      $OCI run -it --privileged --rm --volume=$(pwd):/src $IMG src/build_rpi.sh compile $IMG
      cp build/src/tracker/milkytracker milkytracker.$( echo $IMG | sed 's/[\/:]/-/g')
    done
  }

  raspbian(){
    ARCHS="linux/arm/v6 linux/arm/v7 linux/arm64"
    for ARCH in $ARCHS; do
      set -x
      $OCI run --rm --privileged docker/binfmt:820fdd95a9972a5308930a2bdfb8573dd4447ad3 # enable arm
      $OCI run -it --privileged --rm --volume=$(pwd):/src --platform=$ARCH navikey/raspbian-buster src/build_rpi.sh compile raspbian 
      cp build/src/tracker/milkytracker milkytracker.$( echo $ARCH | sed 's/linux//g;s/[\/:]//g')
    done

  }
 
  raspbian
  #alpine
}

"$@"
