#include <windows.h>
#include <commctrl.h>
#include <Aygshell.h>
#include "resource.h"
#include "KeyConfigDlg.h"
#include "Registry.h"
#include "TrackerSettingsDatabase.h"
#include "VirtualKeyToScanCodeTable.h"
#include "XMFile.h"
#include "PPSystem_WIN32.h"
#include "Event.h"

#define NUMTABS 3

#ifndef _WIN32_WCE_EMULATION
short		nInvalidKeys[]	= {13,17,91,131,132};
#else
short		nInvalidKeys[]	= {91,132};
#endif

#define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)

#ifndef ListView_SetCheckState 
#define ListView_SetCheckState(h, i, f) \
        ListView_SetItemState(h, i, INDEXTOSTATEIMAGEMASK((f) + 1), LVIS_STATEIMAGEMASK) 
#endif

LPCTSTR szExecutable = _T("MilkyTracker.exe");

LPCTSTR szValidExtensions[] = 
	{_T("669"),_T("Composer 669"),
	_T("amf"), _T("Asylum Music Format 1.0"),
	_T("ams"), _T("Velvet Studio / Extreme Tracker"),
	_T("cba"), _T("Chuck Biscuits & Black Artist"),
	_T("dbm"), _T("Digibooster Pro"),
	_T("digi"), _T("Digibooster"),
	_T("dsm"), _T("Digisound Interface Kit / Dynamic Studio"),
	_T("dtm"), _T("Digitrekker / Digital Tracker (Atari)"),
	_T("far"), _T("Farandole Composer"),
	_T("gdm"), _T("General Digimusic"),
	_T("gmc"), _T("Game Music Creator"),
	_T("imf"), _T("Imago Orpheus"),
	_T("it"), _T("Impulse Tracker"),
	_T("mdl"), _T("Digitracker 3"),
	_T("mod"), _T("Protracker"),
	_T("mtm"), _T("Multitracker"),
	_T("mxm"), _T("Cubic Tiny XM"),
	_T("okt"), _T("Oktalyzer"),
	_T("okta"), _T("Oktalyzer"),
	_T("plm"), _T("DisorderTracker 2"),
	_T("psm"), _T("Epic Megagames MASI"),
	_T("ptm"), _T("Polytracker"),
	_T("s3m"), _T("Screamtracker 3"),
	_T("stm"), _T("Screamtracker 2"),
	_T("ult"), _T("Ultratracker"),
	_T("uni"), _T("MikMod"),
	_T("xm"), _T("Fasttracker II"),
	NULL, NULL};

static BOOL						bSetKey				= FALSE;
static HWND						hWndMain			= NULL;
static HINSTANCE				hInstance			= NULL;
static int						NUM_KEYS;

// forward declarations
BOOL CALLBACK KeyDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK OtherDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK FileAssociationDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HWND						g_hWndTabs[NUMTABS]			= {NULL};
static DWORD					g_nTabDialogIDs[NUMTABS]	= {IDD_KEYDIALOG, IDD_OTHERDIALOG, IDD_FILEASSOCIATIONDIALOG};
static DLGPROC					g_pDialogProcs[NUMTABS]		= {KeyDialogProc, OtherDialogProc, FileAssociationDialogProc};

static TrackerSettingsDatabase* keyDatabase			= NULL;

struct TButtonMapping
{
	TCHAR		functionName[512];
	WORD		buttonCode;
	BYTE		keyModifiers;
	BYTE		virtualKeyCode;
};

TButtonMapping	mappings[256];

TButtonMapping* currentMapping;

unsigned int DEFINEKEY(unsigned int vk, unsigned int modifier = 0, unsigned int buttonCode = 0xFFFF)
{
	return (vk << 24) + (modifier << 16) + buttonCode;
}

