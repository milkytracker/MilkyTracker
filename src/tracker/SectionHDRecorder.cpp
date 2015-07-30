/*
 *  tracker/SectionHDRecorder.cpp
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
 *  SectionHDRecorder.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.10.05.
 *
 */

#include "SectionHDRecorder.h"
#include "Container.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "ModuleEditor.h"
#include "ModuleServices.h"
#include "PlayerController.h"
#include "PlayerMaster.h"
#include "ResamplerHelper.h"

#include "PPUIConfig.h"
#include "CheckBox.h"
#include "RadioGroup.h"
#include "Seperator.h"
#include "Slider.h"
#include "StaticText.h"
#include "ListBox.h"
#include "PatternEditorControl.h"
#include "DialogListBox.h"

#include "SampleEditor.h"

#include "PPSavePanel.h"

#include "ControlIDs.h"

enum ControlIDs
{
	HDRECORD_BUTTON_EXIT			= 33000,
	HDRECORD_RADIOGROUP_FREQUENCIES,
	HDRECORD_BUTTON_RESAMPLING,
	HDRECORD_CHECKBOX_RAMPING,
	HDRECORD_CHECKBOX_ALLOWMUTING,
	HDRECORD_STATICTEXT_START,
	HDRECORD_BUTTON_START_PLUS,
	HDRECORD_BUTTON_START_MINUS,
	HDRECORD_STATICTEXT_END,
	HDRECORD_BUTTON_END_PLUS,
	HDRECORD_BUTTON_END_MINUS,
	HDRECORD_BUTTON_RECORD,
	HDRECORD_BUTTON_RECORD_AS,
	HDRECORD_RADIOGROUP_AMPLIFY,
	HDRECORD_SLIDER_MIXERVOLUME,
	HDRECORD_STATICTEXT_MIXERVOLUME,
	HDRECORD_BUTTON_RECORDINGMODE,
	HDRECORD_STATICTEXT_SAVETOFILENAME,
	HDRECORD_STATICTEXT_INS,
	HDRECORD_STATICTEXT_SMP,
	HDRECORD_BUTTON_INS_PLUS,
	HDRECORD_BUTTON_INS_MINUS,
	HDRECORD_BUTTON_SMP_PLUS,
	HDRECORD_BUTTON_SMP_MINUS,
	HDRECORD_BUTTON_MIXER_AUTO,
	
	RESPONDMESSAGEBOX_SELECTRESAMPLER
};

// Class which responds to the message box clicks
class DialogResponderHDRec : public DialogResponder
{
private:
	SectionHDRecorder& section;
	
public:
	DialogResponderHDRec(SectionHDRecorder& section) :	
		section(section)
	{
	}
	
	virtual pp_int32 ActionOkay(PPObject* sender)
	{
		switch (reinterpret_cast<PPDialogBase*>(sender)->getID())
		{
			case RESPONDMESSAGEBOX_SELECTRESAMPLER:
			{
				PPListBox* listBox = reinterpret_cast<DialogListBox*>(sender)->getListBox();
				section.storeResampler(listBox->getSelectedIndex());
				break;
			}
		}
		return 0;
	}
	
	virtual pp_int32 ActionCancel(PPObject* sender)
	{
		return 0;
	}
};


void SectionHDRecorder::validate()
{
	if (insIndex < 0)
		insIndex = 0;
	if (insIndex > tracker.moduleEditor->getNumInstruments() - 1)
		insIndex = tracker.moduleEditor->getNumInstruments() - 1;

	if (smpIndex < 0)
		smpIndex = 0;
	if (smpIndex > tracker.moduleEditor->getNumSamples(insIndex) - 1)
		smpIndex = tracker.moduleEditor->getNumSamples(insIndex) - 1;
}

SectionHDRecorder::SectionHDRecorder(Tracker& tracker) :
	SectionUpperLeft(tracker, NULL, new DialogResponderHDRec(*this)),
	recorderMode(RecorderModeToFile),
	fromOrder(0), toOrder(0), mixerVolume(256), 
	resampler(1),
	insIndex(0), smpIndex(0),
	currentFileName(TrackerConfig::untitledSong)
{
}

