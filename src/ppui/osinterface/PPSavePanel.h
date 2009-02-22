/*
 *  ppui/osinterface/PPSavePanel.h
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
 *  PPSavePanel.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 05 2005.
 *
 */

#ifndef PPSAVEPANEL__H
#define PPSAVEPANEL__H

#include "PPOpenPanel.h"

class PPSavePanel : public PPOpenPanel
{
protected:
	PPSystemString defaultFileName;

public:
	PPSavePanel(PPScreen* screen, const char* caption, const PPSystemString& defaultFileName) : 
		PPOpenPanel(screen, caption)
	{
		this->defaultFileName = defaultFileName;
	}
	
	virtual ~PPSavePanel() {}
	
	virtual ReturnCodes runModal();
};

#endif
