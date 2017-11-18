/*
 *  ppui/osinterface/amiga/PPOpenPanel_Amiga.cpp
 *
 *  Copyright 2017 Juha Niemimaki
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
#include "AslRequester.h"

PPOpenPanel::PPOpenPanel(PPScreen* screen, const char* caption) :
	PPModalDialog(screen)
{
    this->caption = new char[strlen(caption) + 1];
    strcpy(this->caption, caption);
}

PPOpenPanel::~PPOpenPanel()
{
    delete[] caption;
}

void PPOpenPanel::addExtension(const PPString& ext, const PPString& desc)
{
#if 0
    // TODO: this is not functional yet
	Descriptor* d = new Descriptor(ext, desc);

	items.add(d);
#endif
}

PPOpenPanel::ReturnCodes PPOpenPanel::runModal()
{
	ReturnCodes err = ReturnCodeCANCEL;
    fileName = GetFileName(caption);

    if (fileName.length() > 0) {
        err = ReturnCodeOK;
    }
	
	return err;
}
