/*
 *  TabHeaderControl.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 09.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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
	TabHeaderControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size);
	virtual ~TabHeaderControl();

	virtual void setSize(PPSize size);
	virtual void setLocation(PPPoint location);

	void setColor(const PPColor& color) { this->color = &color; backgroundButton->setColor(color); }

	const PPColor& getColor() { return *color; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);

	virtual	bool isVisible() { return PPControl::isVisible() && getNumTabs() > 1; }

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
