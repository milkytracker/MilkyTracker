/*
 *  ToolInvokeHelper.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.11.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef TOOLINVOKEHELPER__H
#define TOOLINVOKEHELPER__H

#include "RespondMessageBox.h"

class Tracker;

class ToolInvokeHelper : public RespondListenerInterface
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
	RespondMessageBox* respondMessageBox;
		
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
