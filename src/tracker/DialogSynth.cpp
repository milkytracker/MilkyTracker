/*
 *  tracker/DialogSynth.cpp
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
 *  DialogSynth.cpp
 *  MilkyTracker
 *
 */

#include "DialogSynth.h"
#include "MessageBoxContainer.h"
#include "RadioGroup.h"
#include "StaticText.h"
#include "Slider.h"
#include "CheckBox.h"
#include "SynthHarmonica.h"
#include "Screen.h"
#include "XModule.h"
#include "Tracker.h"
#include "ModuleEditor.h"
#include "SectionSamples.h"
#include "tmm.h"
#include <math.h>
#include <time.h>

enum {
	RADIOGROUP_SYNTHTYPE = 1337000,
	RADIOGROUP_NOISETYPE,
	RADIOGROUP_NOISEPHASENOISETYPE,
	CONTAINER_SINE,
	CONTAINER_PULSE,
	CONTAINER_NOISE,
	CONTAINER_ADDITIVE,
	SYNTH_HARMONICA,
	STATICTEXT_ADDITIVE_HARMONICS,
	STATICTEXT_ADDITIVE_BASEFREQ,
	STATICTEXT_ADDITIVE_USESCALE,
	STATICTEXT_ADDITIVE_BANDWIDTH,
	STATICTEXT_ADDITIVE_DETUNE,
	STATICTEXT_ADDITIVE_BWSCALE,
	STATICTEXT_ADDITIVE_RNDSEED,
	STATICTEXT_ADDITIVE_ENV_ATT,
	STATICTEXT_ADDITIVE_ENV_DEC,
	STATICTEXT_ADDITIVE_ENV_SUS,
	STATICTEXT_ADDITIVE_ENV_HOLD,
	STATICTEXT_ADDITIVE_ENV_REL,
	STATICTEXT_ADDITIVE_LPFREQ,
	STATICTEXT_ADDITIVE_HPFREQ,
	STATICTEXT_ADDITIVE_WAVEFORM,
	STATICTEXT_ADDITIVE_PHASENOISE,
	STATICTEXT_ADDITIVE_DESTROYER,
	STATICTEXT_SINE_BASEFREQ,
	STATICTEXT_PULSE_BASEFREQ,
	STATICTEXT_PULSE_WIDTH,
	STATICTEXT_NOISE_TYPE,
	VALUE_ADDITIVE_HARMONICS,
	VALUE_ADDITIVE_BASEFREQ,
	VALUE_ADDITIVE_BANDWIDTH,
	VALUE_ADDITIVE_DETUNE,
	VALUE_ADDITIVE_BWSCALE,
	VALUE_ADDITIVE_RNDSEED,
	VALUE_ADDITIVE_LPFREQ,
	VALUE_ADDITIVE_HPFREQ,
	VALUE_ADDITIVE_ENV_ATT,
	VALUE_ADDITIVE_ENV_DEC,
	VALUE_ADDITIVE_ENV_SUS,
	VALUE_ADDITIVE_ENV_HOLD,
	VALUE_ADDITIVE_ENV_REL,
	VALUE_PULSE_BASEFREQ,
	VALUE_PULSE_WIDTH,
	VALUE_SINE_BASEFREQ,
	SLIDER_ADDITIVE_HARMONICS,
	SLIDER_ADDITIVE_BASEFREQ,
	SLIDER_ADDITIVE_BANDWIDTH,
	SLIDER_ADDITIVE_DETUNE,
	SLIDER_ADDITIVE_BWSCALE,
	SLIDER_ADDITIVE_RNDSEED,
	SLIDER_ADDITIVE_LPFREQ,
	SLIDER_ADDITIVE_HPFREQ,
	SLIDER_ADDITIVE_ENV_ATT,
	SLIDER_ADDITIVE_ENV_DEC,
	SLIDER_ADDITIVE_ENV_SUS,
	SLIDER_ADDITIVE_ENV_HOLD,
	SLIDER_ADDITIVE_ENV_REL,
	SLIDER_PULSE_BASEFREQ,
	SLIDER_PULSE_WIDTH,
	SLIDER_SINE_BASEFREQ,
	CHECKBOX_ADDITIVE_USESCALE,
	CHECKBOX_ADDITIVE_DESTROYER,
	BUTTON_HARMONICA_GEN_ZERO,
	BUTTON_HARMONICA_GEN_RANDOM,
	BUTTON_HARMONICA_GEN_FULL,
	BUTTON_HARMONICA_MOD_LINEAR,
	BUTTON_HARMONICA_MOD_RECIPROCAL,
	BUTTON_HARMONICA_MULT_FORMANTS,
	BUTTON_HARMONICA_HALF_ALL,
	BUTTON_HARMONICA_DOUBLE_ALL,
	BUTTON_HARMONICA_HALF_EVEN,
	BUTTON_HARMONICA_DOUBLE_EVEN,
	BUTTON_HARMONICA_HALF_ODD,
	BUTTON_HARMONICA_DOUBLE_ODD,
	BUTTON_HARMONICA_REVERSE,
	BUTTON_HARMONICA_INVERT,
	BUTTON_HARMONICA_SMOOTH,
	BUTTON_HARMONICA_UNDO,
	BUTTON_LOAD_INSTRUMENT,
	BUTTON_SAVE_INSTRUMENT,
	CHECKBOX_LOOP_FORWARD,
	CHECKBOX_FIX_ZERO_CROSSING,
	CHECKBOX_FIX_DC,
	STATICTEXT_LOOP_FORWARD,
	STATICTEXT_FIX_ZERO_CROSSING,
	STATICTEXT_FIX_DC
};

