#
#  src/ppui/CMakeLists.txt
#
#  Copyright 2016 Dale Whinham
#
#  This file is part of MilkyTracker.
#
#  MilkyTracker is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  MilkyTracker is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with MilkyTracker.  If not, see <http://www.gnu.org/licenses/>.
#

add_subdirectory(osinterface)

set(
    SOURCES
    Button.cpp
    CheckBox.cpp
	CheckBoxLabel.cpp
    Container.cpp
    ContextMenu.cpp
    Control.cpp
    DialogBase.cpp
    DialogFileSelector.cpp
    Dictionary.cpp
    DictionaryKey.cpp
    Event.cpp
    Font.cpp
    Graphics_15BIT.cpp
    Graphics_16BIT.cpp
    Graphics_24bpp_generic.cpp
    Graphics_32bpp_generic.cpp
    Graphics_ARGB32.cpp
    Graphics_BGR24.cpp
    Graphics_BGR24_SLOW.cpp
#   Graphics_OGL.cpp
    KeyboardBindingHandler.cpp
    ListBox.cpp
    ListBoxFileBrowser.cpp
    Menu.cpp
    MessageBoxContainer.cpp
    PPUIConfig.cpp
    RadioGroup.cpp
    Screen.cpp
    Scrollbar.cpp
    Seperator.cpp
    Slider.cpp
    StaticText.cpp
    Tools.cpp
    TransparentContainer.cpp
)

set(
    HEADERS
    BasicTypes.h
    Button.h
    CheckBox.h
	CheckBoxLabel.h
    Container.h
    ContextMenu.h
    Control.h
    DialogBase.h
    DialogFileSelector.h
    Dictionary.h
    DictionaryKey.h
    DisplayDeviceBase.h
    Event.h
    fastfill.h
    Font.h
    GraphicsAbstract.h
    Graphics.h
#   Graphics_OGL.h
    KeyBindings.h
    KeyboardBindingHandler.h
    ListBoxFileBrowser.h
    ListBox.h
    Menu.h
    MessageBoxContainer.h
    Object.h
    PPPath.h
    PPUIConfig.h
    PPUI.h
    RadioGroup.h
    ScanCodes.h
    Screen.h
    ScrollBar.h
    Seperator.h
    SimpleVector.h
    Singleton.h
    Slider.h
    StaticText.h
    Tools.h
    TransparentContainer.h
    UndoStack.h
    VirtualKeys.h
)

# Add platform-specific sources and include paths
if(APPLE)
    list(APPEND SOURCES cocoa/DisplayDevice_COCOA.mm)
    list(APPEND HEADERS cocoa/DisplayDevice_COCOA.h)
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
        ${PROJECT_SOURCE_DIR}/src/tracker/cocoa
    )
elseif(WIN32)
    list(APPEND SOURCES win32/DisplayDevice_WIN32.cpp)
    list(APPEND HEADERS win32/DisplayDevice_WIN32.h)
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/win32
        ${PROJECT_SOURCE_DIR}/src/milkyplay
    )
else()
    list(APPEND SOURCES
        sdl/DisplayDeviceFB_SDL.cpp
        sdl/DisplayDevice_SDL.cpp
    )
    list(APPEND HEADERS
        sdl/DisplayDeviceFB_SDL.h
        sdl/DisplayDevice_SDL.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
        ${SDL2_INCLUDE_DIRS}
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface
)

add_library(ppui ${SOURCES} ${HEADERS})

if(APPLE)
    # Enable ARC (automatic reference counting) for OS X build
    set_property(
        TARGET ppui APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc"
    )
endif()

target_link_libraries(
    ppui
    osinterface
)
