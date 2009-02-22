/*
 *  tracker/SystemMessage.cpp
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
 *  SystemErrorMessage.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.12.07.
 *
 */

#include "SystemMessage.h"
#include "Screen.h"
#include "PPMessageBox.h"

SystemMessage::SystemMessage(PPScreen& screen, Messages message) :
	screen(screen),
	message(message)
{
}
	
void SystemMessage::show()
{
	const char* header = NULL;
	const char* message = NULL;

	switch (this->message)
	{
		case MessageSoundDriverInitFailed:
			header = "Error";
			message = "Sound driver initialization failed. Try different settings or driver.";
			break;

		case MessageFullScreenFailed:
			header = "Error";
			message = "The video mode you've selected isn't available on your gfx card/monitor.";
			break;
			
		case MessageResChangeRestart:
			header = "Please restart";
			message = "MilkyTracker needs to be restarted to apply new video mode.";
			break;

		case MessageLimitedInput:
			header = "Note";
			message = "Due to limited input this edit mode might not be fully usable on a low-res device.";
			break;
	}
	
	if (header && message)
	{
		PPMessageBox messageBox(&screen, header, message);		
		messageBox.runModal();
	}
}
