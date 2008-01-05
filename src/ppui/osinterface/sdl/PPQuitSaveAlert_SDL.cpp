/*
 *  PPQuitSaveAlert_SDL.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "PPQuitSaveAlert.h"
#include <SDL/SDL.h>
#include "SDL_ModalLoop.h"
#include "RespondMessageBoxFileSelector.h"

PPQuitSaveAlert::ReturnCodes PPQuitSaveAlert::runModal()
{
	// Create a message box (the message box will invoke the responder)
	RespondMessageBox* respondMessageBox = new RespondMessageBox(screen, NULL, PP_DEFAULT_ID, "Save current changes?", RespondMessageBox::MessageBox_YESNOCANCEL);

	ReturnCodes result = SDL_runModalLoop(screen, respondMessageBox);
	
	delete respondMessageBox;
	
	return result;
}
