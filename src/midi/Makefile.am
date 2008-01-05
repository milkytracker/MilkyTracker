noinst_LIBRARIES = libmidi.a
libmidi_a_SOURCES = posix/MidiReceiver_pthread.cpp rtmidi/RtMidi.cpp
noinst_HEADERS = posix/MidiReceiver_pthread.h rtmidi/RtError.h rtmidi/RtMidi.h
AM_CPPFLAGS=-D__LINUX_ALSASEQ__ -D__LINUX__ -DMILKYTRACKER ${ALSA_CFLAGS}
INCLUDES = -I../milkyplay -I../ppui -I../tracker -I../ppui/osinterface -I../ppui/osinterface/posix -Irtmidi
