/*
 *  ppui/osinterface/sdl/SDL_ModalLoop.h
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
 *  SDL_ModalLoop.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 12.03.06.
 *
 */

#ifndef __CUSTOMEVENTLISTENER_H__
#define __CUSTOMEVENTLISTENER_H__

#include "BasicTypes.h"
#include "Event.h"
#include "PPModalDialog.h"

class CustomEventListener : public EventListenerInterface
{
private:
	EventListenerInterface* oldEventListener;
	
public:
	CustomEventListener(EventListenerInterface* oldListener) :
		oldEventListener(oldListener)
	{
	}

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event)
	{
		if (event->getID() == eTimer)
		{
			return oldEventListener->handleEvent(sender, event);
		}
		return 0;
	}
	
};

class ModalLoopResponder : public DialogResponder
{
private:
	bool& exitModalLoop;
	PPModalDialog::ReturnCodes returnCode;
		
public:
	ModalLoopResponder(bool& exitModalLoopFlag) :
		exitModalLoop(exitModalLoopFlag),
		returnCode(PPModalDialog::ReturnCodeCANCEL)
	{
	}
	
	PPModalDialog::ReturnCodes getReturnCode() { return returnCode; }
		
	virtual pp_int32 ActionOkay(PPObject* sender)
	{
		exitModalLoop = true;
		returnCode = PPModalDialog::ReturnCodeOK;
		return 0;
	}
	
	virtual pp_int32 ActionCancel(PPObject* sender)
	{
		exitModalLoop = true;
		returnCode = PPModalDialog::ReturnCodeCANCEL;
		return 0;
	}

	virtual pp_int32 ActionNo(PPObject* sender)
	{
		exitModalLoop = true;
		returnCode = PPModalDialog::ReturnCodeNO;
		return 0;
	}
};

class PPScreen;
class PPDialogBase;

PPModalDialog::ReturnCodes SDL_runModalLoop(PPScreen* screen, PPDialogBase* dialog);

#endif
