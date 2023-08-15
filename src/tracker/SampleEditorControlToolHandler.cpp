/*
 *  tracker/SampleEditorControlToolHandler.cpp
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
 *  SampleEditorControlToolHandler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.10.05.
 *
 */

#include "SampleEditorControl.h"
#include "DialogWithValues.h"
#include "DialogResample.h"
#include "DialogGroupSelection.h"
#include "DialogEQ.h"
#include "DialogSliders.h"
#include "SimpleVector.h"
#include "FilterParameters.h"

bool SampleEditorControl::invokeToolParameterDialog(SampleEditorControl::ToolHandlerResponder::SampleToolTypes type)
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}
	
	toolHandlerResponder->setSampleToolType(type);
	
	switch (type)
	{
		case ToolHandlerResponder::SampleToolTypeNew:
			dialog = new DialogWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Create new sample" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterOneValue);
			static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Enter size in samples:");
			static_cast<DialogWithValues*>(dialog)->setValueOneRange(3, 1024*1024*10-1, 0); 
			if (lastValues.newSampleSize != SampleEditorControlLastValues::invalidIntValue())
				static_cast<DialogWithValues*>(dialog)->setValueOne((float)lastValues.newSampleSize);
			else
				static_cast<DialogWithValues*>(dialog)->setValueOne(100.0f);
			break;
			
		case ToolHandlerResponder::SampleToolTypeVolume:{
			dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Sample Volume", 1, sampleEditor, &SampleEditor::tool_scaleSample );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      float value = lastValues.boostSampleVolume != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.boostSampleVolume : 100.0f;
			sliders->initSlider(0,0.0f, 300.0f, value,"Volume");
			break;
    }

		case ToolHandlerResponder::SampleToolTypeFade:{
			dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Custom Fade", 2, sampleEditor, &SampleEditor::tool_scaleSample );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      float value = lastValues.fadeSampleVolumeStart != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.fadeSampleVolumeStart : 100.0f;
			sliders->initSlider(0,0.0f, 300.0f, value,"Start");
      value = lastValues.fadeSampleVolumeStart != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.fadeSampleVolumeStart : 100.0f;
			sliders->initSlider(1,0.0f, 300.0f, value,"End");
			break;
    }
			
		case ToolHandlerResponder::SampleToolTypeChangeSign:
			dialog = new DialogWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Change sign" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterOneValue);
			static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Ignore bits from MSB [0..]");
			static_cast<DialogWithValues*>(dialog)->setValueOneRange(0, (sampleEditor->is16Bit() ? 16 : 8), 0);
			static_cast<DialogWithValues*>(dialog)->setValueOne(lastValues.changeSignIgnoreBits != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.changeSignIgnoreBits : 1);
			break;

		case ToolHandlerResponder::SampleToolTypeDCOffset:
			dialog = new DialogWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "DC offset" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterOneValue);
			static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Enter offset in percent [-100..100]");
			static_cast<DialogWithValues*>(dialog)->setValueOneRange(-100, 100.0f, 2); 
			static_cast<DialogWithValues*>(dialog)->setValueOne(lastValues.DCOffset != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.DCOffset : 0.0f);
			break;

		case ToolHandlerResponder::SampleToolTypeResample:
			dialog = new DialogResample(parentScreen, toolHandlerResponder, PP_DEFAULT_ID);
			if (sampleEditor->isValidSample())
			{
				if (lastValues.resampleInterpolationType != SampleEditorControlLastValues::invalidIntValue()) 
					static_cast<DialogResample*>(dialog)->setInterpolationType(lastValues.resampleInterpolationType);

				static_cast<DialogResample*>(dialog)->setAdjustFtAndRelnote(lastValues.adjustFtAndRelnote);

				static_cast<DialogResample*>(dialog)->setRelNote(sampleEditor->getRelNoteNum());
				static_cast<DialogResample*>(dialog)->setFineTune(sampleEditor->getFinetune());
				static_cast<DialogResample*>(dialog)->setSize(sampleEditor->getSampleLen());
			}
			break;

		case ToolHandlerResponder::SampleToolTypeEQ3Band:{
			dialog = new DialogEQ(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, DialogEQ::EQ3Bands);
      DialogEQ *eq = static_cast<DialogEQ*>(dialog);
			if (lastValues.hasEQ3BandValues)
			{
				for (pp_int32 i = 0; i < 3; i++)
					eq->setBandParam(i, lastValues.EQ3BandValues[i]);
			}
      eq->setSampleEditor(sampleEditor);
			break;
    }

		case ToolHandlerResponder::SampleToolTypeEQ10Band:
		case ToolHandlerResponder::SampleToolTypeSelectiveEQ10Band:{
			dialog = new DialogEQ(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, DialogEQ::EQ10Bands);
      DialogEQ *eq = static_cast<DialogEQ*>(dialog);
			if (lastValues.hasEQ10BandValues)
			{
				for (pp_int32 i = 0; i < 10; i++)
					eq->setBandParam(i, lastValues.EQ10BandValues[i]);
			}
      eq->setSampleEditor(sampleEditor);
			break;
    }

		case ToolHandlerResponder::SampleToolTypeGenerateSilence:
			dialog = new DialogWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Insert silence" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterOneValue);
			static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Enter size in samples:");
			static_cast<DialogWithValues*>(dialog)->setValueOneRange(3, 1024*1024*10-1, 0); 
			if (lastValues.silenceSize != SampleEditorControlLastValues::invalidIntValue())
				static_cast<DialogWithValues*>(dialog)->setValueOne((float)lastValues.silenceSize);
			else
				static_cast<DialogWithValues*>(dialog)->setValueOne(100.0f);
			break;

		case ToolHandlerResponder::SampleToolTypeGenerateNoise:
		{
			PPSimpleVector<PPString> noiseFilterTypes;

			noiseFilterTypes.add(new PPString("White"));
			noiseFilterTypes.add(new PPString("Pink"));
			noiseFilterTypes.add(new PPString("Brown"));

			dialog = new DialogGroupSelection(parentScreen, 
											  toolHandlerResponder, 
											  PP_DEFAULT_ID, 
											  "Select noise type" PPSTR_PERIODS,
											  noiseFilterTypes);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeReverb:{
			dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Reverb", 4, sampleEditor, &SampleEditor::tool_reverb );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);

      float value = lastValues.reverbDryWet       != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.reverbDryWet : 33.0f;
			sliders->initSlider(0,0,100,value,"Dry..Wet");
      value = lastValues.reverbSize   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.reverbSize : 100.0f;
			sliders->initSlider(1,0,100,value,"Size");
      value = lastValues.reverbDecay  != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.reverbDecay : 90.0f;
			sliders->initSlider(2,0,99,value,"Decay");
      value = lastValues.reverbColour  != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.reverbColour : 0.0f;
			sliders->initSlider(3,-6,6,value,"Colour");
			break;
    }

		case ToolHandlerResponder::SampleToolTypeSaturate:{
			dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Saturation", 1, sampleEditor, &SampleEditor::tool_saturate );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);

      float value = lastValues.saturate   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.saturate : 200.0f;
			sliders->initSlider(0,1,4000,value,"Harmonics");
			break;
    }

		case ToolHandlerResponder::SampleToolTypeFilter:{
			dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Filter", 5, sampleEditor, &SampleEditor::tool_filter );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      int sampleRate = 44100/2;
      float value = lastValues.filterCutoffH  != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.filterCutoffH : 45.0f;
			sliders->initSlider(0,1,sampleRate/2,value,"Highpass");
      value = lastValues.filterCutoffL   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.filterCutoffL : ((float)sampleRate)-1.0f;
			sliders->initSlider(1,1,sampleRate,value,"Lowpass");
      value = lastValues.filterRes   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.filterRes : 0.0f;
			sliders->initSlider(2,0,9,value,"Resonance");
      value = lastValues.saturate   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.saturate : 0.0f;
			sliders->initSlider(3,1,500,value,"Drive");
      value = lastValues.boostSampleVolume != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.boostSampleVolume : 100.0f;
			sliders->initSlider(4,0.0f, 300.0f, value,"Volume");
			break;
    }

		case ToolHandlerResponder::SampleToolTypeTimeStretch:{
			dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "90s Timestretch", 2, sampleEditor, &SampleEditor::tool_timestretch );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
			sliders->initSlider(0,1,10000,3900,"Grainsize");
			sliders->initSlider(1,0,8,3,"Stretch");
			break;
    }
		
		case ToolHandlerResponder::SampleToolTypeGenerateSine:
		case ToolHandlerResponder::SampleToolTypeGenerateSquare:
		case ToolHandlerResponder::SampleToolTypeGenerateTriangle:
		case ToolHandlerResponder::SampleToolTypeGenerateSawtooth:
		case ToolHandlerResponder::SampleToolTypeGenerateHalfSine:
		case ToolHandlerResponder::SampleToolTypeGenerateAbsoluteSine:
		case ToolHandlerResponder::SampleToolTypeGenerateQuarterSine:
		{
			dialog = new DialogWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Generate waveform" PPSTR_PERIODS, DialogWithValues::ValueStyleEnterTwoValues);
			static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Volume in percent:");
			static_cast<DialogWithValues*>(dialog)->setValueTwoCaption("Number of periods:");
			static_cast<DialogWithValues*>(dialog)->setValueOneRange(-1000.0f, 1000.0f, 2); 
			static_cast<DialogWithValues*>(dialog)->setValueTwoRange(0.0f, (float)(1024*1024*5), 2);
			static_cast<DialogWithValues*>(dialog)->setValueOne(lastValues.waveFormVolume != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.waveFormVolume : 100.0f);
			static_cast<DialogWithValues*>(dialog)->setValueTwo(lastValues.waveFormNumPeriods != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.waveFormNumPeriods : 1.0f);
			break;
		}
		default:
			break;
	}
	
	dialog->show();
	
	return true;
}

