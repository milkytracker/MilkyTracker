/*
 *  tracker/ToolInvokeHelper.cpp
 *
 *  Copyright 2008 Peter Barth
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
#include "RespondMessageBoxWithValues.h"
#include "RespondMessageBoxQuickChooseInstrument.h"

ToolInvokeHelper::ToolInvokeHelper(Tracker& theTracker) :
	tracker(theTracker),
	respondMessageBox(NULL)
{
	resetLastValues();
}

ToolInvokeHelper::~ToolInvokeHelper()
{
	delete respondMessageBox;
}

void ToolInvokeHelper::resetLastValues()
{
	lastValues.volumeScaleStart = lastValues.volumeScaleEnd = -1.0f;
}


bool ToolInvokeHelper::invokeTool(ToolTypes toolType, pp_int16 keyDownKeyCode/* = -1*/)
{
	lastToolType = toolType;

	if (respondMessageBox)
	{
		delete respondMessageBox;
		respondMessageBox = NULL;
	}
	
	switch (toolType)
	{
		case ToolTypePatternVolumeScale:
			respondMessageBox = new RespondMessageBoxWithValues(tracker.screen, this, PP_DEFAULT_ID, "Volume scale pattern"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterTwoValues);
			break;

		case ToolTypeTrackVolumeScale:
			respondMessageBox = new RespondMessageBoxWithValues(tracker.screen, this, PP_DEFAULT_ID, "Volume scale track"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterTwoValues);
			break;

		case ToolTypeSelectionVolumeScale:
			respondMessageBox = new RespondMessageBoxWithValues(tracker.screen, this, PP_DEFAULT_ID, "Volume scale block"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterTwoValues);
			break;

		case ToolTypeQuickChooseInstrument:
		{
			respondMessageBox = new RespondMessageBoxQuickChooseInstrument(tracker.screen, this, PP_DEFAULT_ID, "Choose instrument"PPSTR_PERIODS);
			static_cast<RespondMessageBoxQuickChooseInstrument*>(respondMessageBox)->setValueCaption("Enter hex value:");
			pp_uint16 value = static_cast<RespondMessageBoxQuickChooseInstrument*>(respondMessageBox)->numPadKeyToValue(keyDownKeyCode);
			static_cast<RespondMessageBoxQuickChooseInstrument*>(respondMessageBox)->setValue(value);
			respondMessageBox->setKeyDownInvokeKeyCode(keyDownKeyCode);
			respondMessageBox->show();
			return true;
			break;
		}
			
		default:
			return false;
	}
	
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneCaption("Enter start scale:");
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwoCaption("Enter end scale:");
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneRange(0, 100.0f, 2); 
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwoRange(0, 100.0f, 2); 
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneIncreaseStep(0.01f); 
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwoIncreaseStep(0.01f); 
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne(lastValues.volumeScaleStart != -1.0f ? lastValues.volumeScaleStart : 1.0f);
	static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwo(lastValues.volumeScaleEnd != -1.0f ? lastValues.volumeScaleEnd : 1.0f);
	
	respondMessageBox->setKeyDownInvokeKeyCode(keyDownKeyCode);
	
	respondMessageBox->show();

	return true;
}

pp_int32 ToolInvokeHelper::ActionOkay(PPObject* sender)
{
	switch (lastToolType)
	{
		case ToolTypePatternVolumeScale:
			lastValues.volumeScaleStart = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.volumeScaleEnd = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			tracker.getPatternEditor()->scaleVolumePattern(lastValues.volumeScaleStart, lastValues.volumeScaleEnd);
			break;

		case ToolTypeTrackVolumeScale:
			lastValues.volumeScaleStart = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.volumeScaleEnd = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			tracker.getPatternEditor()->scaleVolumeTrack(lastValues.volumeScaleStart, lastValues.volumeScaleEnd);
			break;

		case ToolTypeSelectionVolumeScale:
			lastValues.volumeScaleStart = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.volumeScaleEnd = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			tracker.getPatternEditor()->scaleVolumeSelection(lastValues.volumeScaleStart, lastValues.volumeScaleEnd);
			break;
			
		case ToolTypeQuickChooseInstrument:
		{
			pp_int32 value = static_cast<RespondMessageBoxQuickChooseInstrument*>(respondMessageBox)->getValue();
			if (value == 0)
				tracker.enableInstrument(false);
			else
			{
				tracker.enableInstrument(true);
				tracker.selectInstrument(value);
				tracker.listBoxInstruments->setSelectedIndex(value-1, false);
			}
		}
	}

	return 0;
}

pp_int32 ToolInvokeHelper::ActionCancel(PPObject* sender)
{
	return 0;
}
