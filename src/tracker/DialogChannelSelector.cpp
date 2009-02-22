/*
 *  tracker/DialogChannelSelector.cpp
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
 *  DialogChannelSelector.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#include "DialogChannelSelector.h"
#include "MessageBoxContainer.h"
#include "ListBox.h"

DialogChannelSelector::DialogChannelSelector(PPScreen* screen, 
											 DialogResponder* responder,
											 pp_int32 id,
											 const PPString& caption) :
	PPDialogBase()
{
	initDialog(screen, responder, id, caption, 290, 142, 26, "Ok", "Cancel");

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	
	pp_int32 width = getMessageBoxContainer()->getSize().width;

	PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_YES));	
	pp_int32 y2 = button->getLocation().y;
	pp_int32 x2 = x + width / 2 - 30;
	button->setLocation(PPPoint(x2+5,y2));

	button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_CANCEL));
	x2 = x + width / 2 + 40;
	button->setLocation(PPPoint(x2+5,y2));
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER1, screen, this, PPPoint(x+width/2-100-10, y2), PPSize(60, 11));
	button->setText("Mix");
	messageBoxContainerGeneric->addControl(button);
	
	y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y + 18;
	x2 = x + width / 2 - 50;

	listBox	= new PPListBox(MESSAGEBOX_LISTBOX_USER1, screen, this, PPPoint(x2, y2), PPSize(100,4 + 7*8), true, false, true, true);
	listBox->setShowIndex(true);
	messageBoxContainerGeneric->addControl(listBox);
}

