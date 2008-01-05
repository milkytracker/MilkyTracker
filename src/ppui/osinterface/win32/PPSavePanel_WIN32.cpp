/*
 *  PPSavePanel.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 05 2005.
 *  Copyright (c) 2005 milkytracker.net All rights reserved.
 *
 */
 
#include <windows.h>
#ifdef _WIN32_WCE
	#include <Commdlg.h>
#endif
#include <tchar.h>
#include "PPSavePanel.h"

extern HWND hWnd;

PPSavePanel::ReturnCodes PPSavePanel::runModal()
{
	TCHAR			szFile[MAX_PATH+1];
	OPENFILENAME	ofn;
	
	memset(szFile, 0, sizeof(szFile));
	_tcscpy(szFile, defaultFileName);
	memset(&ofn, 0, sizeof(ofn));
	
	ofn.lStructSize   = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;

	PPString sourceFilter;

	pp_int32 i = 0;

	for (i = 0; i < items.size(); i++)
	{
		sourceFilter.append(items.get(i)->description);
		sourceFilter.append(" (.");
		sourceFilter.append(items.get(i)->extension);
		sourceFilter.append(")|");
		sourceFilter.append("*.");
		sourceFilter.append(items.get(i)->extension);
		sourceFilter.append("|");
	}

	const char* src = sourceFilter;

	TCHAR* dstFilter = new TCHAR[sourceFilter.length()+2];
	memset(dstFilter, 0, (sourceFilter.length()+2)*sizeof(TCHAR));
	
	for (i = 0; i < (signed)sourceFilter.length(); i++)
	{
		if (src[i] == '|')
			dstFilter[i] = '\0';
		else
			dstFilter[i] = src[i];
	}
	
	ofn.lpstrFilter = dstFilter;
	ofn.lpstrTitle = _T("Save File");
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	PPSystemString defaultExtension(items.get(0)->extension);
	ofn.lpstrDefExt = defaultExtension;

	ReturnCodes err = ReturnCodeCANCEL;

	if (GetSaveFileName(&ofn))
	{
		fileName = szFile;
		err = ReturnCodeOK;
	}

	delete[] dstFilter;
	
	return err;
}
