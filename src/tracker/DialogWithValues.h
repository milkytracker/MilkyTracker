/*
 *  tracker/DialogWithValues.h
 *
 *  Copyright 2009 Peter Barth
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
 *  DialogWithValues.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#ifndef __DIALOGWITHVALUES_H__
#define __DIALOGWITHVALUES_H__

#include "DialogBase.h"

class DialogWithValues : public PPDialogBase
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
	DialogWithValues(PPScreen* screen, 
					 DialogResponder* responder,
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

	virtual void show(bool b = true);

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

