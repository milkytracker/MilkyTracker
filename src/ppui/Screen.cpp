/*
 *  ppui/Screen.cpp
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

#include "DisplayDeviceBase.h"
#include "GraphicsAbstract.h"
#include "Screen.h"
#include "Event.h"
#include "ContextMenu.h"
#include "TransparentContainer.h"

void PPScreen::paintDragHighlite(PPGraphicsAbstract* g)
{
	g->setRect(0,0,getWidth(), getHeight());
	g->setColor(46517>>8, 54740>>8, 65535>>8);
	g->drawHLine(0, getWidth(), 0);
	g->drawHLine(0, getWidth(), 1);
	g->drawHLine(0, getWidth(), 2);
	
	g->drawHLine(0, getWidth(), getHeight() - 1);
	g->drawHLine(0, getWidth(), getHeight() - 2);
	g->drawHLine(0, getWidth(), getHeight() - 3);
	
	g->drawVLine(0, getHeight(), 0);
	g->drawVLine(0, getHeight(), 1);
	g->drawVLine(0, getHeight(), 2);
	
	g->drawVLine(0, getHeight(), getWidth() - 1);
	g->drawVLine(0, getHeight(), getWidth() - 2);
	g->drawVLine(0, getHeight(), getWidth() - 3);
}

PPScreen::PPScreen(PPDisplayDeviceBase* displayDevice, EventListenerInterface* eventListener /* = NULL */) :
	displayDevice(displayDevice),
	eventListener(eventListener),	
	focusedControl(NULL),
	beforeModalFocusedControl(NULL),
	modalControl(NULL),
	showDragHilite(false),
	rootContainer(NULL),
	lastMouseOverControl(NULL)
{
	contextMenuControls = new PPSimpleVector<PPControl>(16, false);
	timerEventControls = new PPSimpleVector<PPControl>(16, false);
	
	rootContainer = new PPTransparentContainer(-1, this, eventListener, 
											   PPPoint(0, 0), 
											   PPSize(displayDevice->getSize()));
}

PPScreen::~PPScreen()
{
	delete contextMenuControls;
	delete timerEventControls;
}

void PPScreen::adjustEventMouseCoordinates(PPEvent* event)
{
	if (!displayDevice->supportsScaling())
		return;

	const pp_int32 scaleFactor = displayDevice->getScaleFactor();

	if (scaleFactor == 1)
		return;

	PPPoint* p = (PPPoint*)event->getDataPtr();

	p->x /= scaleFactor;
	p->y /= scaleFactor;
}

