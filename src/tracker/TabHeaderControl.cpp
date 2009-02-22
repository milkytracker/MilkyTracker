/*
 *  tracker/TabHeaderControl.cpp
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

/*
 *  TabHeaderControl.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 09.12.07.
 *
 */

#include "TabHeaderControl.h"
#include "Event.h"
#include "Screen.h"
#include "Button.h"
#include "Font.h"
#include "PPUIConfig.h"
#include "GlobalColorConfig.h"

void TabHeaderControl::handleTabClick(const PPPoint& p)
{
	pp_int32 hitIndex = -1;
	for (pp_int32 i = 0; i < tabButtons.size(); i++)
	{
		if (tabButtons.get(i)->hit(p))
		{
			hitIndex = i;
		}
	}
	
	if (hitIndex != -1)
	{
		for (pp_int32 i = 0; i < tabButtons.size(); i++)
		{
			PPButton* button = tabButtons.get(i);
			button->setPressed(i == hitIndex);
		}
		
		parentScreen->paintControl(this);
		
		this->hitIndex = hitIndex + startIndex;
	}
}

TabHeaderControl::TabHeaderControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
								   const PPPoint& location, const PPSize& size) :
	PPControl(id, parentScreen, eventListener, location, size),
	color(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorContainer)),
	leftButton(NULL),
	rightButton(NULL),
	minSize(60),
	maxSize(200),
	caughtControl(NULL),
	startIndex(0)
{
	backgroundButton = new PPButton(0, parentScreen, NULL, location, size, false, false);
	backgroundButton->setColor(*color);	
}

TabHeaderControl::~TabHeaderControl()
{
	delete backgroundButton;
	delete leftButton;
	delete rightButton;
}

void TabHeaderControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	
	backgroundButton->paint(g);
	
	if (leftButton != NULL)
	{
		leftButton->enable(startIndex > 0);
		leftButton->paint(g);
	}
	
	if (rightButton != NULL)
	{
		rightButton->enable(startIndex + tabButtons.size() < tabHeaders.size());
		rightButton->paint(g);
	}

	paintControls(g);
}

pp_int32 TabHeaderControl::dispatchEvent(PPEvent* event)
{ 
	switch (event->getID())
	{
		case eLMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			if (leftButton && leftButton->isEnabled() && leftButton->isVisible() && leftButton->hit(*p))
			{
				caughtControl = leftButton;
				caughtControl->dispatchEvent(event);
			}
			else if (rightButton && rightButton->isEnabled() && rightButton->isVisible() && rightButton->hit(*p))
			{
				caughtControl = rightButton;
				caughtControl->dispatchEvent(event);				
			}
			else
			{
				oldHitIndex = hitIndex;
				handleTabClick(*p);
			}
			break;
		}
		
		case eLMouseUp:
			if (caughtControl == NULL)
			{
				if (hitIndex != oldHitIndex)
				{
					PPEvent e(eSelection, &hitIndex, sizeof(hitIndex));
					eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);				
				}
				break;
			}

			caughtControl->dispatchEvent(event);
			caughtControl = NULL;			
			break;		
			
		default:
			if (caughtControl != NULL)
				caughtControl->dispatchEvent(event);
	}
	
	return 0;
}

pp_int32 TabHeaderControl::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case 0:
			{
				shiftTabs(-1);
				break;
			}
			
			case 1:
			{
				shiftTabs();
				break;
			}
		}
	}
	return 0;
}

void TabHeaderControl::setSize(const PPSize& size)
{
	this->size = size;
	backgroundButton->setSize(size);
}

void TabHeaderControl::setLocation(const PPPoint& location)
{
	this->location = location;
	backgroundButton->setLocation(location);	
}

void TabHeaderControl::addTab(const TabHeader& tabHeader)
{
	tabHeaders.add(new TabHeader(tabHeader));
	setNumTabs(tabHeaders.size());
	setSelectedTab(tabHeaders.size()-1);
	adjustLabels();
}

const TabHeaderControl::TabHeader* TabHeaderControl::getTab(pp_int32 index) const
{
	return (index >= 0 && index < tabHeaders.size()) ? tabHeaders.get(index) : NULL;
}

bool TabHeaderControl::removeTab(pp_int32 index)
{
	if (index < 0 || index >= tabHeaders.size())
		return false;
		
	return tabHeaders.remove(index);
}

