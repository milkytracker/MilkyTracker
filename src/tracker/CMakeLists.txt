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

add_executable(tracker
    # Sources
    AnimatedFXControl.cpp
    ColorExportImport.cpp
    ColorPaletteContainer.cpp
    DialogHelp.cpp
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
    Reverb.cpp
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

    # Headers
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
    Reverb.h
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

# Set target name for the executable
if(APPLE OR WIN32)
    # OS X and Windows get a mixed-case binary name
    set_target_properties(tracker PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
else()
    # Linux/other UNIX get a lower-case binary name
    set_target_properties(tracker PROPERTIES OUTPUT_NAME ${PROJECT_NAME_LOWER})
endif()

target_include_directories(tracker
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        # Include the CMake-generated version header from the build directory
        ${PROJECT_BINARY_DIR}/src/tracker
)

target_link_libraries(tracker
    fx
    milkyplay
    osinterface
    ppui
)

if(CMAKE_VERSION GREATER_EQUAL 3.12)
    # CMake >=3.12 can just "link" to object libraries and inherit any objects
    # and include paths
    target_link_libraries(tracker compression)
else()
    # Do things the hard way for older CMake
    target_sources(tracker PRIVATE
        $<TARGET_OBJECTS:compression>
    )
    target_include_directories(tracker PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../compression
    )
endif()

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

    target_sources(tracker PRIVATE
        # Sources
        ${ICONS}
        cocoa/AppDelegate.mm
        cocoa/MTKeyTranslator.mm
        cocoa/MTTrackerView.mm
        cocoa/main.mm
        cocoa/resources/Application.xib

        # Headers
        cocoa/AppDelegate.h
        cocoa/MTKeyTranslator.h
        cocoa/MTTrackerView.h
    )

    target_include_directories(tracker PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cocoa
    )
elseif(WIN32)
    target_sources(tracker PRIVATE
        # Sources
        win32/PreferencesDialog.cpp
        win32/ThreadTimer.cpp
        win32/Win32_main.cpp
        win32/Win32_resources.rc

        # Headers
        win32/PreferencesDialog.h
        win32/ThreadTimer.h
        win32/Win32_resource.h
    )
else()
    target_sources(tracker PRIVATE
        # Sources
        sdl/SDL_KeyTranslation.cpp
        sdl/SDL_Main.cpp

        # Headers
        sdl/SDL_KeyTranslation.h
    )
endif()

if(APPLE)
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
        set(RESOURCES_DIR $<TARGET_FILE_DIR:tracker>/../Resources)

        # Ensure the destination directory of the compiled xib exists
        add_custom_command(
            TARGET tracker PRE_BUILD COMMAND mkdir -p ${RESOURCES_DIR}
        )

        # Compile the xib file
        add_custom_command(
            TARGET tracker POST_BUILD COMMAND ${IBTOOL} --errors
            --warnings --notices --output-format human-readable-text
            --minimum-deployment-target 10.7 --compile
            ${RESOURCES_DIR}/${MACOSX_BUNDLE_NSMAIN_NIB_FILE}.nib ${XIB_FILE}
            COMMENT "Compiling ${XIB_FILE}"
        )
    endif()

    # Build as an Application Bundle
    set_target_properties(tracker PROPERTIES MACOSX_BUNDLE TRUE)

    # Pass in the Info.plist template, whose variables are defined in the
    # top-level CMakeLists.txt
    set_target_properties(tracker PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/resources/Info.plist.in
    )

    # Enable ARC (automatic reference counting) for OS X build
    target_compile_options(tracker PRIVATE -fobjc-arc)

    target_link_libraries(tracker
        midi
        ${CORE_VIDEO_LIBRARY}
        ${OPENGL_LIBRARY}
    )
elseif(WIN32)
    set_target_properties(tracker PROPERTIES WIN32_EXECUTABLE TRUE)

    target_link_libraries(tracker midi)
else()
    if(ALSA_FOUND AND RTMIDI_FOUND)
        target_compile_definitions(tracker PRIVATE -DHAVE_LIBRTMIDI)
        target_link_libraries(tracker midi)
    endif()
endif()

if(ZLIB_FOUND)
    target_link_libraries(tracker ${ZLIB_LIBRARIES})
endif()

if(UNIX)
    if(ZZIPLIB_FOUND)
        target_link_libraries(tracker ${ZZIPLIB_LIBRARIES})
    endif()

    if(LHASA_FOUND)
        target_link_libraries(tracker ${LHASA_LIBRARIES})
    endif()
endif()

# OS X and Windows install to the root of the prefix, the others install to bin
if(APPLE OR WIN32)
    set(INSTALL_DEST .)
else()
    set(INSTALL_DEST ${CMAKE_INSTALL_BINDIR})
endif()

install(TARGETS tracker DESTINATION ${INSTALL_DEST})
