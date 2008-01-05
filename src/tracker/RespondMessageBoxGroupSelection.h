/*
 *  RespondMessageBoxGroupSelection.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 23.06.06.
 *  Copyright (c) 2006 milkytracker.net, All rights reserved.
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

