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
