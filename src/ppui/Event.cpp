/*
 *  PPEvent.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 23.05.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "Event.h"

pp_uint32 keyModifier = 0;
pp_uint32 forceKeyModifier = 0;

void QueryKeyModifiers();

void setKeyModifier(KeyModifiers eModifier)
{
	keyModifier |= eModifier;
}

void clearKeyModifier(KeyModifiers eModifier)
{
	keyModifier &= ~eModifier;
}

void setForceKeyModifier(KeyModifiers eModifier)
{
	forceKeyModifier |= eModifier;
}

void clearForceKeyModifier(KeyModifiers eModifier)
{
	forceKeyModifier &= ~eModifier;
}

pp_uint32 getKeyModifier()
{
	QueryKeyModifiers();
	return keyModifier | forceKeyModifier; 
}
