/*
 *  tracker/carbon/Carbon_main.cpp
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

// ------------------------ carbon ------------------------
#include <Carbon/Carbon.h>
#include "Carbon_Definitions.h"
#include "KeyTranslation.h"
#include "PreferencesDialog.h"
#include "PPMutex.h"
#include "PPMessageBox.h"
// ----------------------- POSIX --------------------------
#include <sys/stat.h>
#include <signal.h>
// ----------------------- tracker ------------------------
#include "PPUI.h"
#include "DisplayDevice_CARBON.h"
#include "Screen.h"
#include "Tracker.h"
#include "PPSystem.h"
#include "MidiReceiver_pthread.h"

// globals -------------------------------------
WindowRef					mainWindow, waitWindow, preferencesWindow;
MenuRef						mainMenu;

static PreferencesDialog*	preferencesDialog	= NULL;
static PPMutex*				globalMutex			= NULL;

static int					localMouseX,localMouseY;

static PPScreen*			myTrackerScreen		= NULL;
static Tracker*				myTracker			= NULL;
static PPDisplayDevice*		myDisplayDevice		= NULL;
static MidiReceiver*		myMidiReceiver		= NULL;

// basic OS event globals ----------------------
static pp_uint32	lmyTime;
static PPPoint		llastClickPosition		= PPPoint(0,0);
static pp_uint16	lClickCount				= 0;

static pp_uint32	rmyTime;
static PPPoint		rlastClickPosition		= PPPoint(0,0);
static pp_uint16	rClickCount				= 0;

static bool			lMouseDown				= false;
static pp_uint32	lButtonDownStartTime;

static bool			rMouseDown				= false;
static pp_uint32	rButtonDownStartTime;

static pp_uint32	timerTicker				= 0;
//static pp_int32		keyTimerTicker			= 0;
static pp_uint32	lastModifierKeyState	= 0;

static PPPoint p;

static bool			sixteenBitColorDepth	= false;

pp_uint32 PPGetTickCount()
{
	UnsignedWide microTickCount;
	Microseconds (&microTickCount);	
	pp_int64 time = (((pp_int64)microTickCount.hi) << 32) + microTickCount.lo;
	return time/1000;		
}

static void crashHandler(int signum) 
{
	// Save backup.xm
	static char buffer[1024]; // Should be enough :p
	static char message[1024];
	strncpy(buffer, getenv("HOME"), 1010);
	strcat(buffer, "/BACKUP00.XM");
	struct stat statBuf;
	int num = 1;
	while (stat(buffer, &statBuf) == 0 && num <= 100)
		snprintf(buffer, sizeof(buffer), "%s/BACKUP%02i.XM", getenv("HOME"), num++);

	if(num != 100) 
	{
		myTracker->saveModule(buffer);
		fputs("\n", stderr);
		fprintf(stderr, "\nA backup has been saved to %s\n\n", buffer);
	}

	if(signum == 15)
	{
		snprintf(message, sizeof(message), "TERM signal received.\n");
		fputs(message, stderr);
	}
	else
	{
		snprintf(message, sizeof(message), "MilkyTracker crashed with signal %i\n"
				"Please submit a bug report stating exactly what you were doing at "
				"the time of the crash, as well as the above signal number.  "
				"Also note if it is possible to reproduce this crash.\n", signum);
		fprintf(stderr, "\n");
		fputs(message, stderr);
		
		PPMessageBox infoBox(myTrackerScreen, "Error", message);
		infoBox.runModal();
	}

	exit(0);
}

static void installCrashHandler()
{
	// Initialise crash handler
	struct sigaction act;
	struct sigaction oldAct;
	memset(&act, 0, sizeof(act));
	act.sa_handler = crashHandler;
	act.sa_flags = SA_RESETHAND;
	sigaction(SIGTERM | SIGILL | SIGABRT | SIGFPE | SIGSEGV, &act, &oldAct);
	sigaction(SIGILL, &act, &oldAct);
	sigaction(SIGABRT, &act, &oldAct);
	sigaction(SIGFPE, &act, &oldAct);
	sigaction(SIGSEGV, &act, &oldAct);
}

// Some forwards
pascal OSStatus MainWindowEventHandler(EventHandlerCallRef myHandler,EventRef event,void *userData);

void RaiseEventSynchronized(PPEvent* event)
{
	if (globalMutex->tryLock())
	{
		if (myTrackerScreen)
			myTrackerScreen->raiseEvent(event);
			
		globalMutex->unlock();
	}
}

void TimerEventHandler(__EventLoopTimer*, void*)
{
	if (!myTrackerScreen)
		return;

	PPEvent myEvent(eTimer);	
	RaiseEventSynchronized(&myEvent);
	
	/*if ((keyTimerTicker > 0) && !(keyTimerTicker % 3))
	{
		if (TestForKeyDown(kVirtualCapsLockKey))
		{
			pp_uint16 chr[2];
			chr[0] = VK_CAPITAL;
			chr[1] = SC_CAPSLOCK;
			PPEvent event(eKeyDown, &chr, sizeof(chr));
			myTrackerScreen->raiseEvent(&event);
		}
	}
	
	keyTimerTicker++;*/
	
	timerTicker++;
	
	if (lMouseDown &&
		(timerTicker - lButtonDownStartTime) > 25 && !(timerTicker%3))
	{
		PPEvent myEvent(eLMouseRepeat, &p, sizeof(PPPoint));		
		RaiseEventSynchronized(&myEvent);
	}

	if (rMouseDown &&
		(timerTicker - rButtonDownStartTime) > 25)
	{
		PPEvent myEvent(eRMouseRepeat, &p, sizeof(PPPoint));		
		RaiseEventSynchronized(&myEvent);
	}

}

