/*
 *  ppui/Screen.cpp
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
	// no focused control yet
	focusedControl(NULL),
	beforeModalFocusedControl(NULL),
	modalControl(NULL),
	showDragHilite(false),
	rootContainer(NULL),
	lastMouseOverControl(NULL)
{
	// Set our display device
	this->displayDevice = displayDevice;

	// set our event listener
	this->eventListener = eventListener;
	
	contextMenuControls = new PPSimpleVector<PPControl>(16, false);
	timerEventControls = new PPSimpleVector<PPControl>(16, false);
	
	rootContainer = new PPTransparentContainer(-1, this, eventListener, PPPoint(0, 0), PPSize(displayDevice->getWidth(), displayDevice->getHeight()));
}

PPScreen::~PPScreen()
{
	delete contextMenuControls;
	delete timerEventControls;
}

void PPScreen::raiseEvent(PPEvent* event)
{
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
			
			control->callEventListener(event);
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
		
		modalControl->callEventListener(event);
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
					contextMenuControl->callEventListener(event);
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
							contextMenuControl->callEventListener(event);
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
						contextMenuControl->callEventListener(event);
				}
			}
		}
		if (handled)
			return;
	}

	rootContainer->callEventListener(event);

#if 0
	pp_int32 i;
	bool handled = false;

	/*if (contextMenuControls->size() && event->getID() == eKeyDown)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
		if (keyCode == VK_ESCAPE)
		{
			setContextMenuControl(NULL);
			return;
		}
	}*/

	// route events to event listener first
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), event);

	if (event->getID() == eInvalid)
		goto exit;

	// ------- handle modal control -----------------------------------
	if (modalControl && modalControl->isVisible())
	{
		if (modalControl->getEventListener() && modalControl->getEventListener() != eventListener)
			modalControl->getEventListener()->handleEvent(reinterpret_cast<PPObject*>(this), event);
	
		if (!modalControl)
			goto exit;
	
		switch (event->getID())
		{
			case eLMouseDown:
			case eLMouseDoubleClick:
			case eRMouseDown:
			case eRMouseDoubleClick:
			case eFocusGained:
			case eFocusGainedNoRepaint:
			{
				PPPoint* p = (PPPoint*)event->getDataPtr();
				
				if (modalControl->hit(*p) && modalControl->isActive())
				{
					modalControl->callEventListener(event);
				}
				break;
			}
			default:
				modalControl->callEventListener(event);
		}
		
		if (event->getID() != eTimer)
			goto exit;
	}

	// ------- handle context menu -----------------------------------
	if (contextMenuControls->size() && event->getID() != eTimer)
	{
		handled = true;
		
		bool mouseMoveHandeled = false;
		if (event->getID() == eMouseMoved || event->getID() == eLMouseDrag)
		{
			for (i = 0; i < contextMenuControls->size(); i++)
			{
				PPControl* contextMenuControl = contextMenuControls->get(contextMenuControls->size()-1);
				PPPoint* p = (PPPoint*)event->getDataPtr();
						
				if (contextMenuControl->hit(*p) && contextMenuControl->isActive())
				{
					contextMenuControl->callEventListener(event);
					mouseMoveHandeled = true;
				}
			}
		}
		if (mouseMoveHandeled)
			goto exit;
		
		for (i = 0; i < contextMenuControls->size(); i++)
		{
			//PPControl* contextMenuControl = contextMenuControls->get(contextMenuControls->size()-1);
			
			PPControl* contextMenuControl = contextMenuControls->get(i);
			if (contextMenuControl && contextMenuControl->isVisible())
			{
				switch (event->getID())
				{
					case eLMouseDown:
						//case eLMouseDoubleClick:
					case eRMouseDown:
						//case eRMouseDoubleClick:
					{
						PPPoint* p = (PPPoint*)event->getDataPtr();
						
						if (contextMenuControl->hit(*p) && contextMenuControl->isActive())
						{
							contextMenuControl->callEventListener(event);
						}
						else //if (!contextMenuControl->hit(*p))
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
									handled = false;
								setContextMenuControl(NULL);
							}
						}
						break;
					}
					default:
						contextMenuControl->callEventListener(event);
				}
			}
		}
		if (handled)
			goto exit;
	}
	
	// route timer event
	if (event->getID() == eTimer)
	{
		for (i = 0; i < timerEventControls->size(); i++)
		{
			PPControl* control = timerEventControls->get(i);
			if (!control->isVisible() || !control->receiveTimerEvent())
				continue;
			
			control->callEventListener(event);
		}
	}
	// handle events which are routed to focused control
	else if (focusedControl && 
		event->getID() != eLMouseDown &&
		event->getID() != eLMouseDoubleClick &&
		event->getID() != eRMouseDown &&
		event->getID() != eRMouseDoubleClick &&
		event->getID() != eFocusGained &&
		event->getID() != eFocusGainedNoRepaint &&
		event->getID() != eMouseWheelMoved &&
		event->getID() != eMouseMoved)
	{

		switch (event->getID())
		{
			case eRMouseUp:
			{
				focusedControl->callEventListener(event);

				PPPoint* p = (PPPoint*)event->getDataPtr();
				for (i = 0; i < controls.size(); i++)
				{
					PPControl* control = controls.get(i);
					if (!control->isVisible() || control == focusedControl)
						continue;
					if (control->hit(*p))
					{
						control->callEventListener(event);
					}
				}

				// reset focus if control cannot gain focus
				if (!focusedControl->gainsFocus())
					focusedControl = lastFocusedControl;

				goto exit;
				break;
			}

			// Mouse button up event
			case eLMouseUp:
			{
				focusedControl->callEventListener(event);

				// reset focus if control cannot gain focus
				if (focusedControl && !focusedControl->gainsFocus())
					focusedControl = lastFocusedControl;
				
				goto exit;
			}; break; 

			/*case eLMouseDrag:
				focusedControl->callEventListener(event);
				break;

			case eLMouseRepeat:
				focused*/
			default:
				focusedControl->callEventListener(event);
				break;
		
		}
	
	
	}

