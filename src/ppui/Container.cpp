/*
 *  ppui/Container.cpp
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

#include "Container.h"
#include "Event.h"
#include "Screen.h"
#include "PPUIConfig.h"

PPContainer::PPContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener,
						 const PPPoint& location, const PPSize& size,
						 bool border /* = true */) :
	PPControl(id, parentScreen, eventListener, location, size),
	color(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorContainer)),
	focusedControl(NULL),
	lastMouseOverControl(NULL),
	lastMousePoint(PPPoint(0,0)),
	caughtControl(NULL),
	currentlyPressedMouseButtons(0)
{
	this->border = border;

	backgroundButton = new PPButton(0, parentScreen, NULL, location, size, border, false);
	backgroundButton->setColor(*color);

	timerEventControls = new PPSimpleVector<PPControl>(16, false);
}

PPContainer::~PPContainer()
{
	delete timerEventControls;

	for (pp_int32 i = 0; i < controls.size(); i++)
	{
		if (controls.get(i)->receiveTimerEvent())
		{
			parentScreen->removeTimerEventControl(controls.get(i));
		}
	}

	delete backgroundButton;
}

void PPContainer::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	backgroundButton->paint(g);

	paintControls(g);
}

pp_int32 PPContainer::dispatchEvent(PPEvent* event)
{
	if (event->getID() == eInvalid)
		return 0;

	pp_int32 i, res = 0;
	bool handleEvent = false;

	if (caughtControl)
	{
		PPControl* control = caughtControl;

		switch (event->getID())
		{
			case eLMouseDown:
			case eRMouseDown:
			case eMMouseDown:
				currentlyPressedMouseButtons++;
				res = control->dispatchEvent(event);
				goto exit;

			case eLMouseUp:
			case eRMouseUp:
			case eMMouseUp:
				currentlyPressedMouseButtons--;
				if (currentlyPressedMouseButtons == 0)
				{
					caughtControl = NULL;
					if (control->gainedFocusByMouse())
					{
						setFocus(control);
					}
				}
				// break is missing intentionally
			case eLMouseDoubleClick:
			case eRMouseDoubleClick:
			case eMMouseDoubleClick:
			case eLMouseDrag:
			case eLMouseRepeat:
			case eRMouseDrag:
			case eRMouseRepeat:
			case eMMouseDrag:
			case eMMouseRepeat:
			case eMouseMoved:
				res = control->dispatchEvent(event);
				goto exit;
			default:
				break;
		}
	}

	if (focusedControl)
	{
		PPControl* control = focusedControl;

		bool mouseEvent = false;

		switch (event->getID())
		{
			case eLMouseDown:
			case eRMouseDown:
			case eMMouseDown:
			case eLMouseUp:
			case eRMouseUp:
			case eMMouseUp:
			case eLMouseDoubleClick:
			case eRMouseDoubleClick:
			case eMMouseDoubleClick:
			case eLMouseDrag:
			case eLMouseRepeat:
			case eRMouseDrag:
			case eRMouseRepeat:
			case eMMouseDrag:
			case eMMouseRepeat:
			case eMouseMoved:
			case eMouseWheelMoved:
				mouseEvent = true;
				break;
			default:
				break;
		}

		if (!mouseEvent)
		{
			res = control->dispatchEvent(event);

			// we're getting send an focus lost event
			// so remove all references to focused controls
			if (event->getID() == eFocusLost ||
				event->getID() == eFocusLostNoRepaint)
			{
				focusedControl = NULL;
			}

			goto exit;
		}
	}

	switch (event->getID())
	{
		case eLMouseDown:
		case eRMouseDown:
		case eMMouseDown:
		case eMouseWheelMoved:
		case eMouseMoved:
			handleEvent = true;
			break;

		// we got a focus gained message
		// if we don't have some focused control set,
		// we simply search for a control that could gain the focus
		// and assign focus to that
		case eFocusGainedNoRepaint:
		case eFocusGained:
			if (focusedControl != NULL || caughtControl != NULL)
				break;

			for (pp_int32 j = 0; j < controls.size(); j++)
			{
				PPControl* control = controls.get(j);
				if (control->gainsFocus())
				{
					setFocus(control, event->getID() == eFocusGained);
					break;
				}
			}
			break;
		default:
			break;
	}

	if (!handleEvent)
		goto exit;

	// handle other events
	for (i = 0; i < controls.size(); i++)
	{
		PPControl* control = controls.get(i);

		if (!control->isVisible() || !control->isEnabled())
			continue;

		bool abortLoop = false;

		switch (event->getID())
		{
			// mouse wheel has moved
			case eMouseWheelMoved:
			{
				TMouseWheelEventParams* params = (TMouseWheelEventParams*)event->getDataPtr();
				if (control->hit(params->pos) && control->isActive())
				{
					control->dispatchEvent(event);
					abortLoop = true;
				}
				break;
			}

			// mouse has been moved also gets this
			case eMouseMoved:
			{
				PPPoint* p = (PPPoint*)event->getDataPtr();

				if (control->isActive())
				{
					bool bHit = control->hit(*p);
					bool bLastHit = control->hit(lastMousePoint);

					if (!bLastHit && bHit)
					{
						PPEvent e(eMouseEntered, p, sizeof(PPPoint));
						control->dispatchEvent(&e);
					}
					else if (bLastHit && !bHit)
					{
						PPEvent e(eMouseLeft, p, sizeof(PPPoint));
						control->dispatchEvent(&e);
					}

					if (bHit)
					{
						if (control != lastMouseOverControl && lastMouseOverControl)
							lastMouseOverControl->dispatchEvent(event);

						res = control->dispatchEvent(event);
						lastMouseOverControl = control;
						abortLoop = true;
					}
				}
				break;
			}

			// Mouse button down event
			case eLMouseDown:
			case eRMouseDown:
			case eMMouseDown:
				if (control->hit(*(PPPoint*)event->getDataPtr()) &&
					control->isActive() &&
					caughtControl == NULL)
				{
					currentlyPressedMouseButtons++;
					caughtControl = control;
					control->dispatchEvent(event);
					abortLoop = true;
				}
				break;
			default:
				break;
}

		if (abortLoop || event->getID() == eInvalid)
			break;
	}

exit:
	if (event->getID() == eMouseMoved)
	{
		PPPoint* p = (PPPoint*)event->getDataPtr();
		lastMousePoint = *p;
	}

	return res;
}

