/*
 *  ppui/CheckBox.h
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

/////////////////////////////////////////////////////////////////
//
//	PPButton control class
//
/////////////////////////////////////////////////////////////////
#ifndef CHECKBOX__H
#define CHECKBOX__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPButton;

class PPCheckBox : public PPControl, public EventListenerInterface
{
private:
	PPButton* button;

public:
	PPCheckBox(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
			   const PPPoint& location, 
			   bool checked = true);
	
	virtual ~PPCheckBox();

	bool isChecked() const;

	void checkIt(bool checked);

	// from control
	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 dispatchEvent(PPEvent* event);

	virtual void enable(bool b);
	
	virtual void setSize(const PPSize& size);
	virtual void setLocation(const PPPoint& location);

	// from EventListenerInterface
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);
};

#endif