bool SampleEditorControl::invokeTool(ToolHandlerResponder::SampleToolTypes type)
{
	if (!sampleEditor->isValidSample())
		return false;

	switch (type)
	{
		case ToolHandlerResponder::SampleToolTypeNew:
		{
			FilterParameters par(2);
			lastValues.newSampleSize = (pp_int32)static_cast<DialogWithValues*>(dialog)->getValueOne();
			par.setParameter(0, FilterParameters::Parameter(lastValues.newSampleSize));
			par.setParameter(1, FilterParameters::Parameter(sampleEditor->is16Bit() ? 16 : 8));
			sampleEditor->tool_newSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeVolume:
		{
			lastValues.boostSampleVolume = static_cast<DialogSliders*>(dialog)->getSlider(0);
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.boostSampleVolume));
			par.setParameter(1, FilterParameters::Parameter(lastValues.boostSampleVolume));
			sampleEditor->tool_scaleSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeReverb:
		{
			DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
			lastValues.reverbDryWet  = sliders->getSlider(0);
      lastValues.reverbSize    = sliders->getSlider(1);
      lastValues.reverbDecay   = sliders->getSlider(2);
      lastValues.reverbColour  = sliders->getSlider(3);
			FilterParameters par(4);
			par.setParameter(0, FilterParameters::Parameter(lastValues.reverbDryWet));
			par.setParameter(1, FilterParameters::Parameter(lastValues.reverbSize));
			par.setParameter(2, FilterParameters::Parameter(lastValues.reverbDecay));
			par.setParameter(3, FilterParameters::Parameter(lastValues.reverbColour));
			sampleEditor->tool_reverb(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeSaturate:
		{
			DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      lastValues.saturate   = sliders->getSlider(0);
			FilterParameters par(1);
			par.setParameter(0, FilterParameters::Parameter(lastValues.saturate));
			sampleEditor->tool_saturate(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeTimeStretch:
		{
			DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter( (float)sliders->getSlider(0) ));
			par.setParameter(1, FilterParameters::Parameter( (float)sliders->getSlider(1) ));
			sampleEditor->tool_timestretch(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeFilter:
		{
			DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      lastValues.filterCutoffH     = sliders->getSlider(0);
      lastValues.filterCutoffL     = sliders->getSlider(1);
      lastValues.filterRes         = sliders->getSlider(2);
      lastValues.saturate          = sliders->getSlider(3);
      lastValues.boostSampleVolume = sliders->getSlider(4);
			FilterParameters par(5);
			par.setParameter(0, FilterParameters::Parameter(lastValues.filterCutoffH));
			par.setParameter(1, FilterParameters::Parameter(lastValues.filterCutoffL));
			par.setParameter(2, FilterParameters::Parameter(lastValues.filterRes));
			par.setParameter(3, FilterParameters::Parameter(lastValues.saturate));
			par.setParameter(4, FilterParameters::Parameter(lastValues.boostSampleVolume));
			sampleEditor->tool_filter(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeFade:
		{
			lastValues.fadeSampleVolumeStart = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.fadeSampleVolumeEnd = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.fadeSampleVolumeStart / 100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.fadeSampleVolumeEnd / 100.0f));
			sampleEditor->tool_scaleSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeChangeSign:
		{
			lastValues.changeSignIgnoreBits = (pp_int32)static_cast<DialogWithValues*>(dialog)->getValueOne();
			FilterParameters par(1);
			par.setParameter(0, FilterParameters::Parameter(lastValues.changeSignIgnoreBits));
			sampleEditor->tool_changeSignSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeDCOffset:
		{
			FilterParameters par(1);
			lastValues.DCOffset = static_cast<DialogWithValues*>(dialog)->getValueOne();
			par.setParameter(0, FilterParameters::Parameter(lastValues.DCOffset / 100.0f));
			sampleEditor->tool_DCOffsetSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeResample:
		{
			lastValues.resampleInterpolationType = static_cast<DialogResample*>(dialog)->getInterpolationType();
			lastValues.adjustFtAndRelnote = static_cast<DialogResample*>(dialog)->getAdjustFtAndRelnote();
			lastValues.adjustSampleOffsetCommand = static_cast<DialogResample*>(dialog)->getAdjustSampleOffsetCommand();

			FilterParameters par(4);
			par.setParameter(0, FilterParameters::Parameter(static_cast<DialogResample*>(dialog)->getC4Speed()));
			par.setParameter(1, FilterParameters::Parameter(static_cast<pp_int32>(lastValues.resampleInterpolationType)));
			par.setParameter(2, FilterParameters::Parameter(lastValues.adjustFtAndRelnote ? 1 : 0));
			par.setParameter(3, FilterParameters::Parameter(lastValues.adjustSampleOffsetCommand ? 1 : 0));
			sampleEditor->tool_resampleSample(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeEQ3Band:
		case ToolHandlerResponder::SampleToolTypeEQ10Band:
		case ToolHandlerResponder::SampleToolTypeSelectiveEQ10Band:
		{
			pp_uint32 numBands = static_cast<DialogEQ*>(dialog)->getNumBandsAsInt();
			
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
			
			DialogEQ *dialogEQ = static_cast<DialogEQ*>(dialog);
			FilterParameters par(numBands);
			for (pp_uint32 i = 0; i < numBands; i++)
			{
				float val = dialogEQ->getBandParam(i);
				if (last)
					last[i] = val;
				par.setParameter(i, FilterParameters::Parameter(val));
			}
			sampleEditor->tool_eqSample(&par,type==ToolHandlerResponder::SampleToolTypeSelectiveEQ10Band);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateSilence:
		{
			FilterParameters par(1);
			par.setParameter(0, FilterParameters::Parameter((pp_int32)(static_cast<DialogWithValues*>(dialog)->getValueOne())));
			sampleEditor->tool_generateSilence(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateNoise:
		{
			FilterParameters par(1);
			par.setParameter(0, FilterParameters::Parameter((pp_int32)(static_cast<DialogGroupSelection*>(dialog)->getSelection())));
			sampleEditor->tool_generateNoise(&par);
			break;
		}
		
		case ToolHandlerResponder::SampleToolTypeGenerateSine:
		{
			lastValues.waveFormVolume = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.waveFormVolume/100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.waveFormNumPeriods));
			sampleEditor->tool_generateSine(&par);
			break;
		}
		
		case ToolHandlerResponder::SampleToolTypeGenerateSquare:
		{
			lastValues.waveFormVolume = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.waveFormVolume/100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.waveFormNumPeriods));
			sampleEditor->tool_generateSquare(&par);
			break;
		}
		
		case ToolHandlerResponder::SampleToolTypeGenerateTriangle:
		{
			lastValues.waveFormVolume = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.waveFormVolume/100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.waveFormNumPeriods));
			sampleEditor->tool_generateTriangle(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateSawtooth:
		{
			lastValues.waveFormVolume = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.waveFormVolume/100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.waveFormNumPeriods));
			sampleEditor->tool_generateSawtooth(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateHalfSine:
		{
			lastValues.waveFormVolume = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.waveFormVolume / 100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.waveFormNumPeriods));
			sampleEditor->tool_generateHalfSine(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateAbsoluteSine:
		{
			lastValues.waveFormVolume = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.waveFormVolume / 100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.waveFormNumPeriods));
			sampleEditor->tool_generateAbsoluteSine(&par);
			break;
		}

		case ToolHandlerResponder::SampleToolTypeGenerateQuarterSine:
		{
			lastValues.waveFormVolume = static_cast<DialogWithValues*>(dialog)->getValueOne();
			lastValues.waveFormNumPeriods = static_cast<DialogWithValues*>(dialog)->getValueTwo();
			FilterParameters par(2);
			par.setParameter(0, FilterParameters::Parameter(lastValues.waveFormVolume / 100.0f));
			par.setParameter(1, FilterParameters::Parameter(lastValues.waveFormNumPeriods));
			sampleEditor->tool_generateQuarterSine(&par);
			break;
		}

		default:
			break;
	}
	
	return true;
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
