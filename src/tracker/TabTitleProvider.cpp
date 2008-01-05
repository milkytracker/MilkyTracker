/*
 *  TabTitleProvider.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 11.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "TabTitleProvider.h"
#include "ModuleEditor.h"

PPString TabTitleProvider::getTabTitle()
{
	char temp[ModuleEditor::MAX_TITLETEXT+1];
	memset(temp, 0, sizeof(temp));
	moduleEditor.getTitle(temp, ModuleEditor::MAX_TITLETEXT);

	PPString tabTitle = temp;
	if (tabTitle.length() != 0)
		return tabTitle;
	
	PPSystemString fileName = moduleEditor.getModuleFileName();
	fileName = fileName.stripPath();
	char* nameASCIIZ = moduleEditor.getModuleFileName().toASCIIZ();
	tabTitle = nameASCIIZ;
	delete[] nameASCIIZ;

	return tabTitle;
}
