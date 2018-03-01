# MilkyTracker ChangeLog

## 24/02/2018 (v1.02):

### What's new:
*  99 channel MOD support
*  Alternative keybindings for inc/dec Row Insert

### Bugs fixed:
*  Fix Lxx command to also set panning envelope if volume sustain point is enabled.
*  Infinite loop when processing 8 digit hexadecimal numbers
*  SDL crash with -orientation command line parameter
*  SDL mouse drag issue
*  Fix fine global volume slide down handling
*  Various loader memory corruption bugs

## 29/07/2017 (v1.01):

### What's new:
*  Channel limit increased to 128 channels
*  New sample editor filters:
   *  Phase modulation
   *  AM & FM modulation
   *  Selective equaliser
*  OSX: Insert key mapped to F13
*  GUI: Clickable checkbox labels
*  GNU: Use GNUInstallDirs

### Bugs fixed:
*  Errant characters inserts when selecting patterns using ALT + cursor keys
*  Windows: Window size now takes scale factor into consideration
*  Build: Default value used for PATH_MAX if not set (for GNU/Hurd)
*  Instrument editor:
   *  Prevent point inserts overlapping existing points
   *  Reset cursor when changing presets or pasting envelopes
*  PT3 octave limit not reset when switching playback mode back to FT2
*  Sample editor:
   *  Invert function now works correctly
   *  Equaliser constants tweaked
   *  No longer crashes when generating silence with no selection active
   *  Noise generator on 64-bit systems
*  OSX: Core audio sample-rate switching
*  GUS frequencies lower than lowest note
*  Off-by-semitone issue in GUS patch loader
*  Corrupt display with window sizes that aren't multiples of 4
*  Windows: Window position gradually moving off-screen on restarts

## 11/03/2017 (v1.00.00):

### What's new:

