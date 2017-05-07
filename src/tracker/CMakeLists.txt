#
#  src/tracker/CMakeLists.txt
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

set(
    SOURCES
    AnimatedFXControl.cpp
    ColorExportImport.cpp
    ColorPaletteContainer.cpp
    DialogChannelSelector.cpp
    DialogEQ.cpp
    DialogGroupSelection.cpp
    DialogHandlers.cpp
    DialogListBox.cpp
    DialogPanning.cpp
    DialogQuickChooseInstrument.cpp
    DialogResample.cpp
    DialogWithValues.cpp
    DialogZap.cpp
    EQConstants.cpp
    EditorBase.cpp
    EnvelopeContainer.cpp
    EnvelopeEditor.cpp
    EnvelopeEditorControl.cpp
    Equalizer.cpp
    FileExtProvider.cpp
    FileIdentificator.cpp
    GlobalColorConfig.cpp
    InputControlListener.cpp
    LogoBig.cpp
    LogoSmall.cpp
    ModuleEditor.cpp
    ModuleServices.cpp
    PatternEditor.cpp
    PatternEditorClipBoard.cpp
    PatternEditorControl.cpp
    PatternEditorControlEventListener.cpp
    PatternEditorControlKeyboard.cpp
    PatternEditorControlTransposeHandler.cpp
    PatternEditorTools.cpp
    PatternTools.cpp
    PeakLevelControl.cpp
    Piano.cpp
    PianoControl.cpp
    PlayerController.cpp
    PlayerLogic.cpp
    PlayerMaster.cpp
    RecPosProvider.cpp
    RecorderLogic.cpp
    ResamplerHelper.cpp
    SampleEditor.cpp
    SampleEditorControl.cpp
    SampleEditorControlToolHandler.cpp
    SampleEditorResampler.cpp
    SamplePlayer.cpp
    ScopesControl.cpp
    SectionAbout.cpp
    SectionAbstract.cpp
    SectionAdvancedEdit.cpp
    SectionDiskMenu.cpp
    SectionHDRecorder.cpp
    SectionInstruments.cpp
    SectionOptimize.cpp
    SectionQuickOptions.cpp
    SectionSamples.cpp
    SectionSettings.cpp
    SectionSwitcher.cpp
    SectionTranspose.cpp
    SectionUpperLeft.cpp
    SongLengthEstimator.cpp
    SystemMessage.cpp
    TabHeaderControl.cpp
    TabManager.cpp
    TabTitleProvider.cpp
    TitlePageManager.cpp
    ToolInvokeHelper.cpp
    Tracker.cpp
    TrackerConfig.cpp
    TrackerInit.cpp
    TrackerKeyboard.cpp
    TrackerSettings.cpp
    TrackerSettingsDatabase.cpp
    TrackerShortCuts.cpp
    TrackerShutDown.cpp
    TrackerStartUp.cpp
    TrackerUpdate.cpp
    Undo.cpp
    VRand.cpp
    Zapper.cpp
)

