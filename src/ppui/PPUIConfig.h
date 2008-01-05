/*
 *  PPUIConfig.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 01.07.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef PPUICONFIG__H
#define PPUICONFIG__H

#include "BasicTypes.h"

class PPUIConfig
{
public:
	enum PPUIColors
	{
		ColorSelection = 0,
		ColorGrayedOutSelection,
		ColorDefaultButton,
		ColorDefaultButtonText,
		ColorContainer,
		ColorMessageBoxContainer,
		ColorMenuBackground,
		ColorMenuTextDark,
		ColorMenuTextBright,
		ColorListBoxBorder,
		ColorListBoxBackground,
		ColorRadioGroupButton,
		ColorStaticText,
		ColorLast 
	};


private:
	static PPUIConfig* instance;
	PPColor colors[ColorLast];
	
	PPUIConfig();
	
public:
	static PPUIConfig* getInstance();
	
	const PPColor& getColor(PPUIColors whichColor) { return colors[whichColor]; }
	void setColor(PPUIColors whichColor, const PPColor& color) { colors[whichColor] = color; }
};

#endif

