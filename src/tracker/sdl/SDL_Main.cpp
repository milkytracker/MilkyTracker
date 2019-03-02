/*
 *  tracker/sdl/SDL_Main.cpp
 *
 *  Copyright 2009 Peter Barth, Christopher O'Neill, Dale Whinham
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
 *  SDL_Main.cpp
 *  MilkyTracker SDL front end
 *
 *  Created by Peter Barth on 19.11.05.
 *
 *  12/5/14 - Dale Whinham
 *    - Port to SDL2
 *    - Removed SDLMain.m for Mac - no longer required
 *    - OSX: '-psn_xxx' commandline argument ignored if Finder passes it to the executable
 *    - Removed GP2X-specific stuff; I don't think SDL2 is available for this platform yet
 *    - Added X-Y mousewheel support - other MilkyTracker files have changed to support this
 *
 *    TODO: - Further cleanups - can we remove QTopia too?
 *          - Do we need that EEEPC segfault fix still with SDL2?
 *          - Look at the OpenGL stuff
 *
 *  15/2/08 - Peter Barth
 *  This code needs major clean up, there are too many workarounds going on
 *  for different platforms/configurations (MIDI, GP2X etc.)
 *  Please do not further pollute this single source code when possible
 *
 *  14/8/06 - Christopher O'Neill
 *  Ok, there are so many changes in this file that I've lost track...
 *  Here are some I remember:
 *    - ALSA Midi Support
 *    - GP2X mouse emulator (awaiting a rewrite one day..)
 *    - Various command line options
 *    - Fix for french azerty keyboards (their number keys are shifted)
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

#include <SDL.h>
#include "SDL_KeyTranslation.h"
// ---------------------------- Tracker includes ----------------------------
#include "PPUI.h"
#include "DisplayDevice_SDL.h"
#include "DisplayDeviceFB_SDL.h"
#include "Screen.h"
#include "Tracker.h"
#include "PPMutex.h"
#include "PPSystem_POSIX.h"
#include "PPPath_POSIX.h"

#ifdef HAVE_LIBASOUND
#include "../midi/posix/MidiReceiver_pthread.h"
#endif
// --------------------------------------------------------------------------

static SDL_TimerID			timer;

// Tracker globals
static PPScreen*			myTrackerScreen		= NULL;
static Tracker*				myTracker			= NULL;
static PPDisplayDevice*		myDisplayDevice		= NULL;
#ifdef HAVE_LIBASOUND
static MidiReceiver*		myMidiReceiver		= NULL;
#endif

// Okay what else do we need?
PPMutex*			globalMutex				= NULL;
static bool			ticking					= false;

struct MouseState {
	pp_uint32 myTime;
	PPPoint lastClickPosition;
	pp_uint16 clickCount;
	bool mouseDown;
	pp_uint32 buttonDownStartTime;
};

static MouseState mouseLeft = { 0, PPPoint(0,0), 0, false, 0 };
static MouseState mouseRight = { 0, PPPoint(0,0), 0, false, 0 };
static MouseState mouseMiddle = { 0, PPPoint(0,0), 0, false, 0 };

static pp_uint32	timerTicker				= 0;

static PPPoint		p;

// This needs to be visible from outside
pp_uint32 PPGetTickCount()
{
	return SDL_GetTicks();
}

// Same as above
void QueryKeyModifiers()
{
	pp_uint32 mod = SDL_GetModState();

	if((mod & KMOD_LSHIFT) || (mod & KMOD_RSHIFT))
		setKeyModifier(KeyModifierSHIFT);
	else
		clearKeyModifier(KeyModifierSHIFT);
#ifndef __APPLE__
	if((mod & KMOD_LCTRL) || (mod & KMOD_RCTRL))
#else
	if((mod & KMOD_LGUI) || (mod & KMOD_RGUI))
#endif
		setKeyModifier(KeyModifierCTRL);
	else
		clearKeyModifier(KeyModifierCTRL);

	if((mod & KMOD_LALT) || (mod & KMOD_RALT))
		setKeyModifier(KeyModifierALT);
	else
		clearKeyModifier(KeyModifierALT);
}

static void RaiseEventSerialized(PPEvent* event)
{
	if (myTrackerScreen && myTracker)
	{
		globalMutex->lock();
		myTrackerScreen->raiseEvent(event);
		globalMutex->unlock();
	}
}

enum SDLUserEvents
{
	SDLUserEventTimer,
	SDLUserEventLMouseRepeat,
	SDLUserEventRMouseRepeat,
	SDLUserEventMMouseRepeat,
	SDLUserEventMidiKeyDown,
	SDLUserEventMidiKeyUp,
};

static Uint32 SDLCALL timerCallback(Uint32 interval, void* param)
{
	if (!myTrackerScreen || !myTracker || !ticking)
	{
		return interval;
	}

	SDL_UserEvent ev;
	ev.type = SDL_USEREVENT;

	if (!(timerTicker % 1))
	{
		ev.code = SDLUserEventTimer;
		SDL_PushEvent((SDL_Event*)&ev);

		//PPEvent myEvent(eTimer);
		//RaiseEventSerialized(&myEvent);
	}

	timerTicker++;

	if (mouseLeft.mouseDown &&
		(timerTicker - mouseLeft.buttonDownStartTime) > 25)
	{
		ev.code = SDLUserEventLMouseRepeat;
		ev.data1 = reinterpret_cast<void*>(p.x);
		ev.data2 = reinterpret_cast<void*>(p.y);
		SDL_PushEvent((SDL_Event*)&ev);

		//PPEvent myEvent(eLMouseRepeat, &p, sizeof(PPPoint));
		//RaiseEventSerialized(&myEvent);
	}

	if (mouseRight.mouseDown &&
		(timerTicker - mouseRight.buttonDownStartTime) > 25)
	{
		ev.code = SDLUserEventRMouseRepeat;
		ev.data1 = reinterpret_cast<void*>(p.x);
		ev.data2 = reinterpret_cast<void*>(p.y);
		SDL_PushEvent((SDL_Event*)&ev);

		//PPEvent myEvent(eRMouseRepeat, &p, sizeof(PPPoint));
		//RaiseEventSerialized(&myEvent);
	}

	if (mouseMiddle.mouseDown &&
		(timerTicker - mouseMiddle.buttonDownStartTime) > 25)
	{
		ev.code = SDLUserEventMMouseRepeat;
		ev.data1 = reinterpret_cast<void*>(p.x);
		ev.data2 = reinterpret_cast<void*>(p.y);
		SDL_PushEvent((SDL_Event*)&ev);

		//PPEvent myEvent(eRMouseRepeat, &p, sizeof(PPPoint));
		//RaiseEventSerialized(&myEvent);
	}

	return interval;
}

#ifdef HAVE_LIBASOUND
class MidiEventHandler : public MidiReceiver::MidiEventHandler
{
public:
	virtual void keyDown(int note, int volume)
	{
		SDL_UserEvent ev;
		ev.type = SDL_USEREVENT;
		ev.code = SDLUserEventMidiKeyDown;
		ev.data1 = (void*)note;
		ev.data2 = (void*)volume;
		SDL_PushEvent((SDL_Event*)&ev);

		//globalMutex->lock();
		//myTracker->sendNoteDown(note, volume);
		//globalMutex->unlock();
	}

	virtual void keyUp(int note)
	{
		SDL_UserEvent ev;
		ev.type = SDL_USEREVENT;
		ev.code = SDLUserEventMidiKeyUp;
		ev.data1 = (void*)note;
		SDL_PushEvent((SDL_Event*)&ev);

		//globalMutex->lock();
		//myTracker->sendNoteUp(note);
		//globalMutex->unlock();
	}
} midiEventHandler;


void StopMidiRecording()
{
	if (myMidiReceiver)
	{
		myMidiReceiver->stopRecording();
	}
}

void StartMidiRecording(unsigned int devID)
{
	if (devID == (unsigned)-1)
		return;

	StopMidiRecording();

	myMidiReceiver = new MidiReceiver(midiEventHandler);
	if (!myMidiReceiver->startRecording(devID))
	{
		// Deal with error
		fprintf(stderr, "Failed to initialise ALSA MIDI support.\n");
	}
}

void InitMidi()
{
	StartMidiRecording(0);
}
#endif

void translateMouseDownEvent(pp_int32 mouseButton, pp_int32 localMouseX, pp_int32 localMouseY)
{
	if (mouseButton > 3 || !mouseButton)
		return;

	myDisplayDevice->transform(localMouseX, localMouseY);

	p.x = localMouseX;
	p.y = localMouseY;

	// -----------------------------
	if (mouseButton == 1)
	{
		PPEvent myEvent(eLMouseDown, &p, sizeof(PPPoint));

		RaiseEventSerialized(&myEvent);

		mouseLeft.mouseDown = true;
		mouseLeft.buttonDownStartTime = timerTicker;

		if (!mouseLeft.clickCount)
		{
			mouseLeft.myTime = PPGetTickCount();
			mouseLeft.lastClickPosition.x = localMouseX;
			mouseLeft.lastClickPosition.y = localMouseY;
		}
		else if (mouseLeft.clickCount == 2)
		{
			pp_uint32 deltat = PPGetTickCount() - mouseLeft.myTime;

			if (deltat > 500)
			{
				mouseLeft.clickCount = 0;
				mouseLeft.myTime = PPGetTickCount();
				mouseLeft.lastClickPosition.x = localMouseX;
				mouseLeft.lastClickPosition.y = localMouseY;
			}
		}

		mouseLeft.clickCount++;

	}
	else if (mouseButton == 2)
	{
		PPEvent myEvent(eMMouseDown, &p, sizeof(PPPoint));

		RaiseEventSerialized(&myEvent);

		mouseMiddle.mouseDown = true;
		mouseMiddle.buttonDownStartTime = timerTicker;

		if (!mouseMiddle.clickCount)
		{
			mouseMiddle.myTime = PPGetTickCount();
			mouseMiddle.lastClickPosition.x = localMouseX;
			mouseMiddle.lastClickPosition.y = localMouseY;
		}
		else if (mouseMiddle.clickCount == 2)
		{
			pp_uint32 deltat = PPGetTickCount() - mouseRight.myTime;

			if (deltat > 500)
			{
				mouseMiddle.clickCount = 0;
				mouseMiddle.myTime = PPGetTickCount();
				mouseMiddle.lastClickPosition.x = localMouseX;
				mouseMiddle.lastClickPosition.y = localMouseY;
			}
		}

		mouseMiddle.clickCount++;
	}
	else if (mouseButton == 3)
	{
		PPEvent myEvent(eRMouseDown, &p, sizeof(PPPoint));

		RaiseEventSerialized(&myEvent);

		mouseRight.mouseDown = true;
		mouseRight.buttonDownStartTime = timerTicker;

		if (!mouseRight.clickCount)
		{
			mouseRight.myTime = PPGetTickCount();
			mouseRight.lastClickPosition.x = localMouseX;
			mouseRight.lastClickPosition.y = localMouseY;
		}
		else if (mouseRight.clickCount == 2)
		{
			pp_uint32 deltat = PPGetTickCount() - mouseRight.myTime;

			if (deltat > 500)
			{
				mouseRight.clickCount = 0;
				mouseRight.myTime = PPGetTickCount();
				mouseRight.lastClickPosition.x = localMouseX;
				mouseRight.lastClickPosition.y = localMouseY;
			}
		}

		mouseRight.clickCount++;
	}
}

void translateMouseUpEvent(pp_int32 mouseButton, pp_int32 localMouseX, pp_int32 localMouseY)
{
	if (mouseButton > 3 || !mouseButton)
		return;

	myDisplayDevice->transform(localMouseX, localMouseY);

	p.x = localMouseX;
	p.y = localMouseY;

	// -----------------------------
	if (mouseButton == 1)
	{
		mouseLeft.clickCount++;

		if (mouseLeft.clickCount >= 4)
		{
			pp_uint32 deltat = PPGetTickCount() - mouseLeft.myTime;

			if (deltat < 500)
			{
				p.x = localMouseX; p.y = localMouseY;
				if (abs(p.x - mouseLeft.lastClickPosition.x) < 4 &&
					abs(p.y - mouseLeft.lastClickPosition.y) < 4)
				{
					PPEvent myEvent(eLMouseDoubleClick, &p, sizeof(PPPoint));
					RaiseEventSerialized(&myEvent);
				}
			}

			mouseLeft.clickCount = 0;
		}

		p.x = localMouseX; p.y = localMouseY;
		PPEvent myEvent(eLMouseUp, &p, sizeof(PPPoint));
		RaiseEventSerialized(&myEvent);
		mouseLeft.mouseDown = false;
	}
	else if (mouseButton == 2)
	{
		mouseMiddle.clickCount++;

		if (mouseMiddle.clickCount >= 4)
		{
			pp_uint32 deltat = PPGetTickCount() - mouseMiddle.myTime;

			if (deltat < 500)
			{
				p.x = localMouseX; p.y = localMouseY;
				if (abs(p.x - mouseMiddle.lastClickPosition.x) < 4 &&
					abs(p.y - mouseMiddle.lastClickPosition.y) < 4)
				{
					PPEvent myEvent(eMMouseDoubleClick, &p, sizeof(PPPoint));
					RaiseEventSerialized(&myEvent);
				}
			}

			mouseMiddle.clickCount = 0;
		}

		p.x = localMouseX; p.y = localMouseY;
		PPEvent myEvent(eMMouseUp, &p, sizeof(PPPoint));
		RaiseEventSerialized(&myEvent);
		mouseMiddle.mouseDown = false;
	}
	else if (mouseButton == 3)
	{
		mouseRight.clickCount++;

		if (mouseRight.clickCount >= 4)
		{
			pp_uint32 deltat = PPGetTickCount() - mouseRight.myTime;

			if (deltat < 500)
			{
				p.x = localMouseX; p.y = localMouseY;
				if (abs(p.x - mouseRight.lastClickPosition.x) < 4 &&
					abs(p.y - mouseRight.lastClickPosition.y) < 4)
				{
					PPEvent myEvent(eRMouseDoubleClick, &p, sizeof(PPPoint));
					RaiseEventSerialized(&myEvent);
				}
			}

			mouseRight.clickCount = 0;
		}

		p.x = localMouseX; p.y = localMouseY;
		PPEvent myEvent(eRMouseUp, &p, sizeof(PPPoint));
		RaiseEventSerialized(&myEvent);
		mouseRight.mouseDown = false;
	}
}

void translateMouseWheelEvent(pp_int32 wheelX, pp_int32 wheelY) {
	TMouseWheelEventParams mouseWheelParams;

	// Deltas from wheel event
	mouseWheelParams.deltaX = wheelX;
	mouseWheelParams.deltaY = wheelY;

	// Use last stored coordinates
	mouseWheelParams.pos.x = p.x;
	mouseWheelParams.pos.y = p.y;

	PPEvent myEvent(eMouseWheelMoved, &mouseWheelParams, sizeof(mouseWheelParams));
	RaiseEventSerialized(&myEvent);
}

void translateMouseMoveEvent(pp_uint32 mouseState, pp_int32 localMouseX, pp_int32 localMouseY)
{
	myDisplayDevice->transform(localMouseX, localMouseY);

	p.x = localMouseX;
	p.y = localMouseY;

	if (mouseState == 0)
	{
		PPEvent myEvent(eMouseMoved, &p, sizeof(PPPoint));
		RaiseEventSerialized(&myEvent);
	}
	else
	{
		if (mouseState & ~(SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
		{
			return;
		}

		if (mouseState == SDL_BUTTON_LMASK && mouseLeft.mouseDown)
		{
			PPEvent myEvent(eLMouseDrag, &p, sizeof(PPPoint));
			RaiseEventSerialized(&myEvent);
		}
		else if (mouseState == SDL_BUTTON_RMASK && mouseRight.mouseDown)
		{
			PPEvent myEvent(eRMouseDrag, &p, sizeof(PPPoint));
			RaiseEventSerialized(&myEvent);
		}
	}
}

void preTranslateKey(SDL_Keysym& keysym)
{
	// Rotate cursor keys if necessary
	switch (myDisplayDevice->getOrientation())
	{
		case PPDisplayDevice::ORIENTATION_ROTATE90CW:
			switch (keysym.sym)
			{
				case SDLK_UP:
					keysym.sym = SDLK_LEFT;
					break;
				case SDLK_DOWN:
					keysym.sym = SDLK_RIGHT;
					break;
				case SDLK_LEFT:
					keysym.sym = SDLK_DOWN;
					break;
				case SDLK_RIGHT:
					keysym.sym = SDLK_UP;
					break;
			}
			break;

		case PPDisplayDevice::ORIENTATION_ROTATE90CCW:
			switch (keysym.sym)
			{
				case SDLK_DOWN:
					keysym.sym = SDLK_LEFT;
					break;
				case SDLK_UP:
					keysym.sym = SDLK_RIGHT;
					break;
				case SDLK_RIGHT:
					keysym.sym = SDLK_DOWN;
					break;
				case SDLK_LEFT:
					keysym.sym = SDLK_UP;
					break;
			}
			break;
			// ROTATE180 and UNKNOWN not handled
			default: break;
	}

}

void translateTextInputEvent(const SDL_Event& event)
{
#ifdef DEBUG
	printf ("DEBUG: Text input: %s\n", event.text.text);
#endif

	char character = event.text.text[0];

	// Only deal with ASCII characters
	if (character >= 32 && character <= 127)
	{
		PPEvent myEvent(eKeyChar, &character, sizeof(character));
		RaiseEventSerialized(&myEvent);
	}
}

void translateKeyDownEvent(const SDL_Event& event)
{
	SDL_Keysym keysym = event.key.keysym;

	// ALT+RETURN = Fullscreen toggle
	if (keysym.sym == SDLK_RETURN && (keysym.mod & KMOD_LALT))
	{
		PPEvent myEvent(eFullScreen);
		RaiseEventSerialized(&myEvent);
		return;
	}

	preTranslateKey(keysym);

#ifdef DEBUG
	printf ("DEBUG: Key pressed: VK: %d, SC: %d, Scancode: %d\n", toVK(keysym), toSC(keysym), keysym.sym);
#endif

	pp_uint16 chr[3] = {toVK(keysym), toSC(keysym), static_cast<pp_uint16> (keysym.sym)};

	PPEvent myEvent(eKeyDown, &chr, sizeof(chr));
	RaiseEventSerialized(&myEvent);
}

void translateKeyUpEvent(const SDL_Event& event)
{
	SDL_Keysym keysym = event.key.keysym;

	preTranslateKey(keysym);

	pp_uint16 chr[3] = {toVK(keysym), toSC(keysym), static_cast<pp_uint16> (keysym.sym)};

	PPEvent myEvent(eKeyUp, &chr, sizeof(chr));
	RaiseEventSerialized(&myEvent);
}

void processSDLEvents(const SDL_Event& event)
{
	pp_uint32 mouseButton = 0;

	switch (event.type)
	{
		case SDL_MOUSEBUTTONDOWN:
			mouseButton = event.button.button;
			translateMouseDownEvent(mouseButton, event.button.x, event.button.y);
			break;

		case SDL_MOUSEBUTTONUP:
			mouseButton = event.button.button;
			translateMouseUpEvent(mouseButton, event.button.x, event.button.y);
			break;

		case SDL_MOUSEMOTION:
			translateMouseMoveEvent(event.motion.state, event.motion.x, event.motion.y);
			break;

		case SDL_MOUSEWHEEL:
			translateMouseWheelEvent(event.wheel.x, event.wheel.y);
			break;

		case SDL_TEXTINPUT:
			translateTextInputEvent(event);
			break;

		case SDL_KEYDOWN:
			translateKeyDownEvent(event);
			break;

		case SDL_KEYUP:
			translateKeyUpEvent(event);
			break;
	}
}

void processSDLUserEvents(const SDL_UserEvent& event)
{
	union {
		void *ptr;
		pp_int32 i32;
	} data1, data2;
	data1.ptr = event.data1;
	data2.ptr = event.data2;

	switch (event.code)
	{
		case SDLUserEventTimer:
		{
			// Prevent new timer events being pushed while we are processing the current one
			ticking = false;
			PPEvent myEvent(eTimer);
			RaiseEventSerialized(&myEvent);
			ticking = true;
			break;
		}

		case SDLUserEventLMouseRepeat:
		{
			PPPoint p;
			p.x = data1.i32;
			p.y = data2.i32;
			PPEvent myEvent(eLMouseRepeat, &p, sizeof(PPPoint));
			RaiseEventSerialized(&myEvent);
			break;
		}

		case SDLUserEventRMouseRepeat:
		{
			PPPoint p;
			p.x = data1.i32;
			p.y = data2.i32;
			PPEvent myEvent(eRMouseRepeat, &p, sizeof(PPPoint));
			RaiseEventSerialized(&myEvent);
			break;
		}

		case SDLUserEventMidiKeyDown:
		{
			pp_int32 note = data1.i32;
			pp_int32 volume = data2.i32;
			globalMutex->lock();
			myTracker->sendNoteDown(note, volume);
			globalMutex->unlock();
			break;
		}

		case SDLUserEventMidiKeyUp:
		{
			pp_int32 note = data1.i32;
			globalMutex->lock();
			myTracker->sendNoteUp(note);
			globalMutex->unlock();
			break;
		}

	}
}

#ifdef __unix__
void crashHandler(int signum)
{
	// Save backup.xm
	static char buffer[1024]; // Should be enough :p
	strncpy(buffer, getenv("HOME"), 1010);
	strcat(buffer, "/BACKUP00.XM");
	struct stat statBuf;
	int num = 1;
	while(stat(buffer, &statBuf) == 0 && num <= 100)
		snprintf(buffer, sizeof(buffer), "%s/BACKUP%02i.XM", getenv("HOME"), num++);

	if (signum == 15)
	{
		fprintf(stderr, "\nTERM signal received.\n");
		SDL_Quit();
		return;
	}
	else
	{
		fprintf(stderr, "\nCrashed with signal %i\n"
				"Please submit a bug report stating exactly what you were doing "
				"at the time of the crash, as well as the above signal number. "
				"Also note if it is possible to reproduce this crash.\n", signum);
	}

	if (num != 100)
	{
		if (myTracker->saveModule(buffer) == MP_DEVICE_ERROR)
		{
			fprintf(stderr, "\nUnable to save backup (read-only filesystem?)\n\n");
		}
		else
		{
			fprintf(stderr, "\nA backup has been saved to %s\n\n", buffer);
		}
	}

	// Try and quit SDL
	SDL_Quit();
}
#endif

void initTracker(pp_uint32 bpp, PPDisplayDevice::Orientations orientation,
				 bool swapRedBlue, bool noSplash)
{
	// Initialize SDL
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	// Enable drag and drop
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

#if (defined(unix) || defined(__unix__) || defined(_AIX) || defined(__OpenBSD__)) && \
	(!defined(__CYGWIN32__) && !defined(ENABLE_NANOX) && \
	 !defined(__QNXNTO__) && !defined(__AROS__))

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
#endif

	// ------------ Initialise tracker ---------------
	myTracker = new Tracker();

	PPSize windowSize = myTracker->getWindowSizeFromDatabase();
	bool fullScreen = myTracker->getFullScreenFlagFromDatabase();
	pp_int32 scaleFactor = myTracker->getScreenScaleFactorFromDatabase();

#ifdef __LOWRES__
	windowSize.width = DISPLAYDEVICE_WIDTH;
	windowSize.height = DISPLAYDEVICE_HEIGHT;
#endif

#ifdef __OPENGL__
	myDisplayDevice = new PPDisplayDeviceOGL(windowSize.width, windowSize.height, scaleFactor, bpp, fullScreen, orientation, swapRedBlue);
#else
	myDisplayDevice = new PPDisplayDeviceFB(windowSize.width, windowSize.height, scaleFactor,
											bpp, fullScreen, orientation, swapRedBlue);
#endif

	SDL_SetWindowTitle(myDisplayDevice->getWindow(), "Loading MilkyTracker...");
	myDisplayDevice->init();

	myTrackerScreen = new PPScreen(myDisplayDevice, myTracker);
	myTracker->setScreen(myTrackerScreen);

	// Kickstart SDL event loop early so that the splash screen is made visible
	SDL_PumpEvents();

	// Startup procedure
	myTracker->startUp(noSplash);

#ifdef HAVE_LIBASOUND
	InitMidi();
#endif

	// Try to create timer
	timer = SDL_AddTimer(20, timerCallback, NULL);

	// Start capturing text input events
	SDL_StartTextInput();

	ticking = true;
}

static bool done;

void exitSDLEventLoop(bool serializedEventInvoked/* = true*/)
{
	PPEvent event(eAppQuit);
	RaiseEventSerialized(&event);

	// it's necessary to make this mutex lock because the SDL modal event loop
	// used in the modal dialogs expects modal dialogs to be invoked by
	// events within these mutex lock calls
	if (!serializedEventInvoked)
		globalMutex->lock();

	bool res = myTracker->shutDown();

	if (!serializedEventInvoked)
		globalMutex->unlock();

	if (res)
		done = 1;
}