void PPScreen::raiseEvent(PPEvent* event)
{
	if (event->isMouseEvent())
		adjustEventMouseCoordinates(event);

	// route events to event listener first
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), event);

	if (event->getID() == eInvalid)
		return;

	// route timer event
	if (event->getID() == eTimer)
	{
		for (pp_int32 i = 0; i < timerEventControls->size(); i++)
		{
			PPControl* control = timerEventControls->get(i);
			if (!control->isVisible() || !control->receiveTimerEvent())
				continue;
			
			control->dispatchEvent(event);
		}
		return;
	}

	if (modalControl && modalControl->isVisible())
	{
		// listener of the modal control also gets a chance to listen to these events
		if (modalControl->getEventListener() && 
			modalControl->getEventListener() != eventListener)
			modalControl->getEventListener()->handleEvent(reinterpret_cast<PPObject*>(this), event);
	
		// if the above listener removed the control we're out of here
		if (!modalControl)
			return;		
		
		modalControl->dispatchEvent(event);
		return;
	}

	// ------- handle context menu -----------------------------------
	if (contextMenuControls->size())
	{	
		pp_int32 i;
		bool handled = true;
		
		bool mouseMoveHandeled = false;
		if (event->getID() == eMouseMoved || event->getID() == eLMouseDrag)
		{
			for (i = 0; i < contextMenuControls->size(); i++)
			{
				PPControl* contextMenuControl = contextMenuControls->get(contextMenuControls->size()-1);
				PPPoint* p = (PPPoint*)event->getDataPtr();
						
				if (contextMenuControl->hit(*p) && contextMenuControl->isActive())
				{
					contextMenuControl->dispatchEvent(event);
					mouseMoveHandeled = true;
				}
			}
		}
		if (mouseMoveHandeled)
			return;
		
		for (i = 0; i < contextMenuControls->size(); i++)
		{
			PPControl* contextMenuControl = contextMenuControls->get(i);
			if (contextMenuControl && contextMenuControl->isVisible())
			{
				switch (event->getID())
				{
					case eLMouseDown:
					case eRMouseDown:
					{
						PPPoint* p = (PPPoint*)event->getDataPtr();
						
						if (contextMenuControl->hit(*p) && contextMenuControl->isActive())
						{
							contextMenuControl->dispatchEvent(event);
						}
						else 
						{
							bool inOtherMenu = false;
							for (pp_int32 j = 0; j < contextMenuControls->size(); j++)
								if (contextMenuControls->get(j) != contextMenuControl &&
									contextMenuControls->get(j)->hit(*p))
								{
									inOtherMenu = true;
									break;
								}
							
							if (!inOtherMenu)
							{
								if (event->getID() == eRMouseDown ||
									event->getID() == eLMouseDown)
									handled = true;
								
								if (static_cast<PPContextMenu*>(contextMenuControl)->getNotifyParentOnHide())
								{
									setContextMenuControl(NULL, false);
									paint(false);
	
									EventListenerInterface* listener = contextMenuControl->getEventListener();
									
									struct MetaData
									{
										pp_int32 id;
										PPPoint p;
									} metaData;
									
									metaData.id = event->getID();
									metaData.p = *p;
									
									PPEvent e(eRemovedContextMenu, &metaData, sizeof(metaData));
									listener->handleEvent(reinterpret_cast<PPObject*>(contextMenuControl), &e);
									
									update();
								}
								else
								{
									setContextMenuControl(NULL);
								}
								
							}
						}
						break;
					}
					default:
						contextMenuControl->dispatchEvent(event);
				}
			}
		}
		if (handled)
			return;
	}

	rootContainer->dispatchEvent(event);
}

void PPScreen::pauseUpdate(bool pause)
{
	displayDevice->allowForUpdates(!pause);
}

void PPScreen::enableDisplay(bool enable)
{
	displayDevice->enable(enable);
}

bool PPScreen::isDisplayEnabled()
{
	return displayDevice->isEnabled();
}

void PPScreen::clear()
{
	PPGraphicsAbstract* g = displayDevice->open();
	if (!g)
		return;

	g->setColor(0,0,0);
	g->setRect(0, 0, g->getWidth(), g->getHeight());
	g->fill();

	displayDevice->close();		
	displayDevice->update();
}

void PPScreen::paint(bool update/*= true*/, bool clean/*=false*/)
{
	if (displayDevice == NULL)
		return;

	PPGraphicsAbstract* g = displayDevice->open();
	if (!g)
		return;

	if (clean)
	{
		g->setColor(0,0,0);

		g->setRect(0, 0, g->getWidth(), g->getHeight());

		g->fill();
	}

	/*g->setColor(255, 255, 255);

	pp_int32 step = 0;
	for (pp_int32 y = 0; y < sizeof(characters)/sizeof(pp_uint16); y++)
	{
	
		for (pp_int32 x = 0; x < 4; x++)
		{
		
			if ((characters[y]>>((3-x)<<2))&1)
			{
				g->setPixel(x+32,y+32+step);
			}
		
		}

		step += ((y%5)==4)*2;

	}*/

	//g->setFont(PPFont::getFont(PPFont::FONT_SYSTEM));
	//g->setColor(255, 255, 255);
	//g->drawChar('A',0,0);

	pp_int32 i;
	
	/*for (i = 0; i < controls.size(); i++)
	{
		PPControl* control = controls.get(i);
		if (control->isVisible())
			controls.get(i)->paint(g);	
	}*/
	
	// modal control overlapping everything
	if (!(modalControl && modalControl->isVisible() && 
		  modalControl->getLocation().x == 0 &&
		  modalControl->getLocation().y == 0 &&
		  modalControl->getSize().width == getWidth() &&
		  modalControl->getSize().height == getHeight()))
	{
		rootContainer->paint(g);
		
		for (i = 0; i < contextMenuControls->size(); i++)
		{
			PPControl* control = contextMenuControls->get(i);
			control->paint(g);	
		}
	}
	
	if (modalControl)
		modalControl->paint(g);

	if (showDragHilite)
		paintDragHighlite(g);

	displayDevice->close();
	
	if (update)
		displayDevice->update();
}

