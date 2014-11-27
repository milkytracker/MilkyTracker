/*
 *  ppui/PPUIConfig.cpp
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
 *  PPUIConfig.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 01.07.05.
 *
 */

#include "PPUIConfig.h"

PPUIConfig::PPUIConfig()
{
	colors[ColorSelection] = PPColor(64*2, 64*2, 128*2-1);
	colors[ColorGrayedOutSelection] = PPColor(128, 128, 128);
	colors[ColorDefaultButton] = PPColor(192, 192, 192);
	colors[ColorDefaultButtonText] = PPColor(0, 0, 0);
	colors[ColorContainer] = PPColor(128, 128, 128);
	colors[ColorMessageBoxContainer] = PPColor(120, 120, 120);
	colors[ColorMenuBackground] = PPColor(224, 224, 224);
	colors[ColorMenuTextDark] = PPColor(0, 0, 0); 
	colors[ColorMenuTextBright] = PPColor(255, 255, 255); 
	colors[ColorListBoxBorder] = PPColor(192, 192, 192);
	colors[ColorListBoxBackground] = PPColor(32, 32, 48);
	colors[ColorScrollBarBackground] = PPColor(32, 48, 64);
	colors[ColorRadioGroupButton] = PPColor(128, 128, 0); 
	colors[ColorStaticText] = PPColor(255, 255, 255); 
}

