/*
 *  tracker/TrackerStartUp.cpp
 *
 *  Copyright 2008 Peter Barth
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
 *  TrackerStartUp.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 20 2005.
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
		
		// everything alright, delete old database and take new one
		delete settingsDatabase;
		settingsDatabase = settingsDatabaseCopy;
		settingsDatabaseCopy = NULL;
	}

	// apply ALL settings, not just the different ones
	applySettings(settingsDatabase, NULL, true, false);
	
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
