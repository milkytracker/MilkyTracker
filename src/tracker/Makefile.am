bin_PROGRAMS = milkytracker

milkytracker_SOURCES = AnimatedFXControl.cpp ColorExportImport.cpp \
ColorPaletteContainer.cpp DialogChannelSelector.cpp DialogEQ.cpp \
DialogGroupSelection.cpp DialogHandlers.cpp DialogListBox.cpp \
DialogPanning.cpp DialogQuickChooseInstrument.cpp DialogResample.cpp \
DialogWithValues.cpp DialogZap.cpp EQConstants.cpp EditorBase.cpp \
EnvelopeContainer.cpp EnvelopeEditor.cpp EnvelopeEditorControl.cpp \
Equalizer.cpp FileExtProvider.cpp FileIdentificator.cpp GlobalColorConfig.cpp \
InputControlListener.cpp LogoBig.cpp LogoSmall.cpp ModuleEditor.cpp \
ModuleServices.cpp PatternEditor.cpp PatternEditorClipBoard.cpp \
PatternEditorControl.cpp PatternEditorControlEventListener.cpp \
PatternEditorControlKeyboard.cpp PatternEditorControlTransposeHandler.cpp \
PatternEditorTools.cpp PatternTools.cpp PeakLevelControl.cpp Piano.cpp \
PianoControl.cpp PlayerController.cpp PlayerLogic.cpp PlayerMaster.cpp \
RecPosProvider.cpp RecorderLogic.cpp ResamplerHelper.cpp SampleEditor.cpp \
SampleEditorControl.cpp SampleEditorControlToolHandler.cpp \
SampleEditorResampler.cpp SamplePlayer.cpp ScopesControl.cpp SectionAbout.cpp \
SectionAbstract.cpp SectionAdvancedEdit.cpp SectionDiskMenu.cpp \
SectionHDRecorder.cpp SectionInstruments.cpp SectionOptimize.cpp \
SectionQuickOptions.cpp SectionSamples.cpp SectionSettings.cpp \
SectionSwitcher.cpp SectionTranspose.cpp SectionUpperLeft.cpp \
SongLengthEstimator.cpp SystemMessage.cpp TabHeaderControl.cpp TabManager.cpp \
TabTitleProvider.cpp TitlePageManager.cpp ToolInvokeHelper.cpp Tracker.cpp \
TrackerConfig.cpp TrackerInit.cpp TrackerKeyboard.cpp TrackerSettings.cpp \
TrackerSettingsDatabase.cpp TrackerShortCuts.cpp TrackerShutDown.cpp \
TrackerStartUp.cpp TrackerUpdate.cpp Undo.cpp VRand.cpp Zapper.cpp \
sdl/SDL_KeyTranslation.cpp sdl/SDL_Main.cpp

noinst_HEADERS = AnimatedFXControl.h ColorExportImport.h \
ColorPaletteContainer.h ControlIDs.h DialogChannelSelector.h DialogEQ.h \
DialogGroupSelection.h DialogHandlers.h DialogListBox.h DialogPanning.h \
DialogQuickChooseInstrument.h DialogResample.h DialogWithValues.h DialogZap.h \
EQConstants.h EditModes.h EditorBase.h EnvelopeContainer.h EnvelopeEditor.h \
EnvelopeEditorControl.h Equalizer.h FileExtProvider.h FileIdentificator.h \
FileTypes.h FilterParameters.h GlobalColorConfig.h InputControlListener.h \
LogoBig.h LogoSmall.h ModuleEditor.h ModuleServices.h PatternEditor.h \
PatternEditorControl.h PatternEditorTools.h PatternTools.h PeakLevelControl.h \
Piano.h PianoControl.h PlayerController.h PlayerCriticalSection.h \
PlayerLogic.h PlayerMaster.h RecPosProvider.h RecorderLogic.h \
ResamplerHelper.h SIPButtons.h SampleEditor.h SampleEditorControl.h \
SampleEditorControlLastValues.h SampleEditorResampler.h SamplePlayer.h \
ScopesControl.h SectionAbout.h SectionAbstract.h SectionAdvancedEdit.h \
SectionDiskMenu.h SectionHDRecorder.h SectionInstruments.h SectionOptimize.h \
SectionQuickOptions.h SectionSamples.h SectionSettings.h SectionSwitcher.h \
SectionTranspose.h SectionUpperLeft.h SongLengthEstimator.h SystemMessage.h \
TabHeaderControl.h TabManager.h TabTitleProvider.h TitlePageManager.h \
TitlePageManager.h ToolInvokeHelper.h Tracker.h TrackerConfig.h \
TrackerSettingsDatabase.h Undo.h VRand.h Zapper.h sdl/SDL_KeyTranslation.h

