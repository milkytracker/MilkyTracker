/*
 *  ppui/ContextMenu.h
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
//	Contextmenu control class
//
/////////////////////////////////////////////////////////////////
#ifndef CONTEXTMENU__H
#define CONTEXTMENU__H

#include "BasicTypes.h"
#include "Control.h"
#include "Menu.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPContextMenu;

class PPContextMenu : public PPControl
{
private:
	const PPColor* color;
	const PPColor* selectionColor;
	bool autoAddHide;
	bool notifyParentOnHide;

	PPFont* font;
	
	PPMenu* menu;

	pp_int32 menuSelection;
	bool hasDragged;
	bool pressInvoke;
	bool hadCursor;

	void showSubMenu(PPContextMenu* subMenu);

public:
	PPContextMenu(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
				  const PPPoint& location, 
				  const PPColor& selColor, 
				  bool doAutoAddHide = false, 
				  PPFont* font = NULL);
	
	virtual ~PPContextMenu();

	void setColor(const PPColor& color) { this->color = &color; }

	void setFont(PPFont* font) 
	{ 
		this->font = font; 
		menu->setFont(font);
		PPRect r = menu->getBoundingRect();		
		setSize(PPSize(r.width(), r.height()));
	}
	
	PPFont* getFont() const { return font; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 dispatchEvent(PPEvent* event);
	
	virtual void setSize(const PPSize& size);
	virtual void setLocation(const PPPoint& location);
	
	// menu controlling
	void addEntry(const PPString& s, pp_int32 theId, PPContextMenu* contextMenu = NULL);

	bool setState(pp_int32 theId, pp_uint32 newState) { return menu->setState(theId, newState); }
	
	bool hitMenu(const PPPoint& p) const;

	void processMenuHit(const PPPoint& p);
	
	void setSubMenu(bool bSubMenu) { menu->setSubMenu(bSubMenu); }
	bool isSubMenu() const { return menu->isSubMenu(); }

	void setParentMenu(PPContextMenu* parent) { menu->setParentMenu(parent); }
	PPContextMenu* getParentMenu() const { return menu->getParentMenu(); }
	
	void setNotifyParentOnHide(bool notifyParentOnHide) { this->notifyParentOnHide = notifyParentOnHide; }
	bool getNotifyParentOnHide() const { return notifyParentOnHide; }
};

#endif
