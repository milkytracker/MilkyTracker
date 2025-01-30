/*
 *  tracker/DialogHelp.h
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
 *  DialogHelp.h
 *  MilkyTracker
 *
 *  Created by coderofsalvation / Leon van Kammen on 13-07-2022 
 *
 */

#ifndef __DIALOGHELP_H__
#define __DIALOGHELP_H__

#include "DialogBase.h"

#define HELP_MAX_LINE 255

class DialogHelp : public PPDialogBase
{
private:
	class PPListBox* listBox;
	pp_int32 lineMilkySynth;
	pp_int32 lineShortcuts;
	pp_int32 linePatternFX;
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);

public:
	DialogHelp(PPScreen* screen, 
				  DialogResponder* responder,
				  pp_int32 id,
				  const PPString& caption,
				  bool okCancel = false);
	
	PPListBox* getListBox() { return listBox; }
	void show(bool show);
	static pp_int32 position;

};

#endif