void SendFile(char *file)
{
	PPSystemString finalFile(file);
	PPSystemString* strPtr = &finalFile;

	PPEvent event(eFileDragDropped, &strPtr, sizeof(PPSystemString*));
	RaiseEventSerialized(&event);
}

#if defined(__PSP__)
extern "C" int SDL_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	SDL_Event event;
	char *loadFile = 0;

	pp_int32 defaultBPP = -1;
	PPDisplayDevice::Orientations orientation = PPDisplayDevice::ORIENTATION_NORMAL;
	bool swapRedBlue = false, noSplash = false;
	bool recVelocity = false;

	// Parse command line
	while ( argc > 1 )
	{
		--argc;

#ifdef __APPLE__
		// OSX: Swallow "-psn_xxx" argument passed by Finder on OSX <10.9
		if ( strncmp(argv[argc], "-psn", 4) == 0 )
		{
			continue;
		}
		else
#endif
		if ( strcmp(argv[argc-1], "-bpp") == 0 )
		{
			defaultBPP = atoi(argv[argc]);
			--argc;
		}
		else if ( strcmp(argv[argc], "-nosplash") == 0 )
		{
			noSplash = true;
		}
		else if ( strcmp(argv[argc], "-swap") == 0 )
		{
			swapRedBlue = true;
		}
		else if ( strcmp(argv[argc-1], "-orientation") == 0 )
		{
			if (strcmp(argv[argc], "NORMAL") == 0)
			{
				orientation = PPDisplayDevice::ORIENTATION_NORMAL;
			}
			else if (strcmp(argv[argc], "ROTATE90CCW") == 0)
			{
				orientation = PPDisplayDevice::ORIENTATION_ROTATE90CCW;
			}
			else if (strcmp(argv[argc], "ROTATE90CW") == 0)
			{
				orientation = PPDisplayDevice::ORIENTATION_ROTATE90CW;
			}
			else
				goto unrecognizedCommandLineSwitch;
			--argc;
		}
		else if ( strcmp(argv[argc], "-recvelocity") == 0)
		{
			recVelocity = true;
		}
		else
		{
unrecognizedCommandLineSwitch:
			if (argv[argc][0] == '-')
			{
				fprintf(stderr,
						"Usage: %s [-bpp N] [-swap] [-orientation NORMAL|ROTATE90CCW|ROTATE90CW] [-nosplash] [-recvelocity]\n", argv[0]);
				exit(1);
			}
			else
			{
				loadFile = argv[argc];
			}
		}
	}

	globalMutex = new PPMutex();

	// Store current working path (init routine is likely to change it)
	PPPath_POSIX path;
	PPSystemString oldCwd = path.getCurrent();

	globalMutex->lock();
	initTracker(defaultBPP, orientation, swapRedBlue, noSplash);
	globalMutex->unlock();