static float undo[128] = {0.0f};

DialogSynth::DialogSynth(
	PPScreen* screen,
	DialogResponder* responder,
	pp_int32 id,
	const PPString& caption,
	Tracker* t,
	pp_uint32 index
) : PPDialogBase(), tracker(t)
{
	PPButton *button;

	initDialog(screen, responder, id, caption, 485, 480, 26, "Close");

	mod = tracker->getModuleEditor()->getModule();
	idx = index;
	tmm = new TMM(44100, 16);

	pp_int32 x = getMessageBoxContainer()->getLocation().x, y = getMessageBoxContainer()->getLocation().y;

	radioType = new PPRadioGroup(RADIOGROUP_SYNTHTYPE, screen, this, PPPoint(x + 20, y + 40), PPSize(100, 100));
	radioType->addItem("None");
	radioType->addItem("ADX");
	radioType->addItem("Noise");
	radioType->addItem("Sine");
	radioType->addItem("Pulse");
	radioType->addItem("Additive");

	messageBoxContainerGeneric->addControl(radioType);

	checkBoxLoopForward = new PPCheckBox(CHECKBOX_LOOP_FORWARD, screen, this, PPPoint(x + 20, y + 40 + 100), false);
	messageBoxContainerGeneric->addControl(checkBoxLoopForward);
	messageBoxContainerGeneric->addControl(new PPStaticText(STATICTEXT_LOOP_FORWARD, screen, NULL, PPPoint(x + 20 + 15, y + 40 + 100 + 2), "Loop Fwd", true));

	checkBoxFixZeroCrossing = new PPCheckBox(CHECKBOX_FIX_ZERO_CROSSING, screen, this, PPPoint(x + 20, y + 40 + 100 + 15), false);
	messageBoxContainerGeneric->addControl(checkBoxFixZeroCrossing);
	messageBoxContainerGeneric->addControl(new PPStaticText(STATICTEXT_FIX_ZERO_CROSSING, screen, NULL, PPPoint(x + 20 + 15, y + 40 + 100 + 15 + 2), "Fix 0", true));

	checkBoxFixDC = new PPCheckBox(CHECKBOX_FIX_DC, screen, this, PPPoint(x + 20, y + 40 + 100 + 15 + 15), false);
	messageBoxContainerGeneric->addControl(checkBoxFixDC);
	messageBoxContainerGeneric->addControl(new PPStaticText(STATICTEXT_FIX_DC, screen, NULL, PPPoint(x + 20 + 15, y + 40 + 100 + 15 + 15 + 2), "Fix DC", true));

	button = new PPButton(BUTTON_LOAD_INSTRUMENT, screen, this, PPPoint(x + 20, y + 40 + 100 + 15 + 15 + 25), PPSize(100, 11));
	button->setText("Load TMI");
	messageBoxContainerGeneric->addControl(button);

	button = new PPButton(BUTTON_SAVE_INSTRUMENT, screen, this, PPPoint(x + 20, y + 40 + 100 + 15 + 15 + 25 + 15), PPSize(100, 11));
	button->setText("Save TMI");
	messageBoxContainerGeneric->addControl(button);

	pp_int32 cx = x + 130, cy = y + 40, rx = cx + 335;

	containerNoise = new PPContainer(CONTAINER_NOISE, screen, this, PPPoint(cx, cy), PPSize(335, 400));
	{
		containerNoise->addControl(new PPStaticText(STATICTEXT_NOISE_TYPE, screen, NULL, PPPoint(cx + 5, cy + 5),  "Type", true));

		radioNoiseType = new PPRadioGroup(RADIOGROUP_NOISETYPE, screen, this, PPPoint(rx - 180, cy + 5), PPSize(100, 100));
		radioNoiseType->addItem("White");
		radioNoiseType->addItem("Pink");
		radioNoiseType->addItem("Brown");
		containerNoise->addControl(radioNoiseType);

		containerNoise->hide(true);
	}
	messageBoxContainerGeneric->addControl(containerNoise);

	containerSine = new PPContainer(CONTAINER_SINE, screen, this, PPPoint(cx, cy), PPSize(335, 400));
	{
		containerSine->addControl(new PPStaticText(STATICTEXT_SINE_BASEFREQ, screen, NULL, PPPoint(cx + 5, cy + 5),  "Base freq", true));

		containerSine->addControl(new PPStaticText(VALUE_SINE_BASEFREQ, screen, NULL, PPPoint(rx - 180, cy + 5), ""));
		sliderSineBasefreq = new PPSlider(SLIDER_SINE_BASEFREQ, screen, this, PPPoint(rx - 120, cy + 5), 110, true);
		sliderSineBasefreq->setBarSize(1);
		sliderSineBasefreq->setMinValue(1);
		sliderSineBasefreq->setMaxValue(4400);
		containerSine->addControl(sliderSineBasefreq);

		containerSine->hide(true);
	}
	messageBoxContainerGeneric->addControl(containerSine);

	containerPulse = new PPContainer(CONTAINER_PULSE, screen, this, PPPoint(cx, cy), PPSize(335, 400));
	{
		containerPulse->addControl(new PPStaticText(STATICTEXT_PULSE_BASEFREQ, screen, NULL, PPPoint(cx + 5, cy + 5),  "Base freq",   true));
		containerPulse->addControl(new PPStaticText(STATICTEXT_PULSE_WIDTH,    screen, NULL, PPPoint(cx + 5, cy + 20), "Pulse width", true));

		containerPulse->addControl(new PPStaticText(VALUE_PULSE_BASEFREQ, screen, NULL, PPPoint(rx - 180, cy + 5), ""));
		sliderPulseBasefreq = new PPSlider(SLIDER_PULSE_BASEFREQ, screen, this, PPPoint(rx - 120, cy + 5), 110, true);
		sliderPulseBasefreq->setBarSize(1);
		sliderPulseBasefreq->setMinValue(1);
		sliderPulseBasefreq->setMaxValue(4400);
		containerPulse->addControl(sliderPulseBasefreq);

		containerPulse->addControl(new PPStaticText(VALUE_PULSE_WIDTH, screen, NULL, PPPoint(rx - 180, cy + 20), ""));
		sliderPulseWidth = new PPSlider(SLIDER_PULSE_WIDTH, screen, this, PPPoint(rx - 120, cy + 20), 110, true);
		sliderPulseWidth->setBarSize(1);
		sliderPulseWidth->setMinValue(1);
		sliderPulseWidth->setMaxValue(31);
		containerPulse->addControl(sliderPulseWidth);

		containerPulse->hide(true);
	}
	messageBoxContainerGeneric->addControl(containerPulse);

	containerAdditive = new PPContainer(CONTAINER_ADDITIVE, screen, this, PPPoint(cx, cy), PPSize(335, 400));
	{
		pp_int32 cry = 5;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_HARMONICS, screen, NULL, PPPoint(cx + 5, cy + cry), "Harmonics", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_HARMONICS, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveHarmonics = new PPSlider(SLIDER_ADDITIVE_HARMONICS, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveHarmonics->setBarSize(256);
		sliderAdditiveHarmonics->setMinValue(4);
		sliderAdditiveHarmonics->setMaxValue(64);
		containerAdditive->addControl(sliderAdditiveHarmonics);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_BASEFREQ, screen, NULL, PPPoint(cx + 5, cy + cry), "Base freq", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_BASEFREQ, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveBasefreq = new PPSlider(SLIDER_ADDITIVE_BASEFREQ, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveBasefreq->setBarSize(1);
		sliderAdditiveBasefreq->setMinValue(1);
		sliderAdditiveBasefreq->setMaxValue(1000);
		containerAdditive->addControl(sliderAdditiveBasefreq);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_BANDWIDTH, screen, NULL, PPPoint(cx + 5, cy + cry), "Bandwidth", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_BANDWIDTH, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveBandwidth = new PPSlider(SLIDER_ADDITIVE_BANDWIDTH, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveBandwidth->setBarSize(256);
		sliderAdditiveBandwidth->setMinValue(1);
		sliderAdditiveBandwidth->setMaxValue(200);
		containerAdditive->addControl(sliderAdditiveBandwidth);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_DETUNE, screen, NULL, PPPoint(cx + 5, cy + cry), "Detune", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_DETUNE, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveDetune = new PPSlider(SLIDER_ADDITIVE_DETUNE, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveDetune->setBarSize(256);
		sliderAdditiveDetune->setMinValue(0);
		sliderAdditiveDetune->setMaxValue(128);
		containerAdditive->addControl(sliderAdditiveDetune);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_BWSCALE, screen, NULL, PPPoint(cx + 5, cy + cry), "BW scale", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_BWSCALE, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveBWScale = new PPSlider(SLIDER_ADDITIVE_BWSCALE, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveBWScale->setBarSize(256);
		sliderAdditiveBWScale->setMinValue(10);
		sliderAdditiveBWScale->setMaxValue(1000);
		containerAdditive->addControl(sliderAdditiveBWScale);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_USESCALE, screen, NULL, PPPoint(cx + 5, cy + cry), "Use BW scale", true));
		checkBoxUseScale = new PPCheckBox(CHECKBOX_ADDITIVE_USESCALE, screen, this, PPPoint(rx - 120, cy + cry), false);
		containerAdditive->addControl(checkBoxUseScale);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_DESTROYER, screen, NULL, PPPoint(cx + 5, cy + cry), "Destroyer mode", true));
		checkBoxDestroyer = new PPCheckBox(CHECKBOX_ADDITIVE_DESTROYER, screen, this, PPPoint(rx - 120, cy + cry), false);
		containerAdditive->addControl(checkBoxDestroyer);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_RNDSEED, screen, NULL, PPPoint(cx + 5, cy + cry), "RND seed", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_RNDSEED, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveRandomSeed = new PPSlider(SLIDER_ADDITIVE_RNDSEED, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveRandomSeed->setBarSize(256);
		sliderAdditiveRandomSeed->setMinValue(1);
		sliderAdditiveRandomSeed->setMaxValue(2500);
		containerAdditive->addControl(sliderAdditiveRandomSeed);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_PHASENOISE, screen, NULL, PPPoint(cx + 5, cy + cry), "Phase noise", true));
		radioNoisePhaseNoiseType = new PPRadioGroup(RADIOGROUP_NOISEPHASENOISETYPE, screen, this, PPPoint(rx - 120, cy + cry), PPSize(100, 50));
		radioNoisePhaseNoiseType->addItem("White");
		radioNoisePhaseNoiseType->addItem("Pink");
		radioNoisePhaseNoiseType->addItem("Brown");
		containerAdditive->addControl(radioNoisePhaseNoiseType);
		cry += 50;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_LPFREQ, screen, NULL, PPPoint(cx + 5, cy + cry), "LP cutoff", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_LPFREQ, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveLoPassFreq = new PPSlider(SLIDER_ADDITIVE_LPFREQ, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveLoPassFreq->setBarSize(256);
		sliderAdditiveLoPassFreq->setMinValue(1);
		sliderAdditiveLoPassFreq->setMaxValue(220);
		containerAdditive->addControl(sliderAdditiveLoPassFreq);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_HPFREQ, screen, NULL, PPPoint(cx + 5, cy + cry), "HP cutoff", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_HPFREQ, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveHiPassFreq = new PPSlider(SLIDER_ADDITIVE_HPFREQ, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveHiPassFreq->setBarSize(256);
		sliderAdditiveHiPassFreq->setMinValue(1);
		sliderAdditiveHiPassFreq->setMaxValue(220);
		containerAdditive->addControl(sliderAdditiveHiPassFreq);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_ENV_ATT, screen, NULL, PPPoint(cx + 5, cy + cry), "Attack", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_ENV_ATT, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveEnvAtt = new PPSlider(SLIDER_ADDITIVE_ENV_ATT, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveEnvAtt->setBarSize(256);
		sliderAdditiveEnvAtt->setMinValue(0);
		sliderAdditiveEnvAtt->setMaxValue(32768);
		containerAdditive->addControl(sliderAdditiveEnvAtt);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_ENV_DEC, screen, NULL, PPPoint(cx + 5, cy + cry), "Decay", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_ENV_DEC, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveEnvDec = new PPSlider(SLIDER_ADDITIVE_ENV_DEC, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveEnvDec->setBarSize(256);
		sliderAdditiveEnvDec->setMinValue(0);
		sliderAdditiveEnvDec->setMaxValue(32768);
		containerAdditive->addControl(sliderAdditiveEnvDec);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_ENV_SUS, screen, NULL, PPPoint(cx + 5, cy + cry), "Sustain level", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_ENV_SUS, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveEnvSus = new PPSlider(SLIDER_ADDITIVE_ENV_SUS, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveEnvSus->setBarSize(256);
		sliderAdditiveEnvSus->setMinValue(0);
		sliderAdditiveEnvSus->setMaxValue(32768);
		containerAdditive->addControl(sliderAdditiveEnvSus);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_ENV_HOLD, screen, NULL, PPPoint(cx + 5, cy + cry), "Hold", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_ENV_HOLD, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveEnvHold = new PPSlider(SLIDER_ADDITIVE_ENV_HOLD, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveEnvHold->setBarSize(256);
		sliderAdditiveEnvHold->setMinValue(0);
		sliderAdditiveEnvHold->setMaxValue(32768);
		containerAdditive->addControl(sliderAdditiveEnvHold);
		cry += 15;

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_ENV_REL, screen, NULL, PPPoint(cx + 5, cy + cry), "Release", true));
		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_ENV_REL, screen, NULL, PPPoint(rx - 180, cy + cry), ""));
		sliderAdditiveEnvRel = new PPSlider(SLIDER_ADDITIVE_ENV_REL, screen, this, PPPoint(rx - 120, cy + cry), 110, true);
		sliderAdditiveEnvRel->setBarSize(256);
		sliderAdditiveEnvRel->setMinValue(0);
		sliderAdditiveEnvRel->setMaxValue(32768);
		containerAdditive->addControl(sliderAdditiveEnvRel);
		cry += 15;

		// ------------------------------------------------------

		synthHarmonica = new SynthHarmonica(SYNTH_HARMONICA, screen, this, PPPoint(cx + 5, cy + cry), PPSize(324, 65), this);
		containerAdditive->addControl(synthHarmonica);
		cry += 65 + 5;

		button = new PPButton(BUTTON_HARMONICA_GEN_ZERO, screen, this, PPPoint(cx + 5, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Gen Zero");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_GEN_RANDOM, screen, this, PPPoint(cx + 5 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Gen Random");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_GEN_FULL, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Gen Full");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_MOD_LINEAR, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Mod Linear");
		containerAdditive->addControl(button);
		cry += 11;

		button = new PPButton(BUTTON_HARMONICA_MOD_RECIPROCAL, screen, this, PPPoint(cx + 5, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Mod Reciprocal");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_HALF_ALL, screen, this, PPPoint(cx + 5 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Half all");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_DOUBLE_ALL, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Double all");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_HALF_EVEN, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Half even");
		containerAdditive->addControl(button);
		cry += 11;

		button = new PPButton(BUTTON_HARMONICA_DOUBLE_EVEN, screen, this, PPPoint(cx + 5, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Double even");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_HALF_ODD, screen, this, PPPoint(cx + 5 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Half odd");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_DOUBLE_ODD, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Double odd");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_MULT_FORMANTS, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Vocalize");
		containerAdditive->addControl(button);
		cry += 11;

		button = new PPButton(BUTTON_HARMONICA_INVERT, screen, this, PPPoint(cx + 5, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Invert");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_REVERSE, screen, this, PPPoint(cx + 5 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Reverse");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_SMOOTH, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Smooth");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_UNDO, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1 + 80 + 1, cy + cry), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Undo");
		containerAdditive->addControl(button);

		containerAdditive->hide(true);
	}
	messageBoxContainerGeneric->addControl(containerAdditive);

	loadSettings();
}

DialogSynth::~DialogSynth()
{
	delete tmm;
}


pp_int32 DialogSynth::loadSettings()
{
	TTMMSettings* settings = &mod->instr[idx].tmm;

	radioType->setChoice(settings->type);
	radioNoiseType->setChoice(settings->noise.type);

#	define SLIDER_SET_VALUE(SL, CONT, ST, VAL) do { \
		reinterpret_cast<PPStaticText*>((CONT)->getControlByID(ST))->setValue(VAL, false); \
		(SL)->setCurrentValue(VAL); \
	} while(0);

	SLIDER_SET_VALUE(sliderSineBasefreq,       containerSine,     VALUE_SINE_BASEFREQ,      settings->sine.basefreq);
	SLIDER_SET_VALUE(sliderPulseBasefreq,      containerPulse,    VALUE_PULSE_BASEFREQ,     settings->pulse.basefreq);
	SLIDER_SET_VALUE(sliderPulseWidth,         containerPulse,    VALUE_PULSE_WIDTH,        settings->pulse.width);
	SLIDER_SET_VALUE(sliderAdditiveBasefreq,   containerAdditive, VALUE_ADDITIVE_BASEFREQ,  settings->additive.basefreq);
	SLIDER_SET_VALUE(sliderAdditiveHarmonics,  containerAdditive, VALUE_ADDITIVE_HARMONICS, settings->additive.nharmonics);
	SLIDER_SET_VALUE(sliderAdditiveBandwidth,  containerAdditive, VALUE_ADDITIVE_BANDWIDTH, settings->additive.bandwidth);
	SLIDER_SET_VALUE(sliderAdditiveDetune,     containerAdditive, VALUE_ADDITIVE_DETUNE,    settings->additive.detune);
	SLIDER_SET_VALUE(sliderAdditiveBWScale,    containerAdditive, VALUE_ADDITIVE_BWSCALE,   settings->additive.bwscale);
	SLIDER_SET_VALUE(sliderAdditiveRandomSeed, containerAdditive, VALUE_ADDITIVE_RNDSEED,   settings->additive.rndseed);
	SLIDER_SET_VALUE(sliderAdditiveLoPassFreq, containerAdditive, VALUE_ADDITIVE_LPFREQ,    settings->additive.lpfreq);
	SLIDER_SET_VALUE(sliderAdditiveHiPassFreq, containerAdditive, VALUE_ADDITIVE_HPFREQ,    settings->additive.hpfreq);
	SLIDER_SET_VALUE(sliderAdditiveEnvAtt,     containerAdditive, VALUE_ADDITIVE_ENV_ATT,   settings->additive.envatt);
	SLIDER_SET_VALUE(sliderAdditiveEnvDec,     containerAdditive, VALUE_ADDITIVE_ENV_DEC,   settings->additive.envdec);
	SLIDER_SET_VALUE(sliderAdditiveEnvSus,     containerAdditive, VALUE_ADDITIVE_ENV_SUS,   settings->additive.envsus);
	SLIDER_SET_VALUE(sliderAdditiveEnvHold,    containerAdditive, VALUE_ADDITIVE_ENV_HOLD,  settings->additive.envhold);
	SLIDER_SET_VALUE(sliderAdditiveEnvRel,     containerAdditive, VALUE_ADDITIVE_ENV_REL,   settings->additive.envrel);

	checkBoxUseScale->checkIt(settings->additive.usescale);
	checkBoxDestroyer->checkIt(settings->additive.destroyer);
	checkBoxLoopForward->checkIt(settings->extensions.flags & TMM_FLAG_LOOP_FWD);
	checkBoxFixZeroCrossing->checkIt(settings->extensions.flags & TMM_FLAG_FIX_ZERO);
	checkBoxFixDC->checkIt(settings->extensions.flags & TMM_FLAG_FIX_DC);

	radioNoisePhaseNoiseType->setChoice(settings->additive.phasenoisetype);

	PPEvent e(eValueChanged);
	handleEvent(reinterpret_cast<PPObject*>(sliderAdditiveHarmonics), &e);

	parentScreen->paint();

	for(int i = 0; i < 64; i++) {
		synthHarmonica->wave[i] = ((float)settings->additive.harmonics[i]) / 255.0f;
	}

	parentScreen->paintControl(synthHarmonica);

	enableContainer(mod->instr[idx].tmm.type);

	generateSample();

	return 0;
}


void DialogSynth::enableContainer(pp_uint32 choice)
{
	containerNoise->hide(true);
	containerSine->hide(true);
	containerPulse->hide(true);
	containerAdditive->hide(true);

	switch(choice) {
	case 2: containerNoise->show(true); break;
	case 3: containerSine->show(true); break;
	case 4: containerPulse->show(true); break;
	case 5: containerAdditive->show(true); break;
	}

	parentScreen->paint();
}


pp_int32 DialogSynth::handleEvent(PPObject* sender, PPEvent* event)
{
	switch (event->getID())
	{
	case eKeyDown:
		return tracker->handleEvent(sender, event);
	case eHarmonicaUpdated:
		{
			for(int i = 0; i < 64; i++) {
				mod->instr[idx].tmm.additive.harmonics[i] = (int)(synthHarmonica->wave[i] * 255.0f);
			}

			generateSample();
		}
		break;
	case eCommand:
		{
			bool updateHarmonica = false;
			float store[128];
			memcpy(store, synthHarmonica->wave, 128 * sizeof(float));

			switch (reinterpret_cast<PPControl*>(sender)->getID()) {
			case BUTTON_LOAD_INSTRUMENT:
				{
					tracker->getModuleEditor()->setDialogSynth(this);
					tracker->loadType(FileTypes::FileTypeInstrumentTMI);
				}
				break;
			case BUTTON_SAVE_INSTRUMENT:
				{
					tracker->saveType(FileTypes::FileTypeInstrumentTMI);
				}
				break;
			case CHECKBOX_ADDITIVE_USESCALE:
				{
					bool checked = reinterpret_cast<PPCheckBox*>(sender)->isChecked();
					mod->instr[idx].tmm.additive.usescale = (int)checked;

					generateSample();
				}
				break;
			case CHECKBOX_ADDITIVE_DESTROYER:
				{
					bool checked = reinterpret_cast<PPCheckBox*>(sender)->isChecked();
					mod->instr[idx].tmm.additive.destroyer = (int)checked;

					generateSample();
				}
				break;
			case CHECKBOX_LOOP_FORWARD:
				{
					bool checked = reinterpret_cast<PPCheckBox*>(sender)->isChecked();

					if(checked) {
						mod->instr[idx].tmm.extensions.flags |= TMM_FLAG_LOOP_FWD;
					} else {
						mod->instr[idx].tmm.extensions.flags &= ~TMM_FLAG_LOOP_FWD;
					}

					generateSample();
				}
				break;
			case CHECKBOX_FIX_ZERO_CROSSING:
				{
					bool checked = reinterpret_cast<PPCheckBox*>(sender)->isChecked();

					if(checked) {
						mod->instr[idx].tmm.extensions.flags |= TMM_FLAG_FIX_ZERO;
					} else {
						mod->instr[idx].tmm.extensions.flags &= ~TMM_FLAG_FIX_ZERO;
					}

					generateSample();
				}
				break;
			case CHECKBOX_FIX_DC:
				{
					bool checked = reinterpret_cast<PPCheckBox*>(sender)->isChecked();

					if(checked) {
						mod->instr[idx].tmm.extensions.flags |= TMM_FLAG_FIX_DC;
					} else {
						mod->instr[idx].tmm.extensions.flags &= ~TMM_FLAG_FIX_DC;
					}

					generateSample();
				}
				break;
			case BUTTON_HARMONICA_GEN_ZERO:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] = 0.0f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_GEN_RANDOM:
				srand(time(NULL));
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] = (float)(rand()%32768) / 32768.0f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_GEN_FULL:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] = 1.0f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_MOD_LINEAR:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] *= (float)i / (float)mod->instr[idx].tmm.additive.nharmonics;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_MOD_RECIPROCAL:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] *= 1.0f / (1.0f + (float)i);
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_HALF_ALL:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] *= 0.5f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_DOUBLE_ALL:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] *= 2.0f;
					if(synthHarmonica->wave[i] > 1.0f) synthHarmonica->wave[i] = 1.0f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_HALF_EVEN:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i+=2) {
					synthHarmonica->wave[i] *= 0.5f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_DOUBLE_EVEN:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i+=2) {
					synthHarmonica->wave[i] *= 2.0f;
					if(synthHarmonica->wave[i] > 1.0f) synthHarmonica->wave[i] = 1.0f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_HALF_ODD:
				for(int i = 1; i < mod->instr[idx].tmm.additive.nharmonics; i+=2) {
					synthHarmonica->wave[i] *= 0.5f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_DOUBLE_ODD:
				for(int i = 1; i < mod->instr[idx].tmm.additive.nharmonics; i+=2) {
					synthHarmonica->wave[i] *= 2.0f;
					if(synthHarmonica->wave[i] > 1.0f) synthHarmonica->wave[i] = 1.0f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_MULT_FORMANTS:
				{
					float f1 = 130.81f * powf(2.0f, 4.0f / 12.0f);
					for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
						synthHarmonica->wave[i] *=
							expf(-powf((i * f1 - 600.0f)  / 150.0f, 2.0f)) + expf(-powf((i * f1 - 900.0f)  / 250.0f, 2.0f)) +
							expf(-powf((i * f1 - 2200.0f) / 200.0f, 2.0f)) + expf(-powf((i * f1 - 2600.0f) / 250.0f, 2.0f)) +
							expf(-powf((i * f1) / 3000.0f, 2.0f)) * 0.1f;
					}
					updateHarmonica = true;
				}
				break;
			case BUTTON_HARMONICA_REVERSE:
				{
					float copy[128];
					memcpy(copy, synthHarmonica->wave, 128 * sizeof(float));

					for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
						synthHarmonica->wave[i] = copy[mod->instr[idx].tmm.additive.nharmonics - i - 1];
					}
					updateHarmonica = true;
				}
				break;
			case BUTTON_HARMONICA_INVERT:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] = 1.0f - synthHarmonica->wave[i];
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_SMOOTH:
				{
					float copy[128];
					memcpy(copy, synthHarmonica->wave, 128 * sizeof(float));

					int nh = mod->instr[idx].tmm.additive.nharmonics;
					for(int i = 0; i < nh; i++) {
						double lh = i == 0 ? copy[nh - 1] : copy[i-1];
						double rh = i == nh - 1 ? copy[0] : copy[i+1];

						synthHarmonica->wave[i] = (lh + copy[i] + rh) / 3.0f;
					}
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_UNDO:
				memcpy(synthHarmonica->wave, undo, 128 * sizeof(float));
				updateHarmonica = true;
				break;
			}

			if(updateHarmonica) {
				parentScreen->paintControl(synthHarmonica);

				PPEvent e(eHarmonicaUpdated);
				handleEvent(this, &e);

				memcpy(undo, store, 128 * sizeof(float));
				generateSample();
			}
		}
		break;
	case eSelection:
		{
			switch (reinterpret_cast<PPControl*>(sender)->getID()) {
			case RADIOGROUP_SYNTHTYPE:
				{
					pp_uint32 choice = *((pp_uint32*)event->getDataPtr());
					enableContainer(choice);
					mod->instr[idx].tmm.type = choice;
					generateSample();
				}
				break;
			case RADIOGROUP_NOISETYPE:
				{
					pp_uint32 choice = *((pp_uint32*)event->getDataPtr());
					mod->instr[idx].tmm.noise.type = choice;
					generateSample();
				}
				break;
			case RADIOGROUP_NOISEPHASENOISETYPE:
				{
					pp_uint32 choice = *((pp_uint32*)event->getDataPtr());
					mod->instr[idx].tmm.additive.phasenoisetype = choice;
					generateSample();
				}
				break;
			}
		}
		break;
	case eValueChanged:
		{
			switch (reinterpret_cast<PPControl*>(sender)->getID()) {
			case SLIDER_ADDITIVE_HARMONICS:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();

					int i = 8;
					while(!(val & (1 << i)) && (i >= 0)) i--;
					val = 1 << i;

					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_HARMONICS))->setValue(val, false);
					parentScreen->paint();

					synthHarmonica->setHarmonics(val);

					mod->instr[idx].tmm.additive.nharmonics = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_BWSCALE:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_BWSCALE))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.bwscale = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_RNDSEED:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_RNDSEED))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.rndseed = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_ENV_ATT:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_ENV_ATT))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.envatt = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_ENV_DEC:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_ENV_DEC))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.envdec = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_ENV_SUS:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_ENV_SUS))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.envsus = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_ENV_HOLD:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_ENV_HOLD))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.envhold = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_ENV_REL:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_ENV_REL))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.envrel = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_LPFREQ:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_LPFREQ))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.lpfreq = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_HPFREQ:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_HPFREQ))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.hpfreq = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_BASEFREQ:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_BASEFREQ))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.basefreq = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_BANDWIDTH:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_BANDWIDTH))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.bandwidth = val;

					generateSample();
				}
				break;
			case SLIDER_ADDITIVE_DETUNE:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerAdditive->getControlByID(VALUE_ADDITIVE_DETUNE))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.additive.detune = val;

					generateSample();
				}
				break;
			case SLIDER_SINE_BASEFREQ:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerSine->getControlByID(VALUE_SINE_BASEFREQ))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.sine.basefreq = val;
				}
				break;
			case SLIDER_PULSE_BASEFREQ:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerPulse->getControlByID(VALUE_PULSE_BASEFREQ))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.pulse.basefreq = val;

					generateSample();
				}
				break;
			case SLIDER_PULSE_WIDTH:
				{
					pp_uint32 val = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
					reinterpret_cast<PPStaticText*>(containerPulse->getControlByID(VALUE_PULSE_WIDTH))->setValue(val, false);
					parentScreen->paint();

					mod->instr[idx].tmm.pulse.width = val;

					generateSample();
				}
				break;
			}
		}
		break;
	}

	return PPDialogBase::handleEvent(sender, event);
}

