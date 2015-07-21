/*
 *  tracker/TrackerShutDown.cpp
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
 *  TrackerShutdown.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 20 2005.
 *
 */

#include "Tracker.h"
#include "TabManager.h"
#include "MilkyPlay.h"
#include "PlayerController.h"
#include "PlayerMaster.h"
#include "PlayerLogic.h"
#include "TrackerSettingsDatabase.h"
#include "TrackerConfig.h"
#include "PPSystem.h"
#include "PPSavePanel.h"
#include "PPQuitSaveAlert.h"
#include "ModuleEditor.h"
#include "SectionInstruments.h"
#include "PatternEditorControl.h"
#include "EnvelopeEditorControl.h"
#include "SampleEditorControl.h"
#include "SectionDiskMenu.h"
#include "SectionHDRecorder.h"
#include "SectionOptimize.h"
#include "ScopesControl.h"
#include "GlobalColorConfig.h"
#include "ColorPaletteContainer.h"
#include "SectionSettings.h"
#include "SectionSamples.h"
#include "SectionQuickOptions.h"
#include "Tools.h"
#include "TitlePageManager.h"
#include "version.h"

bool Tracker::checkForChanges(ModuleEditor* moduleEditor/* = NULL*/)
{
	if (moduleEditor == NULL)
		moduleEditor = this->moduleEditor;

	// save current file?
	if (moduleEditor->hasChanged())
	{
		
		PPQuitSaveAlert quitSaveAlertDialog(screen);
		PPQuitSaveAlert::ReturnCodes err = quitSaveAlertDialog.runModal();
		
		if (err == PPQuitSaveAlert::ReturnCodeOK)
		{
			
			PPSavePanel savePanel(screen, "Save Extended Module", moduleEditor->getModuleFileName());
			savePanel.addExtension("xm","Fasttracker 2 Module");
			err = savePanel.runModal();
			if (err == PPSavePanel::ReturnCodeOK)
			{
				const SYSCHAR* file = savePanel.getFileName();
				
				if (file)
				{
					moduleEditor->saveSong(file);
				}
			}
			else if (err == PPSavePanel::ReturnCodeCANCEL)
			{
				return false;
			}
			
		}
		else if (err == PPSavePanel::ReturnCodeCANCEL)
		{
			return false;
		}
		
	}
	
	return true;
}

bool Tracker::checkForChangesOpenModule()
{
	bool openTab = (settingsDatabase->restore("TABS_LOADMODULEINNEWTAB")->getBoolValue() &&
					(moduleEditor->hasChanged() || !moduleEditor->isEmpty()));
	
	if (openTab)
		return true;
	
	return checkForChanges();
}

