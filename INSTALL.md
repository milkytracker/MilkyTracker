# Notes on building MilkyTracker

MilkyTracker now uses CMake to produce the build files. This replaces the
previously separately maintained Autotools, Visual Studio and Xcode project
files. Some historic platform-specific build files remain in the platforms
directory - these are untested.

The CMake configuration will auto-detect the platform it is building on, but can
be configured to force an SDL build if required by setting the `FORCESDL`
option.

**NOTE:** The use of `FORCESDL` is for developers only. No support is provided
for SDL builds on Windows/macOS.

# Dependencies

To build the **SDL port** of MilkyTracker (used for Linux) you will need the
following development libraries installed on your system:

- RtMidi/ALSA (optional, for Linux MIDI support)
- JACK (optional)
- SDL2

For all **non-Windows/macOS ports**, the decompression libs (optional):

- lhasa
- zlib
- zziplib

These are also provided as Git submodules, see below.

For example, on Ubuntu 18.04, you might run the following command to satisfy all
of the above dependencies:
```
$ sudo apt-get install libjack-dev liblhasa-dev \
librtmidi-dev libsdl2-dev libzzip-dev
```

Other distros might use different naming schemes for their packages. Please
search your distro's package manager for the above library names.

# Submodules (Windows/macOS only)

The following Git submodules are provided for linking into the Windows and macOS
binaries:

- RtAudio (Windows only)
- lhasa
- zlib (Windows only, macOS provides this)
- zziplib

To obtain these, `cd` to the MilkyTracker source directory and type:

```
$ git submodule update --init
```

# Building

As with most other CMake-based projects, building MilkyTracker requires two
steps:

1. Generating the build files for your desired build system using CMake
2. Invoking the generated build system

## Step 1
At the command line, step 1 is performed as follows:

```
$ mkdir build
$ cd build
$ cmake ..
```

On macOS, add `-GXcode` to the last command to generate an Xcode project instead
of using GNU make.

Note that you could also use the CMake GUI for this step instead of the command
line.

## Step 2
Step 2 varies depending on the target OS/build system.
On Linux and macOS (when using GNU make):

```
$ make
```

On Windows, you will probably have generated a Visual Studio project instead.
Simply open it with Visual Studio and compile the 'MilkyTracker' target.

On macOS, the same applies if you decided to generate an Xcode project using
`-GXcode`. Open the project up and build the 'MilkyTracker' target.

## macOS specific notes

In addition to CMake, you will need the following extra packages. The
recommended way of obtaining these is by using Homebrew or MacPorts.

- automake
- libtool
- xmlto

The correct way to build a release .DMG for macOS, including
special document icons, is to run the `build_macos.sh` script.
