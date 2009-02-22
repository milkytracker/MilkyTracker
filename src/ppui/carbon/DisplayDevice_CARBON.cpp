/*
 *  ppui/carbon/DisplayDevice_CARBON.cpp
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

#include "DisplayDevice_CARBON.h"
#include "Graphics.h"

#ifdef __DRAWSPROCKETS__
#include <DrawSprocket/DrawSprocket.h>

static DSpContextReference      sDSpContext = NULL;
static CGrafPtr frontBuffer;

CGrafPtr SetScreenTo800x600()
/*	Returns a pointer to the front buffer (a port, on-screen, guaranteed 800x600x32
	even if the screen can't be set to 800x600). Returns NULL if it fails. */
{
    DSpContextAttributes        attr = {
		0,
		800, 600,   // Resolution
		0, 0,
		kDSpColorNeeds_Require, NULL,
		0,
		kDSpDepthMask_32, kDSpDepthMask_32,
		32, 32,
		1,          // Don't need back buffers
	{ 0, 0, 0 }, 0, { 0, 0, 0, 0 }};
    OSStatus                    err;
    GrafPtr                     savPort;
    Rect                        frame;
    CGrafPtr                    frontBuffer;
    
    DSpStartup();
	
    err = DSpFindBestContext(&attr, &sDSpContext);
	
    switch (err)
	{
		case kDSpConfirmSwitchWarning:
			break;
			
		case noErr:
			break;
			
		default:
            return NULL;
    }
	
    /* Reserve the context */
    attr.contextOptions = 0;
    err = DSpContext_Reserve(sDSpContext, &attr);
    if (err)
    {
        return NULL;
    }
    
    /* Fade out (because changing the resolution looks messy) */
    DSpContext_FadeGammaOut(kDSpEveryContext, NULL);
    
    /* Activate context */
    err = DSpContext_SetState(sDSpContext, kDSpContextState_Active);
    if (err)
    {
        DSpContext_FadeGammaIn(kDSpEveryContext, NULL);
        return NULL;
    }
    
    /* Fade in instantly (the screen will remain black) */
    DSpContext_FadeGamma(sDSpContext, 100, NULL);
    
    /* Get a pointer to the front buffer */
    err = DSpContext_GetFrontBuffer(sDSpContext, &frontBuffer);
    if (err)
    {
        return NULL;
    }
    
    return frontBuffer;	
}

void RestoreScreen()
{
    if (!sDSpContext)
    {
        return;
    }
    
#if FADE_OUT_ON_EXIT
    /* 0 if your app will have left the screen black */
    DSpContext_FadeGammaOut(sDSpContext, NULL);
#else
    DSpContext_FadeGamma(sDSpContext, 0, NULL);
#endif
    
    /* Deactivate context */
    DSpContext_SetState(sDSpContext, kDSpContextState_Inactive);
    
    /* Fade in */
    DSpContext_FadeGammaIn(kDSpEveryContext, NULL);
    
    /* Clean up */
    DSpContext_Release(sDSpContext);
    DSpShutdown();
}
#endif

PPDisplayDevice::GWorldWrapper::GWorldWrapper(pp_uint32 width, pp_uint32 height, pp_uint32 bpp/* = 32*/)
{
	visibleRgn = NewRgn();
	SetRect(&pictSize, 0, 0, width, height);
    int err = NewGWorld(&offscreen, bpp, &pictSize, NULL, nil, 0);
    if (err) 
	{
        printf("Could not create offscreen buffer\n");
		exit(1);
	}
}

PPDisplayDevice::GWorldWrapper::~GWorldWrapper()
{
	DisposeGWorld(offscreen);
}