void PPScreen::paintContextMenuControl(PPControl* control, bool update/* = true*/)
{
	if (displayDevice == NULL || !control->isVisible())
		return;

	PPGraphicsAbstract* g = displayDevice->open();
	if (!g)
		return;

	for (pp_int32 i = 0; i < contextMenuControls->size(); i++)
	{
		PPControl* ctrl = contextMenuControls->get(i);
		ctrl->paint(g);
	}
		
	if (modalControl)
	{
		PPControl* ctrl = modalControl;
		ctrl->paint(g);
	}

	displayDevice->close();

	if (update)
	{
		PPRect rect = control->getBoundingRect();
		
		rect.x1--;
		if (rect.x1 < 0) rect.x1 = 0;
		rect.y1--;
		if (rect.y1 < 0) rect.y1 = 0;

		rect.x2++;
		if (rect.x2 > getWidth()) rect.x2 = getWidth();
		rect.y2++;
		if (rect.y2 > getHeight()) rect.y2 = getHeight();
		
		displayDevice->update(rect);
	}

}

void PPScreen::paintControl(PPControl* control, bool update/*= true*/)
{	
	if (displayDevice == NULL || !control->isVisible())
		return;

	PPGraphicsAbstract* g = displayDevice->open();
	if (!g)
		return;

	// modal control overlapping everything
	if (modalControl && modalControl->isVisible() &&
		modalControl->getLocation().x == 0 &&
		modalControl->getLocation().y == 0 &&
		modalControl->getSize().width == getWidth() &&
		modalControl->getSize().height == getHeight())
	{
		// see whether the control we shall paint is a child of the modal control
		// if it's not, we don't need to paint it, because the modal control overlaps us 
		PPControl* parent = control->getOwnerControl();		
		
		bool isModalControlChild = (parent == NULL);

		while (parent != NULL)
		{
			if (parent == modalControl)
			{
				isModalControlChild = true;
				break;
			}
			
			control = parent;
			parent = control->getOwnerControl();
		}
		
		if (!isModalControlChild)
			return;
	}
	
	control->paint(g);	
	
	bool paintContextMenus = false; 
	for (pp_int32 i = 0; i < contextMenuControls->size(); i++)
	{
		PPControl* ctrl = contextMenuControls->get(i);

		if (ctrl->getBoundingRect().intersect(control->getBoundingRect()))
		{
			paintContextMenus = true;
			break;
		}
	}
	
	if (paintContextMenus)
	{
		for (pp_int32 i = 0; i < contextMenuControls->size(); i++)
		{
			PPControl* ctrl = contextMenuControls->get(i);
			ctrl->paint(g);
		}
	}
	
	if (modalControl)
	{
		// if the modal control hits the control to draw we also need to refresh the modal control
		if (modalControl->getBoundingRect().intersect(control->getBoundingRect()))
			modalControl->paint(g);
	}

	displayDevice->close();

	if (update)
	{
		updateControl(control);
	}

}

void PPScreen::paintSplash(const pp_uint8* rawData, pp_uint32 width, pp_uint32 height, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity/* = 256*/)
{
	PPGraphicsAbstract* g = displayDevice->open();
	if (!g)
		return;

	PPPoint p;
	p.x = g->getWidth() / 2 - width / 2;
	p.y = g->getHeight() / 2 - height / 2;
	
	PPSize s;
	s.width = width; s.height = height;
	g->blit(rawData, p, s, pitch, bpp, intensity);

	displayDevice->close();
	
	displayDevice->update();	
}

void PPScreen::update()
{
	if (displayDevice) 
	{
		if (showDragHilite)
		{
			PPGraphicsAbstract* g = displayDevice->open();
			paintDragHighlite(g);		
			displayDevice->close();
		}
		displayDevice->update(); 
	}
}

void PPScreen::updateControl(PPControl* control)
{
	PPRect rect = control->getBoundingRect();
	
	rect.x1--;
	if (rect.x1 < 0) rect.x1 = 0;
	rect.y1--;
	if (rect.y1 < 0) rect.y1 = 0;

	rect.x2++;
	if (rect.x2 > getWidth()) rect.x2 = getWidth();
	rect.y2++;
	if (rect.y2 > getHeight()) rect.y2 = getHeight();
	
	displayDevice->update(rect);
}

