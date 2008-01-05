/*
 *  RespondMessageBoxResample.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef RESPONDMESSAGEBOXRESAMPLE__H
#define RESPONDMESSAGEBOXRESAMPLE__H

#include "RespondMessageBox.h"

class RespondMessageBoxResample : public RespondMessageBox
{
private:
	class PPListBox* listBoxes[3];
	pp_int32 currentSelectedListBox;
	pp_int32 relnote, finetune;
	pp_uint32 size, finalSize;
	pp_uint32 count;
	float c4spd;
	float originalc4spd;

public:
	RespondMessageBoxResample(PPScreen* screen, 
							  RespondListenerInterface* responder,
							  pp_int32 id);

	virtual void show();

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	

	void setRelNote(pp_int32 note);
	void setFineTune(pp_int32 ft);
	void setC4Speed(float c4spd);
	void setSize(pp_uint32 size);
	
	float getC4Speed() { return c4spd; }
	pp_int32 getRelNote() { return relnote; }
	pp_int32 getFineTune() { return finetune; }
	
private:
	void listBoxEnterEditState(pp_int32 id);
	
	void updateListBoxes();
	void updateListBox(pp_int32 id, float val, pp_int32 numDecimals);
	
	void commitChanges();
	
	void switchListBox();

	void toC4Speed();
	void fromC4Speed();

	void calcSize();

	void validate();
};

#endif

