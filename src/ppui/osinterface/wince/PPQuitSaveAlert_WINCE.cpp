/*
 *  ppui/osinterface/wince/PPQuitSaveAlert_WINCE.cpp
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

/*
 *  PPQuitSaveAlert.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *
 */

#include <windows.h>
#include <tchar.h>
#include "gx.h"
#include "PPQuitSaveAlert.h"

extern HWND hWnd;

// from FirstGX.cpp
void SuspendFullScreen();
void ResumeFullScreen();

PPQuitSaveAlert::ReturnCodes PPQuitSaveAlert::runModal()
{
	SuspendFullScreen();

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

	ResumeFullScreen();
	
	return res;
}
