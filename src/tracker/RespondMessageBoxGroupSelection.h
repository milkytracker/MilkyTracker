/*
 *  tracker/RespondMessageBoxGroupSelection.h
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
 *  RespondMessageBoxGroupSelection.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 23.06.06.
 *
 */

#ifndef __RESPONDMESSAGEBOXGROUPSELECTION_H__
#define __RESPONDMESSAGEBOXGROUPSELECTION_H__

#include "RespondMessageBox.h"

class PPString;
template <class T> class PPSimpleVector; 

class RespondMessageBoxGroupSelection : public RespondMessageBox
{
private:
	pp_uint32 selection;

public:
	RespondMessageBoxGroupSelection(PPScreen* screen, 
						   RespondListenerInterface* responder,
						   pp_int32 id,
						   const PPString& caption,
						   const PPSimpleVector<PPString>& choices);

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	

	pp_uint32 getSelection() { return selection; }
};

#endif

