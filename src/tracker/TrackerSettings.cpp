/*
 *  tracker/TrackerSettings.cpp
 *
 *  Copyright 2009 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  TrackerSettings.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 19 2005.
 *
 */

#include "Tracker.h"
#include "ModuleEditor.h"
#include "TrackerSettingsDatabase.h"
#include "PlayerMaster.h"
#include "PlayerController.h"
#include "PlayerLogic.h"
#include "RecorderLogic.h"
#include "TabManager.h"
#include "Dictionary.h"
#include "PatternEditorControl.h"
#include "SampleEditorControl.h"
#include "EnvelopeEditorControl.h"
#include "SectionSamples.h"
#include "SystemMessage.h"
#include "ScopesControl.h"
#include "SectionInstruments.h"
#include "SectionSettings.h"
#include "SectionSamples.h"
#include "SectionDiskMenu.h"
#include "SectionHDRecorder.h"
#include "SectionQuickOptions.h"
#include "SectionOptimize.h"
#include "TrackerConfig.h"
#include "PPUIConfig.h"
#include "ColorPaletteContainer.h"
#include "Tools.h"
#include "TitlePageManager.h"
#include "version.h"

void Tracker::buildDefaultSettings()
{
	if (settingsDatabase == NULL)
		return;

	pp_int32 i;
	char buffer[100];

	// store version string to database
	settingsDatabase->store("VERSION", MILKYTRACKER_VERSION);
	// ---------- Mixer ----------
#ifdef __DEFAULTBUFFERSIZE__
	settingsDatabase->store("BUFFERSIZE", __DEFAULTBUFFERSIZE__);
#else
	settingsDatabase->store("BUFFERSIZE", PlayerMaster::getPreferredBufferSize());
#endif
	settingsDatabase->store("MIXERVOLUME", 256);
	settingsDatabase->store("MIXERSHIFT", 1);
	settingsDatabase->store("RAMPING", 1);
	settingsDatabase->store("INTERPOLATION", 1);
	settingsDatabase->store("MIXERFREQ", PlayerMaster::getPreferredSampleRate());
#ifdef __FORCEPOWEROFTWOBUFFERSIZE__
	settingsDatabase->store("FORCEPOWEROFTWOBUFFERSIZE", 1);
#else
	settingsDatabase->store("FORCEPOWEROFTWOBUFFERSIZE", 0);
#endif
	// Store audio driver
	settingsDatabase->store("AUDIODRIVER", PlayerMaster::getPreferredAudioDriverID());

	// the first key HAS TO BE PLAYMODEKEEPSETTINGS
	settingsDatabase->store("PLAYMODEKEEPSETTINGS", 0);
	settingsDatabase->store("PLAYMODE","FASTTRACKER2");
	settingsDatabase->store("PLAYMODE_ADVANCED_ALLOW8xx",1);
	settingsDatabase->store("PLAYMODE_ADVANCED_ALLOWE8x",0);
	// Only affects protracker playmodes
	settingsDatabase->store("PLAYMODE_ADVANCED_PTPITCHLIMIT",1);
	settingsDatabase->store("PLAYMODE_ADVANCED_PTPANNING", TrackerConfig::defaultProTrackerPanning);

	// ---------- Optimize --------
	for (i = 0; i < (signed)SectionOptimize::getNumFlagGroups(); i++)
	{
		sprintf(buffer, "OPTIMIZER_%i",i);
		settingsDatabase->store(buffer, SectionOptimize::getDefaultFlags(i));
	}

	// ---------- Layout ----------
	settingsDatabase->store("FULLSCREEN", 0);

	settingsDatabase->store("XRESOLUTION", PPScreen::getDefaultWidth());
	settingsDatabase->store("YRESOLUTION", PPScreen::getDefaultHeight());

	settingsDatabase->store("SCREENSCALEFACTOR", 1);

	settingsDatabase->store("ENVELOPEEDITORSCALE", 256);

	for (i = 0; i < PPFont::FONT_LAST; i++)
	{
		const char* keyName = PPFont::getFamilyInternalName((PPFont::FontID)i);
		const char* valueName = PPFont::getCurrentFontFace((PPFont::FontID)i);
		settingsDatabase->store(keyName, valueName);
	}

#ifdef __LOWRES__
	settingsDatabase->store("PATTERNFONT", PPFont::FONT_TINY);
#else
	settingsDatabase->store("PATTERNFONT", PPFont::FONT_SYSTEM);
#endif

	// Scopes?
	settingsDatabase->store("SCOPES", 1);

	// Pattern spacing
	settingsDatabase->store("SPACING", 0);
	// Trace instruments setting
	settingsDatabase->store("INSTRUMENTBACKTRACE", 0);
	// TAB to note?
	settingsDatabase->store("TABTONOTE", 1);
	// mouseclick to cursor?
	settingsDatabase->store("CLICKTOCURSOR", 1);
	// autoresize pattern on paste?
	settingsDatabase->store("PATTERNAUTORESIZE", 0);
	// Show hex row numbers
	settingsDatabase->store("HEXCOUNT", 1);
	// Show zeroes instead of dots for unused effects
	settingsDatabase->store("SHOWZEROEFFECT", 0);
	// Wrap around cursor
	settingsDatabase->store("WRAPAROUND", 1);
	// Beeing prospective
	settingsDatabase->store("PROSPECTIVE", 0);
	// follow song when playing
	settingsDatabase->store("FOLLOWSONG", 1);
	// Live switch
	settingsDatabase->store("LIVESWITCH", 0);
	// Our default edit mode
#ifdef __LOWRES__
	settingsDatabase->store("EDITMODE", EditModeMilkyTracker);
#else
	settingsDatabase->store("EDITMODE", EditModeFastTracker);
#endif
	// Our default scrolling mode
	settingsDatabase->store("SCROLLMODE", ScrollModeStayInCenter);
	// Mute fading value (from 0 to 100 percent)
	settingsDatabase->store("MUTEFADE", 50);
	// Modulo for the first pattern highlight
	settingsDatabase->store("HIGHLIGHTMODULO1", 4);
	// Modulo for the first pattern highlight
	settingsDatabase->store("HIGHLIGHTROW1", 0);
	// Modulo for the second pattern highlight
	settingsDatabase->store("HIGHLIGHTMODULO2", 8);
	// Modulo for the second pattern highlight
	settingsDatabase->store("HIGHLIGHTROW2", 0);

	// Enable sample undobuffer by default
	settingsDatabase->store("SAMPLEEDITORUNDOBUFFER", 1);
	// Auto-mixdown to mono when loading samples
	settingsDatabase->store("AUTOMIXDOWNSAMPLES", 0);
	// Hexadecimal offsets in the sample editor by default
	settingsDatabase->store("SAMPLEEDITORDECIMALOFFSETS", 0);
	// use internal disk browser?
	settingsDatabase->store("INTERNALDISKBROWSER", 0);
	// disk browser settings
	settingsDatabase->store("INTERNALDISKBROWSERSETTINGS", SectionDiskMenu::getDefaultConfigUInt32());
	settingsDatabase->store("INTERNALDISKBROWSERLASTPATH", "");
	// Estimate playtime after a song has been loaded?
#ifdef __LOWRES__
	settingsDatabase->store("AUTOESTPLAYTIME", 0);
#else
	settingsDatabase->store("AUTOESTPLAYTIME", 1);
#endif
	// show splash screen?
	settingsDatabase->store("SHOWSPLASH", 1);
	// orderlist is extended
	settingsDatabase->store("EXTENDEDORDERLIST", 0);
	// current row add
	settingsDatabase->store("ROWINSERTADD", 1);
	// show title field
	settingsDatabase->store("TITLEPAGE", TitlePageManager::PageTitle);
	// sample editor last settings
	settingsDatabase->store("SAMPLEEDITORLASTVALUES", "");
	// no virtual channels for instrument playback
	settingsDatabase->store("VIRTUALCHANNELS", 0);
	// enable multichn recording by default
	settingsDatabase->store("MULTICHN_RECORD", 1);
	// enable multichn keyjazzing by default
	settingsDatabase->store("MULTICHN_KEYJAZZ", 1);
	// disable multichn edit by default
	settingsDatabase->store("MULTICHN_EDIT", 0);
	// enable key off recording by default
	settingsDatabase->store("MULTICHN_RECORDKEYOFF", 1);
	// disable note delay recording
	settingsDatabase->store("MULTICHN_RECORDNOTEDELAY", 0);
	// Invert mousewheel zoom?  (normally wheelup zooms out: if inverted, wheelup zooms in)
	settingsDatabase->store("INVERTMWHEELZOOM", 0);

	// ---------- Tabs ----------
	// Control playing of background tabs
	settingsDatabase->store("TABS_STOPBACKGROUNDBEHAVIOUR", TabManager::StopTabsBehaviourNone);
	settingsDatabase->store("TABS_TABSWITCHRESUMEPLAY", 0);
	settingsDatabase->store("TABS_LOADMODULEINNEWTAB", 0);

	settingsDatabase->store("ACTIVECOLORS", TrackerConfig::defaultColorPalette);

	// Store volume envelopes
	for (i = 0; i < TrackerConfig::numPredefinedEnvelopes; i++)
	{
		sprintf(buffer, "PREDEFENVELOPEVOLUME_%i",i);
		settingsDatabase->store(buffer, TrackerConfig::defaultPredefinedVolumeEnvelope);
	}

	// Store panning envelopes
	for (i = 0; i < TrackerConfig::numPredefinedEnvelopes; i++)
	{
		sprintf(buffer, "PREDEFENVELOPEPANNING_%i",i);
		settingsDatabase->store(buffer, TrackerConfig::defaultPredefinedPanningEnvelope);
	}

	// ---------- HD recorder last settings ----------
	settingsDatabase->store("HDRECORDER_MIXFREQ", 44100);
	settingsDatabase->store("HDRECORDER_MIXERVOLUME", 256);
	settingsDatabase->store("HDRECORDER_MIXERSHIFT", 1);
	settingsDatabase->store("HDRECORDER_RAMPING", 1);
	settingsDatabase->store("HDRECORDER_INTERPOLATION", 1);
	settingsDatabase->store("HDRECORDER_ALLOWMUTING", 0);

	for (i = 0; i < NUMEFFECTMACROS; i++)
	{
		sprintf(buffer, "EFFECTMACRO_%i",i);
		settingsDatabase->store(buffer, 0);
	}

	// store predefined colorsets
	for (i = 0; i < TrackerConfig::numPredefinedColorPalettes; i++)
	{
		sprintf(buffer, "PREDEFCOLORPALETTE_%i",i);
		settingsDatabase->store(buffer, TrackerConfig::predefinedColorPalettes[i]);
	}

	//settingsDatabase->dump();
}