void SetupKeyDatabase()
{
	keyDatabase = new TrackerSettingsDatabase();	

	keyDatabase->store("KEY:Cursor up", DEFINEKEY(VK_UP));
	keyDatabase->store("KEY:Cursor down", DEFINEKEY(VK_DOWN));
	keyDatabase->store("KEY:Cursor left", DEFINEKEY(VK_LEFT));
	keyDatabase->store("KEY:Cursor right", DEFINEKEY(VK_RIGHT));
	keyDatabase->store("KEY:Delete", DEFINEKEY(VK_DELETE));
	keyDatabase->store("KEY:Insert", DEFINEKEY(VK_INSERT));
	keyDatabase->store("KEY:Backspace", DEFINEKEY(VK_BACK));
	keyDatabase->store("KEY:Insert line", DEFINEKEY(VK_INSERT, KeyModifierSHIFT));
	keyDatabase->store("KEY:Backspace line", DEFINEKEY(VK_BACK, KeyModifierSHIFT));
	keyDatabase->store("KEY:Undo", DEFINEKEY('Z', KeyModifierCTRL));
	keyDatabase->store("KEY:Redo", DEFINEKEY('Y', KeyModifierCTRL));
	keyDatabase->store("KEY:Play song", DEFINEKEY(VK_RETURN));
	keyDatabase->store("KEY:Play pattern", DEFINEKEY(VK_RETURN, KeyModifierCTRL));
	keyDatabase->store("KEY:Play pattern from current position", DEFINEKEY(VK_RETURN, KeyModifierSHIFT));
	keyDatabase->store("KEY:Preview row", DEFINEKEY(VK_SPACE, KeyModifierSHIFT));
	keyDatabase->store("KEY:Preview song/pattern", DEFINEKEY(VK_SPACE, KeyModifierALT));
	keyDatabase->store("KEY:Stop playing", DEFINEKEY(VK_ESCAPE));
	keyDatabase->store("KEY:Record mode toggle", DEFINEKEY(VK_SPACE));
	keyDatabase->store("ORIENTATION", 0);
	keyDatabase->store("ALLOWVIRTUALKEYS", 0);
	keyDatabase->store("HIDETASKBAR", 1);
	keyDatabase->store("DOUBLEPIXELS", 0);
	keyDatabase->store("DONTTURNOFFDEVICE", 0);

	if (XMFile::exists(System::getConfigFileName(_T("keys.cfg"))))
	{
		XMFile f(System::getConfigFileName(_T("keys.cfg")));

		keyDatabase->serialize(f);
	}

	const PPDictionaryKey* theKey = keyDatabase->getFirstKey();
	
	int j = 0;
	while (theKey)
	{
		PPString str = theKey->getKey();

		char szText[512];
		strcpy(szText, str);

		if (memcmp(szText,"KEY:",4) == 0)
		{
			for (unsigned int i = 0; i <= strlen(szText); i++)
				mappings[j].functionName[i] = szText[i];
			
			unsigned int v = theKey->getIntValue();
			
			mappings[j].buttonCode = (WORD)(v & 0xFFFF);
			mappings[j].keyModifiers = (BYTE)(v >> 16);
			mappings[j].virtualKeyCode = (BYTE)(v >> 24);
			
			j++;
		}
		theKey = keyDatabase->getNextKey();
	}

	NUM_KEYS = j;
}

void ShutdownKeyDatabase()
{
	bool isKeyboardKey = false;

	for (int j = 0; j < NUM_KEYS; j++)
	{
		// Check if button maps to any existing keyboard virtual key
		if (mappings[j].buttonCode >= 0 && mappings[j].buttonCode < 256)
		{
			if (vkeyToScancode[mappings[j].buttonCode] != -1)
				isKeyboardKey = true;
		}

		char szText[512];
		
		for (unsigned int i = 0; i <= _tcslen(mappings[j].functionName); i++)
			szText[i] = (char)mappings[j].functionName[i];

		unsigned int v = DEFINEKEY(mappings[j].virtualKeyCode, mappings[j].keyModifiers, mappings[j].buttonCode);

		keyDatabase->store(szText, (signed)v);
	}

	// If one of the mapped buttons is used by key on keyboard
	// give warning
	const PPDictionaryKey* theKey = keyDatabase->restore("ALLOWVIRTUALKEYS");
					
	if (theKey && isKeyboardKey)
	{
		BOOL allowExternalKeys = theKey->getIntValue();
		if (allowExternalKeys)
		{
			TCHAR szText[2048];
			::LoadString(hInstance, IDS_WARNING_BUTTONMAPPINGINTERFERENCE, szText, 2048);
			::MessageBox(hWndMain, szText, _T("Warning"), MB_OK);
		}
	}

	XMFile f(System::getConfigFileName(_T("keys.cfg")), true);
	
	keyDatabase->serialize(f);	
}

