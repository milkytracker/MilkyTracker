/*
 *  ppui/osinterface/sdl/SDL_ModalLoop.cpp
 *
 *  Copyright 2009 Peter Barth, Dale Whinham
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
 *  SDL_ModalLoop.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 29.03.06.
 *
 *  12/5/14 - Dale Whinham
 *    - Port to SDL2
 */

#include <SDL.h>
#include "SDL_ModalLoop.h"
#include "Event.h"
#include "Screen.h"
#include "DialogBase.h"
#include "PPMutex.h"

void processSDLEvents(const SDL_Event& event);
void processSDLUserEvents(const SDL_UserEvent& event);

extern PPMutex* globalMutex;

/////////////////////////////////////////////////////////////////
//
// Okay here goes a quick description of how a modal dialog
// using the SDL is realized:
// So when this modal dialog is invoked, we ARE actually between
// two globalLoop mutex calls, because we were invoked
// by some mouse click or other event
// So now we first attach a new event listener to our screen 
// object (so that all events sent end up in our own listener)
// and install our own event handler loop... But this can only
// be done when the globalMutex is unlocked first and locked 
// after...
//
/////////////////////////////////////////////////////////////////
PPModalDialog::ReturnCodes SDL_runModalLoop(PPScreen* screen, PPDialogBase* dialog)
{
	bool exitModalLoop = false;
	SDL_Event event;

	// screen might be disabled in a stackable fashion
	pp_uint32 screenEnableStackCount = 0;
	while (!screen->isDisplayEnabled())
	{
		screen->enableDisplay(true);
		screenEnableStackCount++;
	}

	// This is the responder for buttons invoked by the modal dialog
	ModalLoopResponder modalLoopResponder(exitModalLoop);

	dialog->setResponder(&modalLoopResponder);

	// Detach the event listener from the screen, this will actually detach the entire tracker from the screen
	EventListenerInterface* eventListener = screen->detachEventListener();

	// Instantinate our own event listener
	CustomEventListener customEventListener(eventListener);
	
	// Attach it
	screen->attachEventListener(&customEventListener);

	// Show it
	dialog->show();

	// Now to the tricky part, since a modal dialog has been invoked through a OS event, globalMutex is in locked state
	// so to allow further event processing we must unlock the mutex first
	// -- really messy and critical --
	if (globalMutex)
		globalMutex->unlock();

	// Create our own event loop
	while (!exitModalLoop && SDL_WaitEvent(&event)) 
	{
		switch (event.type) 
		{
			case SDL_MOUSEMOTION:
			{
				// ignore old mouse motion events in the event queue
				SDL_Event new_event;
#if defined(__AMIGA__) || defined(__AROS__) //SDL-1.2
				if (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0)
				{
					while (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0);
#else
				if (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0)
				{
					while (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0);
#endif
					processSDLEvents(new_event);
				} 
				else 
				{
					processSDLEvents(event);
				}
				break;
			}
			
			case SDL_USEREVENT:
				processSDLUserEvents((const SDL_UserEvent&)event);
				break;
			
			default:
				processSDLEvents(event);
				break;
		}
	}	

	// pretend nothing happened at all, continue with main event loop after we're finished here
	if (globalMutex)
		globalMutex->lock();

	// re-attach tracker
	screen->attachEventListener(eventListener);	

	// if screen was disabled we enable it again
	while (screenEnableStackCount > 0)
	{
		screen->enableDisplay(false);
		screenEnableStackCount--;
	}
	
	return modalLoopResponder.getReturnCode();
}