void StopMidiRecording()
{
	if (myMidiReceiver)
	{
		myMidiReceiver->stopRecording();
	}
}

class MidiEventHandler : public MidiReceiver::MidiEventHandler
{
public:
	virtual void keyDown(int note, int volume) 
	{
		globalMutex->lock();
		myTracker->sendNoteDown(note, volume);
		globalMutex->unlock();
	}

	virtual void keyUp(int note) 
	{
		globalMutex->lock();
		myTracker->sendNoteUp(note);
		globalMutex->unlock();
	}
};

MidiEventHandler midiEventHandler;

void StartMidiRecording(unsigned int devID, bool recordMidiVelocity, unsigned int velocityAmplify)
{
	if (devID == (unsigned)-1)
		return;

	StopMidiRecording();

	myMidiReceiver = new MidiReceiver(midiEventHandler);
	myMidiReceiver->setRecordVelocity(recordMidiVelocity);
	myMidiReceiver->setVelocityAmplify(velocityAmplify);	

	if (!myMidiReceiver->startRecording(devID))
	{
		// Deal with error
		PPMessageBox infoBox(myTrackerScreen, "Error","Could not enable MIDI recording.");
		infoBox.runModal();
	}
}

void InitMidi()
{
	if (!preferencesDialog)
		return;

	if (preferencesDialog->getUseMidiDeviceFlag())
		StartMidiRecording(preferencesDialog->getSelectedMidiDeviceID(),
						   preferencesDialog->getRecordVelocityFlag(),
						   preferencesDialog->getVelocityAmplify());
	else
		StopMidiRecording();
}

void ApplyPreferences()
{
	InitMidi();
	enableInsertKeyEmulation((InsertKeyShortcuts)preferencesDialog->getFakeInsertKey());
	if (preferencesDialog->getUse15BitColorDepth() != sixteenBitColorDepth)
	{
		PPMessageBox infoBox(myTrackerScreen, "Please restart",
							 "MilkyTracker needs to be restarted to apply new color depth");
		infoBox.runModal();
	}
}

void initTracker()
{
	myTracker = new Tracker();

	PPSize windowSize = myTracker->getWindowSizeFromDatabase();
	pp_int32 scaleFactor = myTracker->getScreenScaleFactorFromDatabase();
	bool fullScreen = myTracker->getFullScreenFlagFromDatabase();
#ifdef __LOWRES__
	windowSize.width = 320;
	windowSize.height = 240;
#endif

	sixteenBitColorDepth = preferencesDialog->getUse15BitColorDepth();

	myDisplayDevice = new PPDisplayDevice(mainWindow, 
										  waitWindow, 
										  windowSize.width, 
										  windowSize.height, 
										  scaleFactor,
										  sixteenBitColorDepth ? 16 : 32);
	
	myDisplayDevice->init();

	if (fullScreen)
		myDisplayDevice->goFullScreen(fullScreen);
	
	myTrackerScreen = new PPScreen(myDisplayDevice, myTracker);

	myTracker->setScreen(myTrackerScreen);
 
	// Startup procedure
	myTracker->startUp();

	ApplyPreferences();

	// install crash handler
//#ifndef __DEBUG__
//	installCrashHandler();
//#endif
}