void PPScreen::setFocus(PPControl* control, bool repaint/* = true*/)
{
	PPSimpleVector<PPControl> chain(0, false);

	PPControl* ctrl = NULL;
	if (control == NULL)
	{
		rootContainer->setFocus(NULL);
		return;
	}
	else
	{
		ctrl = control;
		chain.add(ctrl);
	}
	
	if (ctrl == NULL)
		return;
	
	while (ctrl->getOwnerControl())
	{
		PPControl* parent = ctrl->getOwnerControl();
		ctrl = parent;
		if (ctrl->isContainer())
			chain.add(ctrl);
	}
	
	for (pp_int32 i = chain.size()-1; i > 0; i--)
	{
		if (chain.get(i)->isContainer())
		{
			static_cast<PPContainer*>(chain.get(i))->setFocus(chain.get(i-1), repaint);
		}
	}
}

PPControl* PPScreen::getFocusedControl() const
{
	// first we need to find the control which is at the end of the focus hierarchy
	PPControl* parent = rootContainer;
	while (parent->isContainer() && static_cast<PPContainer*>(parent)->getFocusedControl())
	{
		parent = static_cast<PPContainer*>(parent)->getFocusedControl();
	}
	
	// if this is still a container this is not what we want => NULL
	return parent->isContainer() ? NULL : parent;
}

bool PPScreen::hasFocus(PPControl* control) const
{ 
	// if the client is asking for container focus we first need to find the control 
	// which is at the end of the focus hierarchy (see above)
	PPControl* parent = control;
	while (parent->isContainer() && static_cast<PPContainer*>(parent)->getFocusedControl())
	{
		parent = static_cast<PPContainer*>(parent)->getFocusedControl();
	}

	// now if those controls match, we're in focus
	return getFocusedControl() == parent;
}

void PPScreen::addControl(PPControl* control) 
{ 
	rootContainer->addControl(control);
}

bool PPScreen::removeControl(PPControl* control)
{
	return rootContainer->removeControl(control);
}

void PPScreen::addTimerEventControl(PPControl* control)
{
	if (control->receiveTimerEvent() && !control->isContainer()) 
		timerEventControls->add(control); 
}

bool PPScreen::removeTimerEventControl(PPControl* control)
{
	bool res = false;
	for (pp_int32 i = 0; i < timerEventControls->size(); i++)
	{
		if (timerEventControls->get(i) == control)
		{
			timerEventControls->remove(i);
			res = true;
		}
	}
	
	return res;
}


PPControl* PPScreen::getControlByID(pp_int32 id) const
{
	return rootContainer->getControlByID(id);
}

void PPScreen::releaseCaughtControls()
{
	// uncatch controls within containers
	PPControl* parent = rootContainer;
	while (parent->isContainer() && 
		   static_cast<PPContainer*>(parent)->caughtControl &&
		   static_cast<PPContainer*>(parent)->currentlyPressedMouseButtons != 0)
	{
		PPControl* caughtControl = static_cast<PPContainer*>(parent)->caughtControl;
		static_cast<PPContainer*>(parent)->caughtControl = NULL;
		static_cast<PPContainer*>(parent)->currentlyPressedMouseButtons = 0;
		parent = caughtControl;
	}
}

void PPScreen::setModalControl(PPControl* control, bool repaint/* = true*/)
{ 
	// Hide open menus first
	setContextMenuControl(NULL, false);

	if (modalControl)
	{
		PPEvent event(eFocusLostNoRepaint);
		modalControl->dispatchEvent(&event);
		modalControl->show(false);
	}

	if (control)
	{
		// uncatch controls within containers
		releaseCaughtControls();		
		PPEvent event(eFocusGainedNoRepaint);
		control->dispatchEvent(&event);
		control->show(true);
	}
	else if (focusedControl)
	{
		PPEvent event(eFocusGainedNoRepaint);
		focusedControl->dispatchEvent(&event);
	}

	modalControl = control; 
	
	if (repaint)
		paint(); 
}


