/*
 *  ppui/RadioGroup.h
 *
 *  Copyright 2008 Peter Barth
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
//	PPRadioGroup control class
//
/////////////////////////////////////////////////////////////////
#ifndef RADIOGROUP__H
#define RADIOGROUP__H

#include "BasicTypes.h"
#include "Control.h"
#include "SimpleVector.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPButton;

class PPRadioGroup : public PPControl
{
private:
	const PPColor* radioButtonColor;
	const PPColor* textColor;

	PPSimpleVector<PPString> items;

	pp_uint32 spacerHeight;

	pp_uint32 choice;

	PPFont* font;

	bool horizontal;

	pp_int32 maxWidth;

public:
	enum DEFAULTEXTENTS {
		eDefaultSpacerHeight = 5,
		eDefaultRadioWidth = 14
	};

	PPRadioGroup(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size, pp_uint32 spacerHeight = eDefaultSpacerHeight);
	~PPRadioGroup();

	void setColor(const PPColor& color) { this->radioButtonColor = &color; }

	void addItem(const PPString& item);
	const PPString& getItem(pp_int32 index);

	void setSpacerHeight(pp_uint32 spacerHeight) { this->spacerHeight = spacerHeight; }

	void setFont(PPFont* font) { this->font = font; }

	void setHorizontal(bool b) { horizontal = b; }

	void setChoice(pp_uint32 choice) { this->choice = choice; }

	pp_uint32 getChoice() { return choice; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);

	virtual bool gainsFocus() { return false; }

	virtual bool isActive() { return true; }

	void fitSize(); 
};

#endif