void PPDisplayDevice::GWorldWrapper::UnlockGWorldMemory(unsigned char** Buffer, unsigned int* BytesPerRow)
{
    OSErr	err;
    
	BitMap*		tempBitMap;
	
	ppixMap = (PixMap*)GetPortPixMap(offscreen);
	
    err = LockPixels((PixMapHandle)ppixMap);
    if (!err) 
        printf("LockPixels Error");
	
	tempBitMap = (BitMap*)*(GetPortPixMap(offscreen));
	
    *Buffer = (unsigned char*)tempBitMap->baseAddr;
    *BytesPerRow = (unsigned int)tempBitMap->rowBytes&0x3fff;
}

void PPDisplayDevice::GWorldWrapper::LockGWorldMemory()
{
    UnlockPixels((PixMapHandle)ppixMap);
}

PPDisplayDevice::PPDisplayDevice(WindowPtr mainWindow, WindowPtr waitWindow, 
								 pp_int32 width, pp_int32 height, pp_int32 scaleFactor/* = 1*/,
								 pp_uint32 bpp/* = 32*/) : 
	PPDisplayDeviceBase(width, height, scaleFactor),
	mainWindow(mainWindow),
	waitWindow(waitWindow),	
	oldmode(NULL),
	mode(NULL),
	drawThread(NULL),
	waitWindowVisible(false)
{
	mainGWorld = new GWorldWrapper(width, height, bpp);
	
	pp_uint8* buffer;
	pp_uint32 pitch;
	
	mainGWorld->UnlockGWorldMemory(&buffer,(unsigned int*)&pitch);
    mainGWorld->LockGWorldMemory();
	
	switch (bpp)
	{
		case 16:
			currentGraphics = new PPGraphics_15BIT(getSize().width, getSize().height, pitch, buffer);
			break;
		case 32:
			currentGraphics = new PPGraphics_ARGB32(getSize().width, getSize().height, pitch, buffer);
			break;
	}
	
	this->size.width = width;
	this->size.height = height;
	SizeWindow(mainWindow, width * scaleFactor, height * scaleFactor, false);
	
	currentGraphics->lock = true;	
}

PPDisplayDevice::~PPDisplayDevice()
{
	delete mainGWorld;
	delete waitGWorld;
	
	delete currentGraphics;
}

PPGraphicsAbstract* PPDisplayDevice::open()
{
	if (!isEnabled())
		return NULL;
	
	pp_uint8* buffer;
	pp_uint32 pitch;
    mainGWorld->UnlockGWorldMemory(&buffer, (unsigned int*)&pitch);
	static_cast<PPGraphicsFrameBuffer*>(currentGraphics)->setBufferProperties(pitch, buffer);
	
	currentGraphics->lock = false;
	
	return currentGraphics;
}

void PPDisplayDevice::close()
{
	currentGraphics->lock = true;
	
    mainGWorld->LockGWorldMemory();
}

void PPDisplayDevice::update()
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	GDHandle	SaveGD;
	CGrafPtr	SavePort;
	Rect		windowRect;
	
    GetGWorld(&SavePort, &SaveGD);        
	
    //our current world must be the window, otherwise the offscreen buffer is drawn somewhere outside
    SetGWorld(GetWindowPort(mainWindow), GetGDevice());
    GetPortBounds(GetWindowPort(mainWindow), &windowRect);
	
    //copy our offscreen world into the window 
	CopyBits((BitMap *) *(GetPortPixMap(mainGWorld->offscreen)),
			 GetPortBitMapForCopyBits(GetWindowPort(mainWindow)),
			 &mainGWorld->pictSize, 
			 &windowRect, 
			 srcCopy, 
			 nil);
	
    //we flush the window buffer by passing the window and its visible region
    QDFlushPortBuffer(GetWindowPort(mainWindow), 
					  GetPortVisibleRegion(GetWindowPort(mainWindow),
										   mainGWorld->visibleRgn));
	
    SetGWorld(SavePort, SaveGD);
}

