/*
 *  RespondMessageBoxEQ.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.04.07.
 *  Copyright 2007 milkytracker.net. All rights reserved.
 *
 */

#ifndef __RESPONDMESSAGEBOXEQ_H__
#define __RESPONDMESSAGEBOXEQ_H__

#include "RespondMessageBox.h"

class RespondMessageBoxEQ : public RespondMessageBox
{
public:
	enum EQNumBands
	{
		EQ3Bands,
		EQ10Bands
	};

private:
	EQNumBands numBands;
	pp_uint32 numSliders;

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	
	
	void resetSliders();
	void update();

public:
	RespondMessageBoxEQ(PPScreen* screen, 
						RespondListenerInterface* responder,
						pp_int32 id,
						EQNumBands numBands);

	void setBandParam(pp_uint32 index, float param);
	float getBandParam(pp_uint32 index) const;
	
	EQNumBands getNumBands() const { return numBands; }
	pp_uint32 getNumBandsAsInt() const { return numSliders; }
};


#endif
