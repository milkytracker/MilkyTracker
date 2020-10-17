/*
 *  tracker/DialogListBox.h
 *
 *  Copyright 2011 neoman
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
 *  DialogSynth.h
 *  MilkyTracker
 *
 */

#ifndef __DIALOGSYNTH_H__
#define __DIALOGSYNTH_H__

#include "DialogBase.h"
#include "Tracker.h"

class DialogSynth : public PPDialogBase
{
private:
	class PPRadioGroup   *radioType;
	class PPRadioGroup   *radioNoiseType;
	class PPRadioGroup   *radioDistType;
	class PPContainer    *containerAdditive;
	class PPContainer    *containerSine;
	class PPContainer    *containerPulse;
	class PPContainer    *containerNoise;
	class PPContainer    *containerADX;
	class PPSlider       *sliderPulseBasefreq;
	class PPSlider       *sliderPulseWidth;
	class PPSlider       *sliderSineBasefreq;
	class PPSlider       *sliderAdditiveHarmonics;
	class PPSlider       *sliderAdditiveBasefreq;
	class PPSlider       *sliderAdditiveBandwidth;
	class PPSlider       *sliderAdditiveDetune;
	class PPSlider       *sliderAdditiveBWScale;
	class PPSlider       *sliderAdditiveRandomSeed;
	class PPSlider       *sliderAdditiveLoPassFreq;
	class PPSlider       *sliderAdditiveHiPassFreq;
	class PPSlider       *sliderAdditiveEnvAtt;
	class PPSlider       *sliderAdditiveEnvDec;
	class PPSlider       *sliderAdditiveEnvSus;
	class PPSlider       *sliderAdditiveEnvHold;
	class PPSlider       *sliderAdditiveEnvRel;
	class PPSlider       *sliderAdditiveDistDrive;
	class PPSlider       *sliderAdditiveDistGain;
	class PPCheckBox     *checkBoxUseScale;
	class PPCheckBox     *checkBoxUseFilters;
	class PPCheckBox     *checkBoxUseEnv;
	class PPCheckBox     *checkBoxUseDist;
	class PPCheckBox     *checkBoxDestroyer;
	class PPCheckBox     *checkBoxLoopForward;
	class PPCheckBox     *checkBoxFixZeroCrossing;
	class PPRadioGroup   *radioNoisePhaseNoiseType;
	class SynthHarmonica *synthHarmonica;

	class Tracker *tracker;
	class XModule *mod;

	pp_uint32 idx;
public:
	DialogSynth(
		PPScreen* screen,
		DialogResponder* responder,
		pp_int32 id,
		const PPString& caption,
		Tracker* t,
		pp_uint32 index
	);
	virtual ~DialogSynth() {}

	pp_int32 loadSettings(void);
	void enableContainer(pp_uint32 choice);
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	void generateSample();
};

#endif
