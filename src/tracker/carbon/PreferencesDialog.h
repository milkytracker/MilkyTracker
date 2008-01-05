/*
 *  PreferencesDialog.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 07.04.06.
 *  Copyright 2006 milkytracker.net. All rights reserved.
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

	void updateToggles();
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
	
public:
	PreferencesDialog(WindowRef	windowRef, WindowRef mainWindowRef);
	~PreferencesDialog();
	
	void show();
	void hide();

	UInt32 getSelectedMidiDeviceID();
	bool getUseMidiDeviceFlag();
	bool getRecordVelocityFlag();
	UInt32 getVelocityAmplify();
};

#endif
