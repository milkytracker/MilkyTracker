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

add_library(ppui STATIC
    # Sources
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

    # Headers
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

target_include_directories(ppui
    PUBLIC
        .
    PRIVATE
        osinterface
)

target_link_libraries(ppui
    PRIVATE 
        osinterface
        tmm
)

# Add platform-specific sources and include paths
if(APPLE)
    target_sources(ppui
        PRIVATE
            cocoa/DisplayDevice_COCOA.mm
            cocoa/DisplayDevice_COCOA.h
    )
    target_include_directories(ppui
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src/tracker/cocoa
        PUBLIC
            cocoa
    )
elseif(WIN32)
    target_sources(ppui
        PRIVATE
            win32/DisplayDevice_WIN32.cpp
            win32/DisplayDevice_WIN32.h
    )
    target_include_directories(ppui
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src/milkyplay
        PUBLIC
            win32
    )
else()
    target_sources(ppui
        PRIVATE
            # Sources
            sdl/DisplayDeviceFB_SDL.cpp
            sdl/DisplayDevice_SDL.cpp

            # Headers
            sdl/DisplayDeviceFB_SDL.h
            sdl/DisplayDevice_SDL.h
    )
    target_include_directories(ppui PUBLIC sdl)
endif()

if(APPLE)
    # Enable ARC (automatic reference counting) for OS X build
    target_compile_options(ppui PRIVATE -fobjc-arc)
endif()
