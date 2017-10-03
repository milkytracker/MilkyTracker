/*
 *  ppui/osinterface/amiga/PPSavePanel_Amiga.cpp
 *
 *  Copyright 2017 Peter Barth
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
 
#include "PPSavePanel.h"
#include "AslRequester.h"

PPSavePanel::ReturnCodes PPSavePanel::runModal()
{
    ReturnCodes err = ReturnCodeCANCEL;

#if 0
    // TODO: this is not functional
	for (pp_int32 i = 0; i < items.size(); i++)
	{
		PPSystemString ext(items.get(i)->extension);
		PPSystemString desc(items.get(i)->description);
		//dialog->addExtension(ext, desc);
	}
#endif

    fileName = GetFileName(caption, true, defaultFileName);

    if (fileName.length() > 0) {
        err = ReturnCodeOK;
    }
	
	return err;
}