void Tracker::applySettingByKey(PPDictionaryKey* theKey, TMixerSettings& settings, pp_uint32 version)
{
	PatternEditorControl* patternEditorCtrl = getPatternEditorControl();
	PatternEditor* patternEditor = moduleEditor->getPatternEditor();
	SampleEditor* sampleEditor = moduleEditor->getSampleEditor();
	SampleEditorControl* sampleEditorControl = sectionSamples->getSampleEditorControl();

	pp_int32 v2 = theKey->getIntValue();

	if (theKey->getKey().compareTo("BUFFERSIZE") == 0)
	{
		// check for backward compatibility
		// 0x9071 was the version which allowed for buffer size in samples
		// instead of 250hz "packets"
		if (version > 0x9070)
			settings.bufferSize = v2;
		else
			theKey->store(PlayerMaster::getPreferredBufferSize());
	}
	else if (theKey->getKey().compareTo("MIXERFREQ") == 0)
	{
		settings.mixFreq = v2;
	}
	else if (theKey->getKey().compareTo("MIXERVOLUME") == 0)
	{
		settings.mixerVolume = v2;
	}
	else if (theKey->getKey().compareTo("MIXERSHIFT") == 0)
	{
		settings.mixerShift = 2-v2;
	}
	else if (theKey->getKey().compareTo("RAMPING") == 0)
	{
		settings.ramping = v2;
	}
	else if (theKey->getKey().compareTo("INTERPOLATION") == 0)
	{
		settings.resampler = v2;
	}
	else if (theKey->getKey().compareTo("FORCEPOWEROFTWOBUFFERSIZE") == 0)
	{
		settings.powerOfTwoCompensation = v2;
	}
	else if (theKey->getKey().compareTo("AUDIODRIVER") == 0)
	{
		settings.setAudioDriverName(theKey->getStringValue());
	}
	else if (theKey->getKey().compareTo("PLAYMODEKEEPSETTINGS") == 0)
	{
		sectionQuickOptions->setKeepSettings(v2 != 0);
	}
	else if (theKey->getKey().compareTo("PLAYMODE") == 0)
	{
		PPString str = theKey->getStringValue();
		if (str.compareTo("PROTRACKER2") == 0)
		{
			playerController->switchPlayMode(PlayerController::PlayMode_ProTracker2, false);
			// we set default to 4 channels so people will immediately be able to see
			// which playmode is the active one
			setModuleNumChannels(4);
		}
		else if (str.compareTo("PROTRACKER3") == 0)
		{
			playerController->switchPlayMode(PlayerController::PlayMode_ProTracker3, false);
			// see above comment
			setModuleNumChannels(4);
		}
		else playerController->switchPlayMode(PlayerController::PlayMode_FastTracker2, false);
	}
	else if (theKey->getKey().compareTo("PLAYMODE_ADVANCED_ALLOW8xx") == 0)
	{
		playerController->enablePlayModeOption(PlayerController::PlayModeOptionPanning8xx, v2 != 0);
	}
	else if (theKey->getKey().compareTo("PLAYMODE_ADVANCED_ALLOWE8x") == 0)
	{
		playerController->enablePlayModeOption(PlayerController::PlayModeOptionPanningE8x, v2 != 0);
	}
	else if (theKey->getKey().compareTo("PLAYMODE_ADVANCED_PTPITCHLIMIT") == 0)
	{
		playerController->enablePlayModeOption(PlayerController::PlayModeOptionForcePTPitchLimit, v2 != 0);
	}
	else if (theKey->getKey().compareTo("PLAYMODE_ADVANCED_PTPANNING") == 0)
	{
		pp_uint8* panning = new pp_uint8[TrackerConfig::numPlayerChannels];
		if (PPTools::decodeByteArray(panning, TrackerConfig::numPlayerChannels, theKey->getStringValue()))
		{
			pp_int32 i;
			for (i = 0; i < TrackerConfig::numPlayerChannels; i++)
				playerController->setPanning((pp_uint8)i, panning[i]);
		}
		delete[] panning;
	}
	// ---------------- Virtal channels -------------------
	else if (theKey->getKey().compareTo("VIRTUALCHANNELS") == 0)
	{
		settings.numVirtualChannels = v2;
	}
	else if (theKey->getKey().compareTo("FULLSCREEN") == 0)
	{
		bool fullScreen = (v2 != 0);
		if (fullScreen != screen->isFullScreen())
		{
			bool res = screen->goFullScreen(fullScreen);
			theKey->store(screen->isFullScreen());

			if (!res)
			{
				SystemMessage message(*screen, SystemMessage::MessageFullScreenFailed);
				message.show();
			}
		}
	}
	else if (theKey->getKey().compareTo("ENVELOPEEDITORSCALE") == 0)
	{
		if (sectionInstruments && sectionInstruments->getEnvelopeEditor())
			sectionInstruments->getEnvelopeEditorControl()->setScale(v2);
	}
	else if (theKey->getKey().compareTo("PATTERNFONT") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setFont(PPFont::getFont(v2));
	}
	else if (theKey->getKey().compareTo("SCOPES") == 0)
	{
		showScopes(v2 & 1, v2>>1);
	}
	else if (theKey->getKey().compareTo("SPACING") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setSpacing(v2);
	}
	else if (theKey->getKey().compareTo("HIGHLIGHTMODULO1") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setHighlightSpacingPrimary(v2);
	}
	else if (theKey->getKey().compareTo("HIGHLIGHTROW1") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setHighLightRowPrimary(v2 != 0);
	}
	else if (theKey->getKey().compareTo("HIGHLIGHTMODULO2") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setHighlightSpacingSecondary(v2);
	}
	else if (theKey->getKey().compareTo("HIGHLIGHTROW2") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setHighLightRowSecondary(v2 != 0);
	}
	else if (theKey->getKey().compareTo("INSTRUMENTBACKTRACE") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setInstrumentBackTrace(v2 != 0);
	}
	else if (theKey->getKey().compareTo("TABTONOTE") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setTabToNote(v2 != 0);
	}
	else if (theKey->getKey().compareTo("CLICKTOCURSOR") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setClickToCursor(v2 != 0);
	}
	else if (theKey->getKey().compareTo("PATTERNAUTORESIZE") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setAutoResize(v2 != 0);
	}
	else if (theKey->getKey().compareTo("HEXCOUNT") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setHexCount(v2 != 0);
	}
	else if (theKey->getKey().compareTo("SHOWZEROEFFECT") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->showZeroEffect(v2 != 0);
	}
	else if (theKey->getKey().compareTo("WRAPAROUND") == 0)
	{
		setCursorWrapAround(v2 != 0, false);
	}
	else if (theKey->getKey().compareTo("PROSPECTIVE") == 0)
	{
		setProspectiveMode(v2 != 0, false);
	}
	else if (theKey->getKey().compareTo("FOLLOWSONG") == 0)
	{
		setFollowSong(v2 != 0, false);
	}
	else if (theKey->getKey().compareTo("LIVESWITCH") == 0)
	{
		setLiveSwitch(v2 != 0, false);
	}
	else if (theKey->getKey().compareTo("EDITMODE") == 0)
	{
		switchEditMode((EditModes)v2);
	}
	else if (theKey->getKey().compareTo("SCROLLMODE") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setScrollMode((ScrollModes)v2);
	}
	else if (theKey->getKey().compareTo("MUTEFADE") == 0)
	{
		if (patternEditorCtrl)
			patternEditorCtrl->setMuteFade((v2*65536)/100);
	}
	else if (theKey->getKey().compareTo("SAMPLEEDITORUNDOBUFFER") == 0)
	{
		if (sampleEditor)
			sampleEditor->enableUndoStack(v2 != 0);
	}
	else if (theKey->getKey().compareTo("SAMPLEEDITORDECIMALOFFSETS") == 0)
	{
		if (sectionSamples)
			sectionSamples->setOffsetFormat(v2);
	}
	else if (theKey->getKey().compareTo("SAMPLEEDITORLASTVALUES") == 0)
	{
		if (sampleEditorControl)
		{
			PPDictionary* dict = PPDictionary::createFromString(theKey->getStringValue());
			if (dict)
				sampleEditorControl->getLastValues().restoreFromDictionary(*dict);

			delete dict;
		}
	}
	else if (theKey->getKey().compareTo("INTERNALDISKBROWSER") == 0)
	{
		useClassicBrowser = (v2 != 0);
	}
	else if (theKey->getKey().compareTo("INTERNALDISKBROWSERSETTINGS") == 0)
	{
		if (sectionDiskMenu)
			sectionDiskMenu->setConfigUInt32(v2);
	}
	else if (theKey->getKey().compareTo("INTERNALDISKBROWSERLASTPATH") == 0)
	{
		if (sectionDiskMenu)
		{
			PPSystemString path(theKey->getStringValue());
			sectionDiskMenu->setCurrentPath(path, false);
		}
	}
	else if (theKey->getKey().compareTo("AUTOESTPLAYTIME") == 0)
	{
		if (v2)
			estimateSongLength();
	}
	else if (theKey->getKey().compareTo("EXTENDEDORDERLIST") == 0)
	{
		expandOrderlist(v2 != 0);
	}
	else if (theKey->getKey().compareTo("ROWINSERTADD") == 0)
	{
		getPatternEditorControl()->setRowInsertAdd(v2);
	}
	else if (theKey->getKey().compareTo("TITLEPAGE") == 0)
	{
		TitlePageManager titlePageManager(*screen);
		titlePageManager.showTitlePage((TitlePageManager::Pages)v2, false);
	}
	// ---------------- HD Recorder settings -------------------
	else if (theKey->getKey().compareTo("HDRECORDER_MIXFREQ") == 0)
	{
		sectionHDRecorder->setSettingsFrequency(v2);
	}
	else if (theKey->getKey().compareTo("HDRECORDER_MIXERVOLUME") == 0)
	{
		sectionHDRecorder->setSettingsMixerVolume(v2);
	}
	else if (theKey->getKey().compareTo("HDRECORDER_MIXERSHIFT") == 0)
	{
		sectionHDRecorder->setSettingsMixerShift(v2);
	}
	else if (theKey->getKey().compareTo("HDRECORDER_RAMPING") == 0)
	{
		sectionHDRecorder->setSettingsRamping(v2 != 0);
	}
	else if (theKey->getKey().compareTo("HDRECORDER_INTERPOLATION") == 0)
	{
		sectionHDRecorder->setSettingsResampler(v2);
	}
	else if (theKey->getKey().compareTo("HDRECORDER_ALLOWMUTING") == 0)
	{
		sectionHDRecorder->setSettingsAllowMuting(v2 != 0);
	}
	// ---------------- Recording & stuff ------------------
	else if (theKey->getKey().compareTo("MULTICHN_RECORD") == 0)
	{
		playerMaster->setMultiChannelRecord(v2 != 0);
	}
	else if (theKey->getKey().compareTo("MULTICHN_KEYJAZZ") == 0)
	{
		playerMaster->setMultiChannelKeyJazz(v2 != 0);
	}
	else if (theKey->getKey().compareTo("MULTICHN_EDIT") == 0)
	{
		patternEditorCtrl->setMultiChannelEdit(v2 != 0);
	}
	else if (theKey->getKey().compareTo("MULTICHN_RECORDKEYOFF") == 0)
	{
		recorderLogic->setRecordKeyOff(v2 != 0);
	}
	else if (theKey->getKey().compareTo("MULTICHN_RECORDNOTEDELAY") == 0)
	{
		recorderLogic->setRecordNoteDelay(v2 != 0);
	}
	else if (theKey->getKey().compareTo("INVERTMWHEELZOOM") == 0)
	{
		sectionSamples->getSampleEditorControl()->setInvertMWheelZoom(v2 != 0);
		sectionInstruments->getEnvelopeEditorControl()->setInvertMWheelZoom(v2 != 0);
	}
	// ----------------------- Tabs -------------------------
	else if (theKey->getKey().compareTo("TABS_STOPBACKGROUNDBEHAVIOUR") == 0)
	{
		switch (v2)
		{
			case TabManager::StopTabsBehaviourNone:
				tabManager->setStopOnTabSwitch(false);
				playerLogic->setStopBackgroundOnPlay(false);
				break;
			case TabManager::StopTabsBehaviourOnTabSwitch:
				tabManager->setStopOnTabSwitch(true);
				playerLogic->setStopBackgroundOnPlay(false);
				break;
			case TabManager::StopTabsBehaviourOnPlayback:
				tabManager->setStopOnTabSwitch(false);
				playerLogic->setStopBackgroundOnPlay(true);
				break;
		}
	}
	else if (theKey->getKey().compareTo("TABS_TABSWITCHRESUMEPLAY") == 0)
	{
		tabManager->setResumeOnTabSwitch(v2 != 0);
	}
	// ------------------ color palette  --------------------
	else if (theKey->getKey().compareTo("ACTIVECOLORS") == 0)
	{
		TColorPalette pal;
		PPString str = theKey->getStringValue();
		pal = ColorPaletteContainer::decodePalette(str);

		// Set colors added in 0.90.87 to default values if config file is from older version
		if (version < 0x009087)
		{
			pal.colors[GlobalColorConfig::ColorScrollBarBackground] = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorScrollBarBackground);
			pal.colors[GlobalColorConfig::ColorRecordModeButtonText] = TrackerConfig::colorRecordModeButtonText;
			pal.colors[GlobalColorConfig::ColorScopesRecordIndicator] = TrackerConfig::colorScopesRecordIndicator;
			pal.colors[GlobalColorConfig::ColorPeakClipIndicator] = TrackerConfig::colorPeakClipIndicator;
			pal.colors[GlobalColorConfig::ColorSampleEditorWaveform] = TrackerConfig::colorSampleEditorWaveform;
		}

		for (pp_int32 i = 0; i < pal.numColors; i++)
		{
			if (i < GlobalColorConfig::ColorLast)
				GlobalColorConfig::getInstance()->setColor((GlobalColorConfig::GlobalColors)i, pal.colors[i]);
		}
	}
	else if (theKey->getKey().startsWith("PREDEFCOLORPALETTE_"))
	{
		if (sectionSettings)
		{
			PPString str = (const char*)theKey->getKey()+19;
			pp_int32 i = str.getIntValue();
			str = theKey->getStringValue();
			sectionSettings->setEncodedPalette(i, str);
		}
	}
	// ------------------ envelopes  --------------------
	else if (theKey->getKey().startsWith("PREDEFENVELOPEVOLUME_"))
	{
		if (sectionInstruments)
		{
			PPString str = (const char*)theKey->getKey()+21;
			pp_int32 i = str.getIntValue();
			str = theKey->getStringValue();
			sectionInstruments->setEncodedEnvelope(SectionInstruments::EnvelopeTypeVolume, i, str);
		}
	}
	else if (theKey->getKey().startsWith("PREDEFENVELOPEPANNING_"))
	{
		if (sectionInstruments)
		{
			PPString str = (const char*)theKey->getKey()+22;
			pp_int32 i = str.getIntValue();
			str = theKey->getStringValue();
			sectionInstruments->setEncodedEnvelope(SectionInstruments::EnvelopeTypePanning, i, str);
		}
	}
	// ------------------ effect macros  --------------------
	else if (theKey->getKey().startsWith("EFFECTMACRO_"))
	{
		if (patternEditor)
		{
			PPString str = (const char*)theKey->getKey()+12;
			pp_int32 i = str.getIntValue();
			patternEditor->setMacroOperands(i, (pp_uint8)(v2 >> 8), (pp_uint8)(v2 & 0xff));
		}
	}
	else if (theKey->getKey().startsWith("OPTIMIZER_"))
	{
		if (sectionOptimize)
		{
			PPString str = (const char*)theKey->getKey()+10;
			pp_int32 i = str.getIntValue();
			sectionOptimize->setOptimizeCheckBoxFlags(i, v2);
		}
	}
	else
	{
		for (pp_uint32 i = 0; i < PPFont::FONT_LAST; i++)
		{
			const char* keyName = PPFont::getFamilyInternalName((PPFont::FontID)i);

			if (theKey->getKey().compareTo(keyName) == 0)
			{
				PPFont::selectFontFace((PPFont::FontID)i, theKey->getStringValue());
			}
		}
	}
}

