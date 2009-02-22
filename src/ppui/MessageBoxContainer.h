/*
 *  ppui/MessageBoxContainer.h
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
//	PPContainer control that looks like a Message box 
//  (can contain other controls)
//
/////////////////////////////////////////////////////////////////
#ifndef MESSAGEBOXCONTAINER__H
#define MESSAGEBOXCONTAINER__H

#include "BasicTypes.h"
#include "Container.h"

class PPButton;

class PPMessageBoxContainer : public PPContainer
{
private:
	PPString caption;
	pp_int32 captionSize;
	
	PPPoint buttonLocation;
	PPSize buttonSize;
	PPButton* button;

	bool captured;
	PPPoint lastCapturePoint;

public:
	PPMessageBoxContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
						  const PPPoint& location, const PPSize& size, const PPString& caption);

	virtual ~PPMessageBoxContainer();

	virtual void paint(PPGraphicsAbstract* graphics);

	virtual void setSize(const PPSize& size);
	virtual void setLocation(const PPPoint& location);

	virtual pp_int32 dispatchEvent(PPEvent* event);

	void setCaption(const PPString& caption) { this->caption = caption; }
	
private:
	bool isPointInCaption(const PPPoint& point);
	
	bool handleMove(PPPoint point);	
};

#endif
