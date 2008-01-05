/*
 *  PPQuitSaveAlert.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *  Copyright 2005 milkytracker.net All rights reserved.
 *
 */

#include <windows.h>
#include <tchar.h>
#include "PPQuitSaveAlert.h"

extern HWND hWnd;

PPQuitSaveAlert::ReturnCodes PPQuitSaveAlert::runModal()
{
	ReturnCodes res = ReturnCodeCANCEL;
	
	int nID = MessageBox(hWnd, 
						 _T("Do you want to save the changes you made to your documents?"), 
						 _T("Save changes"), MB_YESNOCANCEL | MB_ICONQUESTION);
			
	if (nID == IDYES)
	{
		res = ReturnCodeOK;
	}
	else if (nID == IDNO)
	{
		res = ReturnCodeNO;
	}
	else if (nID == IDCANCEL)
	{
		res = ReturnCodeCANCEL;
	}
	
	return res;
}
