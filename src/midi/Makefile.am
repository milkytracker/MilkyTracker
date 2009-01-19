noinst_LIBRARIES = libmidi.a

libmidi_a_SOURCES = posix/MidiReceiver_pthread.cpp rtmidi/RtMidi.cpp

noinst_HEADERS = posix/MidiReceiver_pthread.h rtmidi/RtError.h rtmidi/RtMidi.h \
MidiTools.h

AM_CPPFLAGS=-D__LINUX_ALSASEQ__ -D__LINUX__ -DMILKYTRACKER ${ALSA_CFLAGS} \
${SDL_CFLAGS}

INCLUDES = -I$(top_srcdir)/src/milkyplay -I$(top_srcdir)/src/ppui \
-I$(top_srcdir)/src/tracker -I$(top_srcdir)/src/ppui/osinterface \
-I$(top_srcdir)/src/ppui/osinterface/sdl \
-I$(top_srcdir)/src/ppui/osinterface/posix -I$(top_srcdir)/src/midi/rtmidi

EXTRA_DIST = win32/LongMsg.cpp win32/LongMsg.h win32/MIDIInDevice.cpp \
win32/MIDIInDevice.h win32/MIDIMsg.h win32/MidiReceiver_win32.cpp \
win32/MidiReceiver_win32.h win32/ShortMsg.cpp win32/ShortMsg.h win32/midi.h
