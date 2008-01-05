/*
 *  SystemErrorMessage.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 27.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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
		screen.enableDisplay(true);
		PPMessageBox messageBox(&screen, header, message);		
		messageBox.runModal();
	}
}
