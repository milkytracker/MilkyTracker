/*
 *  tracker/RespondMessageBoxHandlers.h
 *
 *  Copyright 2008 Peter Barth
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
 *  RespondMessageBoxHandlers.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#ifndef __RESPONDMESSAGEBOXHANDLERS_H__
#define __RESPONDMESSAGEBOXHANDLERS_H__

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
