/*
 *  ppui/osinterface/sdl/PPQuitSaveAlert_SDL.cpp
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
 *  PPQuitSaveAlert_SDL.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *
 */

#include "PPQuitSaveAlert.h"
#include <SDL.h>
#include "SDL_ModalLoop.h"
#include "DialogFileSelector.h"

PPQuitSaveAlert::ReturnCodes PPQuitSaveAlert::runModal()
{
	// Create a message box (the message box will invoke the responder)
	PPDialogBase* dialog = new PPDialogBase(screen, NULL, PP_DEFAULT_ID, "Save current changes?", PPDialogBase::MessageBox_YESNOCANCEL);

	ReturnCodes result = SDL_runModalLoop(screen, dialog);
	
	delete dialog;
	
	return result;
}
