/*
 *  tracker/carbon/KeyTranslation.h
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
 *  KeyTranslation.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.05.
 *
 */

#ifndef KEYTRANSLATION__H
#define KEYTRANSLATION__H 

#include "BasicTypes.h"

// some constants
enum MacKeyModifiers
{
	MacKeyModifierCommand	= 256,
	MacKeyModifierShift		= 512,
	MacKeyModifierCapsLock	= 1024,
	MacKeyModifierAlt		= 2048,
	MacKeyModifierCtrl		= 4096
};

// translate mac style key codes to their PC counterparts
void InitKeyCodeTranslation();
pp_uint16 KeyCodeToVK(UInt32 keyCode);
pp_uint16 KeyCodeToSC(UInt32 keyCode);
void QueryKeyModifiers();

// provide means to simulate missing insert key on mac notebook keyboard
enum InsertKeyShortcuts
{
	InsertKeyShortcutNone,
	InsertKeyShortcutCtrlUp,
	InsertKeyShortcutCtrlEnter,
	InsertKeyShortcutCtrlBackspace,
};

void enableInsertKeyEmulation(InsertKeyShortcuts shortcut);

#endif
