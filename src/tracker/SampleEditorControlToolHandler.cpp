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
#include "ControlIDs.h"
#include "DialogWithValues.h"
#include "DialogResample.h"
#include "DialogGroupSelection.h"
#include "DialogEQ.h"
#include "DialogSliders.h"
#include "SimpleVector.h"
#include "FilterParameters.h"
#include "SectionSamples.h"
#include "PatternEditor.h"
#include "Addon.h"
#include "SampleLoaderSF2.h"

bool SampleEditorControl::invokeToolParameterDialog(SampleEditorControl::ToolHandlerResponder::SampleToolTypes type)
{
  if (dialog)
  {
	tracker->screen->setModalControl(NULL);
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
	  sliders->process();
      break;
    }

    case ToolHandlerResponder::SampleToolTypeFade:{
      dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Custom Fade", 2, sampleEditor, &SampleEditor::tool_scaleSample );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      float value = lastValues.fadeSampleVolumeStart != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.fadeSampleVolumeStart : 100.0f;
      sliders->initSlider(0,0.0f, 300.0f, value,"Start");
      value = lastValues.fadeSampleVolumeStart != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.fadeSampleVolumeStart : 100.0f;
      sliders->initSlider(1,0.0f, 300.0f, value,"End");
	  sliders->process();
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
        static_cast<DialogResample*>(dialog)->setAdjustSampleOffsetCommand(lastValues.adjustSampleOffsetCommand);
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

    case ToolHandlerResponder::SampleToolTypeSoothen:{
      // convolute with itself (= soothen = inversed selfresonance)													
	  sampleEditor->selectAll();														 
	  sampleEditor->copy();														 
	  sampleEditor->setLoopType(1);														 
      dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Soothen", 3, sampleEditor, &SampleEditor::tool_convolution );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      sliders->initSlider(0,0,100,100,"Dry..Wet");
      sliders->initSlider(1,1,100,1,"Smear");
      sliders->initSlider(2,0,5,5,"Soothe");
	  sliders->process();
	  break;
	}

    case ToolHandlerResponder::SampleToolTypeConvolution:{
      if ( sampleEditor->clipBoardIsEmpty() ){
        tracker->showMessageBoxSized(MESSAGEBOX_UNIVERSAL, "copy a different waveform first", Tracker::MessageBox_OK);
        return false;
      }
      dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Convolution with clipboard", 7, sampleEditor, &SampleEditor::tool_convolution );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      sliders->initSlider(0,0,100,100,"Dry..Wet");
      sliders->initSlider(1,1,100,1,"IR length");
      sliders->initSlider(2,0,5,5,"Convolve");
      sliders->initSlider(3,-100,100,0,"Contrast");
      sliders->initSlider(4,0,630,0,"Rotation");
      sliders->initSlider(5,0,100,0,"Random phase");
      sliders->initSlider(6,1,100,100,"Octave");
	  sliders->process();
	  break;
	}

    case ToolHandlerResponder::SampleToolTypeReverb:{
      dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Reverb", 2, sampleEditor, &SampleEditor::tool_reverb );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);

      float value = lastValues.reverbDryWet       != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.reverbDryWet : 20.0f;
      sliders->initSlider(0,0,100,value,"Dry..Wet");
      value = lastValues.reverbSize   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.reverbSize : 20.0f;
      sliders->initSlider(1,1,100,value,"Size");
	  sliders->process();
      break;
    }

    case ToolHandlerResponder::SampleToolTypeFoldSampleXfade:
    case ToolHandlerResponder::SampleToolTypeFoldSample:{
      FilterParameters par(1);
      par.setParameter(0, FilterParameters::Parameter( type == ToolHandlerResponder::SampleToolTypeFoldSampleXfade ? 1.0f : 0.0f ) );
      sampleEditor->tool_foldSample(&par);
      break;
    }

    case ToolHandlerResponder::SampleToolTypeSaturate:{
        TXMSample *sample = sampleEditor->getSample();
        dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Saturation", 5, sampleEditor, &SampleEditor::tool_saturate );
        DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
        float value = lastValues.saturator[0]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.saturator[0] : 18.0f;
        sliders->initSlider(0,1,100,value,"Harmonics");

        value = lastValues.saturator[1]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.saturator[1] : 0.0f;
        sliders->initSlider(1,0,100,value,"Bandpass");

        value = lastValues.saturator[2]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.saturator[2] : 50.0f;
        sliders->initSlider(2,0,150,value,"Compand");

        value = lastValues.saturator[3]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.saturator[3] : 50.0f;
        sliders->initSlider(3,1,100,value,"Dry \x1d Wet");

        value = lastValues.saturator[4]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.saturator[4] : 150.0f;
        sliders->initSlider(4,0.0f, 1000.0f, value,"Volume");
	    sliders->process();
        break;
    }

    case ToolHandlerResponder::SampleToolTypeMTBoost:{
        TXMSample *sample = sampleEditor->getSample();
        dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Milky Exciter", 4, sampleEditor, &SampleEditor::tool_MTboostSample );
        DialogSliders *sliders = static_cast<DialogSliders*>(dialog);

        float value = lastValues.milkyexcite[0]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.milkyexcite[0] : 20000.0f;
        sliders->initSlider(0,2000,28000,value,"Freq");
        value = lastValues.milkyexcite[1]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.milkyexcite[1] : 10.0f;
        sliders->initSlider(1,1,100,value,"Smear");
        value = lastValues.milkyexcite[2]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.milkyexcite[2] : 0.0f;
        sliders->initSlider(2,0,20,value,"Phase");
        value = lastValues.milkyexcite[3]   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.milkyexcite[3] : 20.0f;
        sliders->initSlider(3,1,100,value,"Wet");
	    sliders->process();
        break;
    }

    case ToolHandlerResponder::SampleToolTypeVocode:{
        if ( sampleEditor->clipBoardIsEmpty() ){
          tracker->showMessageBoxSized(MESSAGEBOX_UNIVERSAL, "copy a different waveform first", Tracker::MessageBox_OK);
          return false;
        }
        TXMSample *sample = sampleEditor->getSample();
        dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Vocode (with clipboard)", 6, sampleEditor, &SampleEditor::tool_vocodeSample );
        DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
        sliders->initSlider(0,1,9,5,"envelope");
        sliders->initSlider(1,0,1,1,"swap samples");
        sliders->initSlider(2,0,100,50,"high amp");
        sliders->initSlider(3,1,100,50,"high band");
        sliders->initSlider(4,0,100,0,"high q");
        sliders->initSlider(5,0,100,50,"volume");
	    sliders->process();
        break;
    }

    case ToolHandlerResponder::SampleToolTypeFilter:{
      dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "90s Filter", 5, sampleEditor, &SampleEditor::tool_filter );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      TXMSample *sample = sampleEditor->getSample();
      pp_int32 sampleRate = (int)(1.25 * 48000.0); // allow frequency overflow to inject aliasing 90s 'grit'
      float value = lastValues.filterCutoffH  != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.filterCutoffH : 45.0f;
      sliders->initSlider(0,1,28000,value,"Highpass");
      value = lastValues.filterCutoffL   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.filterCutoffL : ((float)sampleRate/2)-1.0f;
      PPString str = "Lowpass";
      str.append("\x1d");
      str.append("grit");
      sliders->initSlider(1,1,sampleRate/2,value,str);
      value = lastValues.filterRes   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.filterRes : 0.0f;
      sliders->initSlider(2,0,9,value,"Resonance");
      value = lastValues.filterSweep   != SampleEditorControlLastValues::invalidFloatValue() ? lastValues.filterSweep : 0.0f;
      sliders->initSlider(3,0,3,value,"Sweep");
      sliders->initSlider(4,0.0f, 1000.0f, 100.0f,"Volume");
	  sliders->process();
      break;
    }

    case ToolHandlerResponder::SampleToolTypeTimeStretch:{
      dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "90s Timestretch", 2, sampleEditor, &SampleEditor::tool_timestretch );
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      sliders->initSlider(0,1,10000,3900,"Grainsize");
      sliders->initSlider(1,0,20,3,"Stretch");
	  sliders->process();
      break;
    }

    case ToolHandlerResponder::SampleToolTypeDelay:{
        dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Delay Toolkit", 6, sampleEditor, &SampleEditor::tool_delay );
        int sampleRate = 48000;
        DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
        PPString str = "Flange";
        str.append("\x1d");
        str.append("Delay");
        sliders->initSlider(0,1,100000,15000,str);
        str = "Flange";
        str.append("\x1d");
        str.append("Echos");
        sliders->initSlider(1,1,8,3,str);
        sliders->initSlider(2,0,100,0,"Chorus detune");
        sliders->initSlider(3,0,sampleRate/2,0,"Bandpass");
        sliders->initSlider(4,1,100,0,"Saturate");
        sliders->initSlider(5,0,100,50,"Dry / Wet");
	    sliders->process();
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

    case ToolHandlerResponder::SampleToolTypeSynth:
    {
      // triggered when user presses 'synth'-button
      getSampleEditor()->getSynth()->ASCIISynthImport( PPString( getSampleEditor()->getSample()->name) );
      PatternEditor *pe = this->tracker->getPatternEditor();
      pe->getCursor().inner = 0; // force note-column to hear notes playing on keyboard
      getSampleEditor()->getSynth()->attach( getSampleEditor(), parentScreen, toolHandlerResponder, tracker );
      dialog = getSampleEditor()->getSynth()->dialog();
      static_cast<DialogSliders*>(dialog)->process();
	  tracker->screen->setFocus(this); // dont clutter live notes to pattern 
      break;
    }

	case ToolHandlerResponder::SampleToolTypeSoundfont:
	{
		dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "select soundfont sample", 1, sampleEditor, &SampleEditor::tool_soundfont );
		DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
		sliders->initSlider(0, float(0), float( SampleLoaderSF2::lastSF2FileSamples-1 ), float(0), "sample" );
		break;
	}

    case ToolHandlerResponder::SampleToolTypeAddon:
    {
		Param *params = Addon::params;
		int nparams = Addon::param_count;
		if( nparams > 0 ){ 
			dialog = new DialogSliders(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, Addon::selectedName, nparams, sampleEditor, &SampleEditor::tool_addon );
			DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
			for (int i = 0; i < nparams; i++) {
			  sliders->initSlider(i, float(params[i].min), float(params[i].max), float(params[i].value), params[i].label );
			}
	        sliders->process();
		}else{
		    FilterParameters par(0);
			sampleEditor->tool_addon(&par);
		}
      break;
    }

    default:
      break;
  }

  if( dialog != NULL ) dialog->show();

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
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeSoothen:{
      // we don't do anything here since dialogsliders processes inplace already
      break;
	}

    case ToolHandlerResponder::SampleToolTypeConvolution:{
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      lastValues.reverbDryWet  = sliders->getSlider(0);
      lastValues.reverbSize    = sliders->getSlider(1);
      // we don't do anything here since dialogsliders processes inplace already
      break;
	}

    case ToolHandlerResponder::SampleToolTypeReverb:
    {
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      lastValues.reverbDryWet  = sliders->getSlider(0);
      lastValues.reverbSize    = sliders->getSlider(1);
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeVocode:
    {
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeMTBoost:
    {
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
	  for( pp_uint8 i = 0; i < 4; i++ ) lastValues.milkyexcite[i]   = sliders->getSlider(i);
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeSaturate:
    {
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
	  for( pp_uint8 i = 0; i < 5; i++ ) lastValues.saturator[i]     = sliders->getSlider(i);
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeTimeStretch:
    {
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeDelay:
    {
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeFilter:
    {
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      lastValues.filterCutoffH     = sliders->getSlider(0);
      lastValues.filterCutoffL     = sliders->getSlider(1);
      lastValues.filterRes         = sliders->getSlider(2);
      lastValues.filterSweep       = sliders->getSlider(3);
      lastValues.boostSampleVolume = sliders->getSlider(4);
      // we don't do anything here since dialogsliders processes inplace already
      break;
    }

    case ToolHandlerResponder::SampleToolTypeFade:
    {
      DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
      lastValues.fadeSampleVolumeStart = sliders->getSlider(0);
      lastValues.fadeSampleVolumeEnd   = sliders->getSlider(1);
	  printf("%f %f\n",lastValues.fadeSampleVolumeStart, lastValues.fadeSampleVolumeEnd);
      // we don't do anything here since dialogsliders processes inplace already
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
      FilterParameters par(2);
      par.setParameter(0, FilterParameters::Parameter(1.0f) );
      par.setParameter(1, FilterParameters::Parameter((pp_int32)(static_cast<DialogGroupSelection*>(dialog)->getSelection())));
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

    case ToolHandlerResponder::SampleToolTypeSynth:
    {
      this->tracker->updateSamplesListBox();
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
