/*
 *  tracker/cocoa/AppDelegate.h
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

// ---------- Tracker ---------
#import "BasicTypes.h"

// Defined in main.mm
pp_uint32 PPGetTickCount();

// Forward declarations
@class MTTrackerView;

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (assign) IBOutlet NSWindow* myWindow;
@property (assign) IBOutlet MTTrackerView* myTrackerView;
@property (assign) IBOutlet NSWindow* myProgressWindow;
@property (assign) IBOutlet NSProgressIndicator* myProgressIndicator;

- (void)showProgress:(BOOL)yes;
@end