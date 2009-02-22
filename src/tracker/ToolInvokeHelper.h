/*
 *  tracker/ToolInvokeHelper.h
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
 *  ToolInvokeHelper.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.11.05.
 *
 */

#ifndef TOOLINVOKEHELPER__H
#define TOOLINVOKEHELPER__H

#include "DialogBase.h"

class Tracker;

class ToolInvokeHelper : public DialogResponder
{
public:
	enum ToolTypes
	{
		ToolTypeNone,
		ToolTypePatternVolumeScale,
		ToolTypeTrackVolumeScale,
		ToolTypeSelectionVolumeScale,
		ToolTypeQuickChooseInstrument
	};

private:
	struct TLastValues
	{
		float volumeScaleStart, volumeScaleEnd;
	};
	
	Tracker& tracker;
	TLastValues lastValues;
	ToolTypes lastToolType;
	PPDialogBase* dialog;
		
public:
	ToolInvokeHelper(Tracker& theTracker);
	virtual ~ToolInvokeHelper();	

	bool invokeTool(ToolTypes toolType, pp_int16 keyDownKeyCode = -1);
	
private:
	void resetLastValues();

	virtual pp_int32 ActionOkay(PPObject* sender);
	virtual pp_int32 ActionCancel(PPObject* sender);
};

#endif