set(
    HEADERS
    ${PROJECT_BINARY_DIR}/src/tracker/version.h
    AnimatedFXControl.h
    ColorExportImport.h
    ColorPaletteContainer.h
    ControlIDs.h
    DialogChannelSelector.h
    DialogEQ.h
    DialogGroupSelection.h
    DialogHandlers.h
    DialogListBox.h
    DialogPanning.h
    DialogQuickChooseInstrument.h
    DialogResample.h
    DialogWithValues.h
    DialogZap.h
    EQConstants.h
    EditModes.h
    EditorBase.h
    EnvelopeContainer.h
    EnvelopeEditor.h
    EnvelopeEditorControl.h
    Equalizer.h
    FileExtProvider.h
    FileIdentificator.h
    FileTypes.h
    FilterParameters.h
    GlobalColorConfig.h
    InputControlListener.h
    LogoBig.h
    LogoSmall.h
    ModuleEditor.h
    ModuleServices.h
    PatternEditor.h
    PatternEditorControl.h
    PatternEditorTools.h
    PatternTools.h
    PeakLevelControl.h
    Piano.h
    PianoControl.h
    PlayerController.h
    PlayerCriticalSection.h
    PlayerLogic.h
    PlayerMaster.h
    RecPosProvider.h
    RecorderLogic.h
    ResamplerHelper.h
    SIPButtons.h
    SampleEditor.h
    SampleEditorControl.h
    SampleEditorControlLastValues.h
    SampleEditorResampler.h
    SamplePlayer.h
    ScopesControl.h
    SectionAbout.h
    SectionAbstract.h
    SectionAdvancedEdit.h
    SectionDiskMenu.h
    SectionHDRecorder.h
    SectionInstruments.h
    SectionOptimize.h
    SectionQuickOptions.h
    SectionSamples.h
    SectionSettings.h
    SectionSwitcher.h
    SectionTranspose.h
    SectionUpperLeft.h
    SongLengthEstimator.h
    SystemMessage.h
    TabHeaderControl.h
    TabManager.h
    TabTitleProvider.h
    TitlePageManager.h
    ToolInvokeHelper.h
    Tracker.h
    TrackerConfig.h
    TrackerSettingsDatabase.h
    Undo.h
    VRand.h
    Zapper.h
)

include_directories(
    # Include the CMake-generated version header from the build directory
    ${PROJECT_BINARY_DIR}/src/tracker
    ${PROJECT_SOURCE_DIR}/src/compression
    ${PROJECT_SOURCE_DIR}/src/fx
    ${PROJECT_SOURCE_DIR}/src/milkyplay
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface
    ${PROJECT_SOURCE_DIR}/src/ppui/sdl
    ${PROJECT_SOURCE_DIR}/src/tracker
)

# Add the compression library.
# The compression library is special in that each decompressor has a constructor
# which statically self-registers the decompressor into a global decompressors
# list. Because they are not each individually referenced in code, the linker
# will discard them if they are archived into a static library. We could work
# around it with compiler-specific flags such as GCC's --whole-archive or
# Clang's --force_load, but instead we use this special CMake feature which lets
# us easily pull in the individual objects without having to manually list them.
list(APPEND SOURCES $<TARGET_OBJECTS:compression>)

