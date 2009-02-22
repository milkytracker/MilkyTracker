/*
 *  tracker/win32/PreferencesDialog.h
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

#ifndef __PREFERENCESDIALOG_H__
#define __PREFERENCESDIALOG_H__

#include <windows.h>

class TrackerSettingsDatabase;

class CPreferencesDialog
{
private:
	HWND						m_hWndDlg;
	HWND						m_hWndMain;
	HINSTANCE					m_hInstance;
	TrackerSettingsDatabase*	m_dataBase;
	TrackerSettingsDatabase*	m_dataBaseCopy;

	static CPreferencesDialog*	s_prefDlg;
	static BOOL CALLBACK DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int					s_threadPriorities[5];

	void initDataBase();
	void shutdownDataBase();

	void backupDataBase();
	void restoreDataBase();

	void updateToggles();
	void updateSliderMidiRecordPriority();
	void updateSliderVelocityAmplify();
	void initDialog();
	
	UINT getComboSelection();

	void toggleUseMidiDevice();
	void toggleSavePreferences();
	void toggleRecordVelocity();
	void storeMidiDeviceName(UINT deviceID);
	void storeMidiRecordPriority(UINT priority);
	void storeVelocityAmplify(UINT amplify);

	UINT getMidiDevIDFromString(const char* string);

public:
	CPreferencesDialog(HWND hWnd, HINSTANCE hInst);
	~CPreferencesDialog();

	UINT runModal();

	UINT getSelectedMidiDeviceID();
	bool getUseMidiDeviceFlag();
	UINT getMidiRecordThreadPriority();
	bool getRecordVelocityFlag();
	UINT getVelocityAmplify();
};

#endif