void Tracker::getMixerSettingsFromDatabase(TMixerSettings& mixerSettings,
										   TrackerSettingsDatabase& currentSettings)
{
	mixerSettings.mixFreq = currentSettings.restore("MIXERFREQ")->getIntValue();
	mixerSettings.bufferSize = currentSettings.restore("BUFFERSIZE")->getIntValue();
	mixerSettings.mixerVolume = currentSettings.restore("MIXERVOLUME")->getIntValue();
	mixerSettings.mixerShift = 2 - currentSettings.restore("MIXERSHIFT")->getIntValue();
	mixerSettings.powerOfTwoCompensation = currentSettings.restore("FORCEPOWEROFTWOBUFFERSIZE")->getIntValue();
	mixerSettings.resampler = currentSettings.restore("INTERPOLATION")->getIntValue();
	mixerSettings.ramping = currentSettings.restore("RAMPING")->getIntValue();
	mixerSettings.setAudioDriverName(currentSettings.restore("AUDIODRIVER")->getStringValue());
	mixerSettings.numVirtualChannels = currentSettings.restore("VIRTUALCHANNELS")->getIntValue();
}

void Tracker::applySettings(TrackerSettingsDatabase* newSettings,
							TrackerSettingsDatabase* currentSettings/* = NULL*/,
							bool applyMixerSettings/* = true*/,
							bool allowMixerRestart/* = true*/)
{
	pp_uint32 version = MILKYTRACKER_VERSION;

	PPDictionaryKey* versionKey = newSettings->restore("VERSION");
	if (versionKey != NULL)
		version = versionKey->getIntValue();

	PPDictionaryKey* theKey = newSettings->getFirstKey();

	TMixerSettings newMixerSettings;

	while (theKey)
	{
		if (currentSettings != NULL)
		{
			PPDictionaryKey* dKey = currentSettings->restore(theKey->getKey());

			if (dKey)
			{
				if (theKey->getStringValue().compareTo(dKey->getStringValue()) != 0)
				{
					applySettingByKey(theKey, newMixerSettings, version);
				}
			}
		}
		else
		{
			applySettingByKey(theKey, newMixerSettings, version);
		}

		theKey = newSettings->getNextKey();
	}

	if (applyMixerSettings)
	{
		bool res = playerMaster->applyNewMixerSettings(newMixerSettings, allowMixerRestart);
		if (!res)
		{
			SystemMessage message(*screen, SystemMessage::MessageSoundDriverInitFailed);
			message.show();
		}
	}
}
