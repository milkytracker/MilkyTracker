/*
 *  RespondMessageBoxWithValues.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef RESPONDMESSAGEBOXWITHVALUES__H
#define RESPONDMESSAGEBOXWITHVALUES__H

#include "RespondMessageBox.h"

class RespondMessageBoxWithValues : public RespondMessageBox
{
public:
	enum ValueStyles
	{
		ValueStyleEnterOneValue,
		ValueStyleEnterTwoValues
	};

private:
	float valueOne, valueTwo;
	float valueOneRangeStart, valueOneRangeEnd, valueOneIncreaseStep;
	float valueTwoRangeStart, valueTwoRangeEnd, valueTwoIncreaseStep;
	pp_int32 numValueOneDecimals, numValueTwoDecimals;
	
public:
	RespondMessageBoxWithValues(PPScreen* screen, 
								RespondListenerInterface* responder,
							    pp_int32 id,
								const PPString& caption,
								ValueStyles style);

	float getValueOne() { return valueOne; }
	float getValueTwo() { return valueTwo; }
	
	void setValueOne(float val) { if (val < valueOneRangeStart) val = valueOneRangeStart; if (val > valueOneRangeEnd) val = valueOneRangeEnd; valueOne = val; updateListBoxes(); }
	void setValueTwo(float val) { if (val < valueTwoRangeStart) val = valueTwoRangeStart; if (val > valueTwoRangeEnd) val = valueTwoRangeEnd; valueTwo = val; updateListBoxes(); }

	void setValueOneRange(float start, float end, pp_int32 numDecimals); 
	void setValueOneIncreaseStep(float step) { valueOneIncreaseStep = step; }
	void setValueTwoRange(float start, float end, pp_int32 numDecimals); 
	void setValueTwoIncreaseStep(float step) { valueTwoIncreaseStep = step; }
	
	void setValueOneCaption(const PPString& caption);
	void setValueTwoCaption(const PPString& caption);

	virtual void show();

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	

private:
	void fitListBoxes();
	
	void fitListBox(pp_int32 id, float valueOneRangeStart, float valueOneRangeEnd, pp_int32 numDecimals);

	void updateListBoxes();
	
	void updateListBox(pp_int32 id, float val, pp_int32 numDecimals);
	
	void listBoxEnterEditState(pp_int32 id);
	
	void commitChanges();
	
	void switchListBox();
};

#endif

