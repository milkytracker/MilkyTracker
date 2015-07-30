/*
 *  tracker/DialogZap.cpp
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
 *  DialogZap.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.2.08.
 *
 */

#include "DialogZap.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "Font.h"

enum ControlIDs
{
	MESSAGEBOXZAP_BUTTON_ALL = PP_MESSAGEBOX_BUTTON_USER1,
	MESSAGEBOXZAP_BUTTON_SONG,
	
	MESSAGEBOXZAP_BUTTON_PATT,
	MESSAGEBOXZAP_BUTTON_INS
};

DialogZap::DialogZap(PPScreen* screen, 
					 DialogResponder* responder,
					 pp_int32 id) :
	PPDialogBase()
{
	parentScreen = screen;
	setResponder(responder);

	const pp_int32 height = 74;
	const pp_int32 width = 290;

	pp_int32 x = screen->getWidth() / 2 - width/2;
	pp_int32 y = screen->getHeight() / 2 - height/2;

	PPMessageBoxContainer* container = new PPMessageBoxContainer(id, screen, this, PPPoint(x, y), PPSize(width,height), "System request");

	pp_int32 x2 = x + width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth("Total devastation of the" PPSTR_PERIODS) / 2);
	pp_int32 y2 = y + 20;

	container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), "Total devastation of the" PPSTR_PERIODS, true));

	PPButton* button = new PPButton(MESSAGEBOXZAP_BUTTON_ALL, screen, this, PPPoint(x+10, y2 + 15), PPSize(60, 11));
	button->setText("All");
	container->addControl(button);

	button = new PPButton(MESSAGEBOXZAP_BUTTON_SONG, screen, this, PPPoint(x + 10 + 60 + 10, y2 + 15), PPSize(60, 11));
	button->setText("Song");
	container->addControl(button);

	button = new PPButton(MESSAGEBOXZAP_BUTTON_PATT, screen, this, PPPoint(x + 10 + 60*2 + 10*2, y2 + 15), PPSize(60, 11));
	button->setText("Pattern");
	container->addControl(button);

	button = new PPButton(MESSAGEBOXZAP_BUTTON_INS, screen, this, PPPoint(x + 10 + 60*3 + 10*3, y2 + 15), PPSize(60, 11));
	button->setText("Instr.");
	container->addControl(button);

	button = new PPButton(PP_MESSAGEBOX_BUTTON_CANCEL, screen, this, PPPoint(x + 10 + 60*3 + 10*3, y2 + 15*2+4), PPSize(60, 11));
	button->setText("Cancel");
	container->addControl(button);
	
	messageBoxContainerGeneric = container;
}