void PPDisplayDevice::update(const PPRect& r)
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	GDHandle	SaveGD;
	CGrafPtr	SavePort;  
	Rect		windowRect;
	
    GetGWorld(&SavePort, &SaveGD);        
	
    //our current world must be the window, otherwise the offscreen buffer is drawn somewhere outside
    SetGWorld(GetWindowPort(mainWindow), GetGDevice());
    GetPortBounds(GetWindowPort(mainWindow), &windowRect);
    
	float xScale = (float)(windowRect.right - windowRect.left) / (float)getSize().width;
	float yScale = (float)(windowRect.bottom - windowRect.top) / (float)getSize().height;
	
	Rect srcRect;
	srcRect.top = r.y1;
	srcRect.left = r.x1;
	srcRect.bottom = r.y2;
	srcRect.right = r.x2;
	
	Rect dstRect;
	dstRect.top = (int)(srcRect.top*yScale);
	dstRect.left = (int)(srcRect.left*xScale);
	dstRect.bottom = (int)(srcRect.bottom*yScale);
	dstRect.right = (int)(srcRect.right*xScale);
	
    //copy our offscreen world into the window 
	CopyBits((BitMap *) *(GetPortPixMap(mainGWorld->offscreen)),
			 GetPortBitMapForCopyBits(GetWindowPort(mainWindow)),
			 &srcRect, 
			 &dstRect, 
			 srcCopy, 
			 nil);
	
    //we flush the window buffer by passing the window and its visible region
    QDFlushPortBuffer(GetWindowPort(mainWindow),
					  GetPortVisibleRegion(GetWindowPort(mainWindow),
										   mainGWorld->visibleRgn));
	
	SetGWorld(SavePort, SaveGD);
}

bool PPDisplayDevice::init()
{
	Rect bounds;
	
	RepositionWindow (mainWindow, NULL, kWindowCenterOnMainScreen);
	
    // The windows were created hidden so show them.
	ShowWindow(mainWindow);        
	
	GetWindowBounds (waitWindow, kWindowContentRgn, &bounds);	
	
	waitWindowWidth = bounds.right - bounds.left;
	waitWindowHeight = bounds.bottom - bounds.top;
	
	// setup wait screen offscreen buffers
	waitGWorld = new GWorldWrapper(waitWindowWidth, waitWindowHeight);
	return true;
}

void PPDisplayDevice::setTitle(const PPSystemString& title)
{
	CFStringRef	CFTitleRef = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);	
	SetWindowTitleWithCFString(mainWindow, CFTitleRef);
	CFRelease(CFTitleRef);
}

void PPDisplayDevice::setSize(const PPSize& size)
{
	SizeWindow(mainWindow, size.width * scaleFactor, size.height * scaleFactor, false);
	
	if (!bFullScreen)
		RepositionWindow (mainWindow, NULL, kWindowCenterOnMainScreen);
	
	this->size = size;
}

bool PPDisplayDevice::goFullScreen(bool b)
{
	OSStatus err;
	
	if (b && !bFullScreen)
		// switch display res
	{
		// switch screen resolution
		CGDirectDisplayID display=CGMainDisplayID();
		boolean_t exact;
		oldmode = CGDisplayCurrentMode(display);
		mode = CGDisplayBestModeForParametersAndRefreshRate(display, 32, size.width * scaleFactor, size.height * scaleFactor, 75.0, &exact);
		
		if (exact) 
		{
			HideWindow(mainWindow);
			err=CGDisplaySwitchToMode(display,mode);	
			
			HideMenuBar();
			
			Rect r1,r2;
			GetWindowBounds(mainWindow, kWindowStructureRgn, &oldRc);
			r1 = oldRc;
			r2 = r1;
			
			SInt32 dh = (r1.bottom - r1.top) - size.height * scaleFactor;
			
			r2.left-=r1.left;
			r2.top-=r1.top+dh;
			r2.right-=r1.left;
			r2.bottom-=r1.top+dh;
			SetWindowBounds(mainWindow, kWindowStructureRgn, &r2);
			bFullScreen = true;
			ShowWindow(mainWindow);
			return true;
		}
		return false;
	}
	else if (!b && bFullScreen)
	{
		HideWindow(mainWindow);
		// switch screen resolution
		CGDirectDisplayID display=CGMainDisplayID();
		err=CGDisplaySwitchToMode(display,oldmode);	
		ShowMenuBar();
		SetWindowBounds(mainWindow, kWindowStructureRgn, &oldRc);
		ShowWindow(mainWindow);
		bFullScreen = false;
		
		return true;
	}
	
	return false;
}

