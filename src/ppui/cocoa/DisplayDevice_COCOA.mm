/*
 *  ppui/cocoa/DisplayDevice_COCOA.mm
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

#include "DisplayDevice_COCOA.h"

PPDisplayDevice::PPDisplayDevice(NSWindow* window, MTTrackerView* trackerView, pp_int32 width, pp_int32 height, pp_int32 scaleFactor, pp_uint32 bpp) :
	PPDisplayDeviceBase(width, height, scaleFactor),
	theWindow(window),
	theTrackerView(trackerView),
	immediateUpdates(NO)
{
	// Allocate a pixel buffer and create a PPGraphics context
	pixelBuffer = new uint8_t[width * height * 3];
	currentGraphics = static_cast<PPGraphicsAbstract*>(new PPGraphics_BGR24(width, height, 3 * width, pixelBuffer));
	
	// Set up the TrackerView with pointer to pixel buffer and dimensions
	trackerView.pixelData = pixelBuffer;
	trackerView.width = width;
	trackerView.height = height;
	trackerView.bpp = bpp;
	
	// Set up texture for rendering
	[trackerView initTexture];
	
	// Resize window
	NSSize size = NSMakeSize(width, height);
	[theWindow setContentSize:size];
	
	// Set minimum size
	[theWindow setMinSize:theWindow.frame.size];
	
	// Lock aspect ratio
	[theWindow setContentAspectRatio:size];
	
	// Listen for mouse movement events
	[theWindow setAcceptsMouseMovedEvents:YES];
	
	// Set responder and give window focus
	[theWindow makeFirstResponder:theTrackerView];
	[theWindow makeKeyAndOrderFront:nil];
}

PPDisplayDevice::~PPDisplayDevice()
{
	delete currentGraphics;
	delete[] pixelBuffer;
}

PPGraphicsAbstract* PPDisplayDevice::open()
{
	if (!isEnabled())
		return NULL;
	
	currentGraphics->lock = false;
	
	return currentGraphics;
}

void PPDisplayDevice::close()
{
	currentGraphics->lock = true;
}

void PPDisplayDevice::update()
{
	// Full screen update
	if (immediateUpdates)
		[theTrackerView display];
	else
		[theTrackerView setNeedsDisplay:YES];
}

void PPDisplayDevice::update(const PPRect& r)
{
	// Partial screen update
	NSRect r1 = NSMakeRect(r.x1, r.y1, r.width(), r.height());
	
	if (immediateUpdates)
		[theTrackerView displayRect:r1];
	else
		[theTrackerView setNeedsDisplayInRect:r1];
}

void PPDisplayDevice::setTitle(const PPSystemString& title)
{
	[theWindow setTitle:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
}

void PPDisplayDevice::setSize(const PPSize& size)
{
	// Unused
}

bool PPDisplayDevice::goFullScreen(bool b)
{
	if (b != bFullScreen)
	{
		[theWindow toggleFullScreen:nil];
		bFullScreen = b;
		return true;
	}
	return false;
}

PPSize PPDisplayDevice::getDisplayResolution() const
{
	CGSize screenSize = [NSScreen mainScreen].frame.size;
	
	return PPSize(screenSize.width, screenSize.height);
}

void PPDisplayDevice::shutDown()
{
	[NSApp terminate:[NSApplication sharedApplication]];
}

void PPDisplayDevice::signalWaitState(bool b, const PPColor& color)
{
	[[NSApp delegate] showProgress:b];
}

void PPDisplayDevice::setMouseCursor(MouseCursorTypes type)
{
	currentCursorType = type;
	
	switch (type)
	{
		case MouseCursorTypeStandard:
			[[NSCursor arrowCursor] set];
			break;
			
		case MouseCursorTypeResizeLeft:
			[[NSCursor resizeLeftCursor] set];
			break;
			
		case MouseCursorTypeResizeRight:
			[[NSCursor resizeRightCursor] set];
			break;
			
		case MouseCursorTypeHand:
			[[NSCursor pointingHandCursor] set];
			break;
		
		case MouseCursorTypeWait:
		default:
			break;
	}
}