static Boolean gApprovedDrag = false; /* set to true if the drag is approved */
static Boolean gInIconBox = false; /* set to true if the drag is approved */

static void dropFileHandler(const FSSpec& spec)
{
	FSRef fileToOpen;
	FSpMakeFSRef(&spec, &fileToOpen);
	// gib ihm
	char buffer[PATH_MAX+1];
	
	FSRefMakePath (&fileToOpen, (UInt8*)buffer, sizeof(buffer)-1);
	PPSystemString finalFile(buffer);
	PPSystemString* strPtr = &finalFile;
	
	PPEvent event(eFileDragDropped, &strPtr, sizeof(PPSystemString*));
	RaiseEventSynchronized(&event);
}

static pascal OSErr myDragReceiveHandler(WindowPtr theWindow, void *refcon, DragReference theDragRef) 
{
	ItemReference theItem;
	HFSFlavor targetFile;
	//PromiseHFSFlavor targetPromise;
	//FSSpec targetSpec;
	Size theSize;
	OSErr err = eventNotHandledErr;
	
	if (!gApprovedDrag)
		goto bail;

	// get the first item reference 
	if ((err = GetDragItemReferenceNumber(theDragRef, 1, &theItem)) != noErr) goto bail;
		
	// try to get a  HFSFlavor
	theSize = sizeof(HFSFlavor);
	err = GetFlavorData(theDragRef, theItem, flavorTypeHFS, &targetFile, &theSize, 0);
	if (err == noErr) 
	{
		dropFileHandler(targetFile.fileSpec);
		return noErr;
	} 
	else if (err != badDragFlavorErr) 
		goto bail;
	
bail:
	return err;
}

static pascal OSErr approveDragReference(DragReference theDragRef, Boolean *approved) {
	OSErr err;
	UInt16 itemCount;
	DragAttributes dragAttrs;
	FlavorFlags flavorFlags;
	ItemReference theItem;
		
	// we cannot drag to our own window 
	if ((err = GetDragAttributes(theDragRef, &dragAttrs)) != noErr) goto bail;
	if ((dragAttrs & kDragInsideSenderWindow) != 0) { err = userCanceledErr; goto bail; }
	
	// we only accept drags containing one item 
	if ((err = CountDragItems(theDragRef, &itemCount)) != noErr) goto bail;
	if (itemCount != 1) { err = paramErr; goto bail; }
		
	// gather information about the drag & a reference to item one.
	if ((err = GetDragItemReferenceNumber(theDragRef, 1, &theItem)) != noErr) goto bail;
		
	// check for flavorTypeHFS
	err = GetFlavorFlags(theDragRef, theItem, flavorTypeHFS, &flavorFlags);
	if (err == noErr) {
		*approved = true;
		return noErr;
	} else if (err != badDragFlavorErr)
		goto bail;
		
	// check for flavorTypePromiseHFS 
	//err = GetFlavorFlags(theDragRef, theItem, flavorTypePromiseHFS, &flavorFlags);
	//if (err == noErr) {
	//	*approved = true;
	//	return noErr;
	//} else if (err != badDragFlavorErr)
	//	goto bail;
		
	// none of our flavors were found
	*approved = false;
	return noErr;
bail:
	// an error occured, clean up.  set result to false.
	*approved = false;
	return err;
}

