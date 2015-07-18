/*
 *  tracker/cocoa/MTTrackerView.h
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
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>

// ---------- Tracker ---------
#import "Event.h"
#import "MTKeyTranslator.h"

@interface MTTrackerView : NSOpenGLView

// ---- Surface Dimensions ----
@property uint8_t* pixelData;
@property int width;
@property int height;
@property int bpp;

// ---- Defined in main.mm ----
void RaiseEventSynchronized(PPEvent* event);

- (void)initTexture;
@end
