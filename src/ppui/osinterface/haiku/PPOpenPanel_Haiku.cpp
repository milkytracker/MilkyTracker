/*
 *  Copyright 2012 Julian Harnath
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

#include "PPOpenPanel.h"
#include "SynchronousFilePanel.h"


PPOpenPanel::PPOpenPanel(PPScreen* screen, const char* caption)
	:
	PPModalDialog(screen)
{
	this->caption = (char*)caption;
}


PPOpenPanel::~PPOpenPanel()
{
}


void PPOpenPanel::addExtension(const PPString& ext, const PPString& desc)
{
	Descriptor* d = new Descriptor(ext, desc);

	items.add(d);
}


PPOpenPanel::ReturnCodes PPOpenPanel::runModal()
{
	SynchronousFilePanel* openPanel = new SynchronousFilePanel(B_OPEN_PANEL,
		caption);

	BString path = openPanel->Go();

	// Quit() destroys the object
	openPanel->Quit();

	if (path != "") {
		fileName = path.String();
		return ReturnCodeOK;
	} else
		return ReturnCodeCANCEL;
}