# Add platform-specific sources and include paths
if(APPLE)
    # If generating for Xcode, pass in the Interface Builder source as a
    # resource and Xcode will take care of compiling it properly
    if(${CMAKE_GENERATOR} STREQUAL "Xcode")
        set_source_files_properties(
            cocoa/resources/Application.xib
            PROPERTIES
            MACOSX_PACKAGE_LOCATION
            Resources
        )
    endif()

    # Application and document icons
    file(
        GLOB ICONS ${PROJECT_SOURCE_DIR}/resources/pictures/docicons/osx/*.icns
    )
    list(APPEND ICONS ${PROJECT_SOURCE_DIR}/resources/pictures/carton.icns)

    # Ensure icons are copied to the correct bundle location
    set_source_files_properties(
        ${ICONS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )

    list(
        APPEND SOURCES
        ${ICONS}
        cocoa/AppDelegate.mm
        cocoa/MTKeyTranslator.mm
        cocoa/MTTrackerView.mm
        cocoa/main.mm
        cocoa/resources/Application.xib
    )
    list(
        APPEND HEADERS
        cocoa/AppDelegate.h
        cocoa/MTKeyTranslator.h
        cocoa/MTTrackerView.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/midi/osx
        ${PROJECT_SOURCE_DIR}/src/ppui/cocoa
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
        ${PROJECT_SOURCE_DIR}/src/tracker/cocoa
    )
elseif(WIN32)
    list(
        APPEND SOURCES
        win32/PreferencesDialog.cpp
        win32/ThreadTimer.cpp
        win32/Win32_main.cpp
        win32/Win32_resources.rc
    )
    list(
        APPEND HEADERS
        win32/PreferencesDialog.h
        win32/ThreadTimer.h
        win32/Win32_resource.h
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/midi/win32
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/win32
        ${PROJECT_SOURCE_DIR}/src/ppui/win32
    )
else()
    list(
        APPEND SOURCES
        sdl/SDL_KeyTranslation.cpp
        sdl/SDL_Main.cpp
    )
    list(
        APPEND HEADERS
        sdl/SDL_KeyTranslation.h
    )
    include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
    )
endif()

add_definitions(-DMILKYTRACKER)

# Set target names for the executables
if(APPLE OR WIN32)
    # OS X and Windows get a mixed-case binary name
    set(TARGET_NAME ${PROJECT_NAME})
else()
    # Linux/other UNIX get a lower-case binary name
    set(TARGET_NAME ${PROJECT_NAME_LOWER})
endif()

if(APPLE)
    add_executable(${TARGET_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS})

    # Xcode can deal with Interface Builder xibs automatically - if we are not
    # generating for Xcode, then we must manually compile and install any xibs
    if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
        # Locate ibtool
        find_program(IBTOOL ibtool HINTS /usr/bin ${OSX_DEVELOPER_ROOT}/usr/bin)
        if(IBTOOL STREQUAL "IBTOOL-NOTFOUND")
            message(SEND_ERROR "Unable to find ibtool. Is Xcode installed?")
        endif()

        # The Interface Builder xib file to be compiled
        set(
            XIB_FILE
            ${PROJECT_SOURCE_DIR}/src/tracker/cocoa/resources/Application.xib
        )

        # Destination for compiled xib
        set(RESOURCES_DIR $<TARGET_FILE_DIR:${TARGET_NAME}>/../Resources)

        # Ensure the destination directory of the compiled xib exists
        add_custom_command(
            TARGET ${TARGET_NAME} PRE_BUILD COMMAND mkdir -p ${RESOURCES_DIR}
        )

        # Compile the xib file
        add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD COMMAND ${IBTOOL} --errors
            --warnings --notices --output-format human-readable-text
            --minimum-deployment-target 10.7 --compile
            ${RESOURCES_DIR}/${MACOSX_BUNDLE_NSMAIN_NIB_FILE}.nib ${XIB_FILE}
            COMMENT "Compiling ${XIB_FILE}"
        )
    endif()

    # Pass in the Info.plist template, whose variables are defined in the
    # top-level CMakeLists.txt
    set_target_properties(
        ${TARGET_NAME}
        PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${PROJECT_SOURCE_DIR}/src/tracker/cocoa/resources/Info.plist.in
    )

    # Enable ARC (automatic reference counting) for OS X build
    set_property(
        TARGET ${TARGET_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc"
    )

    target_link_libraries(
        ${TARGET_NAME}
        midi
        ${CORE_VIDEO_LIBRARY}
        ${OPENGL_LIBRARY}
    )
elseif(WIN32)
    add_executable(${TARGET_NAME} WIN32 ${SOURCES} ${HEADERS})

    target_link_libraries(${TARGET_NAME} midi)
else()
    add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS})

    target_link_libraries(${TARGET_NAME} ${SDL2_LIBRARIES})

    if(ALSA_FOUND AND RTMIDI_FOUND)
        add_definitions(-DHAVE_LIBASOUND)
        target_link_libraries(${TARGET_NAME} midi ${RTMIDI_LIBRARIES})
    endif()
endif()

if(ZLIB_FOUND)
    target_link_libraries(${TARGET_NAME} ${ZLIB_LIBRARIES})
endif()

if(UNIX)
    if(ZZIPLIB_FOUND)
        target_link_libraries(${TARGET_NAME} ${ZZIPLIB_LIBRARIES})
    endif()

    if(LHASA_FOUND)
        target_link_libraries(${TARGET_NAME} ${LHASA_LIBRARIES})
    endif()
endif()

target_link_libraries(
    ${TARGET_NAME}
    fx
    milkyplay
    osinterface
    ppui
)

# OS X and Windows install to the root of the prefix, the others install to bin
if(APPLE OR WIN32)
    set(INSTALL_DEST .)
else()
    set(INSTALL_DEST ${CMAKE_INSTALL_BINDIR})
endif()

install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DEST})
