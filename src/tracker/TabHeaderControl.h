/*
 *  tracker/TabHeaderControl.h
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
 *  TabHeaderControl.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 09.12.07.
 *
 */

#ifndef __TABHEADERCONTROL_H__
#define __TABHEADERCONTROL_H__

#include "BasicTypes.h"
#include "Control.h"
#include "Button.h"
#include "SimpleVector.h"

class TabHeaderControl : public PPControl, public EventListenerInterface
{
public:
	struct TabHeader
	{
		PPString text;
		pp_uint32 ID;

		TabHeader() :
			text(""),
			ID(0)
		{
		}
		
		TabHeader(const PPString& text, pp_uint32 ID) :
			text(text),
			ID(ID)
		{
		}

		~TabHeader()
		{
		}
	};

private:
	const PPColor* color;
	PPButton* backgroundButton;
	PPButton* leftButton;
	PPButton* rightButton;

	pp_int32 minSize;
	pp_int32 maxSize;

	PPSimpleVector<PPButton> tabButtons;
	PPSimpleVector<TabHeader> tabHeaders;

	// Control caught by mouse button press (left & right)
	PPControl* caughtControl;
	
	pp_int32 startIndex;
	pp_int32 hitIndex;
	
	pp_int32 oldHitIndex;
	
	void paintControls(PPGraphicsAbstract* g)
	{
		for (pp_int32 i = 0; i < tabButtons.size(); i++)
		{
			PPControl* ctrl = tabButtons.get(i);
			if (ctrl->isVisible())
				ctrl->paint(g);
		}
	}

	void handleTabClick(const PPPoint& p);
	void setNumTabs(pp_uint32 numTabs);
	void adjustLabels();
	void shiftTabs(pp_int32 offset = 1, bool repaint = true);
	void assureTabVisible(pp_uint32 index);
	
public:
	TabHeaderControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
					 const PPPoint& location, const PPSize& size);
	virtual ~TabHeaderControl();

	virtual void setSize(const PPSize& size);
	virtual void setLocation(const PPPoint& location);

	void setColor(const PPColor& color) { this->color = &color; backgroundButton->setColor(color); }

	const PPColor& getColor() const { return *color; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 dispatchEvent(PPEvent* event);

	virtual	bool isVisible() const { return PPControl::isVisible() && getNumTabs() > 1; }

	// from EventListenerInterface
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	void setTabMinSize(pp_int32 minSize) { this->minSize = minSize; }
	pp_int32 getTabMinSize() const { return minSize; }

	void setTabMaxSize(pp_int32 maxSize) { this->maxSize = maxSize; }
	pp_int32 getTabMaxSize() const { return maxSize; }
	
	void clear()
	{
		tabHeaders.clear();
		tabButtons.clear();
		startIndex = hitIndex = 0;
	}
	
	pp_uint32 getNumTabs() const { return (unsigned)tabHeaders.size(); }
	void addTab(const TabHeader& tabHeader);
	const TabHeader* getTab(pp_int32 index) const;
	bool removeTab(pp_int32 index);
	
	void setTabHeaderText(pp_int32 index, const PPString& text);
	
	void setSelectedTab(pp_uint32 index);
	pp_int32 getSelectedTabIndex() const { return hitIndex; }
};

#endif