static pascal OSErr myDragTrackingHandler(DragTrackingMessage message, WindowPtr theWindow, void *refCon, DragReference theDragRef) 
{
	// we're drawing into the image well if we hilite... 
	switch (message) {
	
		case kDragTrackingEnterWindow:
			{	
				gApprovedDrag = false;
				if (theWindow == FrontWindow()) {
					if (approveDragReference(theDragRef, &gApprovedDrag) != noErr) break;
					if ( ! gApprovedDrag ) break;
					myTrackerScreen->setShowDragHilite(true);
					gInIconBox = true;
				}
			}
			break;

		/*case kDragTrackingInWindow:
			if (gApprovedDrag) {
				Point mouse;
				SetPortWindowPort(mainWindow);
				GetMouse(&mouse);
				GlobalToLocal(&mouse);
				if (PtInRect(mouse, &gIconBox)) {
					if ( ! gInIconBox) {  // if we're entering the box, hilite... 
						SetPortWindowPort(mainWindow);
						gInIconBox = (ShowDragHiliteBox(theDragRef, &gIconBox) == noErr);
					}
				} else if (gInIconBox) {  // if we're exiting the box, unhilite... 
					HideDragHilite(theDragRef);
					gInIconBox = false;
				}
			}
			break;*/

		case kDragTrackingLeaveWindow:
			if (gApprovedDrag && gInIconBox) {
				//HideDragHilite(theDragRef);
				myTrackerScreen->setShowDragHilite(false);
			}
			gApprovedDrag = gInIconBox = false;
			break;
	}
	
	return noErr; // there's no point in confusing Drag Manager or its caller
}

static Boolean gotRequiredParams(const AppleEvent *theEvent)
{
	DescType returnedType;
	Size 	actualSize;
	OSErr	err = AEGetAttributePtr( theEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, NULL, 0, &actualSize);
	return err == errAEDescNotFound;
}

static OSStatus doOpenAppleEvent(const AppleEvent *theEvent)
{
	OSStatus	error;

	FSSpec		myFSS;
	AEDescList	theList;
	AEKeyword	aeKeyword = keyDirectObject;
	long		itemCount, i;
	DescType	actualType;
	Size		actualSize;
	
	// get event description
	error = AEGetParamDesc(theEvent, keyDirectObject, typeAEList, &theList);
	if( error != noErr )
		return error;

	// make sure event description is correct
	if (!gotRequiredParams(theEvent))
		return error;

	// count items to open
	error = AECountItems(&theList, &itemCount);
	if( error != noErr )
		return error;

	// open all items
	for (i = 1; i <= itemCount; i++)
	{
		error = AEGetNthPtr(&theList, i, typeFSS, &aeKeyword, &actualType, (Ptr) &myFSS, sizeof( FSSpec ), &actualSize);
		if (error == noErr)
			dropFileHandler(myFSS);
	}

	// event was handled successfully
	AEDisposeDesc(&theList);
	
	return noErr;
}

static pascal OSErr coreEventHandler(const AppleEvent *theEvent, AppleEvent *reply, long refCon)
{
	DescType	actualType;
	Size		actualSize;
	DescType	eventID;
	OSErr		error;

	error = AEGetAttributePtr( ( AppleEvent* ) theEvent, keyEventIDAttr, typeType, &actualType, (Ptr) &eventID, sizeof( eventID ), &actualSize);

	if (error)
		return error;
								
	switch( eventID )
	{
		case kAEOpenApplication:
			if (gotRequiredParams(theEvent))
				doOpenAppleEvent(theEvent);
			break;
				
		case kAEOpenDocuments:
			doOpenAppleEvent(theEvent);
			break;
				
	}
	return noErr;
}

