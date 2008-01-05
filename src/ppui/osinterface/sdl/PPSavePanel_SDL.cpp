/*
 *  PPSavePanel.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 05 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */
 
#include "PPSavePanel.h"
#include <SDL/SDL.h>
#include "SDL_ModalLoop.h"
#include "RespondMessageBoxFileSelector.h"

PPSavePanel::ReturnCodes PPSavePanel::runModal()
{
	// Create a message box (the message box will invoke the responder)
	RespondMessageBoxFileSelector* respondMessageBox = new RespondMessageBoxFileSelector(screen, NULL, PP_DEFAULT_ID, this->caption, true, true);
	respondMessageBox->setCurrentEditFileName(defaultFileName);

	for (pp_int32 i = 0; i < items.size(); i++)
	{
		PPSystemString ext(items.get(i)->extension);
		PPSystemString desc(items.get(i)->description);
		respondMessageBox->addExtension(ext, desc);
	}

	ReturnCodes result = SDL_runModalLoop(screen, respondMessageBox);

	PPSystemString pathEntry(respondMessageBox->getSelectedPathFull());
	
	// this is no longer needed
	delete respondMessageBox;
	
	fileName = ((result == ReturnCodeOK) ? pathEntry : ""); 
	
	return result;
}