PPControl* PPContainer::getControlByID(pp_int32 id)
{
	pp_int32 i;

	for (i = 0; i < controls.size(); i++)
	{
		pp_int32 cID = controls.get(i)->getID();
		if (cID == id)
			return controls.get(i);

	}

	// not found, try recursive in sub-containers if any
	for (i = 0; i < controls.size(); i++)
	{
		PPControl* control = controls.get(i);
		if (control->isContainer())
		{
			PPControl* newCtrl = static_cast<PPContainer*>(control)->getControlByID(id);
			if (newCtrl)
				return newCtrl;
		}
	}

	return NULL;
}

void PPContainer::addControl(PPControl* control)
{
	control->setOwnerControl(this);
	controls.add(control);
	if (control->receiveTimerEvent())
	{
		if (this->isVisible())
			parentScreen->addTimerEventControl(control);
		else
			timerEventControls->add(control);
	}
}

bool PPContainer::removeControl(PPControl* control)
{
	pp_int32 i;
	bool res = false;

	if (control->receiveTimerEvent())
	{
		parentScreen->removeTimerEventControl(control);
	}

	for (i = 0; i < controls.size(); i++)
	{
		if (controls.get(i) == control)
		{
			controls.remove(i);
			res = true;
		}
	}

	return res;
}

bool PPContainer::gainsFocus() const
{
	if (caughtControl && caughtControl->gainsFocus())
		return true;

	if (focusedControl)
		return true;

	return false;
}

bool PPContainer::gainedFocusByMouse() const
{
	if (caughtControl && caughtControl->gainedFocusByMouse())
		return true;

	return false;
}