typedef BOOL (__stdcall *UnregisterFunc1Proc)( UINT, UINT ); 


void RegisterKeys(HWND hWnd)
{
	HINSTANCE hCoreDll; 
	
	UnregisterFunc1Proc procUndergisterFunc; 
	
	hCoreDll = LoadLibrary(_T("coredll.dll")); 
	
	ASSERT(hCoreDll); 
	
	procUndergisterFunc = (UnregisterFunc1Proc)GetProcAddress( hCoreDll, _T("UnregisterFunc1")); 
	
	ASSERT(procUndergisterFunc); 
	
	for (int i=0x0; i<=0xc0; i++) 
	{
		procUndergisterFunc(MOD_KEYUP, i); 
		RegisterHotKey(hWnd, i, MOD_KEYUP, i); 
	} 
	
	for (i=0xc1; i<=0xff; i++) 
	{
		procUndergisterFunc(MOD_WIN | MOD_KEYUP, i); 
		RegisterHotKey(hWnd, i, MOD_WIN | MOD_KEYUP, i); 
	} 
	
	FreeLibrary(hCoreDll);
}

void UnregisterKeys(HWND hWnd)
{
	for (int i=0; i<=0xff; i++) 
	{
		UnregisterHotKey(hWnd, i); 
	} 	
}

static void SetupListView(HWND hWndDlg)
{
	HWND hWndListView = GetDlgItem(hWndDlg, IDC_LIST1);
	
	ListView_SetExtendedListViewStyle(hWndListView, 
	ListView_GetExtendedListViewStyle(hWndListView) | LVS_EX_FULLROWSELECT);

	LVCOLUMN lvColumn;
	
	lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
	
	// ---- Setup columns ---
	lvColumn.cx = 140;
	lvColumn.pszText = _T("Function");
	
	ListView_InsertColumn(hWndListView, 0, &lvColumn);
	
	lvColumn.cx = 66;
	lvColumn.pszText = _T("Button");
	
	ListView_InsertColumn(hWndListView, 1, &lvColumn);
}

static void BuildKeyList(HWND hWndDlg)
{
	HWND hWndListView = GetDlgItem(hWndDlg, IDC_LIST1);
	
	ListView_DeleteAllItems(hWndListView);
	
	// ---- Insert items ----
	LVITEM lvItem;
	memset(&lvItem,0,sizeof(lvItem));
	
	lvItem.mask = LVIF_TEXT;
	
	for (int i = 0; i < NUM_KEYS; i++)
	{
		TCHAR szKeyCode[33];
		if (mappings[i].buttonCode == 65535)
			_tcscpy(szKeyCode, _T("(unused)"));
		else
			wsprintf(szKeyCode, _T("%i"), mappings[i].buttonCode);

		lvItem.iItem = i;
		
		lvItem.pszText = mappings[i].functionName+4;
		
		ListView_InsertItem(hWndListView, &lvItem);
		
		ListView_SetItemText(hWndListView, lvItem.iItem ,1, szKeyCode);
	}
	
}

