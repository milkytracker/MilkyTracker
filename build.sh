#!/bin/bash
# https://crascit.com/2016/04/03/scripting-cmake-builds/

cmake -E make_directory build
pushd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cpack .
popd
