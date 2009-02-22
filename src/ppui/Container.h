/*
 *  ppui/Container.h
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
//	PPContainer control class (can contain other controls)
//
/////////////////////////////////////////////////////////////////
#ifndef CONTAINER__H
#define CONTAINER__H

#include "BasicTypes.h"
#include "Control.h"
#include "Button.h"
#include "SimpleVector.h"

class PPContainer : public PPControl
{
protected:
	const PPColor* color;
	bool border;

private:
	PPButton* backgroundButton;

	PPSimpleVector<PPControl> controls;
	PPSimpleVector<PPControl>* timerEventControls;

	PPControl* focusedControl;

	PPPoint lastMousePoint;
	PPControl* lastMouseOverControl;

	// Control caught by mouse button press (left & right)
	PPControl* caughtControl;
	pp_int32 currentlyPressedMouseButtons;

public:
	PPContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
				const PPPoint& location, const PPSize& size, 
				bool border = true);

	virtual ~PPContainer();
	
	virtual void setSize(const PPSize& size);
	virtual void setLocation(const PPPoint& location);

	//void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; backgroundButton->setColor(color); }
	void setColor(const PPColor& color) { this->color = &color; backgroundButton->setColor(color); }

	const PPColor& getColor() const { return *color; }

	void addControl(PPControl* control);

	bool removeControl(PPControl* control);

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 dispatchEvent(PPEvent* event);

	virtual bool gainsFocus() const;
	virtual bool gainedFocusByMouse() const;

	virtual void show(bool visible);
	virtual void hide(bool hidden);

	virtual bool isContainer() const { return true; }

	PPControl* getControlByID(pp_int32 id);

	PPControl* getFocusedControl() { return focusedControl; }

	PPSimpleVector<PPControl>& getControls() { return controls; }

	void setFocus(PPControl* control, bool repaint = true);
	bool hasFocus(PPControl* control) const;
	
	void move(const PPPoint& offset);
	void adjustContainerSize();

protected:
	void paintControls(PPGraphicsAbstract* g)
	{
		for (pp_int32 i = 0; i < controls.size(); i++)
		{
			PPControl* ctrl = controls.get(i);
			if (ctrl->isVisible())
				ctrl->paint(g);
		}
	}
	
	friend class PPScreen;
};

#endif
