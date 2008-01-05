/*
 *  PPMessageBox_SDL.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.09.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "PPMessageBox.h"
#include <SDL/SDL.h>
#include "SDL_ModalLoop.h"
#include "RespondMessageBoxFileSelector.h"

PPMessageBox::ReturnCodes PPMessageBox::runModal()
{
	// Convert texts
	char* captionASCIIZ = this->caption.toASCIIZ();
	char* contentASCIIZ = this->content.toASCIIZ();
	PPString caption(captionASCIIZ);
	PPString content(contentASCIIZ);
	delete[] captionASCIIZ;
	delete[] contentASCIIZ;

	// Create a message box (the message box will invoke the responder)
	RespondMessageBox* respondMessageBox = new RespondMessageBox(screen, NULL, PP_DEFAULT_ID, caption, content);

	ReturnCodes result = SDL_runModalLoop(screen, respondMessageBox);
	
	delete respondMessageBox;
	
	return result;
}