void PPContainer::show(bool visible)
{
	PPControl::show(visible);

	if (visible)
	{
		// add timer event controls to parent screen
		for (pp_int32 i = 0; i < timerEventControls->size(); i++)
		{
			parentScreen->addTimerEventControl(timerEventControls->get(i));
		}

		timerEventControls->clear();
	}
	else
	{
		// remove timer event controls from parent screen
		// and add them to our internal list
		for (pp_int32 i = 0; i < controls.size(); i++)
		{
			if (controls.get(i)->receiveTimerEvent())
			{
				parentScreen->removeTimerEventControl(controls.get(i));
				timerEventControls->add(controls.get(i));
			}
		}
	}

}

void PPContainer::hide(bool hidden)
{
	PPControl::hide(hidden);

	if (!hidden)
	{
		// add timer event controls to parent screen
		for (pp_int32 i = 0; i < timerEventControls->size(); i++)
		{
			parentScreen->addTimerEventControl(timerEventControls->get(i));
		}

		timerEventControls->clear();
	}
	else
	{
		// remove timer event controls from parent screen
		// and add them to our internal list
		for (pp_int32 i = 0; i < controls.size(); i++)
		{
			if (controls.get(i)->receiveTimerEvent())
			{
				parentScreen->removeTimerEventControl(controls.get(i));
				timerEventControls->add(controls.get(i));
			}
		}
	}
}

void PPContainer::setSize(const PPSize& size)
{
	this->size = size;
	backgroundButton->setSize(size);
}

void PPContainer::setLocation(const PPPoint& location)
{
	/*PPPoint offset(location.x - this->location.x, location.y - this->location.y);

	move(offset);*/

	this->location = location;
	backgroundButton->setLocation(location);
}

void PPContainer::setFocus(PPControl* control, bool repaint/* = true*/)
{
	// nothing to do
	if (control == this->focusedControl)
		return;

	// the current control is about to lose focus
	if (this->focusedControl)
	{
		PPEvent e(repaint ? eFocusLost : eFocusLostNoRepaint);
		this->focusedControl->dispatchEvent(&e);
	}

	this->focusedControl = control;

	if (this->focusedControl)
	{
		PPEvent e(repaint ? eFocusGained : eFocusGainedNoRepaint);
		this->focusedControl->dispatchEvent(&e);
	}


#if 0
	if (verbose)
	{
		/*if (focusedControl != control && focusedControl)
		{
			PPEvent e(eFocusLost);
			focusedControl->dispatchEvent(&e);
		}*/

		PPEvent eLost(eFocusLost);
		for (pp_int32 i = 0; i < controls.size(); i++)
		{
			PPControl* ctrl = controls.get(i);
			if (ctrl != control)
				ctrl->dispatchEvent(&eLost);
		}

		if (reGain && control)
		{
			PPEvent eGained(eFocusGained);
			control->dispatchEvent(&eGained);

			lastFocusedControl = control;
		}

	}
	focusedControl = control;
#endif
}

bool PPContainer::hasFocus(PPControl* control) const
{
	return focusedControl == control;
}

void PPContainer::move(const PPPoint& offset)
{
	PPPoint p = getLocation();
	p.x+=offset.x;
	p.y+=offset.y;

	location = p;
	backgroundButton->setLocation(p);

	PPSimpleVector<PPControl>& controls = getControls();

	for (pp_int32 i = 0; i < controls.size(); i++)
	{
		PPControl* control = controls.get(i);

		if (!control->isContainer())
		{
			p = control->getLocation();
			p.x+=offset.x;
			p.y+=offset.y;
			control->setLocation(p);
		}
		else static_cast<PPContainer*>(control)->move(offset);
	}

}

void PPContainer::adjustContainerSize()
{
	PPSimpleVector<PPControl>& controls = getControls();

	pp_int32 x1 = parentScreen->getWidth();
	pp_int32 y1 = parentScreen->getHeight();

	pp_int32 x2 = 0;
	pp_int32 y2 = 0;

	for (pp_int32 i = 0; i < controls.size(); i++)
	{
		PPControl* control = controls.get(i);

		PPPoint p = control->getLocation();
		if (p.x < x1)
			x1 = p.x;
		if (p.y < y1)
			y1 = p.y;

		p.x+=control->getSize().width;
		p.y+=control->getSize().height;

		if (p.x > x2)
			x2 = p.x;
		if (p.y > y2)
			y2 = p.y;
	}

	setLocation(PPPoint(x1, y1));
	setSize(PPSize(x2-x1, y2-y1));
}
