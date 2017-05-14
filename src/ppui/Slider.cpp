/*
 *  ppui/Slider.cpp
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

#include "Slider.h"
#include "GraphicsAbstract.h"
#include "Button.h"
#include "Event.h"
#include "Screen.h"

void PPSlider::initButtons()
{
	PPPoint buttonDownPos, buttonBarPos;
	PPSize buttonBarSize;

	if (horizontal)
	{
		this->size.height = SLIDERBUTTONHEIGHT;
		this->size.width = oneDimSize;
	
		buttonDownPos = PPPoint(location.x + oneDimSize - SLIDERBUTTONWIDTH, location.y);
	
		buttonBarPos = PPPoint(location.x + SLIDERBUTTONWIDTH, location.y);
	
		buttonBarSize = PPSize((oneDimSize - SLIDERBUTTONWIDTH*2), SLIDERBUTTONHEIGHT);  
	}
	else
	{
		this->size.width = SLIDERBUTTONWIDTH;
		this->size.height = oneDimSize;
		
		buttonDownPos = PPPoint(location.x, location.y + oneDimSize - SLIDERBUTTONHEIGHT);
		
		buttonBarPos = PPPoint(location.x, location.y + SLIDERBUTTONHEIGHT);
		
		buttonBarSize = PPSize(SLIDERBUTTONWIDTH, (oneDimSize - SLIDERBUTTONHEIGHT*2));  
	}

	backgroundButton = new PPButton(0, parentScreen, NULL, /*_Point(location.x + (horizontal?0:1), location.y + (horizontal?1:0))*/location, this->size, false);
	backgroundButton->setColor(backgroundColor);
	backgroundButton->setInvertShading(true);

	buttonUp = new PPButton(0, parentScreen, this, location, PPSize(SLIDERBUTTONWIDTH,SLIDERBUTTONHEIGHT), false);
	buttonUp->setText(!buttonSwap ? "-" : "+");

	buttonDown = new PPButton(0, parentScreen, this, buttonDownPos , PPSize(SLIDERBUTTONWIDTH,SLIDERBUTTONHEIGHT), false);
	buttonDown->setText(!buttonSwap ? "+" : "-");

	buttonBar = new PPButton(0, parentScreen, this, buttonBarPos, buttonBarSize, false, false);

	setBarSize(currentBarSize, false);
	setBarPosition(currentBarPosition, false);
}

PPSlider::PPSlider(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
				   const PPPoint& location, pp_int32 size, 
				   bool horizontal, bool buttonSwap) :
	PPControl(id, parentScreen, eventListener, location, PPSize(0,0)),
	oneDimSize(size),
	horizontal(horizontal),
	buttonSwap(buttonSwap),
	controlCaughtByLMouseButton(false), controlCaughtByRMouseButton(false)
{
	// default color
	backgroundColor.r = 64;
	backgroundColor.g = 64;
	backgroundColor.b = 64;

	currentBarSize = 65536;
	currentBarPosition = 0;

	initButtons();
	
	caughtControl = NULL;

	minValue = 0;
	maxValue = 255;
	currentValue = 0;
}

PPSlider::~PPSlider()
{
	delete backgroundButton;	
	delete buttonUp;
	delete buttonDown;
	delete buttonBar;
}

void PPSlider::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	g->setRect(PPRect(location.x, location.y, location.x + size.width, location.y + size.height));

	/*g->setColor(backgroundColor);

	g->fill();*/

	backgroundButton->paint(g);

	buttonDown->paint(g);

	buttonUp->paint(g);

	buttonBar->paint(g);

}

void PPSlider::setBarSize(pp_int32 size, bool repaint /* = true */)
{
	if (size < 0) size = 0;
	if (size > 65536) size = 65536;

	currentBarSize = size;
	
	pp_int32 newSize = (((horizontal?(this->size.width-SLIDERBUTTONWIDTH*2):(this->size.height-SLIDERBUTTONHEIGHT*2))) * size)>>16;

	if (horizontal)
	{
		buttonBar->setSize(PPSize(newSize, this->size.height));		
	}
	else
	{
		buttonBar->setSize(PPSize(this->size.width, newSize));
	}

	if (repaint)
		parentScreen->paintControl(this);

}

void PPSlider::setLocation(const PPPoint& location)
{
	PPControl::setLocation(location);
	
	delete backgroundButton;	
	delete buttonUp;
	delete buttonDown;
	delete buttonBar;

	initButtons();
}

