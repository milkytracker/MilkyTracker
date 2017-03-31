/*
 *  tracker/cocoa/MTKeyTranslator.h
 *
 *  Copyright 2014 Dale Whinham
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

// ------- Cocoa/OpenGL -------
#import <Foundation/NSObject.h>
#import <Carbon/Carbon.h>

// ---------- Tracker ---------
#import "BasicTypes.h"
#import "ScanCodes.h"
#import "VirtualKeys.h"

// This keycode is missing from Events.h in SDK versions <10.12
#if !defined(MAC_OS_X_VERSION_10_12) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
enum
{
	kVK_RightCommand = 0x36
};
#endif

@interface MTKeyTranslator : NSObject
+ (pp_uint16)toVK:(unsigned short) keyCode;
+ (pp_uint16)toSC:(unsigned short) keyCode;
@end
