/*
 *  PPSavePanel.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 05 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
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
