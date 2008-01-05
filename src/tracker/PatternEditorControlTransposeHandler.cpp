/*
 *  PatternEditorControlTransposeHandler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.10.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "PatternEditorControl.h"
#include "RespondMessageBox.h"

void PatternEditorControl::showNoteTransposeWarningMessageBox(pp_int32 fuckups)
{
	if (respondMessageBox)
	{
		delete respondMessageBox;
		respondMessageBox = NULL;
	}
	
	char buffer[100];
	sprintf(buffer, "%i notes will be erased, continue?", fuckups);
	
	respondMessageBox = new RespondMessageBox(parentScreen, transposeHandlerResponder, PP_DEFAULT_ID, buffer);
	respondMessageBox->show();
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
