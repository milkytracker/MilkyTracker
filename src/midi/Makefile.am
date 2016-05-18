noinst_LIBRARIES = libmidi.a

libmidi_a_SOURCES = posix/MidiReceiver_pthread.cpp

noinst_HEADERS = posix/MidiReceiver_pthread.h MidiTools.h

AM_CPPFLAGS = -D__LINUX_ALSASEQ__ -D__LINUX__ -DMILKYTRACKER \
$(SDL_CFLAGS) $(ALSA_CFLAGS) $(RTMIDI_CFLAGS)

AM_CPPFLAGS += -I$(top_srcdir)/src/milkyplay -I$(top_srcdir)/src/ppui \
-I$(top_srcdir)/src/tracker -I$(top_srcdir)/src/ppui/osinterface \
-I$(top_srcdir)/src/ppui/osinterface/sdl \
-I$(top_srcdir)/src/ppui/osinterface/posix

EXTRA_DIST = win32/LongMsg.cpp win32/LongMsg.h win32/MIDIInDevice.cpp \
win32/MIDIInDevice.h win32/MIDIMsg.h win32/MidiReceiver_win32.cpp \
win32/MidiReceiver_win32.h win32/ShortMsg.cpp win32/ShortMsg.h \
win32/midi.h haiku/MidiSetup.cpp haiku/MidiSetup.h \
haiku/MilkyMidiConsumer.cpp haiku/MilkyMidiConsumer.h
