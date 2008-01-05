#include "Registry.h"

void GetFullAppPath(LPCTSTR pszApplicationTitle, HINSTANCE hInst, LPTSTR pszModule, UINT max)
{
    ::GetModuleFileName(hInst, pszModule, max);

	LPTSTR pStr = pszModule+_tcslen(pszModule);

	while (*pStr != '\\' && pStr > pszModule)
		pStr--;

	if (_tcslen(pStr) > 0)
	{
		pStr++;
		*pStr = '\0';
	}

	_tcscat(pszModule, pszApplicationTitle);
}

int SameProgram(LPCTSTR pszStr, LPCTSTR pszModule)
{

	LPCTSTR pStr = pszModule+_tcslen(pszModule);

	while (*pStr != '\\' && pStr > pszModule)
		pStr--;

	if (*pStr == '\\')
		pStr++;

	for (unsigned int i = 0; i < _tcslen(pszStr) - _tcslen(pStr); i++)
	{
		BOOL bMatch = TRUE;
		for (unsigned int j = 0; j < _tcslen(pStr); j++)
			if (pszStr[i+j] != pStr[j])
			{
				bMatch = FALSE;
				break;
			}

		if (bMatch)
			return 0;
	}

	return -1;

}

BOOL RegQueryFileInfo(LPCTSTR pszExecutable, HINSTANCE hInst, LPCTSTR pszExt)
{
    TCHAR szBuff[MAX_PATH + 32];
    TCHAR szBuff2[MAX_PATH + 32];
    TCHAR szModule[MAX_PATH];
    
	GetFullAppPath(pszExecutable, hInst, szModule, MAX_PATH);

    TCHAR wExt[32], wFile[32+4], wCommand[32+15], wIcon[32+10];

    TCHAR wDefault[256];

    _tcscpy(wDefault, _T("Default"));

    _tcscpy(wExt,_T("."));
    _tcscat(wExt,pszExt);
    
	_tcscpy(wFile,pszExt);
    _tcscat(wFile,_T("file"));

    _tcscpy(wCommand,wFile);
    _tcscat(wCommand,_T("\\Shell\\Open\\Command"));

    _tcscpy(wIcon,wFile);
    _tcscat(wIcon,_T("\\DefaultIcon"));

    DWORD dwSize;
    HKEY hKey = 0;

	if (RegOpenKeyEx (HKEY_CLASSES_ROOT, wExt, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwSize = sizeof(szBuff);
		RegQueryValueEx (hKey, wDefault, 0, NULL, (LPBYTE)szBuff, &dwSize);
		RegCloseKey(hKey);

		if (_tcsicmp(szBuff, wFile) != 0)
			return FALSE;
	}
	else 
		return FALSE;

	if (RegOpenKeyEx (HKEY_CLASSES_ROOT, wCommand, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwSize = sizeof(szBuff);
		RegQueryValueEx (hKey, wDefault, 0, NULL, (LPBYTE)szBuff, &dwSize);
		RegCloseKey(hKey);

        wsprintf(szBuff2, _T("\"%s\" %%1"), szModule);
        
		if (SameProgram(szBuff, szModule) != 0)
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;


}

void RegFileInfo(LPCTSTR pszExecutable, HINSTANCE hInst, LPCTSTR pszExt, int idi_app) // IDI_APP
{
    TCHAR szBuff[MAX_PATH + 32];
    TCHAR szBuff2[MAX_PATH + 32];
    TCHAR szModule[MAX_PATH];
    
	GetFullAppPath(pszExecutable, hInst, szModule, MAX_PATH);

    TCHAR wExt[32], wFile[32+4], wCommand[32+15], wIcon[32+10];

    TCHAR wDefault[256];
	TCHAR szBackup[256];

    _tcscpy(wDefault, _T("Default"));

    _tcscpy(wExt,_T("."));
    _tcscat(wExt,pszExt);
    
	_tcscpy(wFile,pszExt);
    _tcscat(wFile,_T("file"));

    _tcscpy(wCommand,wFile);
    _tcscat(wCommand,_T("\\Shell\\Open\\Command"));

    _tcscpy(wIcon,wFile);
    _tcscat(wIcon,_T("\\DefaultIcon"));

    DWORD dwSize;
    HKEY hKey = 0;
    DWORD dwDisposition;

	if (RegOpenKeyEx (HKEY_CLASSES_ROOT, wExt, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwSize = sizeof(szBuff);
		RegQueryValueEx (hKey, wDefault, 0, NULL, (LPBYTE)szBuff, &dwSize);
		RegCloseKey(hKey);

		if (_tcsicmp(szBuff, wFile) != 0)
		{
			_tcscpy(szBuff2,wFile);
			_tcscat(szBuff2,_T("\\OldProg"));
			if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuff2, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)==ERROR_SUCCESS)
			{
				wsprintf(szBackup, _T("LinkBackup"));				
				dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
				RegSetValueEx(hKey, szBackup, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
				RegCloseKey(hKey);
			}
		}

	}

    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, wExt, 0, _T(""),
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
    {
        _tcscpy(szBuff, wFile);
        dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
        RegSetValueEx(hKey, wDefault, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
        RegCloseKey(hKey);
    }

	if (RegOpenKeyEx (HKEY_CLASSES_ROOT, wCommand, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwSize = sizeof(szBuff);
		RegQueryValueEx (hKey, wDefault, 0, NULL, (LPBYTE)szBuff, &dwSize);
		RegCloseKey(hKey);

        wsprintf(szBuff2, _T("\"%s\" %%1"), szModule);
        
		//if (_tcsicmp(szBuff, szBuff2) != 0)
		
		if (SameProgram(szBuff, szModule) != 0)
		{
			_tcscpy(szBuff2,wFile);
			_tcscat(szBuff2,_T("\\OldProg"));
			if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuff2, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)==ERROR_SUCCESS)
			{
				wsprintf(szBackup, _T("ProgramBackup"));				
				dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
				RegSetValueEx(hKey, szBackup, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
				RegCloseKey(hKey);
			}
		}

	}

    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, wCommand, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
    {
        wsprintf(szBuff, _T("\"%s\" %%1"), szModule);
        dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
        RegSetValueEx(hKey, wDefault, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
        RegCloseKey(hKey);
    }


	if (RegOpenKeyEx (HKEY_CLASSES_ROOT, wIcon, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwSize = sizeof(szBuff);
		RegQueryValueEx (hKey, wDefault, 0, NULL, (LPBYTE)szBuff, &dwSize);
		RegCloseKey(hKey);

        wsprintf(szBuff2, _T("%s,-%d"), szModule, idi_app);
        
		//if (_tcsicmp(szBuff, szBuff2) != 0)
		if (SameProgram(szBuff, szModule) != 0)
		{
			_tcscpy(szBuff2,wFile);
			_tcscat(szBuff2,_T("\\OldProg"));
			if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuff2, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
			{
				wsprintf(szBackup, _T("IconBackup"));				
				dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
				RegSetValueEx(hKey, szBackup, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
				RegCloseKey(hKey);
			}
		}

	}

    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, wIcon, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
    {
        wsprintf(szBuff, _T("%s,-%d"), szModule, idi_app);
        dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
        RegSetValueEx(hKey, wDefault, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
        RegCloseKey(hKey);
    }
}

void RegUnregisterFileInfo(LPCTSTR pszExecutable, HINSTANCE hInst, LPCTSTR pszExt) 
{
    TCHAR szBuff[MAX_PATH + 32];
    TCHAR szBuff2[MAX_PATH + 32];
    
	TCHAR szLinkBackup[MAX_PATH + 32];
    TCHAR szIconBackup[MAX_PATH + 32];
    TCHAR szProgramBackup[MAX_PATH + 32];

    TCHAR szModule[MAX_PATH];
    
	GetFullAppPath(pszExecutable, hInst, szModule, MAX_PATH);

    TCHAR wExt[32], wFile[32+4], wCommand[32+15], wIcon[32+10];

    TCHAR wDefault[256];
	
    _tcscpy(wDefault, _T("Default"));

    _tcscpy(wExt,_T("."));
    _tcscat(wExt,pszExt);
    
	_tcscpy(wFile,pszExt);
    _tcscat(wFile,_T("file"));

    _tcscpy(wCommand,wFile);
    _tcscat(wCommand,_T("\\Shell\\Open\\Command"));

    _tcscpy(wIcon,wFile);
    _tcscat(wIcon,_T("\\DefaultIcon"));

    DWORD dwSize;
    HKEY hKey = 0;
	int rc;

	_tcscpy(szBuff2,wFile);
	_tcscat(szBuff2,_T("\\OldProg"));

	// check if we're having backup of previous file association
	if (RegOpenKeyEx (HKEY_CLASSES_ROOT, szBuff2, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwSize = sizeof(szLinkBackup);
		if (RegQueryValueEx (hKey, _T("LinkBackup"), 0, NULL, (LPBYTE)szLinkBackup, &dwSize) != ERROR_SUCCESS)
			memset(szLinkBackup, 0, sizeof(szLinkBackup));

		dwSize = sizeof(szIconBackup);
		if (RegQueryValueEx (hKey, _T("IconBackup"), 0, NULL, (LPBYTE)szIconBackup, &dwSize) != ERROR_SUCCESS)
			memset(szIconBackup, 0, sizeof(szIconBackup));
		
		dwSize = sizeof(szProgramBackup);
		if (RegQueryValueEx (hKey, _T("ProgramBackup"), 0, NULL, (LPBYTE)szProgramBackup, &dwSize) != ERROR_SUCCESS)
			memset(szProgramBackup, 0, sizeof(szProgramBackup));

		RegCloseKey(hKey);		

		if (*szLinkBackup && RegOpenKeyEx (HKEY_CLASSES_ROOT, wExt, 0, KEY_QUERY_VALUE, &hKey)==ERROR_SUCCESS)
		{
			_tcscpy(szBuff, szLinkBackup);
			dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
			RegSetValueEx(hKey, wDefault, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
			RegCloseKey(hKey);
		}
		
		if (*szIconBackup && RegOpenKeyEx (HKEY_CLASSES_ROOT, wIcon, 0, KEY_QUERY_VALUE, &hKey)==ERROR_SUCCESS)
		{
			_tcscpy(szBuff, szIconBackup);
			dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
			RegSetValueEx(hKey, wDefault, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
			RegCloseKey(hKey);
		}
		
		if (*szProgramBackup && RegOpenKeyEx (HKEY_CLASSES_ROOT, wCommand, 0, KEY_QUERY_VALUE, &hKey)==ERROR_SUCCESS)
		{
			_tcscpy(szBuff, szProgramBackup);
			dwSize = (_tcslen(szBuff) + 1) * sizeof(TCHAR);
			RegSetValueEx(hKey, wDefault, 0, REG_SZ, (LPBYTE)szBuff, dwSize);
			RegCloseKey(hKey);
		}
		
		rc = RegDeleteKey(HKEY_CLASSES_ROOT, szBuff2);

	}
	else
	{
		_tcscpy(szBuff,wFile);
		_tcscat(szBuff,_T("\\Shell\\Open\\Command"));
		rc = RegDeleteKey(HKEY_CLASSES_ROOT, szBuff);

		_tcscpy(szBuff,wFile);
		_tcscat(szBuff,_T("\\Shell\\Open"));
		rc = RegDeleteKey(HKEY_CLASSES_ROOT, szBuff);

		_tcscpy(szBuff,wFile);
		_tcscat(szBuff,_T("\\Shell"));
		rc = RegDeleteKey(HKEY_CLASSES_ROOT, szBuff);

		_tcscpy(szBuff,wFile);
		_tcscat(szBuff,_T("\\DefaultIcon"));
		rc = RegDeleteKey(HKEY_CLASSES_ROOT, szBuff);

		_tcscpy(szBuff,wFile);
		rc = RegDeleteKey(HKEY_CLASSES_ROOT, szBuff);
	
		rc = RegDeleteKey(HKEY_CLASSES_ROOT, wExt);
	}
}