static void UpdateKeyList(HWND hWndDlg, TButtonMapping* mapping)
{
	HWND hWndListView = GetDlgItem(hWndDlg, IDC_LIST1);

	LVITEM lvItem;
	memset(&lvItem,0,sizeof(lvItem));

	for (int i = 0; i < NUM_KEYS; i++)
	{
		if (mappings + i == mapping)
		{
			TCHAR szKeyCode[33];
			if (mappings[i].buttonCode == 65535)
				_tcscpy(szKeyCode, _T("(unused)"));
			else
				wsprintf(szKeyCode, _T("%i"), mappings[i].buttonCode);
			
			lvItem.iItem = i;
			
			ListView_SetItemText(hWndListView, lvItem.iItem ,1, szKeyCode);
		}
	}
	
}

void SetNewKey(HWND hWndDlg, TButtonMapping* currentMapping,unsigned short nKeyCode)
{
	if (!currentMapping)
		return;

	// look if key was already assigned
	for (int i = 0; i < NUM_KEYS; i++)
		if (mappings[i].buttonCode == nKeyCode)
		{
			mappings[i].buttonCode = -1;
			
			UpdateKeyList(hWndDlg, mappings + i);
		}
	
	currentMapping->buttonCode = nKeyCode;
}

static void UpdateKeyCodes(HWND hWndDlg, TButtonMapping* currentMapping)
{
	SetDlgItemText(hWndDlg, IDC_KEY, currentMapping->functionName+4);

	if (currentMapping && currentMapping->buttonCode != 65535)
	{
		SetDlgItemInt(hWndDlg, IDC_KEYEDIT1, currentMapping->buttonCode, TRUE);	
	}
	else
	{
		SetDlgItemText(hWndDlg, IDC_KEYEDIT1, _T("N/A"));
	}
	//HWND hWndButton = GetDlgItem(hWndDlg, IDC_KEYBUTTON1);
}

static void ShowInputTextfield(HWND hWndDlg, BOOL bShow)
{
	HWND hWnd = GetDlgItem(hWndDlg, IDC_STATIC1);
	ShowWindow(hWnd, bShow ? SW_SHOW : SW_HIDE);
	
	hWnd = GetDlgItem(hWndDlg, IDC_STATIC2);
	ShowWindow(hWnd, bShow ? SW_SHOW : SW_HIDE);

	hWnd = GetDlgItem(hWndDlg, IDC_STATIC3);
	ShowWindow(hWnd, !bShow ? SW_SHOW : SW_HIDE);

	hWnd = GetDlgItem(hWndDlg, IDC_KEYEDIT1);
	ShowWindow(hWnd, bShow ? SW_SHOW : SW_HIDE);

	hWnd = GetDlgItem(hWndDlg, IDC_KEY);
	ShowWindow(hWnd, bShow ? SW_SHOW : SW_HIDE);

}

static void CheckRadioButtons(HWND hWndRadioButtons[], int numEntries, int checkIndex)
{
	for (int i = 0; i < numEntries; i++)
		if (i != checkIndex)
		{
			bool checked = (::SendMessage(hWndRadioButtons[i], BM_GETCHECK, 0, 0) == BST_CHECKED);

			if (checked)
				::SendMessage(hWndRadioButtons[i], BM_SETCHECK, BST_UNCHECKED, 0);
		}
	
	bool checked = (::SendMessage(hWndRadioButtons[checkIndex], BM_GETCHECK, 0, 0) == BST_CHECKED);
	if (!checked)
		::SendMessage(hWndRadioButtons[checkIndex], BM_SETCHECK, BST_CHECKED, 0);
}

