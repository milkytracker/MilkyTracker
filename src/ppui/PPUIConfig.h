/*
 *  ppui/PPUIConfig.h
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
 *  PPUIConfig.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 01.07.05.
 *
 */

#ifndef PPUICONFIG__H
#define PPUICONFIG__H

#include "BasicTypes.h"
#include "Singleton.h"

class PPUIConfig : public PPSingleton<PPUIConfig>
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
		ColorScrollBarBackground,
		ColorRadioGroupButton,
		ColorStaticText,
		ColorLast 
	};


private:
	PPColor colors[ColorLast];
	
	PPUIConfig();
	
public:
	const PPColor& getColor(PPUIColors whichColor) const { return colors[whichColor]; }
	void setColor(PPUIColors whichColor, const PPColor& color) { colors[whichColor] = color; }
	
	friend class PPSingleton<PPUIConfig>;
};

#endif

