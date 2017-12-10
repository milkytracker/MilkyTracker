MilkyTracker - Cross-Platform XM Tracker
========================================

Notes on building MilkyTracker for Amiga platforms (AmigaOS 3.x, AmigaOS 4.x, WarpOS, MorphOS and AROS)
==============================

./build_gmake to make the makefiles

Building
=============
To compile for different platforms, use make config=release_*platform*.
Currently available platforms:
* m68k-amigaos
* ppc-amigaos

The default is to compile *m68k-amigaos*:
make

Dependencies
============

To build MilkyTracker you will need the following development libraries:
libSDL1.2	- https://github.com/HenrykRichter/libSDL12_Amiga68k

========================================

[![Travis Build Status](https://travis-ci.org/milkytracker/MilkyTracker.svg?branch=master)](https://travis-ci.org/milkytracker/MilkyTracker)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/milkytracker/MilkyTracker?branch=master&svg=true)](https://ci.appveyor.com/project/Deltafire/milkytracker)

MilkyTracker is an multi-platform music application for creating .MOD
and .XM module files. It attempts to recreate the module replay and
user experience of the popular DOS program Fasttracker II, with
special playback modes available for improved Amiga ProTracker 2/3
compatibility.

Refer to http://milkytracker.titandemo.org/?about for further details.

Please read the file [INSTALL.md][] for installation instructions.

The [docs/readme_unix][] file contains notes specific to the SDL port
of MilkyTracker.

[INSTALL.md]:INSTALL.md
[docs/readme_unix]:docs/readme_unix
