#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include <windows.h>

void GetFullAppPath(LPCTSTR pszApplicationTitle, HINSTANCE hInst, LPTSTR pszModule, UINT max);
BOOL RegQueryFileInfo(LPCTSTR pszExecutable, HINSTANCE hInst, LPCTSTR pszExt);
void RegFileInfo(LPCTSTR pszExecutable, HINSTANCE hInst, LPCTSTR pszExt, int idi_app); // IDI_APP
void RegUnregisterFileInfo(LPCTSTR pszExecutable, HINSTANCE hInst, LPCTSTR pszExt);

#endif