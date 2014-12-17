/*
 *  tracker/cocoa/main.mm
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

// -------- Cocoa/OS X --------
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CGEventSource.h>
#import <CoreGraphics/CGEventTypes.h>
#import <HIToolbox/Events.h>
#import <mach/mach.h>
#import <mach/mach_time.h>

// ---------- Tracker ---------
#import "BasicTypes.h"
#import "Event.h"

// ----------------------------------------------------------
//  Returns number of milliseconds elapsed since last reboot
// ----------------------------------------------------------
pp_uint32 PPGetTickCount()
{
	// Static variable guaranteed to be zero-initialised
	static mach_timebase_info_data_t timebaseInfo;
	
	// Timebase info uninitialised?
	if (timebaseInfo.denom == 0)
		mach_timebase_info(&timebaseInfo);
	
	// Get the system tick count in nanoseconds
	uint64_t absTime = mach_absolute_time();
	uint64_t absTimeNanos = absTime * timebaseInfo.numer / timebaseInfo.denom;
	
	// Convert to milliseconds
	return (pp_uint32) (absTimeNanos / 1e6);
}

// ---------------------------------------------------------------
//  Checks modifier key states and sets tracker state accordingly
// ---------------------------------------------------------------
void QueryKeyModifiers()
{
	CGEventSourceStateID eventSource = kCGEventSourceStateCombinedSessionState;
	
	if (CGEventSourceKeyState(eventSource, kVK_Shift) || CGEventSourceKeyState(eventSource, kVK_RightShift))
		setKeyModifier(KeyModifierSHIFT);
	else
		clearKeyModifier(KeyModifierSHIFT);
	
	// Command is mapped to CTRL under OS X
	if (CGEventSourceKeyState(eventSource, kVK_Command))
		setKeyModifier(KeyModifierCTRL);
	else
		clearKeyModifier(KeyModifierCTRL);
	
	// Option is mapped to ALT under OS X
	if (CGEventSourceKeyState(eventSource, kVK_Option) || CGEventSourceKeyState(eventSource, kVK_RightOption))
		setKeyModifier(KeyModifierALT);
	else
		clearKeyModifier(KeyModifierALT);
}

// --------------------------------------
//  Entry point; start Cocoa application
// --------------------------------------
int main(int argc, const char * argv[])
{
	return NSApplicationMain(argc, argv);
}
