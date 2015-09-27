/*
 *  tracker/cocoa/AppDelegate.mm
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
#import <dispatch/dispatch.h>

// ---------- Tracker ---------
#import "DisplayDevice_COCOA.h"
#import "MidiReceiver_CoreMIDI.h"
#import "PPMutex.h"
#import "Screen.h"
#import "Tracker.h"

@implementation AppDelegate

// ---------- Display ---------
@synthesize myWindow;
@synthesize myTrackerView;
@synthesize myProgressWindow;
@synthesize myProgressIndicator;

// ------ Tracker Globals -----
static PPScreen*			myTrackerScreen;
static Tracker*				myTracker;
static PPDisplayDevice*		myDisplayDevice;
static MidiReceiver*		myMidiReceiver;

static PPMutex*				globalMutex;

static BOOL					startupAfterFullScreen;
static BOOL					startupComplete;
static NSMutableArray*		filesToLoad;

static CVDisplayLinkRef		displayLink;

// TODO: Crash handler

void RaiseEventSynchronized(PPEvent* event)
{
	if (myTrackerScreen && globalMutex->tryLock())
	{
		myTrackerScreen->raiseEvent(event);
		globalMutex->unlock();
	}
}

static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now,
									const CVTimeStamp* outputTime, CVOptionFlags flagsIn,
									CVOptionFlags* flagsOut, void* displayLinkContext)
{
	// Raise the event on the main thread
	dispatch_async(dispatch_get_main_queue(), ^
	{
		if (!myTrackerScreen)
			return;

		PPEvent e = PPEvent(eTimer);
		RaiseEventSynchronized(&e);
	});

	return kCVReturnSuccess;
}

- (void)initTracker
{
	[myWindow setTitle:@"Loading MilkyTracker..."];
	[myWindow display];
	
	// Instantiate the tracker
	myTracker = new Tracker();
	
	// Retrieve and apply display settings
	PPSize windowSize = myTracker->getWindowSizeFromDatabase();
	pp_int32 scaleFactor = myTracker->getScreenScaleFactorFromDatabase();
	bool fullScreen = myTracker->getFullScreenFlagFromDatabase();
	
	// Bring up display device
	myDisplayDevice = new PPDisplayDevice(myWindow,
										  myTrackerView,
										  windowSize.width,
										  windowSize.height,
										  scaleFactor,
										  32);
	
	// Enable fullscreen mode if necessary
	myDisplayDevice->goFullScreen(fullScreen);
	
	// Should we wait for fullscreen transition before completing startup?
	startupAfterFullScreen = fullScreen;
	
	// Attach display to tracker
	myTrackerScreen = new PPScreen(myDisplayDevice, myTracker);
	myTracker->setScreen(myTrackerScreen);

	// Init MIDI
	myMidiReceiver = new MidiReceiver(*myTracker, *globalMutex);
	myMidiReceiver->init();
}

- (void)trackerStartUp
{
	// Force immediate screen updates during splash screen because Cocoa loop is blocked
	myDisplayDevice->setImmediateUpdates(true);
	
	// Perform startup
	myTracker->startUp();
	
	// Allow Cocoa to handle refresh again (keeps event processing smooth and responsive)
	myDisplayDevice->setImmediateUpdates(false);
	
	// CVDisplayLink gives us a callback synchronised with vertical blanking
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetOutputCallback(displayLink, &DisplayLinkCallback, NULL);
	CVDisplayLinkStart(displayLink);

	// Signal startup complete
	startupComplete = YES;
	
	// Handle deferred file loading
	for (NSString* filename in filesToLoad)
		[self application: NSApp openFile:filename];
	
	[filesToLoad removeAllObjects];
	filesToLoad = nil;
}

- (void)timerCallback:(NSTimer*)theTimer
{
	if (!myTrackerScreen)
		return;
	
	PPEvent e = PPEvent(eTimer);
	RaiseEventSynchronized(&e);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Initialisation
	globalMutex = new PPMutex();
	[self initTracker];
	
	if (!startupAfterFullScreen)
		[self trackerStartUp];
}

#pragma mark Progress window
- (void)showProgress:(BOOL)yes
{
	if (yes)
	{
		[myProgressWindow makeKeyAndOrderFront:nil];
		[myProgressIndicator startAnimation:nil];
	}
	else
	{
		[myProgressIndicator stopAnimation:nil];
		[myProgressWindow orderOut:nil];
	}
}

#pragma mark Application events
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	return myTracker->shutDown() ? NSTerminateNow : NSTerminateCancel;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	myMidiReceiver->close();
	delete myMidiReceiver;

	CVDisplayLinkStop(displayLink);
	CVDisplayLinkRelease(displayLink);

	delete myTracker;
	delete myTrackerScreen;
	delete myDisplayDevice;
	delete globalMutex;
}

#pragma mark File open events
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	// Startup not complete; hold onto the file path and load it later
	if (!startupComplete)
	{
		if (!filesToLoad)
			filesToLoad = [[NSMutableArray alloc] initWithObjects:filename, nil];
		else
			[filesToLoad addObject:filename];
	}
	else
	{
		// Temp buffer for file path
		char filePath[PATH_MAX + 1];
		
		// Convert to C string
		[filename getCString:filePath maxLength:PATH_MAX encoding:NSUTF8StringEncoding];
		
		// Create system string from C string
		PPSystemString sysString(filePath);
		PPSystemString* sysStrPtr = &sysString;
		
		// Raise file drop event
		PPEvent event(eFileDragDropped, &sysStrPtr, sizeof(PPSystemString*));
		RaiseEventSynchronized(&event);
	}
	
	return YES;
}

- (void)application:(NSApplication *)theApplication openFiles:(NSArray *)filenames
{
	// Call the single openFile delegate method when multiple files are opened
	for (NSString* filename in filenames)
		[[NSApp delegate] application:NSApp openFile:filename];
}

#pragma mark Window events
- (BOOL)windowShouldClose:(id)sender
{
	[NSApp terminate:self];
	return NO;
}

- (void)windowDidResignKey:(NSNotification *)notification
{
	// Clear modifier keys if window loses focus
	clearKeyModifier(KeyModifierCTRL);
	clearKeyModifier(KeyModifierALT);
	clearKeyModifier(KeyModifierSHIFT);
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
	PPEvent event(eFullScreen);
	RaiseEventSynchronized(&event);
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
	PPEvent event(eFullScreen);
	RaiseEventSynchronized(&event);
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
	if (startupAfterFullScreen)
	{
		[self trackerStartUp];
		startupAfterFullScreen = NO;
	}
}
@end
