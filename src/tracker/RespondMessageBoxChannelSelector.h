/*
 *  RespondMessageBoxChannelSelector.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef RESPONDMESSAGEBOXCHANNELSELECTOR__H
#define RESPONDMESSAGEBOXCHANNELSELECTOR__H

#include "RespondMessageBox.h"

class RespondMessageBoxChannelSelector : public RespondMessageBox
{
private:
	class PPListBox* listBox;

public:
	RespondMessageBoxChannelSelector(PPScreen* screen, 
									 RespondListenerInterface* responder,
									 pp_int32 id,
									 const PPString& caption);

	PPListBox* getListBox() { return listBox; }

};

#endif
