#include <windows.h>
#include "PPMessageBox.h"

extern HWND hWnd;

PPMessageBox::ReturnCodes PPMessageBox::runModal()
{
	ReturnCodes res = ReturnCodeOK;

	::MessageBox(hWnd, content, caption, MB_OK);

	return res;
}