PPSize PPDisplayDevice::getDisplayResolution() const
{
	const int MAX_DISPLAYS = 32;
	
	CGDirectDisplayID displayArray [MAX_DISPLAYS];	
	CGDisplayCount numDisplays;
	
	CFNumberRef number;	
	CFBooleanRef booleanValue;
	
	long    height, width, refresh, mode, bpp, bps, spp, rowBytes, gui, ioflags;
	
	int     i;
	
	CGGetActiveDisplayList (MAX_DISPLAYS, displayArray, &numDisplays); 
	
	for (i = 0; i < numDisplays; i++) 
	{ 		
		width = CGDisplayPixelsWide (displayArray[i]);		
		height = CGDisplayPixelsHigh (displayArray[i]);		

		// we are only interested in display properties
		//bpp = CGDisplayBitsPerPixel (displayArray[i]);		
		//bps = CGDisplayBitsPerSample (displayArray[i]);		
		//spp = CGDisplaySamplesPerPixel (displayArray[i]);		
		//rowBytes = CGDisplayBytesPerRow (displayArray[i]);		
		
		//number = (CFNumberRef)CFDictionaryGetValue (CGDisplayCurrentMode (displayArray[i]), kCGDisplayMode);		
		//CFNumberGetValue (number, kCFNumberLongType, &mode);
		
		//number =(CFNumberRef)CFDictionaryGetValue (CGDisplayCurrentMode (displayArray[i]), kCGDisplayRefreshRate);		
		//CFNumberGetValue (number, kCFNumberLongType, &refresh);
		
		//booleanValue = (CFBooleanRef)CFDictionaryGetValue (CGDisplayCurrentMode(displayArray[i]), kCGDisplayModeUsableForDesktopGUI);		
		//CFNumberGetValue (number, kCFNumberLongType, &gui);
		
		//number = (CFNumberRef)CFDictionaryGetValue (CGDisplayCurrentMode (displayArray[i]), kCGDisplayIOFlags);		
		//CFNumberGetValue (number, kCFNumberLongType, &ioflags);		
	}
		
	return PPSize(width, height);
}

void PPDisplayDevice::shutDown()
{
	HICommandExtended aCommand;
	EventRef anEvent;
	
    // send quit event
    BlockZero(&aCommand, sizeof(aCommand));
    aCommand.attributes = kHICommandFromControl;
    aCommand.commandID = 'quit';
	
	CreateEvent(NULL,
				kEventClassCommand,
				kEventCommandProcess,
				GetCurrentEventTime(),
				kEventAttributeUserEvent,
				&anEvent);
				
	SetEventParameter(anEvent,
					  kEventParamDirectObject,
					  typeHICommand,
					  sizeof(aCommand),
					  &aCommand);
	
    SendEventToWindow(anEvent, mainWindow);
    ReleaseEvent(anEvent);
}

////////////////////////////////////////////////////////////////////////////////
// -------------------------- wait window rendering ---------------------------
////////////////////////////////////////////////////////////////////////////////
void DrawPixel32(void* buffer, pp_int32 x, pp_int32 y, pp_int32 pitch, const PPColor& color)
{
	unsigned char* buff = (unsigned char*)buffer;
	buff[y*pitch+x*4+1]=color.r;
	buff[y*pitch+x*4+2]=color.g;
	buff[y*pitch+x*4+3]=color.b;
}

