/*
 *  ppui/StaticText.h
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
//	Static text control class
//
/////////////////////////////////////////////////////////////////
#ifndef STATICTEXT__H
#define STATICTEXT__H

#include "BasicTypes.h"
#include "Control.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPButton;

class PPStaticText : public PPControl
{
private:
	const PPColor* color;

	bool drawShadow;
	bool underlined;
	bool autoShrink;

	PPColor shadowColor;

	PPString text;

	PPFont* font;
	
	PPSize extent;

public:
	PPStaticText(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
				 const PPPoint& location, 
				 const PPString& text, 
				 bool drawShadow = false,
				 bool drawUnderlined = false,
				 bool autoShrink = false);
	
	virtual ~PPStaticText();

	void setColor(const PPColor& color) { this->color = &color; }

	void setText(const PPString& text);

	const PPString& getText() const { return text; }

	void setFont(PPFont* font) { this->font = font; calcExtent(); }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 dispatchEvent(PPEvent* event);

	void setUnderlined(bool b) { underlined = b; }

	void setValue(pp_int32 value, bool hex, pp_uint32 numDigits = 0, bool negative = false);

	void setIntValue(pp_int32 value, pp_uint32 numDecDigits = 0, bool negative = false);

	void setHexValue(pp_int32 value, pp_uint32 numHexDigits = 0);

	void setExtent(PPSize extent) { this->extent = extent; }

private:
	void calcExtent();
};

#endif