forward:
	// handle other events
	handled = false;
	for (i = 0; i < controls.size(); i++)
	{
		
		PPControl* control = controls.get(i);

		if (!control->isVisible()  || !control->isEnabled())
			continue;

		switch (event->getID())
		{
			// Mouse button down event
			case eMouseWheelMoved:
			{
				TMouseWheelEventParams* params = (TMouseWheelEventParams*)event->getDataPtr();
				
				if (control->hit(params->pos) && control->isActive())
				{
					control->callEventListener(event);
					goto exit;
				}
				
				break;
			}
			
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
						control->callEventListener(&e);
					}
					else if (bLastHit && !bHit)
					{
						PPEvent e(eMouseLeft, p, sizeof(PPPoint));
						control->callEventListener(&e);
					}
				
					if (bHit)
					{
						if (control != lastMouseOverControl && lastMouseOverControl)
							lastMouseOverControl->callEventListener(event);

						control->callEventListener(event);
						lastMouseOverControl = control;
						goto exit;
					}
				}
				break;
			}

			case eLMouseUp:
			case eRMouseUp:
			{
				PPPoint* p = (PPPoint*)event->getDataPtr();
				
				if (control->hit(*p) && control->isActive())
				{
					control->callEventListener(event);
					goto exit;
				}
				break;
			}
			
			case eLMouseDown:
			case eLMouseDoubleClick:
			case eRMouseDown:
			case eRMouseDoubleClick:
			case eFocusGained:
			case eFocusGainedNoRepaint:
			{
				PPPoint* p = (PPPoint*)event->getDataPtr();
				
				if (control->hit(*p) && control->isActive())
				{
					if (control != focusedControl)
						lastFocusedControl = focusedControl;
					
					if (control->gainsFocus() && focusedControl != control && focusedControl)
					{
						PPEvent e(event->getID() == eFocusGainedNoRepaint ? eFocusLostNoRepaint : eFocusLost);
						focusedControl->callEventListener(&e);
					}
					
					if (focusedControl != control)
					{
						focusedControl = control;
						if (lastFocusedControl != focusedControl && focusedControl->gainsFocus())
						{
							PPEvent e(event->getID() == eFocusGainedNoRepaint ? eFocusGainedNoRepaint : eFocusGained, p, sizeof(PPPoint));
							focusedControl->callEventListener(&e);
						}
					}

					control->callEventListener(event);
					goto exit;
				}
			}; break;

		}
		
	}	
	if (event->getID() == eMouseMoved)
	{
		PPPoint* p = (PPPoint*)event->getDataPtr();		
		lastMousePoint = *p;
	}
	
exit:;
#endif
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
		return;

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

PPControl* PPScreen::getFocusedControl()
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

bool PPScreen::hasFocus(PPControl* control)
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


PPControl* PPScreen::getControlByID(pp_int32 id)
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
		modalControl->callEventListener(&event);
		modalControl->show(false);
	}

	if (control)
	{
		// uncatch controls within containers
		releaseCaughtControls();		
		PPEvent event(eFocusGainedNoRepaint);
		control->callEventListener(&event);
		control->show(true);
	}
	else if (focusedControl)
	{
		PPEvent event(eFocusGainedNoRepaint);
		focusedControl->callEventListener(&event);
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
		contextMenuControls->get(i)->callEventListener(&event);						

	delete contextMenuControls;
	contextMenuControls = new PPSimpleVector<PPControl>(16, false);

	if (control)
	{
		// uncatch controls within containers
		releaseCaughtControls();		
		control->callEventListener(&event2);
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
		control->callEventListener(&event2);
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

bool PPScreen::hasContextMenu(PPControl* control)
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

pp_int32 PPScreen::getWidth()
{
	if (displayDevice == NULL)
		return -1;

	return displayDevice->getWidth();
}

pp_int32 PPScreen::getHeight()
{
	if (displayDevice == NULL)
		return -1;

	return displayDevice->getHeight();
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

bool PPScreen::goFullScreen(bool b)
{
	if (displayDevice)
		return displayDevice->goFullScreen(b);
		
	return false;
}

bool PPScreen::isFullScreen()
{
	if (displayDevice)
		return displayDevice->isFullScreen();
		
	return false;
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

MouseCursorTypes PPScreen::getCurrentActiveMouseCursor()
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

