/*
 *  tracker/DialogQuickChooseInstrument.h
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
 *  DialogQuickChooseInstrument.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#ifndef __DIALOGQUICKCHOOSEINSTRUMENT_H__
#define __DIALOGQUICKCHOOSEINSTRUMENT_H__

#include "DialogBase.h"

class DialogQuickChooseInstrument : public PPDialogBase
{
private:
	pp_int32 value;
	pp_int32 valueRangeStart, valueRangeEnd, valueIncreaseStep;
	
public:
	DialogQuickChooseInstrument(PPScreen* screen, 
								DialogResponder* responder,
								pp_int32 id,
								const PPString& caption);

	pp_int32 getValue() { return value; }
	
	void setValue(pp_int32 val) { if (val < valueRangeStart) val = valueRangeStart; if (val > valueRangeEnd) val = valueRangeEnd; value = val; updateListBoxes(); }

	void setValueCaption(const PPString& caption);

	virtual void show(bool b = true);

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	

	static pp_uint16 numPadKeyToValue(pp_uint16 keyCode);

private:
	void fitListBoxes();
	
	void fitListBox(pp_int32 id, pp_int32 valueRangeStart, pp_int32 valueRangeEnd);

	void updateListBoxes();
	
	void updateListBox(pp_int32 id, pp_int32 val);
	
	void listBoxEnterEditState(pp_int32 id);
	
	void commitChanges();
};

#endif