void DialogSynth::generateSample()
{
	ModuleEditor* editor = tracker->getModuleEditor();

	editor->enterCriticalSection();

	mp_sint32 smpidx = editor->instruments[idx].usedSamples[0];
	TXMSample* dst = &mod->smp[smpidx];
	if(dst->sample) {
		editor->clearSample(smpidx);
	}

	int freq = XModule::getc4spd(0, 0);

	// @todo support protracker playmode = 8-bit
	dst->type      = 16; // 16-bit
	dst->loopstart = 0;
	dst->venvnum   = editor->instruments[idx].volumeEnvelope+1;
	dst->penvnum   = editor->instruments[idx].panningEnvelope+1;
	dst->fenvnum   = 0;
	dst->vibenvnum = 0;
	dst->vibtype   = editor->instruments[idx].vibtype;
	dst->vibsweep  = editor->instruments[idx].vibsweep;
	dst->vibdepth  = editor->instruments[idx].vibdepth << 1;
	dst->vibrate   = editor->instruments[idx].vibrate;
	dst->volfade   = editor->instruments[idx].volfade << 1;

	dst->sample  = (mp_sbyte *) mod->allocSampleMem(262144 * 2); // @todo depending from synth type
	dst->samplen = this->tmm->GenerateSamples(&mod->instr[idx].tmm, (void *) dst->sample, freq);
	dst->looplen = dst->samplen;

	// Loop?
	if(mod->instr[idx].tmm.extensions.flags & TMM_FLAG_LOOP_FWD) {
		dst->type |= 1;
	}

	switch(mod->instr[idx].tmm.type) {
	case TMM_TYPE_NONE:     strcpy(mod->instr[idx].name, "");                 break;
	case TMM_TYPE_SINE:     strcpy(mod->instr[idx].name, "(magic) sine");     break;
	case TMM_TYPE_NOISE:    strcpy(mod->instr[idx].name, "(magic) noise");    break;
	case TMM_TYPE_PULSE:    strcpy(mod->instr[idx].name, "(magic) pulse");    break;
	case TMM_TYPE_ADDITIVE: strcpy(mod->instr[idx].name, "(magic) additive"); break;
	}

	editor->finishSamples();
	editor->validateInstruments();
	editor->leaveCriticalSection();

	tracker->sectionSamples->updateAfterLoad();
}