#ifdef HAVE_LIBASOUND
	if (myMidiReceiver && recVelocity)
	{
		myMidiReceiver->setRecordVelocity(true);
	}
#endif

	if (loadFile)
	{
		PPSystemString newCwd = path.getCurrent();
		path.change(oldCwd);
		SendFile(loadFile);
		path.change(newCwd);
		pp_uint16 chr[3] = {VK_RETURN, 0, 0};
		PPEvent event(eKeyDown, &chr, sizeof(chr));
		RaiseEventSerialized(&event);
	}

	// Main event loop
	done = 0;
	while (!done && SDL_WaitEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exitSDLEventLoop(false);
				break;
			case SDL_MOUSEMOTION:
			{
				// Ignore old mouse motion events in the event queue
				SDL_Event new_event;

				if (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0)
				{
					while (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0);
					processSDLEvents(new_event);
				}
				else
				{
					processSDLEvents(event);
				}
				break;
			}

			// Open modules drag 'n dropped onto MilkyTracker (currently only works on Dock icon, OSX)
			case SDL_DROPFILE:
				SendFile(event.drop.file);
				SDL_free(event.drop.file);
				break;

			// Refresh GUI if window is unhidden or resized
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
                    case SDL_WINDOWEVENT_EXPOSED:
					case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SHOWN:
						myTrackerScreen->update();
				}
				break;

			case SDL_USEREVENT:
				processSDLUserEvents((const SDL_UserEvent&)event);
				break;

			default:
				processSDLEvents(event);
				break;
		}
	}

	ticking = false;
	SDL_RemoveTimer(timer);

	globalMutex->lock();
#ifdef HAVE_LIBASOUND
	delete myMidiReceiver;
#endif
	delete myTracker;
	myTracker = NULL;
	delete myTrackerScreen;
	myTrackerScreen = NULL;
	delete myDisplayDevice;
	globalMutex->unlock();
	SDL_Quit();
	delete globalMutex;

	return 0;
}