void PPSlider::setBarPosition(pp_int32 pos, bool repaint /* = false */)
{
	if (pos < 0) pos = 0;
	if (pos > 65536) pos = 65536;

	currentBarPosition = pos;

	pp_int32 size = (horizontal?buttonBar->getSize().width:buttonBar->getSize().height);

	pp_int32 entireSize = (horizontal?(this->size.width-SLIDERBUTTONWIDTH*2):(this->size.height-SLIDERBUTTONHEIGHT*2));

	pp_int32 maxPos = entireSize-size;

	pp_int32 newPos = (maxPos*pos)>>16;	

	PPPoint p = location;

	if (horizontal)
		p.x+=newPos + SLIDERBUTTONWIDTH;
	else
		p.y+=newPos + SLIDERBUTTONHEIGHT;

	buttonBar->setLocation(p);

	if (repaint)
		parentScreen->paintControl(this);
}

pp_int32 PPSlider::dispatchEvent(PPEvent* event)
{
	//if (!visible)
	//	return 0;

	switch (event->getID())
	{
		/*case eMouseWheelMoved:
		{

			TMouseWheelEventParams* params = (TMouseWheelEventParams*)event->getDataPtr();
			
			if (params->delta > 0)
			{
				PPEvent e(eLMouseUp);
				handleEvent(reinterpret_cast<PPObject*>(buttonUp), &e);
			}
			else if (params->delta < 0)
			{
				PPEvent e(eLMouseUp);
				handleEvent(reinterpret_cast<PPObject*>(buttonDown), &e);
			}
			
			event->cancel();
			
			break;
		}*/

		case eLMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			if (buttonUp->hit(*p))
			{
				caughtControl = buttonUp;
				caughtControl->dispatchEvent(event);
				controlCaughtByLMouseButton = true;
			}
			else if (buttonDown->hit(*p))
			{
				caughtControl = buttonDown;
				caughtControl->dispatchEvent(event);				
				controlCaughtByLMouseButton = true;
			}
			else if (buttonBar->hit(*p))
			{
				caughtControl = buttonBar;
				// -------------------------
				buttonBar->setPressed(true);
				parentScreen->paintControl(buttonBar);
				// -------------------------
				caughtMouseLocation = *p;
				caughtControlLocation = buttonBar->getLocation();
				caughtControl->dispatchEvent(event);				
				controlCaughtByLMouseButton = true;
			}
			else if (backgroundButton->hit(*p))
			{
				handleEvent(reinterpret_cast<PPObject*>(backgroundButton), event);				
			}

			break;
		}

		case eLMouseUp:
			controlCaughtByLMouseButton = false;

			if (caughtControl == NULL)
				break;

			if (controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				break;
			}

			// -------------------------
			if (reinterpret_cast<PPButton*>(caughtControl) == buttonBar)
			{
				buttonBar->setPressed(false);
				parentScreen->paintControl(buttonBar);
			}
			// -------------------------

			caughtControl->dispatchEvent(event);
			caughtControl = NULL;
			break;

		case eLMouseRepeat:
		{
			if (caughtControl)
			{
				caughtControl->dispatchEvent(event);
				break;
			}
			else
			{
				PPPoint* p = (PPPoint*)event->getDataPtr();
				/*if (backgroundButton->hit(*p))
				{
					handleEvent(reinterpret_cast<PPObject*>(backgroundButton), event);				
				}
				PPPoint* p = (PPPoint*)event->getDataPtr();*/
				if (backgroundButton->hit(*p) && !buttonBar->hit(*p))
				{
					handleEvent(reinterpret_cast<PPObject*>(backgroundButton), event);				
				}
				else if (buttonBar->hit(*p))
				{
					// -------------------------
					buttonBar->setPressed(true);
					parentScreen->paintControl(buttonBar);
					// -------------------------

					caughtControl = buttonBar;
					caughtMouseLocation = *p;
					caughtControlLocation = buttonBar->getLocation();
					caughtControl->dispatchEvent(event);				
				}

			}
			break;
		}

		case eRMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			if (buttonUp->hit(*p))
			{
				caughtControl = buttonUp;
				caughtControl->dispatchEvent(event);
				controlCaughtByRMouseButton = true;
			}
			else if (buttonDown->hit(*p))
			{
				caughtControl = buttonDown;
				caughtControl->dispatchEvent(event);				
				controlCaughtByRMouseButton = true;
			}
			else if (buttonBar->hit(*p))
			{
				caughtControl = buttonBar;
				// -------------------------
				buttonBar->setPressed(true);
				parentScreen->paintControl(buttonBar);
				// -------------------------
				caughtMouseLocation = *p;
				caughtControlLocation = buttonBar->getLocation();
				caughtControl->dispatchEvent(event);				
				controlCaughtByRMouseButton = true;
			}
			else if (backgroundButton->hit(*p))
			{
				handleEvent(reinterpret_cast<PPObject*>(backgroundButton), event);				
			}

			break;
		}

		case eRMouseUp:
			controlCaughtByRMouseButton = false;

			if (caughtControl == NULL)
				break;

			if (controlCaughtByLMouseButton)
			{
				caughtControl->dispatchEvent(event);
				break;
			}

			// -------------------------
			if (reinterpret_cast<PPButton*>(caughtControl) == buttonBar)
			{
				buttonBar->setPressed(false);
				parentScreen->paintControl(buttonBar);
			}
			// -------------------------

			caughtControl->dispatchEvent(event);
			caughtControl = NULL;
			break;

		case eRMouseRepeat:
		{
			if (caughtControl)
			{
				caughtControl->dispatchEvent(event);
				break;
			}
			else
			{
				/*PPPoint* p = (PPPoint*)event->getDataPtr();
				if (backgroundButton->hit(*p))
				{
					handleEvent(reinterpret_cast<PPObject*>(backgroundButton), event);				
				}*/
				PPPoint* p = (PPPoint*)event->getDataPtr();
				if (backgroundButton->hit(*p) && !buttonBar->hit(*p))
				{
					handleEvent(reinterpret_cast<PPObject*>(backgroundButton), event);				
				}
				else if (buttonBar->hit(*p))
				{
					// -------------------------
					buttonBar->setPressed(true);
					parentScreen->paintControl(buttonBar);
					// -------------------------

					caughtControl = buttonBar;
					caughtMouseLocation = *p;
					caughtControlLocation = buttonBar->getLocation();
					caughtControl->dispatchEvent(event);				
				}

			}
			break;
		}

		//case eLMouseDrag:
		default:
			if (caughtControl == NULL)
				break;

			caughtControl->dispatchEvent(event);
			break;

	}

	return 0;
}

