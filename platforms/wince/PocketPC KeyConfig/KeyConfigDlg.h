#ifndef KEYCONFIGDLG__H
#define KEYCONFIGDLG__H

#ifndef _WIN32_WCE_EMULATION
#define NUM_INVALID_KEYS	5
extern short		nInvalidKeys[5];
#else
#define NUM_INVALID_KEYS	2
extern short		nInvalidKeys[2];
#endif

void RegisterKeys(HWND hWnd);
void UnregisterKeys(HWND hWnd);
void CreateKeyDialog(HINSTANCE hInst, HWND hWnd);


#endif