*   5 new customisable colours
*   X-Y mouse scrolling support
*   FT2 panning law
*   MilkyTracker version number stored in .xm file
*   Upgrade to SDL 2
*   SDL2: Window scaling
*   OSX: Deprecated Carbon interface replaced with Cocoa
*   OSX: Drag and drop support
*   OSX: Support audio devices with >2 channels
*   OSX: Add custom document icons for supported file types
*   Posix: Follow XFG basedir spec for config file locations
*   Windows: Use system temp and config directories
*   Sample offset tool/preview ability (hold Ctrl/Cmd in sample editor waveform, left click for preview)
*   OSX: Add selectable filetype filter to file dialog
*   OSX: Native MIDI support
*   OSX: Allow selection of audio output device
*   Windows: WASAPI option for RtAudio driver
*   Shortcut keys to increment/decrement instrument number
*   LHA compressed modules load support
*   Moved to common CMake build system across all platforms
*   3rd-party libraries removed from source tree
*   Horizontal mouse/touchpad scrolling
*   Mouse middle-click to solo channel
*   ..and more, see ([commit log](https://github.com/milkytracker/MilkyTracker/commits/master)) for details

### Bugs Fixed:

*   Segfault when loading <4 byte samples
*   Samples incorrectly offset when exporting .mod files
*   libalsa linker errors on some systems
*   .mod export: Don't zero first word of looping samples
*   Decompress files despite possible mis-identification
*   Windows: Determine window dimensions based on client area
*   Update view if paste changes pattern length
*   SDL: Unresponsive GUI in some situations
*   16-bit sample loading bug
*   Windows: WaveOut crashes and deadlocks
*   Windows: Incorrect behaviour when mouse is dragged outside main window
*   Reversed Oktalyzer portamento effects
*   OSX: Sound driver samplerate mismatch under some circumstances
*   OSX: Miscellanous audio driver reliability problems
*   Windows: MIDI crash on 64-bit build
*   Various compiler warnings (many more remaining!)
*   Windows: Window occasionally appearing off the screen
*   Sample/Ins editor listboxes scrolled right in certain circumstances

## 07/11/13 (v0.90.86):

### Whats New:

*   Unsafe notes now shown in red when "PT 3 octave limit" option enabled
*   Haiku port (thanks jua)
*   Additional FT2 shortcuts ([forum post](http://modarchive.org/forums/index.php?topic=2713.0))
*   Android port (thanks guillaum1)
*   Jack driver now connects to first available physical device
*   MilkyTracker source code now hosted on [GitHub](https://github.com/milkytracker/MilkyTracker)

### Bugs Fixed:

*   Loss of data when using 'backward' sample filter ([forum post](http://forums.modarchive.org/index.php?topic=3093.0))
*   Warning displayed when exporting .mod files with samples over 64k ([forum post](http://forums.modarchive.org/index.php?topic=3235.0))
*   Default panning settings applied when adding channels ([forum post](http://forums.modarchive.org/index.php?topic=3399.0))
*   One-shot looping now works correctly ([forum post](http://forums.modarchive.org/index.php?topic=3391.0))
*   First 2 bytes of samples cleared to zero when exporting to MOD (Amiga compatibility)
*   FT2 weirdness emulated when using the arpreggio command (thanks Saga_Musix)
*   In live mode, don't switch from pattern to song play when selecting a new pattern from the order list
*   Check config file is open for writing before attempting to save to it.
*   Don't assume BACKUPxx.XM has been saved
*   Crash when storing colour presets ([forum post](http://forums.modarchive.org/index.php?topic=3054.0))
*   Segfault when loading XMs with zero-length loop ([forum post](http://forums.modarchive.org/index.php?topic=3063.0))
*   Emulate FT2 behaviour for arpregios when speed>18
*   Able to load some non-standard S3M files
*   Incorrect sample format assumptions in IT loader (thanks Saga_Musix)
*   Various build warnings and errors

## 01/01/10 (v0.90.85):

### Whats New:

*   Ability to move dialogs.
*   Keyboard shortcut Shift-U (unmute all).
*   Unix/SDL version now uses system libzzip.
*   Updated section panel to handle multiple pages.
*   Basic quicktime support on OSX.
*   LHA archive support.
*   Colour table import/export.
*   gzip compression support.
*   New font: IDC-Harmonica (12x12)
*   New font: IDC-MicroKnight (12x12)
*   Ability to detect desktop display resolution.
*   Added '1' as note-off key in MilkyTracker mode.
*   Ability to adjust screen magnification factor.
*   Milkyplay license changed from GPL to BSD (MilkyTracker remains GPL).
*   Working close button in OSX.
*   LZX compressor support.
*   Improved constants for 10 band EQ.
*   New font: Topaz 1.3 & Topaz 2.0 (8x8)

### Bugs Fixed:

*   (Unix) JACK client thread gets zombified causing playback to block.
*   Crash when using the '-orientation ROTATE90CW' switch on SDL versions.
*   Screen input lock after showing a system message.
*   Bug in xm loader for old xm variant causing 8 bit samples to not load correctly.
*   Added FT2 note range clipping to live playback.
*   Improved live recording note-off.
*   Crash when unable to open a directory.
*   Experimental mod exporter fix.
*   Volume scale default volume for empty instrument/samples.
*   Modplug XM importer fix.
*   Crash when MilkyTracker attempts to load a non-existant file.
*   Dialog initalization bug (caused focus to miss in edit fields etc).
*   Font selection bug.
*   Mouse repeat bug on SDL version.
*   Small bug in SDL Midi code.
*   Little bug in the advanced edit panel (limit of subsequent channels).
*   Undefined .XM sample type 0x3 causes sample loop to appear "one shot".
*   Instrument vibrato depth value gets doubled when copying instruments.
*   PSM loader can now store sub-song information correctly.
*   XM loading problems with 16-bit odd sample sizes.
*   Support for callbacks on systems that don't use C style function calls (ie, OS/2)
*   Instrument fadeout value initially doesn't match the slider position when adding new instruments to a loaded module.
*   Selecting an instrument with the numpad doesn't update the instrument or sample editors with the selected instrument, but with the previous one.
*   Bug in auto-detection of playmode for XM modules (panning was not set).
*   Set envelope position after sustain points.
*   Ignored lower 3 bits of finetune for more accurate FT2 playback
*   Crash while loading sample during playback
*   Screen refresh issues
*   Problem with French keyboard layout.
*   The current pattern length under the song title isn't updated when the mod is zapped.
*   Shrinking a 2-row pattern isn't possible.
*   Undoing an action doesn't register as a change.
*   A slight graphical bug occurs when using the seq and cln buttons on the FEth pattern of a mod.
*   Crash when converting sample resolution.
*   Freeze on exit when using DirectSound on Windows 7.
*   Lots of other smalls bugs not worth mentioning.

## 04/13/08 (v0.90.80):

### Important note:

This version of MilkyTracker will update your current configuration file in a way that it's no longer usable with older versions. It is recommended that you keep a backup of your configuration file. On **Windows** the configuration is stored in the application folder. When using MilkyTracker on **OS X** or **Unix** systems the configuration is stored in your home directory in a file called .milkytracker_config.

### What's New:

*   Killer feature: Open up to 32 modules using tabs(*):
    *   Allows simultaneous playback of different tabs
    *   Copy pattern data/instruments/samples between tabs
*   New resamplers for improved sound quality:
    *   Cubic Lagrange
    *   Cubic Spline
    *   Fast Sinc (window size 16, fixed point integer, sinc lookup table)
    *   Precise Sinc (window size 128, double floating point)
    *   Amiga 500
    *   Amiga 500 LED
    *   Amiga 1200
    *   Amiga 1200 LED
*   Render parts of the song directly into a sample slot (from HD recorder)
*   Live switch toggle [L]
*   Track splitter insert note off toggle
*   Sample editor shows time in milliseconds according to currently selected relative note
*   Notes on muted channels appear in grey on the piano
*   Scopes also available with smoothed lines
*   Added option to always mixdown stereo samples, no more questions ;)
*   WAV loader recognizes loop points and also exports them

(*) Only available in the Desktop version of MilkyTracker

### Bugs Fixed:

#### Replay

*   Fixed bug in bidirectional looping which caused clicks on short samples
*   Introduced smart loop area double buffering to eliminate sample looping clicks
*   Fixed issue when switching between 4xx and Vx
*   Fixed issue when having a Dxx on the last order and restart is not zero
*   Milky's note range was going slightly higher than that of Ft2's.

#### Other

*   CRASH/FREEZE: When trying apply crossfade on invalid selections in sample editor.
*   FREEZE: Optimize song can freeze MilkyTracker if the displayed pattern number no longer exists after optimization.
*   Bug in the XI-loader causes instrument vibrato settings not to be applied until some instrument setting is modified.
*   Cutting arpeggio commands by selecting only the operand digits leaves invisible 000 commands on the pattern that repeat the previous arpeggio from effect memory.
*   GUS patch loader note mapping more accurate now
*   Some custom screen resolutions lead to distorted rendering.
*   The Apple AIFF loader doesn't load big endian samples correctly.
*   Rewritten major parts of the GUI & editor code to be more reliable.
*   Countless bugs you might not even have found yet

## 05/20/07 (v0.90.60):

### What's New:

*   New imported (obscure) module formats (mainly for personal satisfaction):
    *   Another DSM format (Digisound Interface Kit library)
    *   Another AMF format (DSMI library)
    *   SFX (SoundFX)
*   Basic AIFF loader (Apple Sound Format, only reads uncompressed AIFF but allows for importing CD tracks directly on OS X)
*   Four new 8x8 fonts (thanks to idc and Rez)
*   Set custom screen resolution
*   Scrollbars in the list boxes dynamically show/hide depending on content (saves some space)
*   Row preview with Shift+Space
*   Song Preview Alt+Space
*   Integrated disk browser (click "flip" in the disk operations panel)
*   Sample editor got EQ filters now (3 and 10 bands)
*   Sample editor got waveform generators for sine, square, triangle and sawtooth
*   Solid scopes (ProTracker style, toggle in the config under the misc. tab)
*   Pattern replay is a bit more like Ft2 now (switching to different pattern doesn't reset cursor to row 0)
*   Clone button to clone the current order
*   "Mix paste" in the sample editor: paste in a sample from the clipboard and get it mixed with the current selection/entire sample.
*   As usually: more Ft2-compatible replay

### Bugs Fixed:

#### Replay

*   A severe volume ramping bug has been fixed
*   Instrument auto vibrato ramp wave forms were swapped
*   Fixed several issues in the replayer which were causing problems when using EDx and Mx and similiar combinations

#### Other

*   Orders containing pattern number FF vanished from modules saved from Ft2.
*   Loading arbitrary files as samples adds the filename to the sample text.
*   The y-axis in the sample editor was flipped (negative side was on the top).
*   Toggling scopes in the config with the keyboard left the checkbox unaffected.
*   Crash: A bug in the IFF reader crashes MilkyTracker when loading some IFF samples.
*   Crash: After adding channels and saving as MOD.
*   Saving MOD with extended octaves (5 instead of 3) produces results like FT2 now.
*   Crash: (on Linux x86_64, strange behavior elsewhere): Caused by illegal 16-bit zero-length samples, as saved by Soundtracker.
*   Crash: when loading modules without instruments.
*   FREEZE: when zooming way in/loading LOOONG instrument envelopes.
*   FREEZE (on Windows Vista): when initializing audio on startup.
*   Assigning samples to notes in the instrument editor by clicking and holding (painting) doesn't have effect on C-0.
*   Certain hot words in song titles cause incorrect module format detection (improved at least ;)).
*   Clear button above sample box didn't work properly.
*   Dxx on the last row of a pattern caused a display error when xx is greater than the number of rows on the current pattern.
*   Insert silence screwed up 16 bit samples.
*   Instrument panning settings defaulted to center on key-release when key-jamming.
*   Optimizer functions "Minimize all samples" and "Samples to 8-bit" triggered one another when only one of them were selected.
*   Selection block is truncated to current pattern length (you know, after being taller on a taller pattern).
*   Several things like applying sample editor filters and using the optimizer reset global volume.
*   Other things I can't remember :P

## 09/26/06 (v0.90.50), Tons of fixes and added features:

### What's New:

*   New platforms supported:
    *   FreeBSD (x86)
    *   Linux (x86, x86_64, PowerPC, ARM, GP2X)
    *   Solaris 9 & 10 (SPARC)
    *   Windows (ANSI)
    *   Windows CE Handheld PCs and the Gizmondo
*   New imported module formats:
    *   DBM (DigiBooster Pro)
    *   GMC (Game Music Creator)
    *   IT (Impulse Tracker)
        *   NOTE: MilkyTracker will remain an Ft2 clone, importing converts everything to this environment and accurate playback is not pursued. It's just like with the rest of the imported formats apart from MOD and XM.
        *   MORE IMPORTANTLY: MilkyTracker refuses to import modules with more than 32 channels. It's not that the XM format couldn't handle it but the Fasttracker II framework doesn't, so there.
*   Low-latency audio driver support:
    *   ALSA on Linux
    *   ASIO and DirectX on Win32, thanks to [RtAudio](http://www.music.mcgill.ca/~gary/rtaudio/)
        *   48kHz mixing resolution now available also on Win32\. NOTE: MMSYSTEM/WaveOut can't handle 48kHz, so don't mix these two.
*   Adjustable MOD channel panning
*   Adjustable opacity for muted channels
*   Alternate view in the main panel showing Add, Octave and Global volume values
*   Fasttracker II edit mode and scrolling style loaded by default
*   MIDI in with velocity and note delay/off recording (works on your QWERTY too), thanks to [RtMidi](http://www.music.mcgill.ca/~gary/rtmidi/)
*   Module optimization panel
*   Pattern row highlighting, two independent intervals
*   Sample editor noise generators
*   Selectable pattern/UI fonts
*   Skippable splash screen
*   Volume boost/fade dialogs accept negative values (useful for chip sample generation, sample phase inversion etc)

### Bugs Fixed:

#### Replay

*   A rogue EDx note delay without a note should retrigger the previous note when x is lesser than or equal to song speed.
*   EDx+EEy, that is delayed notes on a pattern delayed row played when x was larger than or equal to the song speed. Sadly, this shouldn't happen according to Ft2, thus it doesn't anymore.
*   E5x finetune setting implemented.
*   E9x note retrig was a little off, now it's ever closer to Ft2.
*   F00 speed setting wasn't implemented, now it stops playback
    *   Fasttracker II slows down to advancing one row every 10 or so minutes. If you can justify this behavior, i.e. convince us how you can rationally utilize it (a 10 minute pause) in a song, it will be implemented.
*   K00 did not send a noteOff but set the volume to zero.
*   Mx volume column tone portamento acted as M0 when starting a song from scratch and you couldn't actually enter an M0 because a 1 was was automatically filled in as the parameter. All fixed and dandy now.
*   Rxx multiretrig, see E9x.
*   Something clicked in the mixer, literally. Now smoother than a baby's butt.

#### User interface

*   A channel could get stuck playing only one instrument when volume ramping was off. Ain't life peculiar?
*   A crash happened when selecting a range in sample editor and the mouse was released outside the MilkyTracker window followed by a cut or copy operation. You see the selection was still in progress and the poor tracker got confused because it didn't know where the range started/ended.
*   A pattern that was deleted from the orderlist remained on the screen.
*   An instrument stayed selected after it had been removed with the minus button.
*   Block operation areas did not always match block selections.
*   Global volume wasn't reset on stop/play.
*   Key release now effects the correct note (not necessarily the note that is playing).
*   Multisample instruments got rearranged/optimized automatically.
*   Scrollbar positions were reset upon screen switches. Added botox.
*   Standard-breaking XI instruments (w/ stereo and more than 16 samples) were crashing MilkyTracker. Now stereo is converted to mono and surplus samples are dismissed.
*   The MilkyTracker window insisted on staying topmost after switching from fullscreen to windowed.
*   The F10 key brought up the system menu on Win32\. Now it just jumps to row 10h like it's supposed to.
*   The selection disappeared after pasting a block. Doesn't anymore.
*   When tabbing to a channel outside the current display, the screen shifted one whole "page". Very confusing for young, fragile artists. Now it moves one channel at a time, à la Ft2.

## 02/14/06 (v0.90.30), Mainly bugfixes and minor additions:

### Bugs fixed:

*   ProTracker modules using 'one shot'-looped samples seem to crash the replayer sometimes. Fixed.
*   There was a bug in the mod exporter when trying to export samples with a loopstart set to 0\. Fixed.
*   The windows version would crash when inserting MANY points between two points in an envelope. Fixed.
*   The right-left combined mouse click on the scopes didn't work properly. Fixed.
*   switching between the loop types caused crashes. Fixed.
*   Redo filter seems to crash/erase sample in the win32 version. Fixed.
*   Retrig command was still a little bit buggy (when used in combination with delay note). Fixed.
*   Arpeggio was still a little bit buggy (won't be noticed when tickspeed%3 == 0). Fixed.
*   Putting a position jump inbetween a pattern loop will cause the song length estimator to hang in an infinite loop. Fixed.
*   Trying to enable the volume envelope when it says 'none selected' in the envelope editor Milky would crash. Fixed.
*   Block transpose up/down was buggy. Fixed.
*   Pattern Len display isn't updated to reflect new length after block was pasted and pattern grew. Fixed.
*   Scopes didn't update after loading samples/instruments
*   Dozens of other fixes…

### Features added:

*   Multichannel record/keyjazz/editing (like in FT2)
*   Disable pattern grow when pasting
*   Enable "by channel tabbing" (like in FT2)
*   Instrument quick select using numerical keypad
*   Select sample within instrument in FT2 edit mode (Shift+Alt+Up/Down)
*   Emulate insert key with Ctrl+Up in the OSX version (select from the OSX main menu under option "special")
*   Dragging files from the win-explorer/osx finder into the MilkyTracker window loads the file (works for modules, instruments, patterns, tracks, samples etc.)
*   Load/Save IFF samples
*   More WAV sample types recognized and loaded
*   Improved Protracker compatibility
*   More options on the replayer for improved module playback compatibility
*   Playmodes can now be saved to be the default playmode
*   Even more FT2 keyboard shortcuts (F9-F12, look in the doc for more info)
*   Even more customizing support, see config.
*   PocketPC: Added scopes as well
*   PocketPC: Jam page with extended keyboard or pattern

## 11/29/05 (v0.90.00), Lots of improvements, really can't remember them all:

*   Improved mixer, improved FT2 effect compatibility (especially tremolo, tremor, vibrato)
*   Scopes (finally) (only for Desktop versions)
*   Configurable colors (config->layout)
*   Disk writer (Ctrl+R)
*   FT2 like pattern follow: cursor line always in center center
*   Prospective mode and toggle follow song mode
*   Volume scale (Shift/Alt/Ctrl+V like FT2)
*   Load GUS patches (needs to be loaded as instrument)
*   Load every file as sample (good for making noise samples)
*   Dozens of sample filters
*   Reworked GUI (especially menus)
*   Sample offset can be shown in sample editor
*   Drawing samples (but remember to turn on looping, otherwise doesn't sound good ;))
*   Toggle Hex count for rows in pattern editor
*   Show zero effect as 00 instead of dots (see config->layout)
*   Improved Protracker 2.x, 3.x replay modes (see options)
*   Skip through song while it's playing (with cursor keys or scrollwheel)
*   Time length calculation/play time meter
*   Peak meters
*   Sequencer button (see order list)
*   Hold the Alt Key in the sample editor to move entire loop not only loop/end
*   Tons of other fixes…
*   More things to find out…

## 10/07/05:

*   Fixed crash when saving .MODs
*   Implemented Lxx command (set envelope position)
*   New: Hold down Ctrl (Apple on OSX) to resize selection in sample editor
*   New: Zoom sample in/out by using the scroll wheel of your mouse
*   New: Swap channels (select source with cursor and swap with destination channel by menu)
*   New: Split track (spread notes from one channel across subsequent channels)
*   New: Use virtual channels when playing instruments (enable in config first) (instruments played on virtual channels won't cut instruments used in song)
*   Other stuff (don't remember all of it)

## 09/01/05:

*   Fixed lots of crashes =)
*   cut/copy/paste works in sample editor (hopefully, not much tested yet)
*   you're able to change default screen resolution but you need to restart MilkyTracker to apply
*   Save patterns/tracks in FT2 formats .XP/.XT
*   Save Protracker compatible mods
*   Fixed some small issues

## 07/01/05:

*   Fixed lots of crashes
*   Undo/Redo in sample editor (cut/copy/paste is following soon)
*   Minor cosmetical changes
*   Vibrato volume command now works in pattern editor (press "v")
*   Vibrato speed volume command now works in pattern editor (press "s")
*   X command works in pattern editor
*   MOD loader now recognises 15 channel mods
*   Added "large" font for tracking <= 4 channel songs ;)
*   Del. key can be used for deleting note+ins./volume/effect

## 06/22/05:

*   !! Fixed bug that caused MilkyTracker to crash when selecting empty samples (sorry for that one)
*   Fixed bug with ALT+F4 when using FT2 shortcuts and Windows =)
*   Paste of tracks and patterns always insert at row 0, not at cursor
*   Added possibility to expand orderlist listbox
*   Added predefined envelopes
*   Changed instrument color (one day it will be possible to select your own colors ;))
*   More minor fixes

## 06/13/05:

*   Improved sample position markers
*   Added envelope position markers
*   Improved XM replayer (fixed FT2 note clipping when playing arpeggios)
*   Added more FT2 keyboard shortcuts (Ctrl+I, Ctrl +S, Ctrl +X etc.)
*   Minor fixes + Additions
*   Added support for mouse scrollwheel
*   PocketPC: "Full" onscreen keyboard (probably still buggy)

## 05/27/05:

*   Right (1st) & Left click (2nd) on channel box cycles between solo channel/unmute all (Alternatively hold ALT key and left click on channel box)
*   Del. now deletes note & instrument in FT2 mode
*   Pattern scrolling is now buffer independent on Win32/WinCE (Not necessary under OSX because of much smaller buffer sizes =))
*   Added another pattern scroll style (cursor scrolls to the center and keeps center until the last page)

## 05/22/05:

*   Due to request MilkyTracker now features a FT2 compatibility mode. You can switch between MilkyTracker (modern ;)) and FT2 edit modes in the config screen. (See MilkyTracker manual for details.) Note: Not all FT2 commands are implemented yet, but the most important ones for editing are included.
*   Hopefully fixed some keyboard layout problems

## 05/14/05:

*   Fixed some "focus" issues
*   Divided "save" option into "save" and "save as…"
*   Added value interpolation (available in the "Advanced Editor")
*   Fixed an instrument load/save issue
*   PocketPC: Fixed bug, which prevented the progress bar from disappearing when saving instruments/samples.

## 05/06/05:

*   Minor optimizations making the GUI more responsive
*   Toggle focus of pattern editor by pressing space
*   Play song from current order by pressing enter
*   Play pattern by pressing ctrl+enter
*   Play pattern from current position by pressing shift+enter
*   PocketPC: Explicit hiding of SIP (does this solve any problems?)
*   PocketPC: Minor facelifts

## 04/29/05:

*   Added possibility to swap/copy samples from one instrument to another
*   Fixed major bug in XI loader
*   Increased button autorepeat delay
*   PocketPC version: GAPI is now properly shut down before opening the common file dialog

## 04/25/05:

*   First "official" PocketPC pre-release.
