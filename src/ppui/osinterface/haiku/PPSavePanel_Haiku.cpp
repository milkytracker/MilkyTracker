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

#include "PPSavePanel.h"
#include "SynchronousFilePanel.h"


PPSavePanel::ReturnCodes PPSavePanel::runModal()
{
	SynchronousFilePanel* savePanel = new SynchronousFilePanel(B_SAVE_PANEL,
		caption);

	savePanel->SetSaveText(defaultFileName);

	BString path = savePanel->Go();

	// Quit() destroys the object, i.e. lack of delete is not a memory leak
	savePanel->Quit();

	if (path != "") {
		fileName = path.String();
		return ReturnCodeOK;
	} else
		return ReturnCodeCANCEL;
}