static void UpdateCheckboxesAndRadioButtons(HWND hWndDlg)
{
	const PPDictionaryKey* theKey = keyDatabase->restore("ORIENTATION");

	HWND hWndRadio[3];

	hWndRadio[0] = GetDlgItem(hWndDlg, IDC_RADIO_ORIENTATION_90CW);
	hWndRadio[1] = GetDlgItem(hWndDlg, IDC_RADIO_ORIENTATION_90CCW);
	hWndRadio[2] = GetDlgItem(hWndDlg, IDC_RADIO_ORIENTATION_NORMAL);

	if (theKey && theKey->getIntValue() >= 0 && theKey->getIntValue() < 3)
		CheckRadioButtons(hWndRadio, 3, theKey->getIntValue());
	else
		CheckRadioButtons(hWndRadio, 3, 0);

	// allow virtual keys
	theKey = keyDatabase->restore("ALLOWVIRTUALKEYS");

	HWND hWnd = GetDlgItem(hWndDlg, IDC_CHECKALLOWVKEYS);

	if (theKey)
		SendMessage(hWnd, BM_SETCHECK, theKey->getIntValue() ? BST_CHECKED : BST_UNCHECKED, 0);
	else
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);

	// hide taskbar
	theKey = keyDatabase->restore("HIDETASKBAR");

	hWnd = GetDlgItem(hWndDlg, IDC_CHECKHIDETASKBAR);

	if (theKey)
		SendMessage(hWnd, BM_SETCHECK, theKey->getIntValue() ? BST_CHECKED : BST_UNCHECKED, 0);
	else
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	
	// double pixels
	theKey = keyDatabase->restore("DOUBLEPIXELS");

	hWnd = GetDlgItem(hWndDlg, IDC_CHECKDOUBLEPIXELS);

	if (theKey)
		SendMessage(hWnd, BM_SETCHECK, theKey->getIntValue() ? BST_CHECKED : BST_UNCHECKED, 0);
	else
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);

	// device turn off
	theKey = keyDatabase->restore("DONTTURNOFFDEVICE");

	hWnd = GetDlgItem(hWndDlg, IDC_DONTTURNOFFDEVICE);

	if (theKey)
		SendMessage(hWnd, BM_SETCHECK, theKey->getIntValue() ? BST_CHECKED : BST_UNCHECKED, 0);
	else
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
}

static BOOL CALLBACK KeyDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
		case WM_INITDIALOG:
		{
			currentMapping = NULL;
			UpdateKeyCodes(hWndDlg, currentMapping);

			ShowInputTextfield(hWndDlg, FALSE);

			SetupListView(hWndDlg);
			BuildKeyList(hWndDlg);

			RegisterKeys(hWndDlg);
			return TRUE;
		}

		case WM_NOTIFY: 
        {

			LPNMHDR pnmh = (LPNMHDR) lParam;
			
			LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
			NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;

			switch(pnmh->idFrom)
            {
				case IDC_LIST1:
                {
                    switch(pLvdi->hdr.code)
                    {
						case LVN_ITEMACTIVATE:
						{
							currentMapping = mappings + pNm->iItem;
							UpdateKeyCodes(hWndDlg, currentMapping);
							
							ShowInputTextfield(hWndDlg, TRUE);

							bSetKey = TRUE;

							/*LVITEM lvItem;
							memset(&lvItem,0,sizeof(lvItem));

							lvItem.mask = LVIF_PARAM;
							lvItem.iItem = pNm->iItem;
							
							ListView_GetItem(pNm->hdr.hwndFrom,&lvItem);
							
							return 0;*/
							break;
						}
                    }
                }
                break;
            
			
			}
            return 0;
        } 
        break; 


		case WM_HOTKEY:
		{	
			
			UINT nVirtKey = HIWORD(lParam);

			if (!bSetKey)
				break;

			BOOL invalidKey = FALSE;
			for (int i = 0; i < NUM_INVALID_KEYS; i++)
				if ((short)nVirtKey == nInvalidKeys[i])
				{
					invalidKey = TRUE;
					break;
				}

			if (invalidKey) break;

			SetNewKey(hWndDlg, currentMapping,(short)nVirtKey);

			UpdateKeyList(hWndDlg, currentMapping);

			currentMapping = NULL;

			UpdateKeyCodes(hWndDlg, currentMapping);
			
			ShowInputTextfield(hWndDlg, FALSE);

			bSetKey = FALSE;
			
			break;
		}
		
		
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_KEYRESETALL:
				{
					for (int i = 0; i < NUM_KEYS; i++)
						mappings[i].buttonCode = -1;
					
					BuildKeyList(hWndDlg);
					break;
				}
			}
			
			break;            
		}


		case WM_CLOSE:
			UnregisterKeys(hWndDlg);
			
			DestroyWindow(hWndDlg);
			return TRUE;
			break;

		case WM_CTLCOLORSTATIC:
		{
			HBRUSH hbr = (HBRUSH)DefWindowProc(hWndDlg, uMsg, wParam, lParam);
			
			HDC hdcStatic = (HDC)wParam;
			HWND hwndStatic = (HWND)lParam;

			int dlgID = GetDlgCtrlID(hwndStatic); 
			switch (dlgID)
			{
				case IDC_KEY:
					SetTextColor(hdcStatic, RGB(255,0,0));
					break;
			}

			return (BOOL)hbr;
		}

	}

	return FALSE;
}

