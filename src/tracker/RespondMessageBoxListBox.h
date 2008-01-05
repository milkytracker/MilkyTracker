/*
 *  RespondMessageBoxListBox.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef RESPONDMESSAGEBOXLISTBOX__H
#define RESPONDMESSAGEBOXLISTBOX__H

#include "RespondMessageBox.h"

class RespondMessageBoxListBox : public RespondMessageBox
{
private:
	class PPListBox* listBox;

public:
	RespondMessageBoxListBox(PPScreen* screen, 
							 RespondListenerInterface* responder,
							 pp_int32 id,
							 const PPString& caption,
							 bool okCancel = false);
	
	PPListBox* getListBox() { return listBox; }

};

#endif