SectionHDRecorder::~SectionHDRecorder()
{
}

bool SectionHDRecorder::getSettingsRamping()
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPCheckBox* checkBox = static_cast<PPCheckBox*>(container->getControlByID(HDRECORD_CHECKBOX_RAMPING));
	ASSERT(checkBox);
	return checkBox->isChecked();
}

void SectionHDRecorder::setSettingsRamping(bool b)
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPCheckBox* checkBox = static_cast<PPCheckBox*>(container->getControlByID(HDRECORD_CHECKBOX_RAMPING));
	ASSERT(checkBox);
	checkBox->checkIt(b);
}

pp_uint32 SectionHDRecorder::getSettingsResampler()
{
	return resampler;
}

void SectionHDRecorder::setSettingsResampler(pp_uint32 resampler)
{
	this->resampler = resampler;
}

bool SectionHDRecorder::getSettingsAllowMuting()
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPCheckBox* checkBox = static_cast<PPCheckBox*>(container->getControlByID(HDRECORD_CHECKBOX_ALLOWMUTING));
	ASSERT(checkBox);
	return checkBox->isChecked();
}

void SectionHDRecorder::setSettingsAllowMuting(bool b)
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPCheckBox* checkBox = static_cast<PPCheckBox*>(container->getControlByID(HDRECORD_CHECKBOX_ALLOWMUTING));
	ASSERT(checkBox);
	checkBox->checkIt(b);
}

pp_int32 SectionHDRecorder::getSettingsFrequency()
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPRadioGroup* radioGroup = static_cast<PPRadioGroup*>(container->getControlByID(HDRECORD_RADIOGROUP_FREQUENCIES));
	ASSERT(radioGroup);
	ASSERT(radioGroup->getChoice() < (unsigned)TrackerConfig::numMixFrequencies);
	return TrackerConfig::mixFrequencies[radioGroup->getChoice()];
}

void SectionHDRecorder::setSettingsFrequency(pp_int32 freq)
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPRadioGroup* radioGroup = static_cast<PPRadioGroup*>(container->getControlByID(HDRECORD_RADIOGROUP_FREQUENCIES));
	ASSERT(radioGroup);
	
	for (pp_int32 j = 0; j < TrackerConfig::numMixFrequencies; j++)
	{
		if (freq == TrackerConfig::mixFrequencies[j])
		{
			radioGroup->setChoice(j);
			break;
		}
	}	
}

pp_int32 SectionHDRecorder::getSettingsMixerShift()
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPRadioGroup* radioGroup = static_cast<PPRadioGroup*>(container->getControlByID(HDRECORD_RADIOGROUP_AMPLIFY));
	ASSERT(radioGroup);
	ASSERT(radioGroup->getChoice() < 3);
	return 2-radioGroup->getChoice();
}

void SectionHDRecorder::setSettingsMixerShift(pp_int32 shift)
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPRadioGroup* radioGroup = static_cast<PPRadioGroup*>(container->getControlByID(HDRECORD_RADIOGROUP_AMPLIFY));
	ASSERT(radioGroup);
	ASSERT(shift >= 0 && shift < 3);
	radioGroup->setChoice(2-shift);
}

