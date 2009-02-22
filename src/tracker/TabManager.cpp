/*
 *  tracker/TabManager.cpp
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
 *  TabManager.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.12.07.
 *
 */

#include "TabManager.h"
#include "Tracker.h"
#include "SimpleVector.h"
#include "PlayerMaster.h"
#include "PlayerController.h"
#include "PlayerLogic.h"
#include "ModuleEditor.h"
#include "PatternEditor.h"
#include "TabHeaderControl.h"
#include "TabTitleProvider.h"
#include "ControlIDs.h"
#include "Screen.h"
#include "Container.h"
#include "TrackerSettingsDatabase.h"
#include "Tools.h"
#include "Zapper.h"

TabManager::Document::Document(ModuleEditor* moduleEditor, PlayerController* playerController) :
	moduleEditor(moduleEditor),
	playerController(playerController)
{
}
	
TabManager::Document::~Document()
{
	// player controller is deleted from the PlayerMaster
	delete moduleEditor;
}

TabManager::TabManager(Tracker& tracker) :
	tracker(tracker),
	currentDocument(NULL),
	stopOnTabSwitch(false),
	resumeOnTabSwitch(false)
{
	documents = new PPSimpleVector<Document>();	
}

TabManager::~TabManager()
{
	delete documents;
}

TabHeaderControl* TabManager::getTabHeaderControl()
{
	return static_cast<TabHeaderControl*>(tracker.screen->getControlByID(TABHEADER_CONTROL));
}

ModuleEditor* TabManager::createModuleEditor()
{
	ModuleEditor* moduleEditor = new ModuleEditor();
	moduleEditor->createNewSong(tracker.playerController->getPlayMode() == PlayerController::PlayMode_FastTracker2 ? 8 : 4);	
	moduleEditor->setCurrentPatternIndex(moduleEditor->getOrderPosition(0));
	return moduleEditor;
}

PlayerController* TabManager::createPlayerController()
{
	// use "fake" scopes on the PDA
	// fake scopes are always showing linear interpolated output
#ifdef __LOWRES__
	PlayerController* playerController = tracker.playerMaster->createPlayerController(true);
#else
	PlayerController* playerController = tracker.playerMaster->createPlayerController(false);
#endif
	
	if (playerController == NULL)
		return NULL;
	
	applyPlayerDefaults(playerController);
	return playerController;
}

void TabManager::applyPlayerDefaults(PlayerController* playerController)
{
	PPString value = tracker.settingsDatabase->restore("PLAYMODE")->getStringValue();
	if (value.compareTo("PROTRACKER2") == 0)
	{
		playerController->switchPlayMode(PlayerController::PlayMode_ProTracker2, false);
	}
	else if (value.compareTo("PROTRACKER3") == 0)
	{
		playerController->switchPlayMode(PlayerController::PlayMode_ProTracker3, false);
	}
	else
	{
		playerController->switchPlayMode(PlayerController::PlayMode_FastTracker2, false);
	}
	
	bool v = tracker.settingsDatabase->restore("PLAYMODE_ADVANCED_ALLOW8xx")->getBoolValue();
	playerController->enablePlayModeOption(PlayerController::PlayModeOptionPanning8xx, v);

	v = tracker.settingsDatabase->restore("PLAYMODE_ADVANCED_ALLOWE8x")->getBoolValue();
	playerController->enablePlayModeOption(PlayerController::PlayModeOptionPanningE8x, v);

	v = tracker.settingsDatabase->restore("PLAYMODE_ADVANCED_PTPITCHLIMIT")->getBoolValue();
	playerController->enablePlayModeOption(PlayerController::PlayModeOptionForcePTPitchLimit, v);

	value = tracker.settingsDatabase->restore("PLAYMODE_ADVANCED_PTPANNING")->getStringValue();
	{
		pp_uint8* panning = new pp_uint8[TrackerConfig::numPlayerChannels];
		if (PPTools::decodeByteArray(panning, TrackerConfig::numPlayerChannels, value))
		{
			pp_int32 i;
			for (i = 0; i < TrackerConfig::numPlayerChannels; i++)
				playerController->setPanning((pp_uint8)i, panning[i]);
		}
		delete[] panning;		
	}
	
}

void TabManager::openNewTab(PlayerController* playerController/* = NULL*/, ModuleEditor* moduleEditor/* = NULL*/)
{
#ifndef __LOWRES__
	TabHeaderControl* tabHeader = getTabHeaderControl();
	
	if (playerController == NULL)
	{
		playerController = createPlayerController();
		if (playerController == NULL)
		{
			tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Too many open modules.", Tracker::MessageBox_OK);	
			return;
		}
	}
	
	if (moduleEditor == NULL)
	{
		moduleEditor = createModuleEditor();
	}
	
	playerController->attachModuleEditor(moduleEditor);					
	moduleEditor->attachPlayerCriticalSection(playerController->getCriticalSection());
	playerController->setSpeed(moduleEditor->getSongBPM(), moduleEditor->getSongTickSpeed());	
	
	Document* doc = new Document(moduleEditor, playerController);
	documents->add(doc);	

	TabTitleProvider tabTitleProvider(*moduleEditor);
	PPString tabTitle = tabTitleProvider.getTabTitle();
	tabHeader->addTab(TabHeaderControl::TabHeader(tabTitle, documents->size()-1));
	selectModuleEditor(doc);
#else
	if (moduleEditor == NULL || playerController == NULL)
		return;
		
	playerController->attachModuleEditor(moduleEditor);					
	moduleEditor->attachPlayerCriticalSection(playerController->getCriticalSection());
	playerController->setSpeed(moduleEditor->getSongBPM(), moduleEditor->getSongTickSpeed());	

	documents->add(new Document(moduleEditor, playerController));	
#endif	
}

