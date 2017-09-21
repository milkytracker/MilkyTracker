#!/bin/bash
# https://crascit.com/2016/04/03/scripting-cmake-builds/
platform='unknown'
unamestr=`uname`

if [[ "$unamestr" == 'Darwin' ]]; then
  platform='osx'
fi

if [[ "$platform" == 'osx' ]]; then
  pushd resources/pictures/docicons/osx
  wget https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/docerator/docerator-2.0.zip
  unzip -o docerator-2.0.zip -d docerator
  rm docerator-2.0.zip
  ./genicons.py
  popd
fi

cmake -E make_directory build
pushd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.7 ..
cmake --build . --config Release
cpack .
popd
