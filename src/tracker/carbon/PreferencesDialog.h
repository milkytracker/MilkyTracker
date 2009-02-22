/*
 *  tracker/carbon/PreferencesDialog.h
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
 *  PreferencesDialog.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 07.04.06.
 *
 */
#ifndef __PREFERENCESDIALOG_H__
#define __PREFERENCESDIALOG_H__

#include <Carbon/Carbon.h>

class TrackerSettingsDatabase;
class RtMidiIn;

class PreferencesDialog
{
private:
	WindowRef	preferencesWindow, mainWindow;
	static pascal OSStatus WindowEventHandler(EventHandlerCallRef myHandler, EventRef event, void* userData);

	TrackerSettingsDatabase*	m_dataBase;
	TrackerSettingsDatabase*	m_dataBaseCopy;	

	RtMidiIn* midiin;

	void initDataBase();
	void shutdownDataBase();

	void backupDataBase();
	void restoreDataBase();

	void updateControls();
	void initDialog();
	
	UInt32 getComboSelection();

	void toggleRecordVelocity();
	void toggleUseMidiDevice();
	void toggleSavePreferences();
	void updateSliderVelocityAmplify();
	void storeMidiDeviceName(UInt32 deviceID);
	void storeVelocityAmplify(UInt32 amplify);

	UInt32 getMidiDevIDFromString(const char* string);

	UInt32 getNumMidiDevices();
	
	void setFakeInsertKey(UInt32 style);
	void toggleUse15BitColorDepth();	
	
public:
	PreferencesDialog(WindowRef	windowRef, WindowRef mainWindowRef);
	~PreferencesDialog();
	
	void show();
	void hide();

	UInt32 getSelectedMidiDeviceID();
	bool getUseMidiDeviceFlag();
	bool getRecordVelocityFlag();
	UInt32 getVelocityAmplify();
	
	UInt32 getFakeInsertKey();
	bool getUse15BitColorDepth();
};

#endif
