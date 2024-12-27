#!/bin/bash
# https://crascit.com/2016/04/03/scripting-cmake-builds/

# Default build type is Release if not specified
BUILD_TYPE=${1:-Release}

# Validate build type
if [[ "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "Debug" ]]; then
    echo "Invalid build type. Use 'Release' or 'Debug'"
    exit 1
fi

# Set build directory based on build type
BUILD_DIR="build"
CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [[ "$BUILD_TYPE" == "Debug" ]]; then
    BUILD_DIR="build-debug"
    # Add debug-specific flags
    CMAKE_FLAGS="$CMAKE_FLAGS -DCMAKE_CXX_FLAGS_DEBUG='-g3'"
fi

echo "Building MilkyTracker in $BUILD_TYPE mode in $BUILD_DIR..."

cmake -E make_directory $BUILD_DIR
pushd $BUILD_DIR
cmake $CMAKE_FLAGS ..
cmake --build . --config $BUILD_TYPE
cpack .
popd
