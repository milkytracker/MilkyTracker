/*
 *  PPOpenPanel_SDL.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Feb 27 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "PPOpenPanel.h"
#include <SDL/SDL.h>
#include "SDL_ModalLoop.h"
#include "RespondMessageBoxFileSelector.h"

PPOpenPanel::PPOpenPanel(PPScreen* screen, const char* caption) :
	PPModalDialog(screen)
{
	this->caption = new char[strlen(caption)+1];
	strcpy(this->caption, caption);
}

PPOpenPanel::~PPOpenPanel()
{
	if (caption)
		delete caption;
}

void PPOpenPanel::addExtension(const PPString& ext, const PPString& desc)
{
	Descriptor* d = new Descriptor(ext, desc);

	items.add(d);
}

PPOpenPanel::ReturnCodes PPOpenPanel::runModal()
{
	// Create a message box (the message box will invoke the responder)
	RespondMessageBoxFileSelector* respondMessageBox = new RespondMessageBoxFileSelector(screen, NULL, PP_DEFAULT_ID, this->caption);

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
