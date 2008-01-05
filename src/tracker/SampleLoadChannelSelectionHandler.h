/*
 *  SampleLoadChannelSelectionHandler.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SAMPLELOADCHANNELSELECTIONHANDLER__H
#define SAMPLELOADCHANNELSELECTIONHANDLER__H

#include "RespondMessageBox.h"

class Tracker;

class SampleLoadChannelSelectionHandler : public RespondListenerInterface
{
private:
	Tracker& tracker;
	PPSystemString fileName;
	PPSystemString preferredFileName;
	
public:
	bool suspendPlayer;
	
	SampleLoadChannelSelectionHandler(Tracker& theTracker) : 
		tracker(theTracker)
	{
		suspendPlayer = false;
	}

	virtual ~SampleLoadChannelSelectionHandler() { }

	void setCurrentFileName(const PPSystemString& fileName);
	void setPreferredFileName(const PPSystemString& fileName);
	
	virtual pp_int32 ActionOkay(PPObject* sender);
	virtual pp_int32 ActionCancel(PPObject* sender);
	virtual pp_int32 ActionUser1(PPObject* sender);
};

#endif