AM_CPPFLAGS = -DMILKYTRACKER -D__THREADTIMER__ -D__LINUX__ ${SDL_CFLAGS}

AM_CPPFLAGS += -I$(top_srcdir)/src/${RTMIDI_DIR} -I$(top_srcdir)/src/compression \
-I$(top_srcdir)/src/fx -I$(top_srcdir)/src/milkyplay -I$(top_srcdir)/src/ppui \
-I$(top_srcdir)/src/tracker -I$(top_srcdir)/src/midi/posix -I../midi/rtmidi \
-I$(top_srcdir)/src/ppui/osinterface -I$(top_srcdir)/src/ppui/osinterface/sdl \
-I$(top_srcdir)/src/ppui/osinterface/posix -I$(top_srcdir)/src/ppui/sdl -Isdl

# The following cannot be linked as a static library, the linker will regard
# the DecompressorXXX files as unused and therefore won't pull them in:
compressionLib = $(top_builddir)/src/compression/Decompressor.o \
$(top_builddir)/src/compression/DecompressorGZIP.o \
$(top_builddir)/src/compression/DecompressorLHA.o \
$(top_builddir)/src/compression/DecompressorPP20.o \
$(top_builddir)/src/compression/DecompressorUMX.o \
$(top_builddir)/src/compression/DecompressorZIP.o \
$(top_builddir)/src/compression/zziplib/MyIO.o \
$(top_builddir)/src/compression/PP20.o \
$(top_builddir)/src/compression/ZipExtractor.o \
$(top_builddir)/src/compression/lha/unlha.o

# add -lGL for experimental OpenGL support (and see ../ppui/Makefile.am)
milkytracker_LDADD = $(top_builddir)/src/milkyplay/libmilkyplay.a \
$(top_builddir)/src/ppui/osinterface/libosinterface.a \
$(top_builddir)/src/ppui/libppui.a \
$(top_builddir)/src/ppui/osinterface/libosinterface.a \
$(top_builddir)/src/fx/libfx.a $(compressionLib) $(RTMIDI) $(SDL_LIBS) \
$(ALSA_LIBS) $(Z_LIBS) $(ZZIP_LIBS)

EXTRA_DIST = carbon/Carbon_Definitions.h carbon/Carbon_main.cpp \
carbon/InfoPlist.strings carbon/KeyTranslation.cpp carbon/KeyTranslation.h \
carbon/PreferencesDialog.cpp carbon/PreferencesDialog.h carbon/iGetKeys.cpp \
carbon/iGetKeys.h win32/PreferencesDialog.cpp win32/PreferencesDialog.h \
win32/ThreadTimer.cpp win32/ThreadTimer.h win32/Win32_main.cpp \
win32/Win32_resource.h win32/Win32_resources.rc wince/ButtonMapper.cpp \
wince/ButtonMapper.h wince/VirtualKeyToScanCodeTable.cpp \
wince/Logger.cpp wince/Logger.h wince/Simple.h \
wince/VirtualKeyToScanCodeTable.h wince/WaitStateThread.cpp \
wince/WaitStateThread.h wince/WinCE_NEWRES.H wince/WinCE_ThreadTimer.cpp \
wince/WinCE_ThreadTimer.h wince/WinCE_main.cpp wince/WinCE_main.h \
wince/WinCE_resource.h

# Haiku
EXTRA_DIST += haiku/Haiku_main.cpp haiku/MilkyApplication.cpp \
haiku/MilkyApplication.h \
haiku/MilkySettings/InterfaceSettingsView.cpp \
haiku/MilkySettings/InterfaceSettingsView.h \
haiku/MilkySettings/MidiSettingsView.cpp \
haiku/MilkySettings/MidiSettingsView.h \
haiku/MilkySettings/MilkySettingsApplication.cpp \
haiku/MilkySettings/MilkySettingsApplication.h \
haiku/MilkySettings/SettingsMessages.h \
haiku/MilkySettings/SettingsWindow.cpp \
haiku/MilkySettings/SettingsWindow.h haiku/MilkySettings/main.cpp \
haiku/MilkyTracker.rdef haiku/Tools.cpp