void PPDisplayDevice::updateWaitWindow()
{
	GDHandle	SaveGD;
	CGrafPtr	SavePort;
	Rect		windowRect;
	
    GetGWorld(&SavePort, &SaveGD);        
	
    //our current world must be the window, otherwise the offscreen buffer is drawn somewhere outside
    SetGWorld(GetWindowPort(waitWindow), GetGDevice());
    GetPortBounds(GetWindowPort(waitWindow), &windowRect);
	
    //copy our offscreen world into the window 
    CopyBits ((BitMap *) *(GetPortPixMap(waitGWorld->offscreen)),
			  GetPortBitMapForCopyBits(GetWindowPort(waitWindow)),
			  &waitGWorld->pictSize, &windowRect, srcCopy, nil);
	
    //we flush the window buffer by passing the window and its visible region
    QDFlushPortBuffer(GetWindowPort(waitWindow),
					  GetPortVisibleRegion(GetWindowPort(waitWindow),
										   waitGWorld->visibleRgn));
	
    SetGWorld(SavePort, SaveGD);
}

void PPDisplayDevice::renderAndUpdateWaitWindow(pp_uint32 width, pp_uint32 height, const PPColor& waitBarColor)
{
	unsigned int pitch;
	unsigned char* buffer;
	
	waitGWorld->UnlockGWorldMemory(&buffer,&pitch);
	
	DrawWaitBar(width, height, 
				160, 16,
				0xFFFFFF, (waitBarColor.r << 16) + (waitBarColor.g << 8) + (waitBarColor.b),
				buffer, pitch, &DrawPixel32);
	
	waitGWorld->LockGWorldMemory();
	
	updateWaitWindow();
}

void* PPDisplayDevice::UpdateWaitWindowThreadProc(void* theID)
{
	PPDisplayDevice* thisDisplayDevice = (PPDisplayDevice*)theID;
	
	while (true)
	{		
		if (thisDisplayDevice->waitWindowVisible)
			thisDisplayDevice->renderAndUpdateWaitWindow(thisDisplayDevice->waitWindowWidth,
														 thisDisplayDevice->waitWindowHeight,
														 thisDisplayDevice->waitBarColor);
		
		System::msleep(100);
	}
	
	return NULL;
}

void PPDisplayDevice::signalWaitState(bool b, const PPColor& color)
{
	waitBarColor = color;
	
	if (b)
	{		
		// get main window boundaries
		Rect bounds;
		GetWindowBounds (mainWindow, kWindowContentRgn, &bounds);	
		
		// move wait window to fit in the middle of the main window
		MoveWindow (waitWindow, 
					bounds.left + ((bounds.right-bounds.left) >> 1) - (waitWindowWidth>>1),
					bounds.top + ((bounds.bottom-bounds.top) >> 1) - (waitWindowHeight>>1),
					false);
		
		renderAndUpdateWaitWindow(waitWindowWidth, waitWindowHeight, waitBarColor);
		
		ShowWindow(waitWindow);
		
		waitWindowVisible = true;
		
		if (!drawThread)
		{
			if (pthread_create(&drawThread, NULL, UpdateWaitWindowThreadProc , (void*)this))
			{
				// error
				printf("could not create thread");
			}
		}		
	}
	else
	{
		waitWindowVisible = false;
		HideWindow(waitWindow);
	}
}

void PPDisplayDevice::setMouseCursor(MouseCursorTypes type)
{
	currentCursorType = type;
	
	switch (type)
	{
		case MouseCursorTypeStandard:
			SetThemeCursor(kThemeArrowCursor);
			break;
			
		case MouseCursorTypeResizeLeft:
			SetThemeCursor(kThemeResizeLeftCursor);
			break;
			
		case MouseCursorTypeResizeRight:
			SetThemeCursor(kThemeResizeRightCursor);
			break;
			
		case MouseCursorTypeHand:
			SetThemeCursor(kThemeOpenHandCursor);
			break;
	}
}
