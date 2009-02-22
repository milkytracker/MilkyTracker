/*
 *  tracker/win32/PreferencesDialog.cpp
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

#include "PreferencesDialog.h"
#include <commctrl.h>
#include <tchar.h>
#include "Win32_resource.h"
// MIDI headers
#include "MIDIInDevice.h"
// Other utility classes
#include "TrackerSettingsDatabase.h"
#include "XMFile.h"
#include "PPSystem.h"

#define PREFSFILENAME _T("prefs.cfg")

// MIDI Namespaces
using midi::CMIDIInDevice;
using midi::CMIDIReceiver;

CPreferencesDialog* CPreferencesDialog::s_prefDlg = NULL;

BOOL CALLBACK CPreferencesDialog::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	s_prefDlg->m_hWndDlg = hWndDlg;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			::SetWindowText(hWndDlg, _T("Preferences"));

			HWND hWnd = s_prefDlg->m_hWndMain;
 			
			// Center dialog within main window
			RECT parentRc, thisRc;
			::GetWindowRect(hWnd, &parentRc);
			::GetWindowRect(hWndDlg, &thisRc);

			int width = thisRc.right - thisRc.left;
			int height = thisRc.bottom - thisRc.top;
			
			int midx = (parentRc.left + parentRc.right) / 2;
			int midy = (parentRc.top + parentRc.bottom) / 2;
			
			::SetWindowPos(hWndDlg, hWnd, midx - width/2, midy - height/2, width, height, SWP_NOZORDER);

			s_prefDlg->initDialog();
			return TRUE;
		}

		case WM_NOTIFY:
		{
			const NMHDR* nHdr = (NMHDR*)lParam;

			switch (wParam)
			{
				case IDC_SLIDER_MIDITHREADPRIORITY:
				{
					if ((signed)nHdr->code != NM_RELEASEDCAPTURE)
						break;

					int i = SendMessage(nHdr->hwndFrom, TBM_GETPOS, 0, 0);
					
					s_prefDlg->storeMidiRecordPriority(i);
					break;
				}

				case IDC_SLIDER_AMPLIFYVELOCITY:
				{
					if ((signed)nHdr->code != NM_RELEASEDCAPTURE)
						break;

					int i = SendMessage(nHdr->hwndFrom, TBM_GETPOS, 0, 0);
					
					s_prefDlg->storeVelocityAmplify(i);
					break;
				}
			}
			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_COMBO_MIDIDEVICES:
				{
					switch (HIWORD(wParam))
					{
						case CBN_SELCHANGE:
							s_prefDlg->storeMidiDeviceName(s_prefDlg->getComboSelection());
							break;
					}	
					break;
				}
				
				case IDC_CHECK_ENABLE_MIDI:
					s_prefDlg->toggleUseMidiDevice();
					break;

				case IDC_CHECK_SAVESETTINGS:
					s_prefDlg->toggleSavePreferences();
					break;

				case IDC_CHECK_RECORD_VELOCITY:
					s_prefDlg->toggleRecordVelocity();
					break;

				case IDOK:
					::EndDialog(hWndDlg, wParam);
					break;

				case IDCANCEL:
					s_prefDlg->restoreDataBase();
					::EndDialog(hWndDlg, wParam);
					return TRUE;
			}
		}
			
	}

	return FALSE;
}

CPreferencesDialog::CPreferencesDialog(HWND hWnd, HINSTANCE hInst) :
	m_hWndMain(hWnd),
	m_hInstance(hInst),
	m_dataBase(NULL),
	m_dataBaseCopy(NULL)
{
	initDataBase();
}

CPreferencesDialog::~CPreferencesDialog()
{
	shutdownDataBase();
	delete m_dataBase;
	delete m_dataBaseCopy;
}

UINT CPreferencesDialog::runModal()
{
	CPreferencesDialog::s_prefDlg = this;
	
	backupDataBase();

	return ::DialogBox(m_hInstance, (LPCTSTR)IDD_PROPERTIES, m_hWndMain, (DLGPROC)DialogProc);	
}

void CPreferencesDialog::initDataBase()
{
	m_dataBase = new TrackerSettingsDatabase();	

	m_dataBase->store("USEMIDI", 0);
	m_dataBase->store("SAVEPREFS", 0);
	m_dataBase->store("RECORDINGTHREADPRIORITY", CMIDIInDevice::MIDI_THREAD_PRIORITY_NORMAL);
	m_dataBase->store("MIDIDEVICE", "");
	m_dataBase->store("RECORDVELOCITY", 0);
	m_dataBase->store("VELOCITYAMPLIFY", 100);

	if (XMFile::exists(System::getConfigFileName(PREFSFILENAME)))
	{
		XMFile f(System::getConfigFileName(PREFSFILENAME));

		m_dataBase->serialize(f);
	}	
}

void CPreferencesDialog::shutdownDataBase()
{
	if (m_dataBase->restore("SAVEPREFS")->getIntValue())
	{
		XMFile f(System::getConfigFileName(PREFSFILENAME), true);
		m_dataBase->serialize(f);
	}
	else if (XMFile::exists(System::getConfigFileName(PREFSFILENAME)))
	{
		XMFile::remove(System::getConfigFileName(PREFSFILENAME));
	}
}

void CPreferencesDialog::backupDataBase()
{
	if (!m_dataBase)
		return;

	if (m_dataBaseCopy)
		delete m_dataBaseCopy;

	m_dataBaseCopy = new TrackerSettingsDatabase(*m_dataBase);
}

void CPreferencesDialog::restoreDataBase()
{
	if (!m_dataBaseCopy)
		return;

	if (m_dataBase)
		delete m_dataBase;

	m_dataBase = new TrackerSettingsDatabase(*m_dataBaseCopy);
}

void CPreferencesDialog::updateToggles()
{
	bool b = false;
	if (m_dataBase)
		b = m_dataBase->restore("SAVEPREFS")->getIntValue() != 0;
	::CheckDlgButton( m_hWndDlg, IDC_CHECK_SAVESETTINGS, b ? BST_CHECKED : BST_UNCHECKED);

	b = false;
	if (m_dataBase)
		b = m_dataBase->restore("USEMIDI")->getIntValue() != 0;
	::CheckDlgButton( m_hWndDlg, IDC_CHECK_ENABLE_MIDI, b ? BST_CHECKED : BST_UNCHECKED);

	b = false;
	if (m_dataBase)
		b = m_dataBase->restore("RECORDVELOCITY")->getIntValue() != 0;
	::CheckDlgButton( m_hWndDlg, IDC_CHECK_RECORD_VELOCITY, b ? BST_CHECKED : BST_UNCHECKED);
}

void CPreferencesDialog::updateSliderMidiRecordPriority()
{
	HWND hWndTrackbar = ::GetDlgItem(m_hWndDlg, IDC_SLIDER_MIDITHREADPRIORITY);

	if (m_dataBase)
		::SendMessage(hWndTrackbar, TBM_SETPOS, TRUE, m_dataBase->restore("RECORDINGTHREADPRIORITY")->getIntValue());
}

void CPreferencesDialog::updateSliderVelocityAmplify()
{
	HWND hWndTrackbar = ::GetDlgItem(m_hWndDlg, IDC_SLIDER_AMPLIFYVELOCITY);

	if (m_dataBase)
		::SendMessage(hWndTrackbar, TBM_SETPOS, TRUE, m_dataBase->restore("VELOCITYAMPLIFY")->getIntValue());

	HWND hWndToggle = ::GetDlgItem(m_hWndDlg, IDC_CHECK_RECORD_VELOCITY);

	TCHAR buffer[1024];
#ifdef _UNICODE
	wsprintf(buffer, _T("Record velocity (amplify: %i%%)"), m_dataBase->restore("VELOCITYAMPLIFY")->getIntValue());
#else
	sprintf(buffer, _T("Record velocity (amplify: %i%%)"), m_dataBase->restore("VELOCITYAMPLIFY")->getIntValue());
#endif

	SetWindowText(hWndToggle, buffer);
}

void CPreferencesDialog::initDialog()
{
	UINT i;

	// Configure slider for thread priority
	HWND hWndTrackbar = ::GetDlgItem(m_hWndDlg, IDC_SLIDER_MIDITHREADPRIORITY);
	::SendMessage(hWndTrackbar, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 4));

	for (i = 0; i < 5; i++)
		::SendMessage(hWndTrackbar, TBM_SETTIC, 0, i);

	// Configure slider for velocity amplification
	hWndTrackbar = ::GetDlgItem(m_hWndDlg, IDC_SLIDER_AMPLIFYVELOCITY);
	::SendMessage(hWndTrackbar, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 200));

	HWND hWndCombo = ::GetDlgItem(m_hWndDlg, IDC_COMBO_MIDIDEVICES);

	int nSelectedDevice = 0;
	for (i = 0; i < CMIDIInDevice::GetNumDevs(); i++)
	{
		MIDIINCAPS Caps;
		CMIDIInDevice::GetDevCaps(i, Caps);
		
		::SendMessage(hWndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(&Caps.szPname));
		char* pszDeviceName = NULL;
#ifdef UNICODE
		char szDeviceName[1024];
		midi::UnicodeToAnsi(Caps.szPname, szDeviceName);
		pszDeviceName = szDeviceName;
#else
		pszDeviceName = Caps.szPname;
#endif
		if (m_dataBase && 
			m_dataBase->restore("MIDIDEVICE")->getStringValue().compareTo(pszDeviceName) == 0)
		{
			nSelectedDevice = i;
		}
	}
	
	::SendMessage(hWndCombo, CB_SETCURSEL, nSelectedDevice, 0);

	storeMidiDeviceName(nSelectedDevice);
	updateToggles();
	updateSliderMidiRecordPriority();
	updateSliderVelocityAmplify();
}

UINT CPreferencesDialog::getComboSelection()
{
	HWND hWndCombo = ::GetDlgItem(m_hWndDlg, IDC_COMBO_MIDIDEVICES);
	return ::SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
}

void CPreferencesDialog::toggleUseMidiDevice()
{
	if (m_dataBase)
	{
		int i = m_dataBase->restore("USEMIDI")->getIntValue();
		m_dataBase->restore("USEMIDI")->store(!i);
		updateToggles();
	}
}

void CPreferencesDialog::toggleSavePreferences()
{
	if (m_dataBase)
	{
		int i = m_dataBase->restore("SAVEPREFS")->getIntValue();
		m_dataBase->restore("SAVEPREFS")->store(!i);
		updateToggles();
	}
}

void CPreferencesDialog::toggleRecordVelocity()
{
	if (m_dataBase)
	{
		int i = m_dataBase->restore("RECORDVELOCITY")->getIntValue();
		m_dataBase->restore("RECORDVELOCITY")->store(!i);
		updateToggles();
	}
}

void CPreferencesDialog::storeMidiDeviceName(UINT deviceID)
{
	if (m_dataBase && deviceID < CMIDIInDevice::GetNumDevs())
	{
		MIDIINCAPS Caps;
		CMIDIInDevice::GetDevCaps(deviceID, Caps);
		
		char* pszDeviceName = NULL;
#ifdef UNICODE
		char szDeviceName[1024];
		midi::UnicodeToAnsi(Caps.szPname, szDeviceName);
		pszDeviceName = szDeviceName;
#else
		pszDeviceName = Caps.szPname;
#endif
		m_dataBase->restore("MIDIDEVICE")->store(pszDeviceName);		
	}
}

void CPreferencesDialog::storeMidiRecordPriority(UINT priority)
{
	if (m_dataBase)
	{
		m_dataBase->restore("RECORDINGTHREADPRIORITY")->store(priority);
		updateSliderMidiRecordPriority();
	}
}

void CPreferencesDialog::storeVelocityAmplify(UINT amplify)
{
	if (m_dataBase)
	{
		m_dataBase->restore("VELOCITYAMPLIFY")->store(amplify);
		updateSliderVelocityAmplify();
	}
}

UINT CPreferencesDialog::getMidiDevIDFromString(const char* string)
{
	UINT nSelectedDevice = (unsigned)-1;
	for (UINT i = 0; i < CMIDIInDevice::GetNumDevs(); i++)
	{
		MIDIINCAPS Caps;
		CMIDIInDevice::GetDevCaps(i, Caps);
		
		char* pszDeviceName = NULL;
#ifdef UNICODE
		char szDeviceName[1024];
		midi::UnicodeToAnsi(Caps.szPname, szDeviceName);
		pszDeviceName = szDeviceName;
#else
		pszDeviceName = Caps.szPname;
#endif
		if (strcmp(string, pszDeviceName) == 0)
		{
			nSelectedDevice = i;
			break;
		}
	}

	return nSelectedDevice;
}

UINT CPreferencesDialog::getSelectedMidiDeviceID()
{
	if (m_dataBase)
	{
		return getMidiDevIDFromString(m_dataBase->restore("MIDIDEVICE")->getStringValue());
	}

	return (unsigned)-1;
}

bool CPreferencesDialog::getUseMidiDeviceFlag()
{
	if (m_dataBase)
	{
		return m_dataBase->restore("USEMIDI")->getIntValue() != 0;
	}

	return false;
}

UINT CPreferencesDialog::getMidiRecordThreadPriority()
{
	if (m_dataBase)
	{
		return m_dataBase->restore("RECORDINGTHREADPRIORITY")->getIntValue();
	}

	return CMIDIInDevice::MIDI_THREAD_PRIORITY_NORMAL;
}

bool CPreferencesDialog::getRecordVelocityFlag()
{
	if (m_dataBase)
	{
		return m_dataBase->restore("RECORDVELOCITY")->getIntValue() != 0;
	}

	return false;
}

UINT CPreferencesDialog::getVelocityAmplify()
{
	if (m_dataBase)
	{
		return m_dataBase->restore("VELOCITYAMPLIFY")->getIntValue();
	}

	return 100;
}
