/*
 *  TrackerStartUp.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 20 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "Tracker.h"
#include "XMFile.h"
#include "TrackerSettingsDatabase.h"
#include "PPSystem.h"
#include "Screen.h"
#include "PatternEditorControl.h"
#include "PlayerMaster.h"
#include "SystemMessage.h"

PPSize Tracker::getWindowSizeFromDatabase()
{
	PPSize size(PPScreen::getDefaultWidth(), PPScreen::getDefaultHeight());
	
	if (XMFile::exists(System::getConfigFileName()))
	{
		TrackerSettingsDatabase* settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);
		
		XMFile f(System::getConfigFileName());
	
		settingsDatabaseCopy->serialize(f);	
		
		size.height = settingsDatabaseCopy->restore("YRESOLUTION")->getIntValue();
		size.width = settingsDatabaseCopy->restore("XRESOLUTION")->getIntValue();

		delete settingsDatabaseCopy;
	}

	return size;
}

bool Tracker::getFullScreenFlagFromDatabase()
{
	bool fullScreen = false;
	
	if (XMFile::exists(System::getConfigFileName()))
	{
		TrackerSettingsDatabase* settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);
		
		XMFile f(System::getConfigFileName());
	
		settingsDatabaseCopy->serialize(f);	
		
		fullScreen = settingsDatabaseCopy->restore("FULLSCREEN")->getBoolValue();

		delete settingsDatabaseCopy;
	}

	return fullScreen;
}

bool Tracker::getShowSplashFlagFromDatabase()
{
	bool showSplash = true;
	
	if (XMFile::exists(System::getConfigFileName()))
	{
		TrackerSettingsDatabase* settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);
		
		XMFile f(System::getConfigFileName());
	
		settingsDatabaseCopy->serialize(f);	
		
		showSplash = settingsDatabaseCopy->restore("SHOWSPLASH")->getBoolValue();

		delete settingsDatabaseCopy;
	}

	return showSplash;
}

void Tracker::startUp()
{
	if (XMFile::exists(System::getConfigFileName()))
	{
		// create as copy from existing database, so all keys are in there
		settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);
		
		XMFile f(System::getConfigFileName());
	
		// restore keys from disk
		settingsDatabaseCopy->serialize(f);	
		
		// apply ALL settings, not just the different ones
		applySettings(settingsDatabaseCopy, NULL, true, false);
		
		delete settingsDatabase;
		settingsDatabase = settingsDatabaseCopy;
		settingsDatabaseCopy = NULL;
	}
	
	// update version information
	settingsDatabase->store("VERSION", TrackerConfig::version);
	
	// Update info panels
	updateSongInfo(false);
	
	updateWindowTitle();
	
	if (!playerMaster->start())
	{
		SystemMessage systemMessage(*screen, SystemMessage::MessageSoundDriverInitFailed);
		systemMessage.show();
	}
}