void PPScreen::setContextMenuControl(PPControl* control, bool repaint/* = true*/) 
{ 
	if (control == NULL && !contextMenuControls->size())
		return;

	PPEvent event(eFocusLost);
	PPEvent event2(eFocusGained);
	
	for (pp_int32 i = 0; i < contextMenuControls->size(); i++)
		contextMenuControls->get(i)->dispatchEvent(&event);						

	delete contextMenuControls;
	contextMenuControls = new PPSimpleVector<PPControl>(16, false);

	if (control)
	{
		// uncatch controls within containers
		releaseCaughtControls();		
		control->dispatchEvent(&event2);
		contextMenuControls->add(control); 		
	}
	
	if (repaint)
		paint(); 
}

void PPScreen::addContextMenuControl(PPControl* control, bool repaint/* = true*/) 
{ 
	PPEvent event(eFocusLost);
	PPEvent event2(eFocusGained);

	if (control)
	{
		control->dispatchEvent(&event2);
		contextMenuControls->add(control); 		
	}
	
	if (repaint)
		paint(); 
}

bool PPScreen::removeContextMenuControl(PPControl* control, bool repaint/* = true*/)
{
	if (!contextMenuControls->size())
		return false;

	bool res = false;
	for (pp_int32 i = 0; i < contextMenuControls->size(); i++)
		if (contextMenuControls->get(i) == control)
		{
			contextMenuControls->remove(i);
			res = true;
		}

	if (res && repaint)
		paint(); 

	return res;
}

bool PPScreen::removeLastContextMenuControl(bool repaint/* = true*/)
{
	if (contextMenuControls->size())
	{
		contextMenuControls->remove(contextMenuControls->size()-1);

		if (repaint)
			paint(); 
			
		return true;
	}
	
	return false;
}

bool PPScreen::hasContextMenu(PPControl* control) const
{
	if (!contextMenuControls->size())
		return false;
		
	for (pp_int32 i = 0; i < contextMenuControls->size(); i++)
		if (contextMenuControls->get(i) == control)
			return true;
			
	return false;
}

void PPScreen::setShowDragHilite(bool b)
{
	showDragHilite = b; 
	
	if (!b)
	{
		PPGraphicsAbstract* g = displayDevice->open();
		paintDragHighlite(g);
		displayDevice->close();
	}
	
	paint(); 
}

pp_int32 PPScreen::getWidth() const
{
	if (displayDevice == NULL)
		return -1;

	return displayDevice->getSize().width;
}

pp_int32 PPScreen::getHeight() const
{
	if (displayDevice == NULL)
		return -1;

	return displayDevice->getSize().height;
}

pp_int32 PPScreen::getDefaultWidth()
{
	return PPDisplayDeviceBase::getDefaultWidth();
}

pp_int32 PPScreen::getDefaultHeight()
{
	return PPDisplayDeviceBase::getDefaultHeight();
}

void PPScreen::setTitle(const PPSystemString& title)
{
	if (displayDevice)
		displayDevice->setTitle(title);
}
	
void PPScreen::setSize(const PPSize& size)
{
	if (displayDevice)
		displayDevice->setSize(size);
		
	rootContainer->setSize(size);
}

bool PPScreen::supportsScaling() const
{
	if (displayDevice)
		return displayDevice->supportsScaling();
		
	return false;
}

pp_int32 PPScreen::getScaleFactor() const
{
	if (displayDevice)
		return displayDevice->getScaleFactor();

	return 1;
}


bool PPScreen::goFullScreen(bool b)
{
	if (displayDevice)
		return displayDevice->goFullScreen(b);
		
	return false;
}

bool PPScreen::isFullScreen() const
{
	if (displayDevice)
		return displayDevice->isFullScreen();
		
	return false;
}

PPSize PPScreen::getDisplayResolution() const
{
	PPSize size(-1, -1);
	
	if (displayDevice)
		size = displayDevice->getDisplayResolution();
		
	return size;
}

void PPScreen::signalWaitState(bool b, const PPColor& color)
{
	if (displayDevice)
		displayDevice->signalWaitState(b, color);
}

void PPScreen::setMouseCursor(MouseCursorTypes type)
{
	if (displayDevice)
		displayDevice->setMouseCursor(type);
}

MouseCursorTypes PPScreen::getCurrentActiveMouseCursor() const
{
	if (displayDevice)
		return (MouseCursorTypes)displayDevice->getCurrentActiveMouseCursor();

	return MouseCursorTypeStandard;
}

void PPScreen::shutDown()
{
	if (displayDevice)
		displayDevice->shutDown();
}

