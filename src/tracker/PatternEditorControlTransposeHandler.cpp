/*
 *  tracker/PatternEditorControlTransposeHandler.cpp
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
 *  PatternEditorControlTransposeHandler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.10.05.
 *
 */

#include "PatternEditorControl.h"
#include "DialogBase.h"

void PatternEditorControl::showNoteTransposeWarningMessageBox(pp_int32 fuckups)
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}
	
	char buffer[100];
	sprintf(buffer, "%i notes will be erased, continue?", fuckups);
	
	dialog = new PPDialogBase(parentScreen, transposeHandlerResponder, PP_DEFAULT_ID, buffer);
	dialog->show();
}

pp_int32 PatternEditorControl::noteTransposeTrack(const PatternEditorTools::TransposeParameters& transposeParameters)
{
	pp_int32 fuckups = patternEditor->noteTransposeTrackCore(transposeParameters, true);
	
	if (!fuckups)
		return patternEditor->noteTransposeTrackCore(transposeParameters, false);
	
	if (transposeHandlerResponder == NULL)
		return -1;
	
	transposeHandlerResponder->setTransposeParameters(transposeParameters);
	transposeHandlerResponder->setTransposeFunc(&PatternEditor::noteTransposeTrackCore);
	
	showNoteTransposeWarningMessageBox(fuckups);
	return 0;
}

pp_int32 PatternEditorControl::noteTransposePattern(const PatternEditorTools::TransposeParameters& transposeParameters)
{
	pp_int32 fuckups = patternEditor->noteTransposePatternCore(transposeParameters, true);
	
	if (!fuckups)
		return patternEditor->noteTransposePatternCore(transposeParameters, false);
	
	if (transposeHandlerResponder == NULL)
		return -1;
	
	transposeHandlerResponder->setTransposeParameters(transposeParameters);
	transposeHandlerResponder->setTransposeFunc(&PatternEditor::noteTransposePatternCore);
	
	showNoteTransposeWarningMessageBox(fuckups);
	return 0;
}

pp_int32 PatternEditorControl::noteTransposeSelection(const PatternEditorTools::TransposeParameters& transposeParameters)
{
	pp_int32 fuckups = patternEditor->noteTransposeSelectionCore(transposeParameters, true);
	
	if (!fuckups)
		return patternEditor->noteTransposeSelectionCore(transposeParameters, false);
	
	if (transposeHandlerResponder == NULL)
		return -1;
	
	transposeHandlerResponder->setTransposeParameters(transposeParameters);
	transposeHandlerResponder->setTransposeFunc(&PatternEditor::noteTransposeSelectionCore);
	
	showNoteTransposeWarningMessageBox(fuckups);
	return 0;
}

PatternEditorControl::TransposeHandlerResponder::TransposeHandlerResponder(PatternEditorControl& thePatternEditorControl) :
	patternEditorControl(thePatternEditorControl),
	transposeFunc(NULL)
{
}

pp_int32 PatternEditorControl::TransposeHandlerResponder::ActionOkay(PPObject* sender)
{
	if (transposeFunc == NULL)
		return -1;
	
	(patternEditorControl.patternEditor->*transposeFunc)(transposeParameters, false);
	return 0;
}

pp_int32 PatternEditorControl::TransposeHandlerResponder::ActionCancel(PPObject* sender)
{
	return 0;
}
