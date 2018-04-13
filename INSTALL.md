# Notes on building MilkyTracker

MilkyTracker now uses CMake to produce the build files. This replaces the
previously separately maintained Autotools, Visual Studio and Xcode project
files. Some historic platform-specific build files remain in the platforms
directory - these are untested.

The CMake configuration will auto-detect the platform it is building on, but can
be configured to force an SDL build if required by setting the `FORCESDL`
option.


# Dependencies

To build the SDL port of MilkyTracker you will need the following development
libraries installed on your system:

- ALSA (optional, needed for Linux MIDI support)
- RtMidi (for Linux MIDI support)
- JACK (optional)
- SDL2

For all non-Windows/macOS ports, the decompression libs (optional):

- lhasa
- zlib
- zziplib

These are also provided as Git submodules, see below.


# Submodules

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

On Windows there is also a SDL2 version, if you want to build this please scroll down.

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

The correct way to build a release .DMG for macOS 10.7 and above, including
special document icons, is as follows:

```
$ pushd resources/pictures/docicons/osx
$ wget https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/docerator/docerator-2.0.zip
$ unzip docerator-2.0.zip -d docerator
$ rm docerator-2.0.zip
$ ./genicons.py
$ popd
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.7 ..
$ make
$ cpack .
```

## Windows SDL2 specific notes

In addition to CMake you will need SDL2, you can download it from this link:

https://www.libsdl.org/release/SDL2-devel-2.0.8-VC.zip 

At the time of writing this was the latest version, feel free to look for a newer one

Please extract the files from the include folder to the following location on your Windows PC

C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include\SDL2 

In my example 14.0 for Visual Studio 2015, after this extract the library files *.lib to this location

C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib

x64 libraries are going to the amd64 subfolder inside the lib folder and x86 libraries are going
directly inside the lib folder, be carefull !

The corresponding SDL2.dll to place to the same location like the executeable file 

At the command line you can generate the Visual Studio project files with

```
$ mkdir build
$ cd build
$ cmake .. -DFORCESDL=ON -DSDL2_INCLUDE_DIR="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include\SDL2"
```

If you build for example the RelWithDebInfo version of MilkyTracker you will see a console message with
available audio devices when you start the executeable binary

```
SDL: Available audio drivers: wasapi directsound winmm disk dummy
```

From the SDL2 Audio API documentation:

**_The presence of a driver in this list does not mean it will function, it just means SDL is capable of
interacting with that interface. For example, a build of SDL might have esound support,
but if there's no esound server available, SDL's esound driver would fail if used._**

So if Audio is not working when you first start MilkyTracker, you can set the Audio driver that has to
be used by SDL2 with the environment variable

_SDL_AUDIODRIVER_

```
set SDL_AUDIODRIVER=winmm
```

it can also be set permanently from within Windows, if you haven't done this before, please search the Internet