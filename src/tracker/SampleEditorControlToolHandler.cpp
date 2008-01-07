/*
 *  tracker/SampleEditorControlToolHandler.cpp
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
 *  SampleEditorControlToolHandler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.10.05.
 *
 */

#include "SampleEditorControl.h"
#include "RespondMessageBoxWithValues.h"
#include "RespondMessageBoxResample.h"
#include "RespondMessageBoxGroupSelection.h"
#include "RespondMessageBoxEQ.h"
#include "SimpleVector.h"
#include "FilterParameters.h"

bool SampleEditorControl::invokeToolParameterDialog(SampleEditorControl::ToolHandlerResponder::SampleToolTypes type)
{
	if (respondMessageBox)
	{
		delete respondMessageBox;
		respondMessageBox = NULL;
	}
	
	toolHandlerResponder->setSampleToolType(type);
	
	switch (type)
	{
		case ToolHandlerResponder::SampleToolTypeNew:
			respondMessageBox = new RespondMessageBoxWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Create new sample"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterOneValue);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneCaption("Enter size in samples:");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneRange(3, 1024*1024*10-1, 0); 
			if (lastValues.newSampleSize != TLastValues::invalidIntValue())
				static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne((float)lastValues.newSampleSize);
			else
				static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne(100.0f);
			break;
			
		case ToolHandlerResponder::SampleToolTypeVolume:
			respondMessageBox = new RespondMessageBoxWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Boost sample volume"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterOneValue);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneCaption("Enter new volume in percent:");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneRange(-10000.0f, 10000.0f, 2); 
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne(lastValues.boostSampleVolume != TLastValues::invalidFloatValue() ? lastValues.boostSampleVolume : 100.0f);
			break;

		case ToolHandlerResponder::SampleToolTypeFade:
			respondMessageBox = new RespondMessageBoxWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Fade sample"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterTwoValues);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneCaption("Enter start volume in percent:");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwoCaption("Enter end volume in percent:");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneRange(-10000.0f, 10000.0f, 2); 
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwoRange(-10000.0f, 10000.0f, 2); 
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne(lastValues.fadeSampleVolumeStart != TLastValues::invalidFloatValue() ? lastValues.fadeSampleVolumeStart : 100.0f);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwo(lastValues.fadeSampleVolumeEnd != TLastValues::invalidFloatValue() ? lastValues.fadeSampleVolumeEnd : 100.0f);
			break;
			
		case ToolHandlerResponder::SampleToolTypeDCOffset:
			respondMessageBox = new RespondMessageBoxWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "DC offset"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterOneValue);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneCaption("Enter offset in percent [-100..100]");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneRange(-100, 100.0f, 2); 
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne(lastValues.DCOffset != TLastValues::invalidFloatValue() ? lastValues.DCOffset : 0.0f);
			break;

		case ToolHandlerResponder::SampleToolTypeResample:
			respondMessageBox = new RespondMessageBoxResample(parentScreen, toolHandlerResponder, PP_DEFAULT_ID);
			if (sampleEditor->isValidSample())
			{
				if (lastValues.lastResampleInterpolationType != TLastValues::invalidIntValue()) 
					static_cast<RespondMessageBoxResample*>(respondMessageBox)->setInterpolationType(lastValues.lastResampleInterpolationType);

				static_cast<RespondMessageBoxResample*>(respondMessageBox)->setRelNote(sampleEditor->getRelNoteNum());
				static_cast<RespondMessageBoxResample*>(respondMessageBox)->setFineTune(sampleEditor->getFinetune());
				static_cast<RespondMessageBoxResample*>(respondMessageBox)->setSize(sampleEditor->getSampleLen());
			}
			break;

		case ToolHandlerResponder::SampleToolTypeEQ3Band:
			respondMessageBox = new RespondMessageBoxEQ(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, RespondMessageBoxEQ::EQ3Bands);
			if (lastValues.hasEQ3BandValues)
			{
				for (pp_int32 i = 0; i < 3; i++)
					static_cast<RespondMessageBoxEQ*>(respondMessageBox)->setBandParam(i, lastValues.EQ3BandValues[i]);
			}
			break;

		case ToolHandlerResponder::SampleToolTypeEQ10Band:
			respondMessageBox = new RespondMessageBoxEQ(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, RespondMessageBoxEQ::EQ10Bands);
			if (lastValues.hasEQ10BandValues)
			{
				for (pp_int32 i = 0; i < 10; i++)
					static_cast<RespondMessageBoxEQ*>(respondMessageBox)->setBandParam(i, lastValues.EQ10BandValues[i]);
			}
			break;

		case ToolHandlerResponder::SampleToolTypeGenerateSilence:
			respondMessageBox = new RespondMessageBoxWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Insert silence"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterOneValue);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneCaption("Enter size in samples:");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneRange(3, 1024*1024*10-1, 0); 
			if (lastValues.silenceSize != TLastValues::invalidIntValue())
				static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne((float)lastValues.silenceSize);
			else
				static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne(100.0f);
			break;

		case ToolHandlerResponder::SampleToolTypeGenerateNoise:
		{
			PPSimpleVector<PPString> noiseFilterTypes;

			noiseFilterTypes.add(new PPString("White"));
			noiseFilterTypes.add(new PPString("Pink"));
			noiseFilterTypes.add(new PPString("Brown"));

			respondMessageBox = new RespondMessageBoxGroupSelection(parentScreen, 
																	toolHandlerResponder, 
																	PP_DEFAULT_ID, 
																	"Select noise type"PPSTR_PERIODS,
																	noiseFilterTypes);
			break;
		}
		
		case ToolHandlerResponder::SampleToolTypeGenerateSine:
		case ToolHandlerResponder::SampleToolTypeGenerateSquare:
		case ToolHandlerResponder::SampleToolTypeGenerateTriangle:
		case ToolHandlerResponder::SampleToolTypeGenerateSawtooth:
		{
			respondMessageBox = new RespondMessageBoxWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Generate waveform"PPSTR_PERIODS, RespondMessageBoxWithValues::ValueStyleEnterTwoValues);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneCaption("Volume in percent:");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwoCaption("Number of periods:");
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOneRange(-1000.0f, 1000.0f, 2); 
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwoRange(0.0f, 1000.0f, 1); 
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueOne(lastValues.waveFormVolume != TLastValues::invalidFloatValue() ? lastValues.waveFormVolume : 100.0f);
			static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->setValueTwo(lastValues.waveFormNumPeriods != TLastValues::invalidFloatValue() ? lastValues.waveFormNumPeriods : 1.0f);
			break;
		}

	}
	
	respondMessageBox->show();
	
	return true;
}