pp_int32 SectionHDRecorder::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case HDRECORD_BUTTON_START_PLUS:
				if (fromOrder < toOrder)
					fromOrder++;
					
				update();
				break;

			case HDRECORD_BUTTON_START_MINUS:
				if (fromOrder > 0)
					fromOrder--;
					
				update();
				break;

			case HDRECORD_BUTTON_END_PLUS:
				if (toOrder < tracker.moduleEditor->getNumOrders()-1)
					toOrder++;
					
				update();
				break;

			case HDRECORD_BUTTON_END_MINUS:
				if (toOrder > fromOrder)
					toOrder--;
					
				update();
				break;
				
			case HDRECORD_BUTTON_EXIT:	
				if (event->getID() != eCommand)
					break;
					
				show(false);	
				break;
				
			case HDRECORD_BUTTON_RECORD:
				if (event->getID() != eCommand)
					break;

				if (recorderMode == RecorderModeToFile)
				{
					if (TrackerConfig::untitledSong.compareTo(currentFileName.stripExtension()) == 0)
					{
						exportWAVAs(tracker.moduleEditor->getModuleFileNameFull());
					}
					else
					{
						exportWAVAsFileName(currentFileName);
					}
				}
				else
				{
					exportWAVAsSample();
				}
					
				update();
				break;
				
			case HDRECORD_BUTTON_RECORD_AS:
			{
				if (event->getID() != eCommand)
					break;

				ASSERT(recorderMode == RecorderModeToFile);

				exportWAVAs(currentFileName);
				update();
				break;
			}
				
			case HDRECORD_BUTTON_RECORDINGMODE:
				if (recorderMode == RecorderModeToFile)
					recorderMode = RecorderModeToSample;
				else
					recorderMode = RecorderModeToFile;
				update();
				break;
				
			case HDRECORD_BUTTON_MIXER_AUTO:
				if (event->getID() != eCommand)
					break;

				getPeakLevel();

				update();
				break;
				
			case HDRECORD_BUTTON_INS_PLUS:
				// update will validate the values
				insIndex++;
				update();
				break;
				
			case HDRECORD_BUTTON_INS_MINUS:
				// see above
				insIndex--;
				update();
				break;
	
			case HDRECORD_BUTTON_SMP_PLUS:
				// see above
				smpIndex++;
				update();
				break;
				
			case HDRECORD_BUTTON_SMP_MINUS:
				// see above
				smpIndex--;
				update();
				break;
				
			case HDRECORD_BUTTON_RESAMPLING:
				showResamplerMessageBox();
				break;
		}
	}
	else if (event->getID() == eValueChanged)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case HDRECORD_SLIDER_MIXERVOLUME:
			{
				mixerVolume = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				update();
				break;
			}
		}
	}
	
	return 0;
}

void SectionHDRecorder::init(pp_int32 px, pp_int32 py)
{
	PPScreen* screen = tracker.screen;

	PPContainer* container = new PPContainer(CONTAINER_HDRECORDER, tracker.screen, this, PPPoint(px, py), PPSize(320,UPPERLEFTSECTIONHEIGHT), false);
	container->setColor(TrackerConfig::colorThemeMain);	
	tracker.screen->addControl(container);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(px + 2, py + 2), "HD Recorder", true, true));

	PPSize size = container->getSize();

	pp_int32 buttonWidth = 8*4+4;
	pp_int32 buttonHeight = 22;
	
	pp_int32 x = px+container->getSize().width-(buttonWidth+4);
	pp_int32 y = py+container->getSize().height-(buttonHeight+4);
	
	pp_int32 dy = 11+20;
	// Horizontal seperator above exit
	container->addControl(new PPSeperator(0, screen, PPPoint(px+2, py + size.height - dy), size.width-5, container->getColor(), true));	

#ifdef __LOWRES__
	y-=2;
	buttonHeight+=4;

	PPButton* button = new PPButton(MAINMENU_SMPEDIT, screen, &tracker, PPPoint(x, y), PPSize(buttonWidth, buttonHeight/3));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Smp.Ed.");
	container->addControl(button);
	
	y+=button->getSize().height+1;

	button = new PPButton(MAINMENU_INSEDIT, screen, &tracker, PPPoint(x, y), PPSize(buttonWidth, buttonHeight/3));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Ins.Ed.");
	container->addControl(button);

	y+=button->getSize().height+1;

	button = new PPButton(HDRECORD_BUTTON_EXIT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight/3));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Exit");
	container->addControl(button);