void TabManager::switchToTab(pp_uint32 index)
{
#ifndef __LOWRES__
	TabHeaderControl* tabHeader = getTabHeaderControl();
	ASSERT(tabHeader);
	index = tabHeader->getTab(index)->ID;
	selectModuleEditor(documents->get(index));
#endif
}

void TabManager::closeTab(pp_int32 index/* = -1*/)
{
#ifndef __LOWRES__
	TabHeaderControl* tabHeader = getTabHeaderControl();

	// when there is only a single tab open
	// we zap the module if it has changed or is not empty
	if (tabHeader->getNumTabs() <= 1)
	{
		if (tracker.moduleEditor->hasChanged() ||
			!tracker.moduleEditor->isEmpty())
		{
			Zapper zapper(tracker);
			zapper.zapAll();
			tracker.updateSongInfo(false);
			tracker.screen->paint();
		}
		return;
	}

	if (index == -1)
		index = tabHeader->getSelectedTabIndex();

	pp_int32 moduleIndex = tabHeader->getTab(index)->ID;

	bool res = tracker.checkForChanges(documents->get(moduleIndex)->moduleEditor);
	if (!res)
		return;

	TabHeaderControl::TabHeader* tabs = new TabHeaderControl::TabHeader[tabHeader->getNumTabs()];

	pp_int32 j = 0;
	pp_int32 i;
	for (i = 0; i < (signed)tabHeader->getNumTabs(); i++)
	{
		if (i != index)
		{
			tabs[j] = *tabHeader->getTab(i);
			j++;
		}
	}
	
	tabHeader->clear();

	for (i = 0; i < j; i++)
	{
		if ((signed)tabs[i].ID > moduleIndex)	
			tabs[i].ID--;
	}
	
	Document* doc = documents->removeNoDestroy(moduleIndex);
	
	if (index >= documents->size())
		index = documents->size()-1;
	
	for (i = 0; i < j; i++)
	{
		tabHeader->addTab(tabs[i]);
	}
	
	delete[] tabs;

	tabHeader->setSelectedTab(index);

	switchToTab(tabHeader->getTab(index)->ID);
	
	if (doc->moduleEditor != tracker.moduleEditor)
	{
		tracker.playerMaster->destroyPlayerController(doc->playerController);
		delete doc;
	}
#endif
}

void TabManager::selectModuleEditor(Document* document)
{
#ifndef __LOWRES__
	PPContainer* container = static_cast<PPContainer*>(tracker.screen->getControlByID(CONTAINER_OPENREMOVETABS));
	ASSERT(container);
	TabHeaderControl* tabHeader = getTabHeaderControl();
	ASSERT(tabHeader);
	container->show(tabHeader->getNumTabs() > 1);

	if (tracker.moduleEditor != document->moduleEditor)
	{
		// store current position
		tracker.moduleEditor->setCurrentCursorPosition(tracker.getPatternEditor()->getCursor());
		
		// switch
		tracker.moduleEditor = document->moduleEditor;
		tracker.playerController = document->playerController;		
		
		if (stopOnTabSwitch)
			tracker.playerLogic->stopAll();
		
		if (tracker.playerController->isPaused())
		{
			if (!resumeOnTabSwitch)
				tracker.playerLogic->stopSong();
			tracker.playerController->unpause();
		}
		
		// update
		tracker.updateAfterTabSwitch();		
	}
	currentDocument = document;
		
#endif
}

ModuleEditor* TabManager::getModuleEditorFromTabIndex(pp_int32 index)
{
#ifndef __LOWRES__
	TabHeaderControl* tabHeader = getTabHeaderControl();
	return documents->get(tabHeader->getTab(index)->ID)->moduleEditor;
#else
	return tracker.moduleEditor;
#endif
}

PlayerController* TabManager::getPlayerControllerFromTabIndex(pp_int32 index)
{
#ifndef __LOWRES__
	TabHeaderControl* tabHeader = getTabHeaderControl();
	return documents->get(tabHeader->getTab(index)->ID)->playerController;
#else
	return tracker.playerController;
#endif
}

pp_uint32 TabManager::getSelectedTabIndex()
{
#ifndef __LOWRES__
	TabHeaderControl* tabHeader = getTabHeaderControl();
	return tabHeader->getSelectedTabIndex();
#else
	return 0;
#endif
}

void TabManager::cycleTab(pp_int32 offset)
{
#ifndef __LOWRES__
	TabHeaderControl* tabHeader = getTabHeaderControl();
	ASSERT(tabHeader);
	pp_int32 index = tabHeader->getSelectedTabIndex();
	index+=offset;
	if (index >= (signed)tabHeader->getNumTabs())
		index = tabHeader->getNumTabs()-1;
	if (index < 0)
		index = 0;
	
	if (index != tabHeader->getSelectedTabIndex())
	{
		tabHeader->setSelectedTab(index);
		tracker.screen->paintControl(tabHeader);
		switchToTab(index);
	}
#endif
}

pp_int32 TabManager::getNumTabs() const
{
	return documents->size();
}
