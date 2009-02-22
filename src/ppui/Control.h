/*
 *  ppui/Control.h
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
//	Basic control class
//
/////////////////////////////////////////////////////////////////
#ifndef CONTROL__H
#define CONTROL__H

// our basic types
#include "Object.h"
#include "BasicTypes.h"

// Forwards
class PPEvent;
class EventListenerInterface;
class PPGraphicsAbstract;
class PPScreen;

/////////////////////////////////////////////////////////////////
//	Basic interface for all controls
/////////////////////////////////////////////////////////////////
class PPControl : public PPObject
{
private:
	pp_int32 id;

protected:
	PPScreen*	parentScreen;
	EventListenerInterface* eventListener; // this object will receive events
	PPControl* ownerControl;

	PPPoint location;
	PPSize size;

	bool visible;
	bool enabled;

	bool hasFocus;

	PPControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener = NULL);
	PPControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
			  const PPPoint& location, const PPSize& size);

public:
	virtual ~PPControl() {}

	void setEventListener(EventListenerInterface* eventListener) { this->eventListener = eventListener; }
	EventListenerInterface* getEventListener() { return eventListener; }

	void setOwnerControl(PPControl* ownerControl) { this->ownerControl = ownerControl; }
	PPControl* getOwnerControl() const { return ownerControl; }

	virtual pp_int32 dispatchEvent(PPEvent* event);

	virtual void paint(PPGraphicsAbstract* graphics) = 0;

	virtual bool gainsFocus() const { return false; }
	virtual bool gainedFocusByMouse() const { return gainsFocus(); }

	virtual bool isActive() const { return true; }

	virtual bool hit(const PPPoint& p) const; 

	virtual void show(bool visible) { this->visible = visible; }
	virtual void hide(bool hidden) { show(!hidden); }

	virtual bool isVisible() const;
	virtual bool isHidden() const { return !isVisible(); }
	
	virtual void enable(bool b) { enabled = b; }
	virtual bool isEnabled() const { return enabled; }

	const PPPoint& getLocation() const { return location; }
	const PPSize& getSize() const { return size; }

	PPRect getBoundingRect() const
	{
		PPRect result(location.x, location.y, 
					  location.x + size.width, location.y + size.height);
		return result;
	}

	pp_int32 getID() const { return id; }

	virtual void setSize(const PPSize& size) { this->size = size; }
	virtual void setLocation(const PPPoint& location) { this->location = location; }
	virtual bool isContainer() const { return false; }
	virtual bool isListBox() const { return false; }
	virtual bool receiveTimerEvent() const { return false; }

	virtual bool gotFocus() const { return hasFocus; }

protected:
	virtual void translateCoordinates(PPPoint& cp) 
	{ 
		cp.x-=location.x;
		cp.y-=location.y;
	}

	virtual void notifyChanges();
	
	void drawBorder(PPGraphicsAbstract* g, const PPColor& borderColor);
	void drawThickBorder(PPGraphicsAbstract* g, const PPColor& borderColor);
	
};

#endif
