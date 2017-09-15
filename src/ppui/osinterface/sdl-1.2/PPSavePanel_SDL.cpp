/*
 *  ppui/osinterface/sdl/PPSavePanel_SDL.cpp
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

/*
 *  PPSavePanel.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 05 2005.
 *
 */
 
#include "PPSavePanel.h"
#include <SDL.h>
#include "SDL_ModalLoop.h"
#include "DialogFileSelector.h"

PPSavePanel::ReturnCodes PPSavePanel::runModal()
{
	// Create a message box (the message box will invoke the responder)
	DialogFileSelector* dialog = new DialogFileSelector(screen, NULL, PP_DEFAULT_ID, this->caption, true, true);
	dialog->setCurrentEditFileName(defaultFileName);

	for (pp_int32 i = 0; i < items.size(); i++)
	{
		PPSystemString ext(items.get(i)->extension);
		PPSystemString desc(items.get(i)->description);
		dialog->addExtension(ext, desc);
	}

	ReturnCodes result = SDL_runModalLoop(screen, dialog);

	PPSystemString pathEntry(dialog->getSelectedPathFull());
	
	// this is no longer needed
	delete dialog;
	
	fileName = ((result == ReturnCodeOK) ? pathEntry : ""); 
	
	return result;
}
