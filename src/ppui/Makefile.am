SUBDIRS = osinterface

noinst_LIBRARIES = libppui.a

# For OpenGL, add sdl/DisplayDeviceOGL_SDL.cpp and Graphics_OGL.cpp, see
# below
libppui_a_SOURCES = Button.cpp CheckBox.cpp Container.cpp ContextMenu.cpp \
Control.cpp DialogBase.cpp DialogFileSelector.cpp Dictionary.cpp \
DictionaryKey.cpp Event.cpp Font.cpp Graphics_15BIT.cpp Graphics_16BIT.cpp \
Graphics_24bpp_generic.cpp Graphics_32bpp_generic.cpp Graphics_ARGB32.cpp \
Graphics_BGR24.cpp Graphics_BGR24_SLOW.cpp KeyboardBindingHandler.cpp \
ListBox.cpp ListBoxFileBrowser.cpp Menu.cpp MessageBoxContainer.cpp \
PPUIConfig.cpp RadioGroup.cpp Screen.cpp Scrollbar.cpp Seperator.cpp Slider.cpp \
StaticText.cpp Tools.cpp TransparentContainer.cpp sdl/DisplayDeviceFB_SDL.cpp \
sdl/DisplayDevice_SDL.cpp

# Add Graphics_OGL.h and sdl/DisplayDeviceOGL_SDL.h here for OpenGL, see below
noinst_HEADERS = BasicTypes.h Button.h CheckBox.h Container.h ContextMenu.h \
Control.h DialogBase.h DialogFileSelector.h Dictionary.h DictionaryKey.h \
DisplayDeviceBase.h Event.h Font.h Graphics.h GraphicsAbstract.h KeyBindings.h \
KeyboardBindingHandler.h ListBox.h ListBoxFileBrowser.h Menu.h \
MessageBoxContainer.h Object.h PPPath.h PPUI.h PPUIConfig.h RadioGroup.h \
ScanCodes.h Screen.h ScrollBar.h Seperator.h SimpleVector.h Singleton.h \
Slider.h StaticText.h Tools.h TransparentContainer.h UndoStack.h VirtualKeys.h \
fastfill.h sdl/DisplayDeviceFB_SDL.h sdl/DisplayDevice_SDL.h

INCLUDES = -I$(top_srcdir)/src/ppui/osinterface \
-I$(top_srcdir)/src/ppui/osinterface/posix -I$(top_srcdir)/src/ppui/sdl

# Add -D__OPENGL__ here if you want experimental opengl gfx (make clean
# probably required)
AM_CPPFLAGS = ${SDL_CFLAGS} -DMILKYTRACKER -D__LINUX__ -D__THREADTIMER__

EXTRA_DIST = carbon/DisplayDevice_CARBON.cpp carbon/DisplayDevice_CARBON.h \
win32/DisplayDevice_WIN32.cpp win32/DisplayDevice_WIN32.h \
wince/DisplayDevice_GAPI.cpp wince/DisplayDevice_GAPI.h
