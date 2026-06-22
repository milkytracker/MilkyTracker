#!/bin/sh
# this script is for reproducability purposes in case the binary does not work

# install deps
tce-load -wi git.tcz SDL2-dev.tcz gcc.tcz cmake.tcz make.tcz glibc_base-dev.tcz linux-6.18_api_headers.tcz squashfs-tools.tcz

test -d milkytracker || {
  git clone https://github.com/milkytracker/milkytracker
}

set -x
set -e
cd milkytracker
export VERSION=$(git tag | tail -n1)
git submodule update --init
mkdir build
cd build
cmake ..
make

# package
test -d /tmp/package && rm -rf /tmp/package
mkdir -p /tmp/package/usr/bin
cp $0 /tmp/package/usr/bin/milkytracker.build # copy buildscript
cp src/tracker/milkytracker /tmp/package/usr/bin/.
strip /tmp/package/usr/bin/milkytracker
mksquashfs /tmp/package milkytracker-$VERSION.tcz
