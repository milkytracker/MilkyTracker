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
#import <mach/mach.h>
#import <mach/mach_time.h>

// ---------- Tracker ---------
#import "BasicTypes.h"

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

// -------------------------------------------------------------------
//  Dummy function; key modifiers are checked/set in MTTrackerView.mm
// -------------------------------------------------------------------
void QueryKeyModifiers() { }

// --------------------------------------
//  Entry point; start Cocoa application
// --------------------------------------
int main(int argc, const char * argv[])
{
	return NSApplicationMain(argc, argv);
}
