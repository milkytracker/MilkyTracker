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

/////////////////////////////////////////////////////////////////
//
//	Check box label control class
//
/////////////////////////////////////////////////////////////////
#ifndef CHECKBOXLABEL__H
#define CHECKBOXLABEL__H

#include "StaticText.h"
#include "CheckBox.h"

class PPCheckBoxLabel : public PPStaticText
{
private:
	PPCheckBox* checkBox;

public:
	PPCheckBoxLabel(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener,
		const PPPoint& location,
		const PPString& text,
	    PPCheckBox* checkBox,
		bool drawShadow = false,
		bool drawUnderlined = false,
		bool autoShrink = false);

	virtual ~PPCheckBoxLabel();

	virtual pp_int32 dispatchEvent(PPEvent* event);
};
#endif