MilkyTracker - Cross-Platform XM Tracker
========================================

Notes on building MilkyTracker for Amiga68k
==============================

./build_gmake to make the makefiles

Configuration
=============
To compile with ixemul.library in mind run:
make config=ixemul_m68k-amigaos	

To compile with libnix just run:
make

Dependencies
============

To build MilkyTracker you will need the following development libraries:

SDL	- http://aminet.net/package/dev/misc/SDL-Amiga
libz	- http://aminet.net/package/dev/lib/zlib_68k
libjpeg	- http://aminet.net/package/dev/lib/libjpeg9_a68k

Note to package maintainers: MilkyTracker contains an internal copy of
libzzip that has been modified for use with MilkyTracker; An externally
linked libzzip will not work correctly (ZIP support will be broken).

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
