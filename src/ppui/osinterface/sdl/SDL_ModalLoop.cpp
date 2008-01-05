/*
 *  SDL_ModalLoop.cpp
 *  PPUI SDL
 *
 *  Created by Peter Barth on 29.03.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <SDL/SDL.h>
#include "SDL_ModalLoop.h"
#include "Event.h"
#include "Screen.h"
#include "RespondMessageBox.h"
#include "PPMutex.h"

void processSDLEvents(const SDL_Event& event);

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
PPModalDialog::ReturnCodes SDL_runModalLoop(PPScreen* screen, RespondMessageBox* respondMessageBox)
{
	bool exitModalLoop = false;
	SDL_Event event;
	// This is the responder for buttons invoked by the modal dialog
	ModalLoopResponder modalLoopResponder(exitModalLoop);

	respondMessageBox->setResponder(&modalLoopResponder);

	// Detach the event listener from the screen, this will actually detach the entire tracker from the screen
	EventListenerInterface* eventListener = screen->detachEventListener();

	// Instantinate our own event listener
	CustomEventListener customEventListener(eventListener);
	
	// Attach it
	screen->attachEventListener(&customEventListener);

	// Show it
	respondMessageBox->show();

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
				
				if (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0) 
				{
					while (SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0);
					processSDLEvents(new_event);
				} 
				else 
				{
					processSDLEvents(event);
				}
				break;
			}

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
	
	return modalLoopResponder.getReturnCode();
}