static BOOL CALLBACK OtherDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
		case WM_INITDIALOG:
		{
			UpdateCheckboxesAndRadioButtons(hWndDlg);
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_RADIO_ORIENTATION_90CW:
				{
					const PPDictionaryKey* theKey = keyDatabase->restore("ORIENTATION");
					
					if (theKey)
						keyDatabase->store("ORIENTATION", 0);

					UpdateCheckboxesAndRadioButtons(hWndDlg);
					break;
				}
			
				case IDC_RADIO_ORIENTATION_90CCW:
				{
					const PPDictionaryKey* theKey = keyDatabase->restore("ORIENTATION");
					
					if (theKey)
						keyDatabase->store("ORIENTATION", 1);

					UpdateCheckboxesAndRadioButtons(hWndDlg);
					break;
				}
			
				case IDC_RADIO_ORIENTATION_NORMAL:
				{
					const PPDictionaryKey* theKey = keyDatabase->restore("ORIENTATION");
					
					if (theKey)
						keyDatabase->store("ORIENTATION", 2);

					UpdateCheckboxesAndRadioButtons(hWndDlg);
					break;
				}
			

				case IDC_CHECKALLOWVKEYS:
				{
					const PPDictionaryKey* theKey = keyDatabase->restore("ALLOWVIRTUALKEYS");
					
					if (theKey)
					{
						pp_int32 newVal = !theKey->getIntValue();

						keyDatabase->store("ALLOWVIRTUALKEYS", newVal);
					}
					UpdateCheckboxesAndRadioButtons(hWndDlg);
					break;
				}

				case IDC_CHECKHIDETASKBAR:
				{
					const PPDictionaryKey* theKey = keyDatabase->restore("HIDETASKBAR");
					
					if (theKey)
					{
						pp_int32 newVal = !theKey->getIntValue();

						keyDatabase->store("HIDETASKBAR", newVal);
					}
					UpdateCheckboxesAndRadioButtons(hWndDlg);
					break;
				}
				
				case IDC_CHECKDOUBLEPIXELS:
				{
					const PPDictionaryKey* theKey = keyDatabase->restore("DOUBLEPIXELS");
					
					if (theKey)
					{
						pp_int32 newVal = !theKey->getIntValue();

						keyDatabase->store("DOUBLEPIXELS", newVal);
					}
					UpdateCheckboxesAndRadioButtons(hWndDlg);
					break;
				}

				case IDC_DONTTURNOFFDEVICE:
				{
					const PPDictionaryKey* theKey = keyDatabase->restore("DONTTURNOFFDEVICE");
					
					if (theKey)
					{
						pp_int32 newVal = !theKey->getIntValue();

						keyDatabase->store("DONTTURNOFFDEVICE", newVal);
					}
					UpdateCheckboxesAndRadioButtons(hWndDlg);
					break;
				}
			}
			
			break;            
		}


		case WM_CLOSE:
			DestroyWindow(hWndDlg);
			return TRUE;
			break;

	}

	return FALSE;
}