pp_int32 PPSlider::handleEvent(PPObject* sender, PPEvent* event)
{

	if (
		(event->getID() == eLMouseUp &&
		sender == reinterpret_cast<PPObject*>(buttonUp)) ||
		(event->getID() == eLMouseRepeat &&
		sender == reinterpret_cast<PPObject*>(buttonUp)) ||
		(event->getID() == eRMouseUp &&
		sender == reinterpret_cast<PPObject*>(buttonUp)) ||
		(event->getID() == eRMouseRepeat &&
		sender == reinterpret_cast<PPObject*>(buttonUp))
		)
	{
		// Call parent event listener
		//PPEvent e(eBarScrollUp);
		//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
		
		if (currentValue > minValue)
			currentValue--;
	
		float f = (float)(currentValue - minValue)/(maxValue - minValue);

		setBarPosition((pp_int32)(f*65536.0f));
		
		parentScreen->paintControl(this);

		PPEvent e(eValueChanged);

		return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}
	if (
		(event->getID() == eLMouseUp &&
		sender == reinterpret_cast<PPObject*>(buttonDown)) ||
		(event->getID() == eLMouseRepeat &&
		sender == reinterpret_cast<PPObject*>(buttonDown)) ||
		(event->getID() == eRMouseUp &&
		sender == reinterpret_cast<PPObject*>(buttonDown)) ||
		(event->getID() == eRMouseRepeat &&
		sender == reinterpret_cast<PPObject*>(buttonDown))
		)
	{
		// Call parent event listener
		//PPEvent e(eBarScrollDown);
		//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);

		if (currentValue < maxValue)
			currentValue++;

		float f = (float)(currentValue - minValue)/(maxValue - minValue);

		setBarPosition((pp_int32)(f*65536.0f));

		parentScreen->paintControl(this);
		
		PPEvent e(eValueChanged);

		return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}
	else if (/*event->getID() == eLMouseDown &&*/
			 sender == reinterpret_cast<PPObject*>(backgroundButton))
	{
		
		if (horizontal)
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();
			
			if (p->x < buttonBar->getLocation().x)
			{
				pp_int32 bsize = buttonBar->getSize().width;
				pp_int32 entireSize = size.width - SLIDERBUTTONWIDTH*2;

				float f = (float)entireSize/(float)(entireSize - bsize);

				currentBarPosition-=(pp_int32)(currentBarSize*f);
				if (currentBarPosition < 0) currentBarPosition = 0;
				setBarPosition(currentBarPosition);
				
				// Call parent event listener
				//PPEvent e(eBarPosChanged);
				//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			}
			else if (p->x > buttonBar->getLocation().x + buttonBar->getSize().width)
			{
				pp_int32 bsize = buttonBar->getSize().width;
				pp_int32 entireSize = size.width - SLIDERBUTTONWIDTH*2;

				float f = (float)entireSize/(float)(entireSize - bsize);

				currentBarPosition+=(pp_int32)(currentBarSize*f);
				if (currentBarPosition > 65536) currentBarPosition = 65536;
				setBarPosition(currentBarPosition);
				
				// Call parent event listener
				//PPEvent e(eBarPosChanged);
				//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			}
		}
		else 
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();
			
			if (p->y < buttonBar->getLocation().y)
			{
				pp_int32 bsize = buttonBar->getSize().height;
				pp_int32 entireSize = size.height - SLIDERBUTTONHEIGHT*2;

				float f = (float)entireSize/(float)(entireSize - bsize);

				currentBarPosition-=(pp_int32)(currentBarSize*f);
				if (currentBarPosition < 0) currentBarPosition = 0;
				setBarPosition(currentBarPosition);
				
				// Call parent event listener
				//PPEvent e(eBarPosChanged);
				//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			}
			else if (p->y > buttonBar->getLocation().y + buttonBar->getSize().height)
			{
				pp_int32 bsize = buttonBar->getSize().height;
				pp_int32 entireSize = size.height - SLIDERBUTTONHEIGHT*2;

				float f = (float)entireSize/(float)(entireSize - bsize);

				currentBarPosition+=(pp_int32)(currentBarSize*f);
				if (currentBarPosition > 65536) currentBarPosition = 65536;
				setBarPosition(currentBarPosition);
				
				// Call parent event listener
				//PPEvent e(eBarPosChanged);
				//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			}
		}
				

	}
	else if ((event->getID() == eLMouseDrag && sender == reinterpret_cast<PPObject*>(buttonBar)) ||
			 (event->getID() == eRMouseDrag && sender == reinterpret_cast<PPObject*>(buttonBar)))
	{
		PPPoint* p = (PPPoint*)event->getDataPtr();
	
		pp_int32 pos = 0,dx,nx;

		if (horizontal)
		{

			dx = (p->x - caughtMouseLocation.x);
			
			/*if (dx < 0)
			{
				__asm
				{
					int 3
				}
			}*/
			
			nx = caughtControlLocation.x + dx;
			//if (nx < location.x + SLIDERBUTTONSIZE)
			//	nx = location.x + SLIDERBUTTONSIZE;
			//if (nx > location.x + size.width - SLIDERBUTTONSIZE*2 - buttonBar->getSize().width)
			//	nx = location.x + size.width - SLIDERBUTTONSIZE*2 - buttonBar->getSize().width;
			
			nx-=(location.x + SLIDERBUTTONWIDTH);
			
			pp_int32 d = (size.width - buttonBar->getSize().width - SLIDERBUTTONWIDTH*2);
			
			if (d != 0)
				pos = nx*65536 / d;			
		}
		else
		{
			dx = (p->y - caughtMouseLocation.y);
			
			/*if (dx < 0)
				__asm
			{
				{
					int 3
				}
			}*/
			
			nx = caughtControlLocation.y + dx;
			//if (nx < location.x + SLIDERBUTTONSIZE)
			//	nx = location.x + SLIDERBUTTONSIZE;
			//if (nx > location.x + size.width - SLIDERBUTTONSIZE*2 - buttonBar->getSize().width)
			//	nx = location.x + size.width - SLIDERBUTTONSIZE*2 - buttonBar->getSize().width;
			
			nx-=(location.y + SLIDERBUTTONHEIGHT);
			
			pp_int32 d = (size.height - buttonBar->getSize().height - SLIDERBUTTONHEIGHT*2);
			
			if (d != 0)
				pos = nx*65536 / d;
		}
		
		setBarPosition(pos);
		// Call parent event listener
		//PPEvent e(eBarPosChanged);
		//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}
	else
	{
		return 0;
	}

	float f = getBarPosition() / 65536.0f;

	currentValue = (pp_int32)((f * (maxValue - minValue)) + minValue);

	parentScreen->paintControl(this);

	PPEvent e(eValueChanged);

	return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);

}

void PPSlider::setSize(pp_uint32 size)
{
	PPPoint buttonDownPos, buttonBarPos;
	PPSize buttonBarSize;

	if (horizontal)
	{
		this->size.height = SLIDERBUTTONHEIGHT;
		this->size.width = size;
	
		buttonDownPos = PPPoint(location.x + size - SLIDERBUTTONWIDTH, location.y);
	
		buttonBarPos = PPPoint(location.x + SLIDERBUTTONWIDTH, location.y);
	
		buttonBarSize = PPSize((size - SLIDERBUTTONWIDTH*2), SLIDERBUTTONHEIGHT);  
	}
	else
	{
		this->size.width = SLIDERBUTTONWIDTH;
		this->size.height = size;
		
		buttonDownPos = PPPoint(location.x, location.y + size - SLIDERBUTTONHEIGHT);
		
		buttonBarPos = PPPoint(location.x, location.y + SLIDERBUTTONHEIGHT);
		
		buttonBarSize = PPSize(SLIDERBUTTONWIDTH, (size - SLIDERBUTTONHEIGHT*2));  
	}

	backgroundButton->setLocation(location);
	backgroundButton->setSize(this->size);

	buttonUp->setLocation(location);

	buttonDown->setLocation(buttonDownPos);

}
