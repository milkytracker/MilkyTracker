/*
 *  RespondMessageBoxListBox.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "RespondMessageBoxListBox.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "ListBox.h"
#include "ControlIDs.h"

RespondMessageBoxListBox::RespondMessageBoxListBox(PPScreen* screen, 
												   RespondListenerInterface* responder,
												   pp_int32 id,
												   const PPString& caption,
												   bool okCancel/* = false*/) :
	RespondMessageBox()
{
	if (okCancel)
		initRespondMessageBox(screen, responder, id, caption, 290, 142, 26, "Ok", "Cancel");	
	else
		initRespondMessageBox(screen, responder, id, caption, 290, 142, 26, "Okay");

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	pp_int32 y = getMessageBoxContainer()->getLocation().y;
	
	pp_int32 width = getMessageBoxContainer()->getSize().width;
	pp_int32 height = getMessageBoxContainer()->getSize().height;

	PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_YES));	
	pp_int32 y2 = button->getLocation().y;
	pp_int32 x2 = x + width / 2 - 30;
	
	y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y + 18;
	x2 = x + width / 2 - 120;

	listBox	= new PPListBox(MESSAGEBOX_LISTBOX_USER1, screen, this, PPPoint(x2, y2), PPSize(240,4 + 7*8), true, false, true, true);
	listBox->setShowIndex(true);
	messageBoxContainerGeneric->addControl(listBox);
}