static void UpdateRegistryFromListView(HWND hWndDlg)
{
	HWND hWndListView = ::GetDlgItem(hWndDlg, IDC_LIST2);
	
	int i = 0;
	
	while (szValidExtensions[i*2] != NULL)
	{
		BOOL bState = ListView_GetCheckState(hWndListView, i);
		
		if (RegQueryFileInfo(szExecutable, hInstance, szValidExtensions[i*2]) && (!bState))
		{
			RegUnregisterFileInfo(szExecutable, hInstance, szValidExtensions[i*2]);
		}
		else if (bState)
		{						
			RegFileInfo(szExecutable, hInstance, szValidExtensions[i*2], 0); // IDI_APP
		}

		i++;
	}
}

static BOOL CALLBACK FileAssociationDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView = GetDlgItem(hWndDlg, IDC_LIST2);

    switch (uMsg)
    {
		case WM_INITDIALOG:
		{
			ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_CHECKBOXES);

			ListView_SetExtendedListViewStyle(hWndListView, 
											  ListView_GetExtendedListViewStyle(hWndListView) | LVS_EX_FULLROWSELECT);


			int res;
			
			LVCOLUMN lvColumn;
			
			lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
			
			// file list view
			lvColumn.cx = 70;
			lvColumn.pszText = _T("Extension");
			
			res = ListView_InsertColumn(hWndListView, 0, &lvColumn);
			
			lvColumn.cx = 240;
			lvColumn.pszText = _T("Description");
			
			res = ListView_InsertColumn(hWndListView, 1, &lvColumn);

			LVITEM lvItem;
			memset(&lvItem,0,sizeof(lvItem));
			
			lvItem.mask = LVIF_TEXT;
			
			int i = 0;

			while (szValidExtensions[i*2] != NULL)
			{
				lvItem.iItem = i;
				lvItem.pszText = const_cast<LPTSTR>(szValidExtensions[i*2]);
			
				res = ListView_InsertItem(hWndListView,&lvItem);
				
				ListView_SetItemText(hWndListView, lvItem.iItem ,1, const_cast<LPTSTR>(szValidExtensions[i*2+1]));
			
				ListView_SetCheckState(hWndListView, i, RegQueryFileInfo(szExecutable, hInstance, szValidExtensions[i*2]));

				i++;
			}

			return TRUE;
		}

		
		case WM_COMMAND:
		{
			if ((LOWORD(wParam) == IDC_SELECT_ALL))
			{
				int i = 0;
					
				while (szValidExtensions[i*2] != NULL)
				{
					ListView_SetCheckState(hWndListView, i, 1);
					i++;
				}				
				return TRUE;
			}
			else if ((LOWORD(wParam) == IDC_DESELECT_ALL))
			{
				int i = 0;
					
				while (szValidExtensions[i*2] != NULL)
				{
					ListView_SetCheckState(hWndListView, i, 0);
					i++;
				}
				return TRUE;
			}
		}

		case WM_CLOSE:
			DestroyWindow(hWndDlg);
			return TRUE;
			break;

	}

	return FALSE;
}

static void CheckForMilkyTrackerAppExe()
{
    TCHAR szModule[MAX_PATH];
    
	GetFullAppPath(szExecutable, hInstance, szModule, MAX_PATH);

	if (!XMFile::exists(szModule))
	{
		TCHAR szText[2048];
		::LoadString(hInstance, IDS_WARNING_MILKYTRACKERAPPEXENOTFOUND, szText, 2048);
		::MessageBox(hWndMain, szText, _T("Warning"), MB_OK);		
	}
}