#else
	PPButton* button = new PPButton(HDRECORD_BUTTON_EXIT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Exit");
	container->addControl(button);
#endif

	// add frequencies
	pp_int32 x2 = px;
	pp_int32 y2 = py + 11 + 3 + 4;  
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 4, y2), "Output:", true));

	PPRadioGroup* radioGroup = new PPRadioGroup(HDRECORD_RADIOGROUP_FREQUENCIES, screen, this, PPPoint(x2+2, y2+10), PPSize(88, TrackerConfig::numMixFrequencies*14));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	for (pp_int32 j = 0; j < TrackerConfig::numMixFrequencies; j++)
	{
		char buffer[32];
		sprintf(buffer, "%i Hz", TrackerConfig::mixFrequencies[j]);
		radioGroup->addItem(buffer);
	}

	radioGroup->setChoice(2);

	container->addControl(radioGroup);	

	y2+=radioGroup->getSize().height + 17;
	
	// --------- amplify
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 4, y2), "Amplify:", true));

	y2+=9;

	radioGroup = new PPRadioGroup(HDRECORD_RADIOGROUP_AMPLIFY, screen, this, PPPoint(x2 + 2, y2), PPSize(180, 20));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->setHorizontal(true);
	radioGroup->addItem("25%");
	radioGroup->addItem("50%");
	radioGroup->addItem("100%");

	radioGroup->setChoice(1);

	container->addControl(radioGroup);

	y2-=9;

	// --------- mixvol. slider
	container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 146 - 4, y2-3), 28, TrackerConfig::colorThemeMain, false));

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 146, y2), "Mixer Volume:", true));
	container->addControl(new PPStaticText(HDRECORD_STATICTEXT_MIXERVOLUME, NULL, NULL, PPPoint(x2 + 146, y2 + 14), "100%", false));

	PPSlider* slider = new PPSlider(HDRECORD_SLIDER_MIXERVOLUME, screen, this, PPPoint(x2 + 146 + 4*8+4, y2 + 14-1), 91, true);
	slider->setMaxValue(256);
	slider->setBarSize(16384);
	container->addControl(slider);

	button = new PPButton(HDRECORD_BUTTON_MIXER_AUTO, screen, this, PPPoint(x2 + 154 + 12*8+1, y2), PPSize(21, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Auto");
	container->addControl(button);

	container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 154 + 4*8 + 4 + 80 + 5, y2-3), 28, TrackerConfig::colorThemeMain, false));

	// -------------- quality --------------
	y2 = py + 11 + 3 + 4;
	x2 += /*radioGroup->getSize().width*/88;

	pp_int32 x3 = x2 - 6;
	container->addControl(new PPSeperator(0, screen, PPPoint(x2 - 6, py+16 - 2), container->getSize().height - (dy+14), TrackerConfig::colorThemeMain, false));
	
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "Quality:", true));
	
	y2+=13;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "Resampler:", true));
	button = new PPButton(HDRECORD_BUTTON_RESAMPLING, screen, this, PPPoint(x2 + 8*10 + 4, y2-2), PPSize(6*7 + 4, 11));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Select" PPSTR_PERIODS);
	container->addControl(button);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2 + 12), "Volume ramping:", true));
	container->addControl(new PPCheckBox(HDRECORD_CHECKBOX_RAMPING, screen, this, PPPoint(x2 + 15*8, y2-1+12)));
	
	y2+=24;

	// Horizontal seperator above exit
	container->addControl(new PPSeperator(0, screen, PPPoint(x3+1, y2), size.width-5 - (x3-px), container->getColor(), true));	

	y2+=5;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "Allow muting:", true));
	container->addControl(new PPCheckBox(HDRECORD_CHECKBOX_ALLOWMUTING, screen, this, PPPoint(x2 + 15*8, y2-1), false));	

	x2 += 18*8-4;
	container->addControl(new PPSeperator(0, screen, PPPoint(x2 - 6, py+16 - 2), container->getSize().height - (dy+28), TrackerConfig::colorThemeMain, false));

	y2 = py + 11 + 3 + 4;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "Range:", true));

	y2+=13;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "From", true));
	container->addControl(new PPStaticText(HDRECORD_STATICTEXT_START, screen, this, PPPoint(x2+5*8, y2), "xx"));

	button = new PPButton(HDRECORD_BUTTON_START_PLUS, screen, this, PPPoint(x2+7*8+3, y2-1), PPSize(12, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	
	button = new PPButton(HDRECORD_BUTTON_START_MINUS, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y2-1), PPSize(12, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	y2+=12;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "To", true));
	container->addControl(new PPStaticText(HDRECORD_STATICTEXT_END, screen, this, PPPoint(x2+5*8, y2), "xx"));

	button = new PPButton(HDRECORD_BUTTON_END_PLUS, screen, this, PPPoint(x2+7*8+3, y2-1), PPSize(12, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	
	button = new PPButton(HDRECORD_BUTTON_END_MINUS, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y2-1), PPSize(12, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	y2+=15;

	buttonWidth = 6*8 + 4;
	buttonHeight = 12;

	button = new PPButton(HDRECORD_BUTTON_RECORD, screen, this, PPPoint(x2, y2), PPSize(buttonWidth,buttonHeight));
	button->setText("Record");
	container->addControl(button);

	button = new PPButton(HDRECORD_BUTTON_RECORD_AS, screen, this, PPPoint(x2+buttonWidth+1, y2), PPSize(34,buttonHeight));
	button->setText("As" PPSTR_PERIODS);
	container->addControl(button);

	y2+=buttonHeight+2;
	
	container->addControl(new PPSeperator(0, screen, PPPoint(x3+1, y2), size.width-5 - (x3-px), container->getColor(), true));	

	y2+=5;

	x2 = x3+6;

	button = new PPButton(HDRECORD_BUTTON_RECORDINGMODE, screen, this, PPPoint(x2-3, y2-2), PPSize(5*8+2,buttonHeight-1), false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("File:");
	container->addControl(button);
	//container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "File:", true));

	x2+=5*8+4;

	PPStaticText* text = new PPStaticText(HDRECORD_STATICTEXT_SAVETOFILENAME, NULL, NULL, PPPoint(x2, y2), "123456789012345678901234", false, false, true);
	text->setExtent(PPSize(23*8, 8));
	container->addControl(text);

	x2+=2;
	text = new PPStaticText(HDRECORD_STATICTEXT_INS, NULL, NULL, PPPoint(x2, y2), "Ins:xx", false);
	container->addControl(text);

	x2+=text->getSize().width + 1 + 8;

	button = new PPButton(HDRECORD_BUTTON_INS_PLUS, screen, this, PPPoint(x2, y2-1), PPSize(13, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	
	button = new PPButton(HDRECORD_BUTTON_INS_MINUS, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y2-1), PPSize(13, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	x2 = button->getLocation().x + button->getSize().width + 10;

	text = new PPStaticText(HDRECORD_STATICTEXT_SMP, NULL, NULL, PPPoint(x2, y2), "Smp:xx", false);
	container->addControl(text);

	x2+=text->getSize().width + 1 + 8;

	button = new PPButton(HDRECORD_BUTTON_SMP_PLUS, screen, this, PPPoint(x2, y2-1), PPSize(13, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	
	button = new PPButton(HDRECORD_BUTTON_SMP_MINUS, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y2-1), PPSize(13, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	sectionContainer = container;

	initialised = true;

	showSection(false);
}

void SectionHDRecorder::update(bool repaint/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(tracker.screen->getControlByID(CONTAINER_HDRECORDER));
	ASSERT(container);
	
	if (fromOrder < 0)
		fromOrder = 0;
	if (toOrder < fromOrder)
		toOrder = fromOrder;
	if (toOrder >= tracker.moduleEditor->getNumOrders())
		toOrder = tracker.moduleEditor->getNumOrders()-1;
	
	PPStaticText* staticText = static_cast<PPStaticText*>(container->getControlByID(HDRECORD_STATICTEXT_START));
	ASSERT(staticText);
	staticText->setHexValue(fromOrder, 2);

	staticText = static_cast<PPStaticText*>(container->getControlByID(HDRECORD_STATICTEXT_END));
	ASSERT(staticText);
	staticText->setHexValue(toOrder, 2);
	
	// mixer volume
	char buffer[80], buffer2[80];
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(HDRECORD_STATICTEXT_MIXERVOLUME));
	ASSERT(text);
	
	PPSlider* slider = static_cast<PPSlider*>(container->getControlByID(HDRECORD_SLIDER_MIXERVOLUME));
	ASSERT(slider);

	sprintf(buffer, "%i%%", (mixerVolume*100)/256);

	if (strlen(buffer) < 4)
	{
		memset(buffer2, 32, sizeof(buffer2));
		strcpy(buffer2 + 4-strlen(buffer), buffer);
		strcpy(buffer, buffer2);
	}

	text->setText(buffer);

	slider->setCurrentValue(mixerVolume);

	if (recorderMode == RecorderModeToFile)
	{
		PPButton* button = static_cast<PPButton*>(container->getControlByID(HDRECORD_BUTTON_RECORDINGMODE));
		button->setText("File:");
	
		button = static_cast<PPButton*>(container->getControlByID(HDRECORD_BUTTON_RECORD_AS));
		button->enable(true);

		staticText = static_cast<PPStaticText*>(container->getControlByID(HDRECORD_STATICTEXT_SAVETOFILENAME));
		staticText->show(true);
		
		if (TrackerConfig::untitledSong.compareTo(currentFileName.stripExtension()) == 0)
		{
			staticText->setText("<None specified>");
		}
		else
		{
			PPSystemString temp = currentFileName.stripPath();
			char* nameASCIIZ = temp.toASCIIZ();
			staticText->setText(nameASCIIZ);
			delete[] nameASCIIZ;
		}

		container->getControlByID(HDRECORD_STATICTEXT_INS)->show(false);
		container->getControlByID(HDRECORD_BUTTON_INS_PLUS)->show(false);
		container->getControlByID(HDRECORD_BUTTON_INS_MINUS)->show(false);
		container->getControlByID(HDRECORD_STATICTEXT_SMP)->show(false);
		container->getControlByID(HDRECORD_BUTTON_SMP_PLUS)->show(false);
		container->getControlByID(HDRECORD_BUTTON_SMP_MINUS)->show(false);
	}
	else
	{
		validate();

		PPButton* button = static_cast<PPButton*>(container->getControlByID(HDRECORD_BUTTON_RECORDINGMODE));
		button->setText("Samp:");

		button = static_cast<PPButton*>(container->getControlByID(HDRECORD_BUTTON_RECORD_AS));
		button->enable(false);

		staticText = static_cast<PPStaticText*>(container->getControlByID(HDRECORD_STATICTEXT_SAVETOFILENAME));
		staticText->show(false);

		sprintf(buffer, "Ins:%x", insIndex+1);
		staticText = static_cast<PPStaticText*>(container->getControlByID(HDRECORD_STATICTEXT_INS));
		staticText->setText(buffer);

		sprintf(buffer, "Smp:%x", smpIndex);
		staticText = static_cast<PPStaticText*>(container->getControlByID(HDRECORD_STATICTEXT_SMP));
		staticText->setText(buffer);
		
		container->getControlByID(HDRECORD_STATICTEXT_INS)->show(true);
		container->getControlByID(HDRECORD_BUTTON_INS_PLUS)->show(true);
		container->getControlByID(HDRECORD_BUTTON_INS_MINUS)->show(true);
		container->getControlByID(HDRECORD_STATICTEXT_SMP)->show(true);
		container->getControlByID(HDRECORD_BUTTON_SMP_PLUS)->show(true);
		container->getControlByID(HDRECORD_BUTTON_SMP_MINUS)->show(true);
	}
	
	if (repaint)
		tracker.screen->paintControl(container);
}

void SectionHDRecorder::notifyInstrumentSelect(pp_int32 index)
{
	insIndex = index-1;
		
	if (!sectionContainer->isVisible()) 
		return;

	update();
}

void SectionHDRecorder::notifySampleSelect(pp_int32 index)
{
	smpIndex = index-1;

	if (!sectionContainer->isVisible()) 
		return;

	update();
}

void SectionHDRecorder::show(bool bShow)
{ 
	SectionUpperLeft::show(bShow); 
}

void SectionHDRecorder::exportWAVWithPanel(const PPSystemString& defaultFileName)
{
	PPSavePanel savePanel(tracker.screen, "Export Song to WAV", defaultFileName);
	savePanel.addExtension("wav","Uncompressed WAV");
	
	if (savePanel.runModal() == PPModalDialog::ReturnCodeOK)
	{
		const SYSCHAR* finalFileName = savePanel.getFileName();
		
		if (finalFileName)
		{
			currentFileName = finalFileName;
			exportWAVAsFileName(finalFileName);			
		}
	}
}

void SectionHDRecorder::exportWAVAsFileName(const PPSystemString& fileName)
{
	ModuleEditor* moduleEditor = tracker.moduleEditor;

	ModuleServices::WAVWriterParameters parameters;
	parameters.sampleRate = getSettingsFrequency();
	parameters.resamplerType = (getSettingsRamping() ? 1 : 0) | (getSettingsResampler() << 1);
	parameters.playMode = tracker.playerController->getPlayMode();
	parameters.mixerShift = getSettingsMixerShift(); 
	parameters.mixerVolume = mixerVolume;

	mp_ubyte* muting = new mp_ubyte[moduleEditor->getNumChannels()];
	memset(muting, 0, moduleEditor->getNumChannels());
	if (getSettingsAllowMuting())
	{
		for (pp_int32 i = 0; i < (signed)moduleEditor->getNumChannels(); i++)
			muting[i] = (mp_ubyte)tracker.muteChannels[i];
	}

	parameters.muting = muting;
	parameters.panning = tracker.playerController->getPanningTable();
	parameters.fromOrder = fromOrder;
	parameters.toOrder = toOrder;

	tracker.signalWaitState(true);

	pp_int32 res = moduleEditor->getModuleServices()->exportToWAV(fileName, parameters);;

	tracker.signalWaitState(false);

	delete[] muting;

	if (res > 0)
	{
		pp_int32 seconds = (pp_int32)((float)res / (float)getSettingsFrequency());
		char buffer[200];
		sprintf(buffer, "%i:%02i successfully recorded", seconds/60, seconds%60);
		tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, buffer, Tracker::MessageBox_OK);
	}
	else
	{
		tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Recording failed.", Tracker::MessageBox_OK);
	}
}

void SectionHDRecorder::getPeakLevel()
{
	ModuleEditor* moduleEditor = tracker.moduleEditor;

	ModuleServices::WAVWriterParameters parameters;
	parameters.sampleRate = getSettingsFrequency();
	parameters.resamplerType = (getSettingsRamping() ? 1 : 0) | (getSettingsResampler() << 1);
	parameters.playMode = tracker.playerController->getPlayMode();
	parameters.mixerShift = getSettingsMixerShift(); 
	parameters.mixerVolume = 256;

	mp_ubyte* muting = new mp_ubyte[moduleEditor->getNumChannels()];
	memset(muting, 0, moduleEditor->getNumChannels());
	if (getSettingsAllowMuting())
	{
		for (pp_int32 i = 0; i < (signed)moduleEditor->getNumChannels(); i++)
			muting[i] = (mp_ubyte)tracker.muteChannels[i];
	}

	parameters.muting = muting;
	parameters.panning = tracker.playerController->getPanningTable();
	parameters.fromOrder = fromOrder;
	parameters.toOrder = toOrder;

	tracker.signalWaitState(true);

	mixerVolume = moduleEditor->getModuleServices()->estimateMixerVolume(parameters);
	
	tracker.signalWaitState(false);

	delete[] muting;

	tracker.screen->update();
}

void SectionHDRecorder::resetCurrentFileName()
{
	currentFileName = TrackerConfig::untitledSong;
	update(false);
}

void SectionHDRecorder::setCurrentFileName(const PPSystemString& fileName)
{
	currentFileName = fileName;
	update(false);
}

void SectionHDRecorder::exportWAVAs(const PPSystemString& fileName)
{
	if (tracker.useClassicBrowser)
	{
		tracker.saveType(FileTypes::FileTypeSongWAV);
	}
	else
	{
		PPSystemString temp = fileName.stripExtension();
		temp.append(".wav");
		exportWAVWithPanel(temp.stripPath());
	}
}

void SectionHDRecorder::exportWAVAsSample()
{	
	ModuleEditor* moduleEditor = tracker.moduleEditor;

	ModuleServices::WAVWriterParameters parameters;
	parameters.sampleRate = getSettingsFrequency();
	parameters.resamplerType = (getSettingsRamping() ? 1 : 0) | (getSettingsResampler() << 1);
	parameters.playMode = tracker.playerController->getPlayMode();
	parameters.mixerShift = getSettingsMixerShift(); 
	parameters.mixerVolume = mixerVolume;

	mp_ubyte* muting = new mp_ubyte[moduleEditor->getNumChannels()];
	memset(muting, 0, moduleEditor->getNumChannels());
	if (getSettingsAllowMuting())
	{
		for (pp_int32 i = 0; i < (signed)moduleEditor->getNumChannels(); i++)
			muting[i] = (mp_ubyte)tracker.muteChannels[i];
	}

	parameters.muting = muting;
	parameters.panning = tracker.playerController->getPanningTable();
	parameters.fromOrder = fromOrder;
	parameters.toOrder = toOrder;

	tracker.signalWaitState(true);

	mp_sint32 numSamples = moduleEditor->getModuleServices()->estimateWaveLengthInSamples(parameters);
	
	validate();
	pp_int32 curIns = moduleEditor->getCurrentInstrumentIndex();
	pp_int32 curSmp = moduleEditor->getCurrentSampleIndex();
	
	// surpress screen updates from here on...
	tracker.screen->pauseUpdate(true);

	moduleEditor->reloadSample(insIndex, smpIndex);
	
	SampleEditor::WorkSample* workSample = moduleEditor->getSampleEditor()->createWorkSample(numSamples, 16, parameters.sampleRate);
	
	if (workSample)
	{
		pp_int16* buffer = (pp_int16*)workSample->getBuffer();
		moduleEditor->getModuleServices()->exportToBuffer16Bit(parameters, buffer, numSamples, true);
		moduleEditor->getSampleEditor()->pasteOther(*workSample);
		delete workSample;
	}

	moduleEditor->reloadSample(curIns, curSmp);
	
	// ...until here
	tracker.screen->pauseUpdate(false);

	tracker.screen->paint();

	tracker.signalWaitState(false);
}

void SectionHDRecorder::adjustOrders()
{
	fromOrder = 0;
	toOrder = tracker.moduleEditor->getNumOrders()-1;
}

void SectionHDRecorder::showResamplerMessageBox()
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}

	dialog = new DialogListBox(tracker.screen, 
													 responder, 
													 RESPONDMESSAGEBOX_SELECTRESAMPLER, 
													 "Select Resampler",
													 true);
	PPListBox* listBox = static_cast<DialogListBox*>(dialog)->getListBox();

	ResamplerHelper resamplerHelper;

	for (pp_uint32 i = 0; i < resamplerHelper.getNumResamplers(); i++)
		listBox->addItem(resamplerHelper.getResamplerName(i));	

	listBox->setSelectedIndex(resampler, false);

	dialog->show();	
}

void SectionHDRecorder::storeResampler(pp_uint32 resampler)
{
	this->resampler = resampler;
}