static void initAppleEvents()
{
	AEEventHandlerUPP AEHandlerUPP;
	
	AEHandlerUPP = NewAEEventHandlerUPP(coreEventHandler);

	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, AEHandlerUPP, 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, AEHandlerUPP, 0, false);
}

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
	EventLoopRef	loopRef;
    
    OSStatus		err;
  
	globalMutex = new PPMutex();
  
    EventTypeSpec mainSpec[] = 
	{
		{kEventClassCommand,kEventCommandProcess},
		{kEventClassKeyboard,kEventRawKeyDown},
		{kEventClassKeyboard,kEventRawKeyRepeat},
		{kEventClassKeyboard,kEventRawKeyUp},
		{kEventClassKeyboard,kEventRawKeyModifiersChanged},
		{kEventClassMouse,kEventMouseDown},
		{kEventClassMouse,kEventMouseUp},
		{kEventClassMouse,kEventMouseMoved},
		{kEventClassMouse,kEventMouseExited},
		{kEventClassMouse,kEventMouseDragged},
		{kEventClassMouse,kEventMouseWheelMoved},
		{kEventClassTextInput,kEventUnicodeForKeyEvent},
		{kEventClassWindow,kEventWindowDrawContent},
		{kEventClassWindow,kEventWindowBoundsChanged},
		{kEventClassWindow,kEventWindowClose}
	};
	
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
	err = CreateMenuFromNib (nibRef, CFSTR("MenuBar"), &mainMenu);
	require_noerr( err, CantSetMenuBar );

	err = SetRootMenu (mainMenu);
	require_noerr( err, CantSetMenuBar );

    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &mainWindow);
    require_noerr( err, CantCreateWindow );

    // Create wait progress bar window from nib
	err = CreateWindowFromNib(nibRef, CFSTR("WaitWindow"), &waitWindow);
    require_noerr( err, CantCreateWindow );

    // Create preferences window from nib
	err = CreateWindowFromNib(nibRef, CFSTR("PreferencesWindow"), &preferencesWindow);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

	// Install handler for dropping files on application icon
	initAppleEvents();

	// Install drag receive handler
	err = InstallReceiveHandler(myDragReceiveHandler, mainWindow, NULL);
	// Install drag tracking handler
	err = InstallTrackingHandler(myDragTrackingHandler, mainWindow, NULL);

    // Install our own handler for main window
	InstallWindowEventHandler(mainWindow,NewEventHandlerUPP(MainWindowEventHandler),
							  sizeof(mainSpec)/sizeof(EventTypeSpec),(EventTypeSpec*)&mainSpec,(void*)mainWindow,NULL);

	// Create preferences window
	preferencesDialog = new PreferencesDialog(preferencesWindow, mainWindow);
    
	InitKeyCodeTranslation();
	
	// tracker-init 
	initTracker();
	
	loopRef = GetMainEventLoop();	    
	InstallEventLoopTimer(loopRef, 0.0f, 20.0f/1000.0f, TimerEventHandler, NULL, NULL);

    // Call the event loop
    RunApplicationEventLoop();

	globalMutex->lock();
	delete myMidiReceiver;
	globalMutex->unlock();
	delete myTracker;
	delete myTrackerScreen;
	delete myDisplayDevice;
	delete preferencesDialog;

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	delete globalMutex;

	return err;
}

void ProcessKeyEvent(EventRef event, EEventDescriptor targetEvent)
{
    UInt32	keyCode;
	
	GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode);
	
	UniChar* text = NULL;
	UInt32 size;
	
	/* request the size of the Unicode text */
	if (noErr != GetEventParameter(event, kEventParamKeyUnicodes,
								   typeUnicodeText, NULL, 0, &size, NULL))
		goto bail;
	
	if (size % sizeof(UniChar) != 0)
		goto bail;
	
	text = new UniChar[size];
	
	/* fetch the Unicode chars */
	if (noErr != GetEventParameter(event, kEventParamKeyUnicodes,
								   typeUnicodeText, NULL, size, NULL, text))
		goto bail;
	
	
	{
		pp_uint16 chr[3] = {KeyCodeToVK(keyCode), KeyCodeToSC(keyCode), *text};
		PPEvent event(targetEvent, &chr, sizeof(chr));
		RaiseEventSynchronized(&event);
	}
bail:
	if (text)
		delete[] text;
}

