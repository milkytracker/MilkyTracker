noinst_LIBRARIES = libmidi.a
libmidi_a_SOURCES = posix/MidiReceiver_pthread.cpp rtmidi/RtMidi.cpp
noinst_HEADERS = posix/MidiReceiver_pthread.h rtmidi/RtError.h rtmidi/RtMidi.h
AM_CPPFLAGS=-D__LINUX_ALSASEQ__ -D__LINUX__ -DMILKYTRACKER ${ALSA_CFLAGS}
INCLUDES = -I$(top_srcdir)/src/milkyplay -I$(top_srcdir)/src/ppui -I$(top_srcdir)/src/tracker -I$(top_srcdir)/src/ppui/osinterface -I$(top_srcdir)/src/ppui/osinterface/posix -I$(top_srcdir)/src/midi/rtmidi
