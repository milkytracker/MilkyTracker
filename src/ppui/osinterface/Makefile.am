noinst_LIBRARIES = libosinterface.a

libosinterface_a_SOURCES = PPPathFactory.cpp posix/PPPath_POSIX.cpp \
posix/PPSystem_POSIX.cpp sdl/PPMessageBox_SDL.cpp sdl/PPMutex.cpp \
sdl/PPOpenPanel_SDL.cpp sdl/PPQuitSaveAlert_SDL.cpp sdl/PPSavePanel_SDL.cpp \
sdl/SDL_ModalLoop.cpp

noinst_HEADERS = PPMessageBox.h PPModalDialog.h PPOpenPanel.h PPPathFactory.h \
PPQuitSaveAlert.h PPSavePanel.h PPSystem.h posix/PPPath_POSIX.h \
posix/PPSystemString_POSIX.h posix/PPSystem_POSIX.h sdl/PPMutex.h \
sdl/SDL_ModalLoop.h

EXTRA_DIST = carbon/PPMessageBox_CARBON.cpp carbon/PPOpenPanel_CARBON.cpp \
carbon/PPQuitSaveAlert_CARBON.cpp carbon/PPSavePanel_CARBON.cpp \
posix/PPMutex.cpp posix/PPMutex.h sdl/PPMutex.cpp sdl/PPMutex.h \
win32/PPMessageBox_WIN32.cpp win32/PPMutex.cpp win32/PPMutex.h \
win32/PPOpenPanel_WIN32.cpp win32/PPPath_WIN32.cpp win32/PPPath_WIN32.h \
win32/PPQuitSaveAlert_WIN32.cpp win32/PPSavePanel_WIN32.cpp \
win32/PPSystemString_WIN32.h win32/PPSystem_WIN32.cpp win32/PPSystem_WIN32.h \
win32/WaitWindow_WIN32.cpp win32/WaitWindow_WIN32.h \
wince/PPOpenPanel_WINCE.cpp wince/PPQuitSaveAlert_WINCE.cpp \
wince/PPSavePanel_WINCE.cpp

INCLUDES = -I$(top_srcdir)/src/ppui -I$(top_srcdir)/src/ppui/osinterface/posix \
-I$(top_srcdir)/src/ppui/osinterface/sdl

AM_CPPFLAGS = -DMILKYTRACKER -D__THREADTIMER__ -D__LINUX__ ${SDL_CFLAGS}
