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

#include "PPQuitSaveAlert.h"

#include <Alert.h>


PPQuitSaveAlert::ReturnCodes PPQuitSaveAlert::runModal()
{
	BAlert* alert = new BAlert("Save changes?",
		"Do you want to save the changes you made to your documents?",
		"Cancel", "Don't save", "Save",
		B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);

	int32 userChoice = alert->Go();
	// (Alert deletes itself)

	switch (userChoice) {
		case 0: return ReturnCodeCANCEL;
		case 1: return ReturnCodeNO;
		case 2: return ReturnCodeOK;

		default:
			// This shouldn't be possible
			return ReturnCodeCANCEL;
	}
}
