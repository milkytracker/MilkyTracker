/*
 *  tracker/TabTitleProvider.cpp
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
 *  TabTitleProvider.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 11.12.07.
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
