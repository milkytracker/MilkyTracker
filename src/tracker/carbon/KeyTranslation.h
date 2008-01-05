/*
 *  KeyTranslation.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.05.
 *  Copyright 2005 milkytracker.net. All rights reserved.
 *
 */

#ifndef KEYTRANSLATION__H
#define KEYTRANSLATION__H 

#include "BasicTypes.h"

// some Carbon stuff ---------------------------
enum MacKeyModifiers
{
	MacKeyModifierCommand	= 256,
	MacKeyModifierShift		= 512,
	MacKeyModifierCapsLock	= 1024,
	MacKeyModifierAlt		= 2048,
	MacKeyModifierCtrl		= 4096
};

// forwards ------------------------------------
void InitKeyCodeTranslation();
pp_uint16 KeyCodeToVK(UInt32 keyCode);
pp_uint16 KeyCodeToSC(UInt32 keyCode);
void QueryKeyModifiers();

void enableInsertKeyEmulation(bool b);

#endif
