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
	STATICTEXT_ADDITIVE_BWSCALE,
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
	VALUE_ADDITIVE_BWSCALE,
	VALUE_PULSE_BASEFREQ,
	VALUE_PULSE_WIDTH,
	VALUE_SINE_BASEFREQ,
	SLIDER_ADDITIVE_HARMONICS,
	SLIDER_ADDITIVE_BASEFREQ,
	SLIDER_ADDITIVE_BANDWIDTH,
	SLIDER_ADDITIVE_BWSCALE,
	SLIDER_PULSE_BASEFREQ,
	SLIDER_PULSE_WIDTH,
	SLIDER_SINE_BASEFREQ,
	CHECKBOX_ADDITIVE_USESCALE,
	CHECKBOX_ADDITIVE_DESTROYER,
	BUTTON_HARMONICA_GEN_ZERO,
	BUTTON_HARMONICA_GEN_PROPORTIONAL,
	BUTTON_HARMONICA_GEN_RECIPROCAL,
	BUTTON_HARMONICA_GEN_FULL,
	BUTTON_HARMONICA_MULT_FORMANTS,
	BUTTON_HARMONICA_HALF_EACH,
	BUTTON_HARMONICA_DOUBLE_EACH,
	BUTTON_HARMONICA_HALF_EVEN,
	BUTTON_HARMONICA_DOUBLE_EVEN,
	BUTTON_LOAD_INSTRUMENT,
	BUTTON_SAVE_INSTRUMENT,
	CHECKBOX_LOOP_FORWARD,
	CHECKBOX_FIX_ZERO_CROSSING,
	CHECKBOX_FIX_DC,
	STATICTEXT_LOOP_FORWARD,
	STATICTEXT_FIX_ZERO_CROSSING,
	STATICTEXT_FIX_DC
};

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

		radioNoiseType = new PPRadioGroup(RADIOGROUP_NOISETYPE, screen, this, PPPoint(rx - 160, cy + 5), PPSize(100, 100));
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

		containerSine->addControl(new PPStaticText(VALUE_SINE_BASEFREQ, screen, NULL, PPPoint(rx - 160, cy + 5), ""));
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

		containerPulse->addControl(new PPStaticText(VALUE_PULSE_BASEFREQ, screen, NULL, PPPoint(rx - 160, cy + 5), ""));
		sliderPulseBasefreq = new PPSlider(SLIDER_PULSE_BASEFREQ, screen, this, PPPoint(rx - 120, cy + 5), 110, true);
		sliderPulseBasefreq->setBarSize(1);
		sliderPulseBasefreq->setMinValue(1);
		sliderPulseBasefreq->setMaxValue(4400);
		containerPulse->addControl(sliderPulseBasefreq);

		containerPulse->addControl(new PPStaticText(VALUE_PULSE_WIDTH, screen, NULL, PPPoint(rx - 160, cy + 20), ""));
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
		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_HARMONICS,  screen, NULL, PPPoint(cx + 5, cy + 5),  "Harmonics",        true));
		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_BASEFREQ,   screen, NULL, PPPoint(cx + 5, cy + 20), "Base freq",        true));
		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_BANDWIDTH,  screen, NULL, PPPoint(cx + 5, cy + 35), "Bandwidth",        true));
		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_BWSCALE,    screen, NULL, PPPoint(cx + 5, cy + 50), "BW scale",         true));
		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_USESCALE,   screen, NULL, PPPoint(cx + 5, cy + 65), "Use BW scale",     true));
		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_DESTROYER,  screen, NULL, PPPoint(cx + 5, cy + 80), "Destroyer mode",   true));
		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_PHASENOISE, screen, NULL, PPPoint(cx + 5, cy + 95), "Phase noise",      true));

		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_HARMONICS, screen, NULL, PPPoint(rx - 160, cy + 5), ""));
		sliderAdditiveHarmonics = new PPSlider(SLIDER_ADDITIVE_HARMONICS, screen, this, PPPoint(rx - 120, cy + 5), 110, true);
		sliderAdditiveHarmonics->setBarSize(256);
		sliderAdditiveHarmonics->setMinValue(4);
		sliderAdditiveHarmonics->setMaxValue(64);
		containerAdditive->addControl(sliderAdditiveHarmonics);

		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_BASEFREQ, screen, NULL, PPPoint(rx - 160, cy + 20), ""));
		sliderAdditiveBasefreq = new PPSlider(SLIDER_ADDITIVE_BASEFREQ, screen, this, PPPoint(rx - 120, cy + 20), 110, true);
		sliderAdditiveBasefreq->setBarSize(1);
		sliderAdditiveBasefreq->setMinValue(1);
		sliderAdditiveBasefreq->setMaxValue(1000);
		containerAdditive->addControl(sliderAdditiveBasefreq);

		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_BANDWIDTH, screen, NULL, PPPoint(rx - 160, cy + 35), ""));
		sliderAdditiveBandwidth = new PPSlider(SLIDER_ADDITIVE_BANDWIDTH, screen, this, PPPoint(rx - 120, cy + 35), 110, true);
		sliderAdditiveBandwidth->setBarSize(256);
		sliderAdditiveBandwidth->setMinValue(1);
		sliderAdditiveBandwidth->setMaxValue(200);
		containerAdditive->addControl(sliderAdditiveBandwidth);

		containerAdditive->addControl(new PPStaticText(VALUE_ADDITIVE_BWSCALE, screen, NULL, PPPoint(rx - 160, cy + 50), ""));
		sliderAdditiveBWScale = new PPSlider(SLIDER_ADDITIVE_BWSCALE, screen, this, PPPoint(rx - 120, cy + 50), 110, true);
		sliderAdditiveBWScale->setBarSize(256);
		sliderAdditiveBWScale->setMinValue(10);
		sliderAdditiveBWScale->setMaxValue(1000);
		containerAdditive->addControl(sliderAdditiveBWScale);

		checkBoxUseScale = new PPCheckBox(CHECKBOX_ADDITIVE_USESCALE, screen, this, PPPoint(rx - 120, cy + 65), false);
		containerAdditive->addControl(checkBoxUseScale);

		checkBoxDestroyer = new PPCheckBox(CHECKBOX_ADDITIVE_DESTROYER, screen, this, PPPoint(rx - 120, cy + 80), false);
		containerAdditive->addControl(checkBoxDestroyer);

		radioNoisePhaseNoiseType = new PPRadioGroup(RADIOGROUP_NOISEPHASENOISETYPE, screen, this, PPPoint(rx - 120, cy + 95), PPSize(100, 100));
		radioNoisePhaseNoiseType->addItem("White");
		radioNoisePhaseNoiseType->addItem("Pink");
		radioNoisePhaseNoiseType->addItem("Brown");
		containerAdditive->addControl(radioNoisePhaseNoiseType);

		// ------------------------------------------------------

		containerAdditive->addControl(new PPStaticText(STATICTEXT_ADDITIVE_WAVEFORM,  screen, NULL, PPPoint(cx + 5, cy + 180), "Wave Editor", true));

		synthHarmonica = new SynthHarmonica(SYNTH_HARMONICA, screen, this, PPPoint(cx + 5, cy + 195), PPSize(324, 160), this);
		containerAdditive->addControl(synthHarmonica);

		button = new PPButton(BUTTON_HARMONICA_GEN_ZERO, screen, this, PPPoint(cx + 5, cy + 196 + 165), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Gen Zero");
		containerAdditive->addControl(button);

		button = new PPButton(BUTTON_HARMONICA_GEN_PROPORTIONAL, screen, this, PPPoint(cx + 5 + 80 + 1, cy + 196 + 165), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Gen Proportional");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_GEN_RECIPROCAL, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1, cy + 196 + 165), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Gen Reciprocal");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_GEN_FULL, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1 + 80 + 1, cy + 196 + 165), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Gen Full");
		containerAdditive->addControl(button);

		button = new PPButton(BUTTON_HARMONICA_HALF_EACH, screen, this, PPPoint(cx + 5, cy + 196 + 165 + 10 + 1), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Half each");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_DOUBLE_EACH, screen, this, PPPoint(cx + 5 + 80 + 1, cy + 196 + 165 + 10 + 1), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Double each");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_HALF_EVEN, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1, cy + 196 + 165 + 10 + 1), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Half even");
		containerAdditive->addControl(button);
		button = new PPButton(BUTTON_HARMONICA_DOUBLE_EVEN, screen, this, PPPoint(cx + 5 + 80 + 1 + 80 + 1 + 80 + 1, cy + 196 + 165 + 10 + 1), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Double even");
		containerAdditive->addControl(button);

		button = new PPButton(BUTTON_HARMONICA_MULT_FORMANTS, screen, this, PPPoint(cx + 5, cy + 196 + 165 + 10 + 1 + 10 + 1), PPSize(80, 10));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Vocalize!");
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

	SLIDER_SET_VALUE(sliderSineBasefreq,      containerSine,     VALUE_SINE_BASEFREQ,      settings->sine.basefreq);
	SLIDER_SET_VALUE(sliderPulseBasefreq,     containerPulse,    VALUE_PULSE_BASEFREQ,     settings->pulse.basefreq);
	SLIDER_SET_VALUE(sliderPulseWidth,        containerPulse,    VALUE_PULSE_WIDTH,        settings->pulse.width);
	SLIDER_SET_VALUE(sliderAdditiveBasefreq,  containerAdditive, VALUE_ADDITIVE_BASEFREQ,  settings->additive.basefreq);
	SLIDER_SET_VALUE(sliderAdditiveHarmonics, containerAdditive, VALUE_ADDITIVE_HARMONICS, settings->additive.nharmonics);
	SLIDER_SET_VALUE(sliderAdditiveBandwidth, containerAdditive, VALUE_ADDITIVE_BANDWIDTH, settings->additive.bandwidth);
	SLIDER_SET_VALUE(sliderAdditiveBWScale,   containerAdditive, VALUE_ADDITIVE_BWSCALE,   settings->additive.bwscale);

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
			case BUTTON_HARMONICA_GEN_PROPORTIONAL:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] = (float)i / (float)mod->instr[idx].tmm.additive.nharmonics;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_GEN_RECIPROCAL:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] = 1.0f / (float)i;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_GEN_FULL:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] = 1.0f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_HALF_EACH:
				for(int i = 0; i < mod->instr[idx].tmm.additive.nharmonics; i++) {
					synthHarmonica->wave[i] *= 0.5f;
				}
				updateHarmonica = true;
				break;
			case BUTTON_HARMONICA_DOUBLE_EACH:
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
			}

			if(updateHarmonica) {
				parentScreen->paintControl(synthHarmonica);

				PPEvent e(eHarmonicaUpdated);
				handleEvent(this, &e);

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