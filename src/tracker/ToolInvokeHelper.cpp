/*
 *  tracker/ToolInvokeHelper.cpp
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
 *  ToolInvokeHelper.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.11.05.
 *
 */

#include "ToolInvokeHelper.h"
#include "Tracker.h"
#include "ListBox.h"
#include "PatternEditorControl.h"
#include "DialogWithValues.h"
#include "DialogQuickChooseInstrument.h"
#include "Screen.h"

ToolInvokeHelper::ToolInvokeHelper(Tracker& theTracker) :
	tracker(theTracker),
	dialog(NULL)
{
	resetLastValues();
}

ToolInvokeHelper::~ToolInvokeHelper()
{
	delete dialog;
}

void ToolInvokeHelper::resetLastValues()
{
	lastValues.volumeScaleStart = lastValues.volumeScaleEnd = -1.0f;
}


bool ToolInvokeHelper::invokeTool(ToolTypes toolType, pp_int16 keyDownKeyCode/* = -1*/)
{
	if (tracker.screen->getModalControl())
		return false;

	lastToolType = toolType;

	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}
	
	switch (toolType)
	{
		case ToolTypePatternVolumeScale:
			dialog = new DialogWithValues(tracker.screen, this, PP_DEFAULT_ID, "Volume scale pattern" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterTwoValues);
			break;

		case ToolTypeTrackVolumeScale:
			dialog = new DialogWithValues(tracker.screen, this, PP_DEFAULT_ID, "Volume scale track" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterTwoValues);
			break;

		case ToolTypeSelectionVolumeScale:
			dialog = new DialogWithValues(tracker.screen, this, PP_DEFAULT_ID, "Volume scale block" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterTwoValues);
			break;

		case ToolTypeQuickChooseInstrument:
		{
			dialog = new DialogQuickChooseInstrument(tracker.screen, this, PP_DEFAULT_ID, "Choose instrument" PPSTR_PERIODS);
			static_cast<DialogQuickChooseInstrument*>(dialog)->setValueCaption("Enter hex value:");
			pp_uint16 value = static_cast<DialogQuickChooseInstrument*>(dialog)->numPadKeyToValue(keyDownKeyCode);
			static_cast<DialogQuickChooseInstrument*>(dialog)->setValue(value);
			dialog->setKeyDownInvokeKeyCode(keyDownKeyCode);
			dialog->show();
			return true;
			break;
		}
			
		default:
			return false;
	}
	
	static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Enter start scale:");
	static_cast<DialogWithValues*>(dialog)->setValueTwoCaption("Enter end scale:");
	static_cast<DialogWithValues*>(dialog)->setValueOneRange(0, 100.0f, 2); 
	static_cast<DialogWithValues*>(dialog)->setValueTwoRange(0, 100.0f, 2); 
	static_cast<DialogWithValues*>(dialog)->setValueOneIncreaseStep(0.01f); 
	static_cast<DialogWithValues*>(dialog)->setValueTwoIncreaseStep(0.01f); 
	static_cast<DialogWithValues*>(dialog)->setValueOne(lastValues.volumeScaleStart != -1.0f ? lastValues.volumeScaleStart : 1.0f);
	static_cast<DialogWithValues*>(dialog)->setValueTwo(lastValues.volumeScaleEnd != -1.0f ? lastValues.volumeScaleEnd : 1.0f);
	
	dialog->setKeyDownInvokeKeyCode(keyDownKeyCode);
	
	dialog->show();

	return true;
}

pp_int32 ToolInvokeHelper::ActionOkay(PPObject* sender)
{
	switch (lastToolType)
	{
		case ToolTypePatternVolumeScale:
			lastValues.volumeScaleStart = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.volumeScaleEnd = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			tracker.getPatternEditor()->scaleVolumePattern(lastValues.volumeScaleStart, lastValues.volumeScaleEnd);
			break;

		case ToolTypeTrackVolumeScale:
			lastValues.volumeScaleStart = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.volumeScaleEnd = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			tracker.getPatternEditor()->scaleVolumeTrack(lastValues.volumeScaleStart, lastValues.volumeScaleEnd);
			break;

		case ToolTypeSelectionVolumeScale:
			lastValues.volumeScaleStart = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.volumeScaleEnd = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			tracker.getPatternEditor()->scaleVolumeSelection(lastValues.volumeScaleStart, lastValues.volumeScaleEnd);
			break;
			
		case ToolTypeQuickChooseInstrument:
		{
			pp_int32 value = static_cast<DialogQuickChooseInstrument*>(dialog)->getValue();
			if (value == 0)
				tracker.enableInstrument(false);
			else
			{
				tracker.enableInstrument(true);
				// set listbox index before calling Tracker::selectInstrument()
				tracker.listBoxInstruments->setSelectedIndex(value-1, false);
				tracker.selectInstrument(value);
			}
		}
		case ToolTypeNone:
			break;
	}

	return 0;
}

pp_int32 ToolInvokeHelper::ActionCancel(PPObject* sender)
{
	return 0;
}
