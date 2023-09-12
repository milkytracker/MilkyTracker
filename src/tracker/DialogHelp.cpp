/*
 *  tracker/DialogHelp.cpp
 *
 *  Copyright 2022 coderofsalvation / Leon van Kammen 
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
 *  DialogHelp.cpp
 *  MilkyTracker
 *
 *  Created by coderofsalvation / Leon van Kammen on 13-07-2022 
 *
 */

#include "DialogHelp.h"
#include "DialogHelpText.h"
#include "MessageBoxContainer.h"
#include "ListBox.h"
#include "PPUI.h"

DialogHelp::DialogHelp(PPScreen *screen,
					   DialogResponder *responder,
					   pp_int32 id,
					   const PPString &caption,
					   bool okCancel /* = false*/) : PPDialogBase()
{
	pp_int32 w = screen->getWidth() - 12;
	pp_int32 h = screen->getHeight() - 50;
  if( w > 780 ) w = 780;
	if (okCancel)
		initDialog(screen, responder, id, caption, w, h, 26, "Ok", "Cancel");
	else
		initDialog(screen, responder, id, caption, w, h, 26, "Okay");

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	pp_int32 y = getMessageBoxContainer()->getLocation().y;

	pp_int32 width = getMessageBoxContainer()->getSize().width;
	pp_int32 height = getMessageBoxContainer()->getSize().height;

	PPButton *button = static_cast<PPButton *>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_YES));
	pp_int32 y2 = button->getLocation().y;
	pp_int32 x2 = x + width / 2 - 30;

	y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y + 18;
	x2 = x + width / 2 - 120;

	listBox = new PPListBox(MESSAGEBOX_LISTBOX_USER1, screen, this, PPPoint(x + 2, y + 2), PPSize(width - 5, height - (8 * 8)), true, false, true, true);
	listBox->setShowIndex(false);
	
  PPString line = PPString("");
	unsigned char c;
  for( c = 0; c <128; c++ ){
	char msg[255];
	sprintf(msg,"d=%d x=%x %c\n",c,c,c);
	listBox->addItem( msg );
	
  }
//	for (pp_int32 i = 0; i < milkytracker_help_len; i++)
//	{
//		c = milkytracker_help[i];
//		if (c == 0x0a ) // '\n'
//		{
//			listBox->addItem( line );
//      line = PPString("");
//		} else line.append( PPString(c) );
//	}
	messageBoxContainerGeneric->addControl(listBox);
}
