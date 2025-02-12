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
#include "ControlIDs.h"
#include "PPUI.h"
#include "GlobalColorConfig.h"

DialogHelp::DialogHelp(PPScreen *screen,
					   DialogResponder *responder,
					   pp_int32 id,
					   const PPString &caption,
					   bool okCancel /* = false*/) : PPDialogBase(),
	lineMilkySynth(0),
	lineShortcuts(0),
	linePatternFX(0)
{
	pp_int32 w = screen->getWidth() - 12;
	pp_int32 h = screen->getHeight() - (screen->getHeight()/4);
  if( w > 780 ) w = 780;
	if (okCancel)
		initDialog(screen, responder, id, caption, w, h, 26, "Ok", "Cancel");
	else
		initDialog(screen, responder, id, caption, w, h, 26, "Okay");

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	pp_int32 y = getMessageBoxContainer()->getLocation().y;

	pp_int32 width = getMessageBoxContainer()->getSize().width;
	pp_int32 height = getMessageBoxContainer()->getSize().height;

	// position OK button
	PPButton *button = static_cast<PPButton *>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_OK));
	pp_int32 align_right = x + width - button->getSize().width;
	pp_int32 align_bottom = button->getLocation().y;
	pp_int32 top_margin = y+10;

	// add shortcut buttons
	button = new PPButton( BUTTON_HELP_SHORTCUTS, screen, this, PPPoint( x + 4, top_margin+16), PPSize(110, 11));
	button->setText("shortcuts");
	button->setTextColor(GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorSampleEditorWaveform));
	getMessageBoxContainer()->addControl(button);

	button = new PPButton( BUTTON_HELP_MILKYSYNTH, screen, this, PPPoint( (x + 4)+(110*1), top_margin+16), PPSize(110, 11));
	button->setText("milkysynth");
	button->setTextColor(GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorSampleEditorWaveform));
	getMessageBoxContainer()->addControl(button);

	button = new PPButton( BUTTON_HELP_PATTERNFX, screen, this, PPPoint( (x + 4)+(110*2), top_margin+16), PPSize(110, 11));
	button->setText("pattern fx");
	button->setTextColor(GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorSampleEditorWaveform));
	getMessageBoxContainer()->addControl(button);

	pp_int32 y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y + 18;
	pp_int32 x2 = x + width / 2 - 120;

	listBox = new PPListBox(MESSAGEBOX_LISTBOX_USER1, screen, this, PPPoint(x + 4, top_margin + 28), PPSize(width - 6, height - (8 * 8) - 10), true, false, true, true);
	listBox->setSelectedIndex( position );
	listBox->setShowIndex(false);
	listBox->setSelectOnScroll(false);
	listBox->setShowFocus(false);
	listBox->setKeepsFocus(true);
	listBox->showSelection(false);
		

	PPString line = PPString("");
	pp_int32 lineN = 0;
	unsigned char c;
	// uncomment to display all characters
	//for( c = 0; c <128; c++ ){
	//  char msg[255];
	//  sprintf(msg,"d=%d x=%x %c\n",c,c,c);
	//  listBox->addItem( msg );
	//  
	//}
	for (pp_int32 i = 0; i < milkytracker_help_len; i++)
	{
		c = milkytracker_help[i];
		if (c == 0x0a ) // '\n'
		{
			listBox->addItem( line );
			// add bookmarks
			if( line.startsWith("    Milky synth")           ) lineMilkySynth = lineN-1;
			if( line.startsWith("     Alt-Enter Switch")     ) lineShortcuts  = lineN-1;
			if( line.startsWith("       * 0xy [32]Arpeggio") ) linePatternFX  = lineN-1;
			line = PPString("");
			lineN++;
		} else line.append( PPString(c) );
	}
	messageBoxContainerGeneric->addControl(listBox);

}

DialogHelp::~DialogHelp()
{
	// remember scrollposition
	position = listBox->getStartIndex();
}

pp_int32 DialogHelp::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{

			case BUTTON_HELP_PATTERNFX:
				if( linePatternFX > 0 && linePatternFX < listBox->getNumItems() ){
					listBox->setSelectedIndex( linePatternFX );
					parentScreen->paintControl(messageBoxContainerGeneric);
				}
				event->cancel();
				break;

			case BUTTON_HELP_MILKYSYNTH:
				if( lineMilkySynth > 0 && lineMilkySynth < listBox->getNumItems() ){
					listBox->setSelectedIndex( lineMilkySynth );
					parentScreen->paintControl(messageBoxContainerGeneric);
				}
				event->cancel();
				break;

			case BUTTON_HELP_SHORTCUTS:
				if( lineShortcuts > 0 && lineMilkySynth < listBox->getNumItems() ){
					listBox->setSelectedIndex( lineShortcuts );
					parentScreen->paintControl(messageBoxContainerGeneric);
				}
				event->cancel();
				break;
		}
	}
	return PPDialogBase::handleEvent(sender, event);
}

void DialogHelp::show(bool show){
  if( show ){
	listBox->setSelectedIndex( position );
  }else{
	position = listBox->getStartIndex();
  }
  PPDialogBase::show(show);
}

pp_int32 DialogHelp::position = 0;
