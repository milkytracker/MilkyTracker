#!/bin/bash
# https://crascit.com/2016/04/03/scripting-cmake-builds/
unamestr=`uname`

if [[ "$unamestr" != 'Darwin' ]]; then
  echo "Platform is not macOS but $unamestr"
  exit 1
fi

pushd resources/pictures/docicons/osx
curl -O https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/docerator/docerator-2.0.zip
unzip -o docerator-2.0.zip -d docerator
rm docerator-2.0.zip
./genicons.py
popd

cmake -E make_directory build
pushd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cpack .
popd