static void ShowTab(HWND hWndDlg, int nSel)
{
	HWND hWndTabCtrl = GetDlgItem(hWndDlg, IDC_TAB1);

	RECT l_rectClient, l_rectWnd;
	GetClientRect(hWndTabCtrl, &l_rectClient);
	TabCtrl_AdjustRect(hWndTabCtrl, FALSE, &l_rectClient);
	GetWindowRect(hWndTabCtrl, &l_rectWnd);
	
	POINT p1,p2;
	p1.x = l_rectWnd.left;
	p1.y = l_rectWnd.top;
	p2.x = l_rectWnd.right;
	p2.y = l_rectWnd.bottom;

	ScreenToClient(hWndDlg,&p1);
	ScreenToClient(hWndDlg,&p2);
	
	l_rectWnd.left = p1.x;
	l_rectWnd.top = p1.y;
	l_rectWnd.right = p2.x;
	l_rectWnd.bottom = p2.y;
	
	OffsetRect(&l_rectClient, l_rectWnd.left,l_rectWnd.top);
	
	int width = l_rectClient.right - l_rectClient.left;
	int height = l_rectClient.bottom - l_rectClient.top;
	
	SetWindowPos(g_hWndTabs[nSel], HWND_TOP, l_rectClient.left, l_rectClient.top, width, height, SWP_SHOWWINDOW);
}

static BOOL CALLBACK TabDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_INITDIALOG:
		{
#ifdef _WIN32_WCE
			//On Rapier devices you normally create all Dialog's as fullscreen dialog's
			// with an OK button in the upper corner. You should get/set any program settings
			// during each modal dialog creation and destruction
			SHINITDLGINFO shidi;
			// Create a Done button and size it.
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
			shidi.hDlg = hWndDlg;
			//initialzes the dialog based on the dwFlags parameter
			SHInitDialog(&shidi);
#endif

			// Load database
			SetupKeyDatabase();

			// Check for MilkyTracker.exe in current path
			CheckForMilkyTrackerAppExe();

			// tab stuff
			HWND hWndTabCtrl = GetDlgItem(hWndDlg, IDC_TAB1);

			TCITEM item;

			item.pszText = _T("Buttons");
			item.mask = TCIF_TEXT;
			TabCtrl_InsertItem(hWndTabCtrl, 0, &item);

			item.pszText = _T("Other");
			TabCtrl_InsertItem(hWndTabCtrl, 1, &item);

			item.pszText = _T("File associations");
			TabCtrl_InsertItem(hWndTabCtrl, 2, &item);

			for (int i = 0; i < NUMTABS; i++)
			{
				g_hWndTabs[i] = CreateDialog(hInstance, 
											 MAKEINTRESOURCE(g_nTabDialogIDs[i]), 
											 hWndDlg, 
											 g_pDialogProcs[i]);				
			}

			ShowTab(hWndDlg, 0);			

			TabCtrl_SetCurFocus(hWndTabCtrl, 0);

			return TRUE;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			
			switch (pnmh->code)
			{
				case TCN_SELCHANGE:
				{
					
					HWND hWndTabCtrl = GetDlgItem(hWndDlg, IDC_TAB1);
				
					int nSel = TabCtrl_GetCurSel(hWndTabCtrl);
					
					ShowTab(hWndDlg, nSel);			

					break;
				}
			}
			break;

		}
		
		case WM_COMMAND:
		{
			if ((LOWORD(wParam) == IDOK))
			{
				UpdateRegistryFromListView(g_hWndTabs[2]);

				ShutdownKeyDatabase();

				SendMessage(hWndMain, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWndDlg);
				SendMessage (hWndMain, WM_CLOSE, 0, 0);
				
				return TRUE;
			}
			else if ((LOWORD(wParam) == IDCANCEL))
			{
				SendMessage(hWndMain, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWndDlg);
				SendMessage (hWndMain, WM_CLOSE, 0, 0);
				return TRUE;
			}
			break;
		}

		case WM_CLOSE:
			DestroyWindow(hWndDlg);

			memset(g_hWndTabs, 0, sizeof(g_hWndTabs));
			return TRUE;
			break;

	}

	return FALSE;
}

void CreateKeyDialog(HINSTANCE hInst, HWND hWnd)
{
	hWndMain = hWnd;
	hInstance = hInst;

	HWND hWndD = CreateDialog(hInst, MAKEINTRESOURCE(IDD_TABDIALOG), hWnd, TabDialogProc);
	ShowWindow(hWndD, SW_SHOW);
}