pascal OSStatus MainWindowEventHandler(EventHandlerCallRef myHandler,EventRef event,void *userData)
{
    OSStatus	result = eventNotHandledErr;
    
	if (!myTrackerScreen)
		return result;
	
	Rect	windowRect;
    UInt32	eventClass, eventKind;
    
    eventClass = GetEventClass(event);
    eventKind = GetEventKind(event);
	
	// -----------------------------------------
	if (lClickCount > 4)
	{
		lClickCount = 0;
	}	
	  
    switch (eventClass) 
	{
        case kEventClassCommand: 
		{
			switch (eventKind) 
			{
				case kEventCommandProcess: 
				{
					HICommand command;
					GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
					
					switch (command.commandID)
					{
						case 'quit':
						{
							// Shouldn't really handle this event, handle AppQuit instead
							PPEvent event(eAppQuit);
							RaiseEventSynchronized(&event);
							bool res = myTracker->shutDown();
							// shutdown was aborted 
							if (!res)
								result = noErr;
							break;
						}
						
						case kFullscreenCommand:
						{
							PPEvent myEvent(eFullScreen);
							RaiseEventSynchronized(&myEvent);
							result = noErr;
							break;
						}

						case kInvokePreferences:
						{
							if (preferencesDialog)
								preferencesDialog->show();
							result = noErr;
							break;
						}
						
						case kConfirmPreferences:
						{
							if (preferencesDialog)
								preferencesDialog->hide();
							
							ApplyPreferences();
							
							SelectWindow(mainWindow);
							result = noErr;
							break;
						}
							
						case kDiscardPreferences:
						{
							if (preferencesDialog)
								preferencesDialog->hide();
							
							SelectWindow(mainWindow);
							result = noErr;
							break;
						}
					}
					
					break;
				}
			}
			break;
		}
		
        case kEventClassWindow: 
		{
			switch (eventKind) 
			{
				case kEventWindowBoundsChanged:
				case kEventWindowDrawContent:
				{
					myTrackerScreen->update();
					break;
				}
				case kEventWindowClose:
				{
					// This is a single window app, so quit when the window is closed.
					// This could be simplified if we handled AppQuit instead of hooking the
					// quit command above.
					PPEvent event(eAppQuit);
					RaiseEventSynchronized(&event);
					bool res = myTracker->shutDown();
					if (res)
					{
						EventRef quitEvent;
						CreateEvent(NULL, kEventClassApplication, kEventAppQuit, 0, kEventAttributeNone, &quitEvent);
						PostEventToQueue(GetMainEventQueue(), quitEvent, kEventPriorityStandard);
					}
					result = noErr;
					break;
				}
			}
			break;
		}
			
		case kEventClassTextInput:
		{
			switch (eventKind) 
			{
				case kEventUnicodeForKeyEvent:
				{
					UniChar* text = NULL;
					UInt32	size;
					
					/* request the size of the Unicode text */
					if (noErr != GetEventParameter(event, kEventParamTextInputSendText,
												   typeUnicodeText, NULL, 0, &size, NULL))
						goto bail;

					if (size % sizeof(UniChar) != 0)
						goto bail;

					text = new UniChar[size];
					
					/* fetch the Unicode chars */
					if (noErr != GetEventParameter(event, kEventParamTextInputSendText,
												   typeUnicodeText, NULL, size, NULL, text))
						goto bail;
					
					{	
						pp_uint16 chr = *text;
						
						switch (chr)
						{
							// delete
							case 127:
								chr = VK_BACK;
								break;
							// backspace
							case 8:
								chr = VK_BACK;
								break;
							// escape (is 27 already?)
							case 27:
								chr = VK_ESCAPE;
								break;
							// return (is 13 already?)
							case 13:
								chr = VK_RETURN;
								break;
						}
						
						if (chr >= 32)
						{
							PPEvent event2(eKeyChar, &chr, sizeof(chr));
							RaiseEventSynchronized(&event2);
						}
					}
					
						
				bail:
					if (text)
						delete[] text;
				}
			}
			break;
		}
	
        case kEventClassMouse: 
		{
			Point mousePoint;
			GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(mousePoint), NULL, &mousePoint);
			SetPortWindowPort(mainWindow);
			short mouseButton;
			GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(mouseButton), NULL, &mouseButton);
			UInt32 keyModifiers;
			GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(keyModifiers), NULL, &keyModifiers);
			
			GetPortBounds(GetWindowPort(mainWindow),&windowRect);
			//double xScale = (double)myDisplayDevice->getSize().width / (windowRect.right - windowRect.left);
			//double yScale = (double)myDisplayDevice->getSize().height / (windowRect.bottom - windowRect.top);

			GlobalToLocal(&mousePoint);
			//localMouseX = (int)(mousePoint.h*xScale);
			//localMouseY = (int)(mousePoint.v*yScale);
			
			localMouseX = mousePoint.h;
			localMouseY = mousePoint.v;
			
			switch (eventKind) 
			{
				case kEventMouseWheelMoved:
				{
					long wheelDelta;
					GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(wheelDelta), NULL, &wheelDelta);
					
					TMouseWheelEventParams mouseWheelParams;
					mouseWheelParams.pos.x = localMouseX;
					mouseWheelParams.pos.y = localMouseY;
					mouseWheelParams.deltaX = 0;
					mouseWheelParams.deltaY = wheelDelta;
					
					PPEvent myEvent(eMouseWheelMoved, &mouseWheelParams, sizeof(mouseWheelParams));						
					RaiseEventSynchronized(&myEvent);				
					break;
				}
				
				case kEventMouseDown: 
				{
					if (mouseButton > 2 || !mouseButton)
						break;
						
					if (mouseButton == 1 && (keyModifiers & MacKeyModifierCtrl))
						mouseButton++;
						
					// -----------------------------
					p.x = localMouseX;
					p.y = localMouseY;
					
					if (mouseButton == 1)
					{
						//if (rMouseDown)
						//	break;
							
						PPEvent myEvent(eLMouseDown, &p, sizeof(PPPoint));						
						RaiseEventSynchronized(&myEvent);
						
						lMouseDown = true;
						lButtonDownStartTime = timerTicker;
						
						if (!lClickCount)
						{
							lmyTime = ::PPGetTickCount();
							llastClickPosition.x = localMouseX;
							llastClickPosition.y = localMouseY;
						}
						else if (lClickCount == 2)
						{
							pp_uint32 deltat = ::PPGetTickCount() - lmyTime;
							
							if (deltat > 500)
							{
								lClickCount = 0;
								lmyTime = ::PPGetTickCount();
								llastClickPosition.x = localMouseX;
								llastClickPosition.y = localMouseY;
							}
						}
						
						lClickCount++;	
						
					}
					else if (mouseButton == 2)
					{
						//if (lMouseDown)
						//	break;

						PPEvent myEvent(eRMouseDown, &p, sizeof(PPPoint));						
						RaiseEventSynchronized(&myEvent);
						
						rMouseDown = true;
						rButtonDownStartTime = timerTicker;
						
						if (!rClickCount)
						{
							rmyTime = ::PPGetTickCount();
							rlastClickPosition.x = localMouseX;
							rlastClickPosition.y = localMouseY;
						}
						else if (rClickCount == 2)
						{
							pp_uint32 deltat = ::PPGetTickCount() - rmyTime;
							
							if (deltat > 500)
							{
								rClickCount = 0;
								rmyTime = ::PPGetTickCount();
								rlastClickPosition.x = localMouseX;
								rlastClickPosition.y = localMouseY;
							}
						}
						
						rClickCount++;	
					}
					

					break;
				}

				case kEventMouseUp:
				{
					if (mouseButton > 2 || !mouseButton)
						break;

					if (mouseButton == 1 && (keyModifiers & MacKeyModifierCtrl))
						mouseButton++;

					if (!myTrackerScreen)
						break;					
					// -----------------------------
					
					if (mouseButton == 1)
					{
						//if (!lMouseDown)
						//	break;
							
						lClickCount++;
						
						if (lClickCount == 4)
						{
							pp_uint32 deltat = ::PPGetTickCount() - lmyTime;
							
							if (deltat < 500)
							{
								p.x = localMouseX;
								p.y = localMouseY;
								
								if (abs(p.x - llastClickPosition.x) < 4 &&
									abs(p.y - llastClickPosition.y) < 4)
								{
									
									PPEvent myEvent(eLMouseDoubleClick, &p, sizeof(PPPoint));									
									RaiseEventSynchronized(&myEvent);
								}
							}
							
							lClickCount = 0;							
						}
						
						p.x = localMouseX;
						p.y = localMouseY;
						
						PPEvent myEvent(eLMouseUp, &p, sizeof(PPPoint));						
						RaiseEventSynchronized(&myEvent);
						
						lMouseDown = false;
					}
					else if (mouseButton == 2)
					{
						//if (!rMouseDown)
						//	break;

						rClickCount++;
						
						if (rClickCount == 4)
						{
							pp_uint32 deltat = ::PPGetTickCount() - rmyTime;
							
							if (deltat < 500)
							{
								p.x = localMouseX;
								p.y = localMouseY;
								
								if (abs(p.x - rlastClickPosition.x) < 4 &&
									abs(p.y - rlastClickPosition.y) < 4)
								{
									
									PPEvent myEvent(eRMouseDoubleClick, &p, sizeof(PPPoint));									
									RaiseEventSynchronized(&myEvent);
								}
							}
							
							rClickCount = 0;
						}
						
						p.x = localMouseX;
						p.y = localMouseY;
						
						PPEvent myEvent(eRMouseUp, &p, sizeof(PPPoint));						
						RaiseEventSynchronized(&myEvent);
						
						rMouseDown = false;
					}
					
				} 

				case kEventMouseMoved: 
				{
					p.x = localMouseX;
					p.y = localMouseY;

					PPEvent myEvent(eMouseMoved, &p, sizeof(PPPoint));
					RaiseEventSynchronized(&myEvent);
					break;
				}
				
				case kEventMouseExited:
				{
					p.x = -1;
					p.y = -1;

					PPEvent myEvent(eMouseMoved, &p, sizeof(PPPoint));
					RaiseEventSynchronized(&myEvent);
					break;
				};
				
				case kEventMouseDragged: 
				{
					if (mouseButton > 2 || !mouseButton)
						break;

					p.x = localMouseX;
					p.y = localMouseY;

					if (mouseButton == 1 && lMouseDown)
					{
						PPEvent myEvent(eLMouseDrag, &p, sizeof(PPPoint));				
						RaiseEventSynchronized(&myEvent);
					}
					else if (rMouseDown)
					{
						PPEvent myEvent(eRMouseDrag, &p, sizeof(PPPoint));
						RaiseEventSynchronized(&myEvent);
					}
					break;
				}
			}
			
			break;
			
		}
		
		case kEventClassKeyboard: 
		{
			switch (eventKind) 
			{
				case kEventRawKeyDown: 
				case kEventRawKeyRepeat: 
				{
					ProcessKeyEvent(event, eKeyDown);
					break;
				}
				
				case kEventRawKeyUp: 
				{
					ProcessKeyEvent(event, eKeyUp);
					break;
				}

				case kEventRawKeyModifiersChanged: 
				{				
					UInt32 keyModifiers;
				
					pp_uint16 chr[3];
				
					GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(keyModifiers), NULL, &keyModifiers);

					if ((keyModifiers & MacKeyModifierShift) != (lastModifierKeyState & MacKeyModifierShift))
					{
						chr[0] = VK_SHIFT;
						chr[1] = 0;
						chr[2] = 0;
						if (keyModifiers & MacKeyModifierShift)
						{
							::setKeyModifier(KeyModifierSHIFT);
							PPEvent event(eKeyDown, &chr, sizeof(chr));
							RaiseEventSynchronized(&event);						
						}
						else
						{
							PPEvent event(eKeyUp, &chr, sizeof(chr));
							RaiseEventSynchronized(&event);						
							::clearKeyModifier(KeyModifierSHIFT);
						}
					}

					/*if ((keyModifiers & eCapsLockKey) != (lastModifierKeyState & eCapsLockKey))
					{
						chr[0] = VK_CAPITAL;
						chr[1] = SC_CAPSLOCK;
						
						//::setKeyModifier(KeyModifierCTRL);
						//keyTimerTicker = -15;
						PPEvent event(eKeyDown, &chr, sizeof(chr));
						RaiseEventSynchronized(&event);						
						PPEvent event2(eKeyUp, &chr, sizeof(chr));
						RaiseEventSynchronized(&event2);						
					}*/

					if ((keyModifiers & MacKeyModifierAlt) != (lastModifierKeyState & MacKeyModifierAlt))
					{
						chr[0] = VK_ALT;
						chr[1] = 0;
						chr[2] = 0;
						if (keyModifiers & MacKeyModifierAlt)
						{
							::setKeyModifier(KeyModifierALT);
							PPEvent event(eKeyDown, &chr, sizeof(chr));
							RaiseEventSynchronized(&event);						
						}
						else
						{
							PPEvent event(eKeyUp, &chr, sizeof(chr));
							RaiseEventSynchronized(&event);						
							::clearKeyModifier(KeyModifierALT);
						}
					}

					if ((keyModifiers & MacKeyModifierCommand) != (lastModifierKeyState & MacKeyModifierCommand))
					{
						chr[0] = VK_CONTROL;
						chr[1] = 0;
						chr[2] = 0;
						if (keyModifiers & MacKeyModifierCommand)
						{
							::setKeyModifier(KeyModifierCTRL);
							PPEvent event(eKeyDown, &chr, sizeof(chr));
							RaiseEventSynchronized(&event);						
						}
						else
						{
							PPEvent event(eKeyUp, &chr, sizeof(chr));
							RaiseEventSynchronized(&event);						
							::clearKeyModifier(KeyModifierCTRL);
						}
					}

					lastModifierKeyState = keyModifiers;

				}
			}
			break;
		}
    }
	
    return result;
    
}
