/*
*  ppui/CheckBoxLabel.h
*
*  Copyright 2017 Henri Isojärvi
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

#include "CheckBoxLabel.h"

PPCheckBoxLabel::PPCheckBoxLabel(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
	const PPPoint& location, 
	const PPString& text, 
    PPCheckBox* checkBox, 
	bool drawShadow, 
	bool drawUnderlined, 
	bool autoShrink) :
	PPStaticText(id, parentScreen, eventListener, location,
		text,
		drawShadow,
		drawUnderlined,
		autoShrink)
{
	this->checkBox = checkBox;
}

PPCheckBoxLabel::~PPCheckBoxLabel()
{
}

// Toggles the associated check box when the label text is clicked
pp_int32 PPCheckBoxLabel::dispatchEvent(PPEvent * event)
{
	if (!eventListener)
		return -1;

	switch (event->getID())
	{
	case eLMouseDown:
	case eLMouseUp:
	{
		// Create a new event with location on top of the check box.
		// Event location matters because the checkbox button will fire
		// a command event only when mouse button is raised on top of the button.
		const PPPoint checkBoxHitLocation = checkBox->getLocation();
		PPEvent checkBoxEvent = PPEvent(event->getID(), (void*)&checkBoxHitLocation, sizeof(checkBoxHitLocation));
		return checkBox->dispatchEvent(&checkBoxEvent);
	}
	default:
		return -1;
	}
}

