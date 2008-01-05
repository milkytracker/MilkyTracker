/*
 *  SDL_ModalLoop.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 12.03.06.
 *  Copyright 2006 milkytracker.net, All rights reserved.
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

class ModalLoopResponder : public RespondListenerInterface
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
class RespondMessageBox;

PPModalDialog::ReturnCodes SDL_runModalLoop(PPScreen* screen, RespondMessageBox* respondMessageBox);

#endif