void TabHeaderControl::setNumTabs(pp_uint32 numTabs)
{
	pp_int32 width = (size.width - 4) / numTabs;
	pp_int32 height = size.height - 4;
	
	if (minSize && width < minSize)
		width = minSize;
	if (maxSize && width > maxSize)
		width = maxSize;
	
	tabButtons.clear();
	
	if ((signed)numTabs*width > size.width - 4)
	{
		numTabs = (size.width - (4+12+12)) / minSize;
		width = (size.width - (4+12+12)) / numTabs;
	
		delete leftButton;
		leftButton = new PPButton(0, parentScreen, this, 
								  PPPoint(location.x + size.width - (4+12+12) + 2, location.y + 2), PPSize(12, height), false);
		leftButton->setText("<");

		delete rightButton;
		rightButton = new PPButton(1, parentScreen, this, 
								   PPPoint(location.x + size.width - (4+13) + 3, location.y + 2), PPSize(12, height), false);
		rightButton->setText(">");
	}
	else
	{
		delete rightButton;
		rightButton = NULL;
		delete leftButton;
		leftButton = NULL;
	}
	
	PPPoint location = this->location;
	location.x += 2;
	location. y+= 2;
	for (pp_int32 i = 0; i < (signed)numTabs; i++)
	{
		PPButton* button = new PPButton(i+0x1000, parentScreen, this, location, PPSize(width, height), false, true, false);
		button->setText("Blabla");
		button->setColor(*color);
		button->setAutoSizeFont(false);
		tabButtons.add(button);
		location.x+=width;
	}
}

void TabHeaderControl::adjustLabels()
{
	for (pp_int32 i = 0; i < tabButtons.size(); i++)
	{
		PPButton* button = tabButtons.get(i);
		PPFont* font = button->getFont();	
		pp_int32 shrinkWidth = 	button->getSize().width - 4;
		if (shrinkWidth < 1)
			shrinkWidth = 1;

		PPString text = tabHeaders.get(i + startIndex)->text;
		if (text.charAt(text.length()-1) == '*')
		{
			text = font->shrinkString(text, shrinkWidth - font->getCharWidth(), PPFont::ShrinkTypeEnd);
			if (text.charAt(text.length()-1) != '*')
				text.append("*");
		}
		else
			text = font->shrinkString(text, shrinkWidth, PPFont::ShrinkTypeEnd);

		button->setText(text);
		if ((i + startIndex) & 1)
			button->setTextColor(GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorTextHighlited));
		else
			button->setTextColor(GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorForegroundText));
	}
}

void TabHeaderControl::shiftTabs(pp_int32 offset/* = 1*/, bool repaint/* = true*/)
{
	pp_int32 startIndex = this->startIndex + offset;
	
	if (startIndex + tabButtons.size() > tabHeaders.size())
		startIndex-= (startIndex + tabButtons.size()) - tabHeaders.size();

	if (startIndex < 0)
		startIndex = 0;

	if (startIndex != this->startIndex)
	{

		for (pp_int32 i = 0; i < tabButtons.size(); i++)
			tabButtons.get(i)->setPressed(false);
				
		pp_int32 newIndex = (signed)hitIndex - startIndex;
		if (newIndex >= 0 && newIndex < tabButtons.size())
		{
			tabButtons.get(newIndex)->setPressed(true);
		}
	
		this->startIndex = startIndex;
		adjustLabels();
		if (repaint)
			parentScreen->paintControl(this);
	}
}

void TabHeaderControl::assureTabVisible(pp_uint32 index)
{
	pp_int32 newIndex = (signed)index - startIndex;

	if (newIndex >= 0 && newIndex < tabButtons.size())
		return;
		
	if (newIndex < 0)
	{
		while (newIndex < 0)
		{
			startIndex--;
			newIndex = (signed)index - startIndex;
		}
		return;
	}

	if (newIndex >= tabButtons.size())
	{
		while (newIndex >= tabButtons.size())
		{
			startIndex++;
			newIndex = (signed)index - startIndex;
		}
		return;
	}
}

void TabHeaderControl::setSelectedTab(pp_uint32 index)
{
	hitIndex = index;

	assureTabVisible(index);

	for (pp_int32 i = 0; i < tabButtons.size(); i++)
		tabButtons.get(i)->setPressed(false);
				
	pp_int32 newIndex = (signed)index - startIndex;
	if (newIndex >= 0 && newIndex < tabButtons.size())
	{
		tabButtons.get(newIndex)->setPressed(true);
	}
}

void TabHeaderControl::setTabHeaderText(pp_int32 index, const PPString& text)
{
	if (index < 0 || index >= tabHeaders.size())
		return;

	tabHeaders.get(index)->text = text;
	adjustLabels();
}


