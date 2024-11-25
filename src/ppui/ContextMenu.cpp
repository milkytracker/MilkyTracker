/*
 *  ppui/ContextMenu.cpp
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

#include "ContextMenu.h"
#include "GraphicsAbstract.h"
#include "Event.h"
#include "Screen.h"
#include "Font.h"
#include "Menu.h"
#include "PPUIConfig.h"

PPContextMenu::PPContextMenu(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
						 const PPPoint& location, 
						 const PPColor& selColor, 
						 bool doAutoAddHide/* = false*/, 
						 PPFont* font/* = NULL*/) :
	PPControl(id, parentScreen, eventListener, location, PPSize(0,0)),
	color(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorMenuBackground)),
	selectionColor(&selColor),
	autoAddHide(doAutoAddHide),
	notifyParentOnHide(false),
	menuSelection(-1),
	hasDragged(false)
{
	// default color
	if (font == NULL)
		font = PPFont::getFont(PPFont::FONT_SYSTEM);

	menu = new PPMenu(font, *selectionColor, *color);
	menu->setTextBrightColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorMenuTextBright));
	menu->setTextDarkColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorMenuTextDark));
  menu->setBorderColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorScrollBarBackground));
}

PPContextMenu::~PPContextMenu()
{
	delete menu;
}

void PPContextMenu::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	
	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height);
	
	// update menu selection before drawing the menu
	pp_int32 menuSelection = this->menuSelection;
	
	PPContextMenu* subMenu = static_cast<PPContextMenu*>(parentScreen->getLastContextMenuControl());
	for (pp_int32 i = 0; i < menu->items.size(); i++)
	{
		if (menu->items.get(i)->subMenu == subMenu)
			menuSelection = i;
	}
	
	menu->paint(g, location.x, location.y, menuSelection);
}

void PPContextMenu::showSubMenu(PPContextMenu* subMenu)
{
	if (parentScreen->getLastContextMenuControl() == subMenu)
		return;

	PPPoint p;
	p.x = getLocation().x + getSize().width - 2;
	p.y = getLocation().y - 2 + menuSelection * menu->getEntryHeight();
	
	if (p.x + subMenu->getSize().width > parentScreen->getWidth() + (subMenu->getSize().width >> 1))
		p.x = getLocation().x - subMenu->getSize().width;
	
	subMenu->setLocation(p);	
	parentScreen->addContextMenuControl(subMenu);
}

pp_int32 PPContextMenu::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	//if (!visible)
	//	return 0;

	switch (event->getID())
	{
		case eLMouseDown:
		{
			hasDragged = false;
			pressInvoke = false;

			PPPoint* p = (PPPoint*)event->getDataPtr();
			
			processMenuHit(*p);

			if (menuSelection >= 0)
			{
				if (menu->items.get(menuSelection)->subMenu)
				{
					PPContextMenu* subMenu = menu->items.get(menuSelection)->subMenu;

					if (parentScreen->getLastContextMenuControl() == subMenu)
					{
						parentScreen->removeLastContextMenuControl();
						break;
					}
					
					if (parentScreen->getLastContextMenuControl() != this && 
						parentScreen->getLastContextMenuControl() != subMenu)
						parentScreen->removeLastContextMenuControl();					
					
					showSubMenu(subMenu);
				}
			}
			
			break;
		}

		case eRMouseUp:
		case eLMouseUp:
		{
			if (event->getID() == eRMouseUp && !hasDragged) 
				break;

			if (event->getID() && pressInvoke)
				break;

			PPPoint* p = (PPPoint*)event->getDataPtr();
			
			processMenuHit(*p);
			
			if (menuSelection >= 0)
			{
				if (!menu->items.get(menuSelection)->subMenu)
				{
					pp_int32 theId = menu->items.get(menuSelection)->identifier;
					
					if (theId == -1)
						break;
					
					PPEvent e(eCommand, &theId, sizeof(theId));
					eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
					
					parentScreen->setContextMenuControl(NULL);
					menuSelection = -1;
				}
			}
			break;
		}

		case eLMouseRepeat:
			break;
			
		case eMouseMoved:
		case eLMouseDrag:
		case eRMouseDrag:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();
			
			if (hit(*p) && !hadCursor)
			{
				hadCursor = true;
			}
			else if (!hit(*p) && hadCursor && isSubMenu() && getParentMenu() && getParentMenu()->hit(*p))
			{
				parentScreen->removeLastContextMenuControl();
				break;
			}
		
			if (event->getID() == eLMouseDrag ||
				event->getID() == eRMouseDrag)
				hasDragged = true;
		
			processMenuHit(*p);
			
			if (menuSelection >= 0 && menuSelection < menu->items.size())
			{
				PPContextMenu* subMenu = menu->items.get(menuSelection)->subMenu;
				if (subMenu && parentScreen->getLastContextMenuControl() != subMenu)
				{
					PPContextMenu* lastMenu = (PPContextMenu*)parentScreen->getLastContextMenuControl();
					if (lastMenu->getParentMenu())
					{
						parentScreen->removeLastContextMenuControl();					
					}
					
					showSubMenu(subMenu);
				}
				else if (!subMenu && parentScreen->getLastContextMenuControl() != this)
				{
					parentScreen->removeLastContextMenuControl();					
				}
			}
			else if (menuSelection == -1 && 
					 parentScreen->getLastContextMenuControl() != this &&
					 !parentScreen->getLastContextMenuControl()->hit(*p) &&
					 hit(*p))
			{
				parentScreen->removeLastContextMenuControl();					
			}
			
			break;
		}

		case eFocusGained:
		case eFocusGainedNoRepaint:
		{
			menuSelection = -1;
			pressInvoke = true;
			hasDragged = false;
			hadCursor = false;
			break;
		}
			
		case eFocusLost:
		case eFocusLostNoRepaint:
			menuSelection = -1;
			hasDragged = false;
			pressInvoke = false;
			hadCursor = false;
			break;
		default:
			break;
	}

	return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), event); 
}