bool SampleEditorControl::invokeTool(ToolHandlerResponder::SampleToolTypes type)
{
	if (!sampleEditor->isValidSample())
		return false;

	bool res = false;
	switch (type)
	{
		case ToolHandlerResponder::SampleToolTypeNew:
			lastValues.newSampleSize = (pp_int32)static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			res = sampleEditor->tool_newSample(lastValues.newSampleSize, sampleEditor->is16Bit() ? 16 : 8);
			break;

		case ToolHandlerResponder::SampleToolTypeVolume:
		{
			lastValues.boostSampleVolume = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			FilterParameters par(2);
			par.setParameter(0, lastValues.boostSampleVolume / 100.0f);
			par.setParameter(1, lastValues.boostSampleVolume / 100.0f);
			sampleEditor->tool_scaleSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeFade:
		{
			lastValues.fadeSampleVolumeStart = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.fadeSampleVolumeEnd = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, lastValues.fadeSampleVolumeStart / 100.0f);
			par.setParameter(1, lastValues.fadeSampleVolumeEnd / 100.0f);
			sampleEditor->tool_scaleSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeDCOffset:
		{
			FilterParameters par(1);
			lastValues.DCOffset = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			par.setParameter(0, lastValues.DCOffset / 100.0f);
			sampleEditor->tool_DCOffsetSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeResample:
		{
			lastValues.lastResampleInterpolationType = static_cast<RespondMessageBoxResample*>(respondMessageBox)->getInterpolationType();

			FilterParameters par(2);
			par.setParameter(0, static_cast<RespondMessageBoxResample*>(respondMessageBox)->getC4Speed());
			par.setParameter(1, lastValues.lastResampleInterpolationType);
			sampleEditor->tool_resampleSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeEQ3Band:
		case ToolHandlerResponder::SampleToolTypeEQ10Band:
		{
			pp_uint32 numBands = static_cast<RespondMessageBoxEQ*>(respondMessageBox)->getNumBandsAsInt();
			
			float* last = NULL;
			
			if (numBands == 3)
			{
				lastValues.hasEQ3BandValues = true;
				last = lastValues.EQ3BandValues;
			}
			else
			{
				lastValues.hasEQ10BandValues = true;
				last = lastValues.EQ10BandValues;
			}
			
			FilterParameters par(numBands);
			for (pp_uint32 i = 0; i < numBands; i++)
			{
				float val = static_cast<RespondMessageBoxEQ*>(respondMessageBox)->getBandParam(i);
				if (last)
					last[i] = val;
				par.setParameter(i, val);
			}
			sampleEditor->tool_eqSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateSilence:
		{
			FilterParameters par(1);
			par.setParameter(0, (float)static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne());
			sampleEditor->tool_generateSilence(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateNoise:
		{
			FilterParameters par(1);
			par.setParameter(0, (float)static_cast<RespondMessageBoxGroupSelection*>(respondMessageBox)->getSelection());
			sampleEditor->tool_generateNoise(&par);
			break;
		}
		
		case ToolHandlerResponder::SampleToolTypeGenerateSine:
		{
			lastValues.waveFormVolume = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, lastValues.waveFormVolume/100.0f);
			par.setParameter(1, lastValues.waveFormNumPeriods);
			sampleEditor->tool_generateSine(&par);
			break;
		}
		
		case ToolHandlerResponder::SampleToolTypeGenerateSquare:
		{
			lastValues.waveFormVolume = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, lastValues.waveFormVolume/100.0f);
			par.setParameter(1, lastValues.waveFormNumPeriods);
			sampleEditor->tool_generateSquare(&par);
			break;
		}
		
		case ToolHandlerResponder::SampleToolTypeGenerateTriangle:
		{
			lastValues.waveFormVolume = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, lastValues.waveFormVolume/100.0f);
			par.setParameter(1, lastValues.waveFormNumPeriods);
			sampleEditor->tool_generateTriangle(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateSawtooth:
		{
			lastValues.waveFormVolume = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<RespondMessageBoxWithValues*>(respondMessageBox)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, lastValues.waveFormVolume/100.0f);
			par.setParameter(1, lastValues.waveFormNumPeriods);
			sampleEditor->tool_generateSawtooth(&par);
			break;
		}
	}
	return res;
}

SampleEditorControl::ToolHandlerResponder::ToolHandlerResponder(SampleEditorControl& theSampleEditorControl) :
	sampleEditorControl(theSampleEditorControl),
	sampleToolType(SampleToolTypeNone)
{
}

pp_int32 SampleEditorControl::ToolHandlerResponder::ActionOkay(PPObject* sender)
{
	sampleEditorControl.invokeTool(sampleToolType);
	return 0;
}

pp_int32 SampleEditorControl::ToolHandlerResponder::ActionCancel(PPObject* sender)
{
	return 0;
}