bool Tracker::shutDown()
{
	pp_int32 i;
	ModuleEditor* currentEditor = moduleEditor;
	for (i = 0; i < tabManager->getNumTabs(); i++)
	{
		moduleEditor = tabManager->getModuleEditorFromTabIndex(i);
		bool res = checkForChanges();

		if (!res)
			return false;
	}
	moduleEditor = currentEditor;

	playerMaster->stop(true);

	XMFile f(System::getConfigFileName(), true);

	if(f.isOpenForWriting())
	{
		// ----------- Save last settings -----------
		// store version string to database
		settingsDatabase->store("VERSION", MILKYTRACKER_VERSION );

		char buffer[100];
		// playmode settings
		const char* playModeStrings[5] = {"AUTO", "PROTRACKER2", "PROTRACKER3", "SCREAMTRACKER3", "FASTTRACKER2"};

		pp_int32 playMode = playerController->getPlayMode();

		ASSERT(playMode >= 0 && playMode < 5);

		settingsDatabase->store("PLAYMODEKEEPSETTINGS", sectionQuickOptions->keepSettings());
		settingsDatabase->store("PLAYMODE", sectionQuickOptions->keepSettings() ? playModeStrings[playMode] : playModeStrings[4]);

		settingsDatabase->store("PLAYMODE_ADVANCED_ALLOW8xx", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanning8xx));
		settingsDatabase->store("PLAYMODE_ADVANCED_ALLOWE8x", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanningE8x));
		// Only affects protracker playmodes
		settingsDatabase->store("PLAYMODE_ADVANCED_PTPITCHLIMIT", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionForcePTPitchLimit));

		// save default panning for protracker playmodes
		pp_uint8* panning = new pp_uint8[TrackerConfig::numPlayerChannels];
		for (i = 0; i < TrackerConfig::numPlayerChannels; i++)
			panning[i] = playerController->getPanning((pp_uint8)i);

		settingsDatabase->store("PLAYMODE_ADVANCED_PTPANNING", PPTools::encodeByteArray(panning, TrackerConfig::numPlayerChannels));
		delete[] panning;

		// quick options
		settingsDatabase->store("PROSPECTIVE", getProspectiveMode() ? 1 : 0);
		settingsDatabase->store("WRAPAROUND", getCursorWrapAround() ? 1 : 0);
		settingsDatabase->store("FOLLOWSONG", getFollowSong() ? 1 : 0);
		settingsDatabase->store("LIVESWITCH", playerLogic->getLiveSwitch() ? 1 : 0);

		// Disk Operations
		settingsDatabase->store("INTERNALDISKBROWSERSETTINGS", sectionDiskMenu->getConfigUInt32());	
		settingsDatabase->store("INTERNALDISKBROWSERLASTPATH", sectionDiskMenu->getCurrentPathASCII());

		// HD recorder
		settingsDatabase->store("HDRECORDER_MIXFREQ", sectionHDRecorder->getSettingsFrequency());
		settingsDatabase->store("HDRECORDER_MIXERVOLUME", sectionHDRecorder->getSettingsMixerVolume());
		settingsDatabase->store("HDRECORDER_MIXERSHIFT", sectionHDRecorder->getSettingsMixerShift());
		settingsDatabase->store("HDRECORDER_RAMPING", sectionHDRecorder->getSettingsRamping() ? 1 : 0);
		settingsDatabase->store("HDRECORDER_INTERPOLATION", sectionHDRecorder->getSettingsResampler());
		settingsDatabase->store("HDRECORDER_ALLOWMUTING", sectionHDRecorder->getSettingsAllowMuting() ? 1 : 0);

		// sample editor
		settingsDatabase->store("SAMPLEEDITORDECIMALOFFSETS", sectionSamples->getOffsetFormat());
		// Sample editor contol will store last used values here
		PPDictionary dictonary = sectionSamples->getSampleEditorControl()->getLastValues().convertToDictionary();
		PPString lastValues = dictonary.serializeToString();
		settingsDatabase->store("SAMPLEEDITORLASTVALUES", lastValues);

		// Optimizer
		for (i = 0; i < (signed)SectionOptimize::getNumFlagGroups(); i++)
		{
			sprintf(buffer, "OPTIMIZER_%i",i);
			settingsDatabase->store(buffer, sectionOptimize->getOptimizeCheckBoxFlags(i));
		}

		// Scale of envelope editor
		settingsDatabase->store("ENVELOPEEDITORSCALE", sectionInstruments->getEnvelopeEditorControl()->getScale());

		// Orderlist was expanded?
		settingsDatabase->store("EXTENDEDORDERLIST", extendedOrderlist ? 1 : 0);

		// Current row insert add value
		settingsDatabase->store("ROWINSERTADD", getPatternEditorControl()->getRowInsertAdd());

		// Current visible title page
		TitlePageManager titlePageManager(*screen);
		settingsDatabase->store("TITLEPAGE", titlePageManager.getCurrentTitlePage());

		// Save colors
		TColorPalette palette;
		palette.numColors = GlobalColorConfig::ColorLast;
		for (i = 0; i < palette.numColors; i++)
			palette.colors[i] = GlobalColorConfig::getInstance()->getColor((GlobalColorConfig::GlobalColors)i);	

		settingsDatabase->store("ACTIVECOLORS", ColorPaletteContainer::encodePalette(palette));

		// store predefined envelopes
		for (i = 0; i < sectionInstruments->getNumPredefinedEnvelopes(); i++)
		{
			sprintf(buffer, "PREDEFENVELOPEVOLUME_%i",i);
			settingsDatabase->store(buffer, sectionInstruments->getEncodedEnvelope(SectionInstruments::EnvelopeTypeVolume, i));
		}

		for (i = 0; i < sectionInstruments->getNumPredefinedEnvelopes(); i++)
		{
			sprintf(buffer, "PREDEFENVELOPEPANNING_%i",i);		
			settingsDatabase->store(buffer, sectionInstruments->getEncodedEnvelope(SectionInstruments::EnvelopeTypePanning, i));
		}

		// store effect macros from pattern editor control
		for (i = 0; i < NUMEFFECTMACROS; i++)
		{
			sprintf(buffer, "EFFECTMACRO_%i",i);

			pp_uint8 eff, op;
			getPatternEditor()->getMacroOperands(i, eff, op);

			pp_int32 val = (((pp_int32)eff) << 8) + (pp_int32)op;

			settingsDatabase->store(buffer, val);
		}

		for (i = 0; i < sectionSettings->getNumPredefinedColorPalettes(); i++)
		{
			sprintf(buffer, "PREDEFCOLORPALETTE_%i",i);		
			settingsDatabase->store(buffer, sectionSettings->getEncodedPalette(i));
		}

		settingsDatabase->serialize(f);
	} // isOpenForWriting

	return true;
}

// this is used when the tracker crashes
// The os dependent layer can call this and it will save a backup of the current
// module
// TO-DO: Doesn't handle tabs properly yet, will only save the current tab
mp_sint32 Tracker::saveModule(const PPSystemString& fileName)
{
	return moduleEditor->saveBackup(fileName);
}