void PPContextMenu::addEntry(const PPString& s, pp_int32 theId, PPContextMenu* contextMenu)
{
	if (menu)
	{
		if (autoAddHide && menu->items.size() > 0)
		{
			pp_int32 i = menu->items.size();
			if (menu->items.get(i-1)->identifier == MenuCommandIDHide)
			{
				menu->items.remove(i-1);
				pp_int32 i = menu->items.size();
				if (menu->items.get(i-1)->identifier == -1)
					menu->items.remove(i-1);
			}
		}

		menu->items.add(new PPMenu::Entry(s, theId, 0, contextMenu));
		
		// add a hide option automatically
		if (autoAddHide)
		{
			pp_uint32 maxLen = 0;
			pp_int32 i;
			for (i = 0; i < menu->items.size(); i++)
			{
				if (menu->items.get(i)->name.length() > maxLen)
					maxLen = menu->items.get(i)->name.length();
			}
			if (maxLen < 9) maxLen = 9;
			char* buffer = new char[maxLen+1];
			memset(buffer, 0, maxLen+1);
			memset(buffer, '\xc4', maxLen);

			menu->items.add(new PPMenu::Entry(buffer, -1, 0, NULL));
			delete[] buffer;
			menu->items.add(new PPMenu::Entry("Hide menu", MenuCommandIDHide, 0, NULL));
		}

		if (contextMenu)
			contextMenu->setParentMenu(this);
		
		PPRect r = menu->getBoundingRect();
		
		setSize(PPSize(r.width(), r.height()));
	}
}

bool PPContextMenu::hitMenu(const PPPoint& p) const
{
	pp_int32 py = location.x;
	pp_int32 px = location.y;

	PPRect r = menu->getBoundingRect();	

	if ((p.x >= px && p.x < px + r.x2 - 4) &&
		(p.y >= py && p.y < py + r.y2 - 4))
		return true;

	return false;
}

void PPContextMenu::processMenuHit(const PPPoint& p)
{
	PPPoint cp = p;

	pp_int32 px = location.x;
	pp_int32 py = location.y;

	cp.x -= px;
	cp.y -= py;
	cp.y -= 4;

	pp_int32 lastMenuSelection = menuSelection;

	if (cp.x < 0 || cp.x >= menu->getBoundingRect().width() || cp.y < 0)
	{
		menuSelection = -1;																												
		if (lastMenuSelection  != menuSelection)
			parentScreen->paintContextMenuControl(this);
		return;
	}

	menuSelection = cp.y / menu->getEntryHeight();

	if (menuSelection >= menu->items.size())
	{
		menuSelection = -1;
		if (lastMenuSelection  != menuSelection)
			parentScreen->paintContextMenuControl(this);
	}
	else
	{
		if (menuSelection >= 0 && menuSelection < menu->items.size())
		{
			if ((menu->items.get(menuSelection)->state & 1) || (menu->items.get(menuSelection)->identifier==-1))
				menuSelection = -1;
		}
		if (lastMenuSelection  != menuSelection)
			parentScreen->paintContextMenuControl(this);
	}

}

void PPContextMenu::setSize(const PPSize& size)
{
	this->size = size;

	if (location.x + size.width+2 > parentScreen->getWidth())
		location.x-=((location.x + size.width+2)-parentScreen->getWidth());

	if (location.y + size.height+2 > parentScreen->getHeight())
		location.y-=((location.y + size.height+2)-parentScreen->getHeight());
}

void PPContextMenu::setLocation(const PPPoint& location)
{
	PPPoint loc = location;

	if (loc.x + size.width+2 > parentScreen->getWidth())
		loc.x-=((loc.x + size.width+2)-parentScreen->getWidth());

	if (loc.y + size.height+2 > parentScreen->getHeight())
		loc.y-=((loc.y + size.height+2)-parentScreen->getHeight());

	if (loc.x < 0)
		loc.x = 0;

	if (loc.y < 0)
		loc.y = 0;

	this->location = loc;
}

