#!/bin/bash
# https://crascit.com/2016/04/03/scripting-cmake-builds/

# Set defaults
BUILD_TYPE="Release"
BUILD_DMG=ON

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        Release|Debug)
            BUILD_TYPE="$1"
            ;;
        --no-dmg)
            BUILD_DMG=OFF
            ;;
        *)
            echo "Unknown argument: $1"
            echo "Usage: $0 [Release|Debug] [--no-dmg]"
            exit 1
            ;;
    esac
    shift
done

# Set build directory based on build type
BUILD_DIR="build"
CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_DMG=$BUILD_DMG"

if [[ "$BUILD_TYPE" == "Debug" ]]; then
    BUILD_DIR="build-debug"
    # Add debug-specific flags
    CMAKE_FLAGS="$CMAKE_FLAGS -DCMAKE_CXX_FLAGS_DEBUG='-g3'"
fi

echo "Building MilkyTracker in $BUILD_TYPE mode in $BUILD_DIR..."
if [ "$BUILD_DMG" = "OFF" ]; then
    echo "DMG generation: OFF"
fi

cmake -E make_directory $BUILD_DIR
pushd $BUILD_DIR
cmake $CMAKE_FLAGS ..
cmake --build . --config $BUILD_TYPE
cpack .
popd
