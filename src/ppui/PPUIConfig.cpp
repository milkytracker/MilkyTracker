/*
 *  PPUIConfig.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 01.07.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "PPUIConfig.h"

PPUIConfig* PPUIConfig::instance = NULL;

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
	colors[ColorRadioGroupButton] = PPColor(128, 128, 0); 
	colors[ColorStaticText] = PPColor(255, 255, 255); 
}

PPUIConfig* PPUIConfig::getInstance()
{
	if (instance == NULL)
		instance = new PPUIConfig();

	return instance;
}
