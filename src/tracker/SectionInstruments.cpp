/*
 *  tracker/SectionInstruments.cpp
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
 *  SectionInstruments.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 15.04.05.
 *
 */

#include "SectionInstruments.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "TabManager.h"
#include "ModuleEditor.h"
#include "EnvelopeEditor.h"
#include "SampleEditor.h"
#include "PianoControl.h"
#include "PatternTools.h"
#include "SectionSamples.h"

#include "Screen.h"
#include "PPUIConfig.h"
#include "Button.h"
#include "CheckBox.h"
#include "MessageBoxContainer.h"
#include "TransparentContainer.h"
#include "ListBox.h"
#include "RadioGroup.h"
#include "Seperator.h"
#include "Slider.h"
#include "StaticText.h"
#include "EnvelopeEditorControl.h"
#include "PatternEditorControl.h"
#include "DialogBase.h"

#include "PlayerController.h"
#include "InputControlListener.h"
#include "EnvelopeContainer.h"

// OS Interface
#include "PPOpenPanel.h"
#include "PPSavePanel.h"

#include "ControlIDs.h"

// Class which responds to message box clicks
class DialogResponderInstruments : public DialogResponder
{
private:
	SectionInstruments& section;
	
public:
	DialogResponderInstruments(SectionInstruments& section) :
		section(section)
	{
	}
	
	virtual pp_int32 ActionOkay(PPObject* sender)
	{
		switch (reinterpret_cast<PPDialogBase*>(sender)->getID())
		{
			case MESSAGEBOX_ZAPINSTRUMENT:
			{
				section.zapInstrument();
				break;
			}
		}
		return 0;
	}	
};

EnvelopeEditor* SectionInstruments::getEnvelopeEditor()
{
	return tracker.getEnvelopeEditor();
}

void SectionInstruments::showSection(bool bShow)
{
	containerEntire->show(bShow);
}

SectionInstruments::SectionInstruments(Tracker& theTracker) :
	SectionAbstract(theTracker, NULL, new DialogResponderInstruments(*this)),
	containerEntire(NULL),
	containerEnvelopes(NULL),
	containerSampleSlider(NULL),
	containerInstrumentSlider(NULL),
	envelopeEditorControl(NULL),
	pianoControl(NULL),
	currentEnvelopeType(EnvelopeEditor::EnvelopeTypeVolume),
	visible(false),
	storeEnvelope(false)
{
	predefinedVolumeEnvelopes = new EnvelopeContainer(TrackerConfig::numPredefinedEnvelopes);
	predefinedPanningEnvelopes = new EnvelopeContainer(TrackerConfig::numPredefinedEnvelopes);
	
	// Store default predefined envelopes
	for (pp_int32 i = 0; i < TrackerConfig::numPredefinedEnvelopes; i++)
	{
		TEnvelope e = EnvelopeContainer::decodeEnvelope(TrackerConfig::defaultPredefinedVolumeEnvelope); 
		predefinedVolumeEnvelopes->store(i, e);
		
		e = EnvelopeContainer::decodeEnvelope(TrackerConfig::defaultPredefinedPanningEnvelope); 
		predefinedPanningEnvelopes->store(i, e);
	}
}

SectionInstruments::~SectionInstruments()
{
	delete predefinedVolumeEnvelopes;
	delete predefinedPanningEnvelopes;
}

pp_int32 SectionInstruments::handleEvent(PPObject* sender, PPEvent* event)
{
	char buffer[100];
	char buffer2[100];
	ModuleEditor::TEditorInstrument* ins;

	PPScreen* screen = tracker.screen;
	ModuleEditor* moduleEditor = tracker.moduleEditor;

	if (event->getID() == eUpdateChanged)
	{
		tracker.updateWindowTitle();
	}
	else if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case BUTTON_ENVELOPE_PREDEF_0:
			case BUTTON_ENVELOPE_PREDEF_1:
			case BUTTON_ENVELOPE_PREDEF_2:
			case BUTTON_ENVELOPE_PREDEF_3:
			case BUTTON_ENVELOPE_PREDEF_4:
			case BUTTON_ENVELOPE_PREDEF_5:
			case BUTTON_ENVELOPE_PREDEF_6:
			case BUTTON_ENVELOPE_PREDEF_7:
			case BUTTON_ENVELOPE_PREDEF_8:
			case BUTTON_ENVELOPE_PREDEF_9:
			{
				if (event->getID() != eCommand)
					break;

				pp_int32 i = reinterpret_cast<PPControl*>(sender)->getID() - BUTTON_ENVELOPE_PREDEF_0;
				
				EnvelopeContainer* predefinedEnvelopes = NULL;
				
				switch (currentEnvelopeType)
				{
					case EnvelopeEditor::EnvelopeTypeVolume: predefinedEnvelopes = predefinedVolumeEnvelopes; break;
					case EnvelopeEditor::EnvelopeTypePanning: predefinedEnvelopes = predefinedPanningEnvelopes; break;
					default: ASSERT(false);
				}
				
				if (storeEnvelope)
				{
					const TEnvelope* env = envelopeEditorControl->getEnvelope();
					predefinedEnvelopes->store(i, *env);
					storeEnvelope = !storeEnvelope;
					
					PPButton* button = static_cast<PPButton*>(containerEnvelopes->getControlByID(BUTTON_ENVELOPE_PREDEF_STORE));
					button->setPressed(storeEnvelope);
				}
				else
				{
					const TEnvelope* env = predefinedEnvelopes->restore(i);					
					getEnvelopeEditor()->pasteOther(*env);					
				}
				
				update();
				break;
			}

			case BUTTON_ENVELOPE_PREDEF_STORE:
			{
				if (event->getID() != eCommand)
					break;

				storeEnvelope = !storeEnvelope;
				PPButton* button = reinterpret_cast<PPButton*>(sender);
				
				button->setPressed(storeEnvelope);
				
				screen->paintControl(button);
				break;
			}
		
			case BUTTON_ENVELOPE_VOLUME:
				if (event->getID() != eCommand)
					break;

				currentEnvelopeType = EnvelopeEditor::EnvelopeTypeVolume;

				getEnvelopeEditor()->setEnvelopeType(EnvelopeEditor::EnvelopeTypeVolume);
				update();
				break;

			case BUTTON_ENVELOPE_PANNING:
				if (event->getID() != eCommand)
					break;

				currentEnvelopeType = EnvelopeEditor::EnvelopeTypePanning;

				getEnvelopeEditor()->setEnvelopeType(EnvelopeEditor::EnvelopeTypePanning);
				update();
				break;

			case CHECKBOX_ENVELOPE_ON:
				if (event->getID() != eCommand)
					break;

				getEnvelopeEditor()->enableEnvelope(reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			
			case CHECKBOX_ENVELOPE_SUSTAIN:
				if (event->getID() != eCommand)
					break;

				getEnvelopeEditor()->enableSustain(reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			
			case CHECKBOX_ENVELOPE_LOOP:
				if (event->getID() != eCommand)
					break;

				getEnvelopeEditor()->enableLoop(reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;

			case BUTTON_ENVELOPE_UNDO:
				getEnvelopeEditor()->undo();
				break;

			case BUTTON_ENVELOPE_REDO:
				getEnvelopeEditor()->redo();
				break;

			case BUTTON_ENVELOPE_COPY:
				getEnvelopeEditor()->makeCopy();
				update();
				break;

			case BUTTON_ENVELOPE_PASTE:
				getEnvelopeEditor()->pasteCopy();
				update();
				break;

			case BUTTON_ENVELOPE_ADD:
				getEnvelopeEditor()->addPoint();
				update();
				break;

			case BUTTON_ENVELOPE_DELETE:
				getEnvelopeEditor()->deletePoint();
				update();
				break;
			
			case BUTTON_ENVELOPE_SUSTAIN_PLUS:
				getEnvelopeEditor()->selectNextSustainPoint();
				update();
				break;
			
			case BUTTON_ENVELOPE_SUSTAIN_MINUS:
				getEnvelopeEditor()->selectPreviousSustainPoint();
				update();
				break;
			
			case BUTTON_ENVELOPE_LOOPSTART_PLUS:
				getEnvelopeEditor()->selectNextLoopStartPoint();
				update();
				break;
			
			case BUTTON_ENVELOPE_LOOPSTART_MINUS:
				getEnvelopeEditor()->selectPreviousLoopStartPoint();
				update();
				break;
			
			case BUTTON_ENVELOPE_LOOPEND_PLUS:
				getEnvelopeEditor()->selectNextLoopEndPoint();
				update();
				break;
			
			case BUTTON_ENVELOPE_LOOPEND_MINUS:
				getEnvelopeEditor()->selectPreviousLoopEndPoint();
				update();
				break;

			case BUTTON_ENVELOPE_ZOOMIN:
				envelopeEditorControl->setScale(envelopeEditorControl->getScale() >> 1);
				update();
				break;
				
			case BUTTON_ENVELOPE_ZOOMOUT:
				envelopeEditorControl->setScale(envelopeEditorControl->getScale() << 1);
				update();
				break;

			case BUTTON_ENVELOPE_SCALEX:
				if (event->getID() != eCommand)
					break;
				envelopeEditorControl->invokeToolParameterDialog(EnvelopeEditorControl::EnvelopeToolTypeScaleX);
				break;
				
			case BUTTON_ENVELOPE_SCALEY:
				if (event->getID() != eCommand)
					break;
				envelopeEditorControl->invokeToolParameterDialog(EnvelopeEditorControl::EnvelopeToolTypeScaleY);
				break;

			case BUTTON_SAMPLE_RELNOTENUM_OCTUP:
			{
				tracker.getSampleEditor()->increaseRelNoteNum(12);
				update();
				break;
			}

			case BUTTON_SAMPLE_RELNOTENUM_OCTDN:
			{
				tracker.getSampleEditor()->increaseRelNoteNum(-12);
				update();
				break;
			}

			case BUTTON_SAMPLE_RELNOTENUM_NOTEUP:
			{
				tracker.getSampleEditor()->increaseRelNoteNum(1);
				update();
				break;
			}

			case BUTTON_SAMPLE_RELNOTENUM_NOTEDN:
			{
				tracker.getSampleEditor()->increaseRelNoteNum(-1);
				update();
				break;
			}

			case BUTTON_PIANO_EDIT:
				if (event->getID() != eCommand)
					break;
					
				getPianoControl()->setMode(PianoControl::ModeEdit);
				update();
				break;

			case BUTTON_PIANO_PLAY:
				if (event->getID() != eCommand)
					break;
				getPianoControl()->setMode(PianoControl::ModePlay);
				update();
				break;

			case BUTTON_SAMPLE_PLAY_STOP:
				tracker.playerController->stopInstrument(tracker.listBoxInstruments->getSelectedIndex()+1);
				//tracker.stopSong();
				break;

			case BUTTON_INSTRUMENTEDITOR_CLEAR:
				if (event->getID() != eCommand)
					break;

				handleZapInstrument();
				break;
			
			// load instrument
			case BUTTON_INSTRUMENTEDITOR_LOAD:
			{
				if (event->getID() != eCommand)
					break;

				tracker.loadType(FileTypes::FileTypeSongAllInstruments);
				break;
			}

			// save instrument
			case BUTTON_INSTRUMENTEDITOR_SAVE:
			{
				if (event->getID() != eCommand)
					break;

				tracker.saveType(FileTypes::FileTypeInstrumentXI);
				break;
			}

			// test instrument chooser
			case BUTTON_INSTRUMENTEDITOR_COPY:
			{
				if (event->getID() != eCommand)
					break;

				sprintf(buffer, "Copy ins. %x to %x", tracker.listBoxInstruments->getSelectedIndex()+1, 1);
				sprintf(buffer2, "Copy smp. %x to %x", tracker.listBoxSamples->getSelectedIndex(), 0);

				tracker.initInstrumentChooser(INSTRUMENT_CHOOSER_COPY, "Copy ins", "Copy smp", "Copy instrument/sample" PPSTR_PERIODS, 
											  buffer, buffer2, 
											  tracker.listBoxInstruments->getSelectedIndex(), 
											  tracker.listBoxSamples->getSelectedIndex(),
											  tracker.tabManager->getSelectedTabIndex());
				screen->setModalControl(tracker.instrumentChooser);
				break;
			}

			// test instrument chooser
			case BUTTON_INSTRUMENTEDITOR_SWAP:
			{
				if (event->getID() != eCommand)
					break;

				sprintf(buffer, "Swap ins. %x with %x", tracker.listBoxInstruments->getSelectedIndex()+1, 1);
				sprintf(buffer2, "Swap smp. %x with %x", tracker.listBoxSamples->getSelectedIndex(), 0);

				tracker.initInstrumentChooser(INSTRUMENT_CHOOSER_SWAP, "Swap ins", "Swap smp", "Swap instrument/sample" PPSTR_PERIODS, 
											  buffer, buffer2, 
											  tracker.listBoxInstruments->getSelectedIndex(), 
											  tracker.listBoxSamples->getSelectedIndex(),
											  tracker.tabManager->getSelectedTabIndex());
				screen->setModalControl(tracker.instrumentChooser);
				break;
			}
		}
	}
	else if (event->getID() == eSelection)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case RADIOGROUP_SAMPLE_VIBTYPE:
			{
				tracker.setChanged();
				ins = moduleEditor->getInstrumentInfo(tracker.listBoxInstruments->getSelectedIndex());
				ins->vibtype = (pp_uint8)(reinterpret_cast<PPRadioGroup*>(sender)->getChoice());
				moduleEditor->updateInstrumentData(tracker.listBoxInstruments->getSelectedIndex());
				updateInstrumentSliders(true);
				break;
			}

			case PIANO_CONTROL:
			{
				pp_int32 v = *((pp_int32*)event->getDataPtr());

				tracker.inputControlListener->sendNote(v);			
				break;
			}
		}
	}
	else if (event->getID() == eValueChanged)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case SLIDER_SAMPLE_VOLUME:
			{
				tracker.getSampleEditor()->setFT2Volume(reinterpret_cast<PPSlider*>(sender)->getCurrentValue());
				updateSampleSliders();
				break;
			}

			case SLIDER_SAMPLE_PANNING:
			{
				tracker.getSampleEditor()->setPanning(reinterpret_cast<PPSlider*>(sender)->getCurrentValue());
				updateSampleSliders();
				break;
			}

			case SLIDER_SAMPLE_FINETUNE:
			{
				tracker.getSampleEditor()->setFinetune(reinterpret_cast<PPSlider*>(sender)->getCurrentValue()-128);
				updateSampleSliders();
				break;
			}

			case SLIDER_SAMPLE_VOLFADE:
			{
				tracker.setChanged();
				ins = moduleEditor->getInstrumentInfo(tracker.listBoxInstruments->getSelectedIndex());
				ins->volfade = (pp_uint16)(reinterpret_cast<PPSlider*>(sender)->getCurrentValue());
				if (ins->volfade == 0x1000) ins->volfade = 32767;
				moduleEditor->updateInstrumentData(tracker.listBoxInstruments->getSelectedIndex());
				updateInstrumentSliders();
				break;
			}

			case SLIDER_SAMPLE_VIBSPEED:
			{
				tracker.setChanged();
				ins = moduleEditor->getInstrumentInfo(tracker.listBoxInstruments->getSelectedIndex());
				ins->vibrate = (pp_uint8)(reinterpret_cast<PPSlider*>(sender)->getCurrentValue());
				moduleEditor->updateInstrumentData(tracker.listBoxInstruments->getSelectedIndex());
				updateInstrumentSliders();
				break;
			}

			case SLIDER_SAMPLE_VIBDEPTH:
			{
				tracker.setChanged();
				ins = moduleEditor->getInstrumentInfo(tracker.listBoxInstruments->getSelectedIndex());
				ins->vibdepth = (pp_uint8)(reinterpret_cast<PPSlider*>(sender)->getCurrentValue());
				moduleEditor->updateInstrumentData(tracker.listBoxInstruments->getSelectedIndex());
				updateInstrumentSliders();
				break;
			}

			case SLIDER_SAMPLE_VIBSWEEP:
			{
				tracker.setChanged();
				ins = moduleEditor->getInstrumentInfo(tracker.listBoxInstruments->getSelectedIndex());
				ins->vibsweep = (pp_uint8)(reinterpret_cast<PPSlider*>(sender)->getCurrentValue());
				moduleEditor->updateInstrumentData(tracker.listBoxInstruments->getSelectedIndex());
				updateInstrumentSliders();
				break;
			}

			// sample table has been updated
			case PIANO_CONTROL:
			{
				tracker.setChanged();
				pp_int32 i = tracker.listBoxInstruments->getSelectedIndex();	
				moduleEditor->updateSampleTable(i, *(reinterpret_cast<const pp_uint8* const*>(event->getDataPtr())));
				break;
			}		
		}
	}
	else if (event->getID() == eUpdated)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{			
			case CONTAINER_ENVELOPES:
			{
				updateEnvelopeEditor(true, false);
				break;
			}
		}
	}
	
	return 0;
}

void SectionInstruments::init()
{
	init(0, tracker.MAXEDITORHEIGHT()-tracker.INSTRUMENTSECTIONDEFAULTHEIGHT());
}

void SectionInstruments::init(pp_int32 x, pp_int32 y)
{
	PPScreen* screen = tracker.screen;

	containerEntire = new PPTransparentContainer(CONTAINER_ENTIREINSSECTION, screen, this, 
												 PPPoint(0, 0), PPSize(screen->getWidth(), screen->getHeight()));

#ifndef __LOWRES__
	// envelope stuff
	pp_int32 w4 = 165;
	pp_int32 w3 = 39;
	pp_int32 w2 = w4+w3;
	pp_int32 w = screen->getWidth() - w2;
	containerEnvelopes = new PPContainer(CONTAINER_ENVELOPES, screen, this, PPPoint(x, y), PPSize(w,135+4+3), false);
	containerEntire->addControl(containerEnvelopes);
	containerEnvelopes->setColor(TrackerConfig::colorThemeMain);

	envelopeEditorControl = new EnvelopeEditorControl(CONTAINER_ENVELOPES, screen, this, PPPoint(x + 2, y + 2 + 10), 
													  PPSize(containerEnvelopes->getSize().width-96,containerEnvelopes->getSize().height-17));
	envelopeEditorControl->attachEnvelopeEditor(tracker.getEnvelopeEditor());
	envelopeEditorControl->setBorderColor(TrackerConfig::colorThemeMain);
	
	pp_int32 scale = 256*screen->getWidth() / 800;
	
	envelopeEditorControl->setxMax(scale);
	//envelopeEditor->attachEnvelope(moduleEditor->getVolumeEnvelope(0,0));
	
	containerEnvelopes->addControl(envelopeEditorControl);

	PPButton* button = new PPButton(BUTTON_ENVELOPE_VOLUME, screen, this, PPPoint(x + 2, y + 1), PPSize(64, 11), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Volume");
	containerEnvelopes->addControl(button);
	
	button = new PPButton(BUTTON_ENVELOPE_PANNING, screen, this, PPPoint(x + 2 + 64, y + 1), PPSize(64, 11), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Panning");
	containerEnvelopes->addControl(button);

	pp_int32 px = button->getLocation().x + button->getSize().width + (screen->getWidth() < 800 ? 6 : 32);

	PPStaticText* staticText = new PPStaticText(0, NULL, NULL, PPPoint(px, y + 3), "Predef.", true);
	containerEnvelopes->addControl(staticText);

	px+=button->getSize().width;

	// pre-defined envelopes
	for (pp_int32 i = 0; i < TrackerConfig::numPredefinedEnvelopes; i++)
	{
		button = new PPButton(BUTTON_ENVELOPE_PREDEF_0+i, screen, this, PPPoint(px, y + 2), PPSize(9, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText((char)('0'+i));
		containerEnvelopes->addControl(button);
		px+=button->getSize().width+1;
	}
	px+=2;

	button = new PPButton(BUTTON_ENVELOPE_PREDEF_STORE, screen, this, PPPoint(px, y + 2), PPSize(5*6, 9), true, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Store");
	containerEnvelopes->addControl(button);


	if (screen->getWidth() >= 800)
	{
		button = new PPButton(BUTTON_ENVELOPE_UNDO, screen, this, PPPoint(x + envelopeEditorControl->getSize().width-115, y + 2), PPSize(26, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Undo");
		containerEnvelopes->addControl(button);
		
		button = new PPButton(BUTTON_ENVELOPE_REDO, screen, this, PPPoint(x + envelopeEditorControl->getSize().width-88, y + 2), PPSize(26, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Redo");
		containerEnvelopes->addControl(button);
		
		button = new PPButton(BUTTON_ENVELOPE_COPY, screen, this, PPPoint(x + envelopeEditorControl->getSize().width-59, y + 2), PPSize(26, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Copy");
		containerEnvelopes->addControl(button);
		
		button = new PPButton(BUTTON_ENVELOPE_PASTE, screen, this, PPPoint(x + envelopeEditorControl->getSize().width-32, y + 2), PPSize(26, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Paste");
		containerEnvelopes->addControl(button);
	}

	pp_int32 y4 = y+2;

	button = new PPButton(BUTTON_ENVELOPE_ADD, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 8, y4 + 2), PPSize(40, 11));
	button->setText("Add");
	containerEnvelopes->addControl(button);
	
	button = new PPButton(BUTTON_ENVELOPE_DELETE, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 8 + 41, y4 + 2), PPSize(40, 11));
	button->setText("Del");
	containerEnvelopes->addControl(button);

	y4+=18;

	containerEnvelopes->addControl(new PPCheckBox(CHECKBOX_ENVELOPE_ON, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4, y4 + 2)));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 13, y4 + 3), "On", true));

	// sustain
	containerEnvelopes->addControl(new PPCheckBox(CHECKBOX_ENVELOPE_SUSTAIN, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4, y4 + 2 + 18)));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 13, y4 + 2 + 18+1), "Sustain:", true));
	
	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + envelopeEditorControl->getSize().width + 4, y4 + 2 + 18+1 + 12), "Point", true));

	// sustain point field
	containerEnvelopes->addControl(new PPStaticText(STATICTEXT_ENVELOPE_SUSTAINPT, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 6*8, y4 + 2 + 18+1 + 12), "00", false));

	button = new PPButton(BUTTON_ENVELOPE_SUSTAIN_PLUS, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 28 + 40, y4 + 2 + 18+1 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);

	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_SUSTAIN_MINUS, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 28 + 40 + 11, y4 + 2 + 18+1 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);

	containerEnvelopes->addControl(button);

	// loop
	containerEnvelopes->addControl(new PPCheckBox(CHECKBOX_ENVELOPE_LOOP, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4, y4 + 2 + 18*2 + 9)));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 13, y4 + 2 + 18*2 + 10), "Loop:", true));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + envelopeEditorControl->getSize().width + 4, y4 + 2 + 18*2 + 10 + 12), "Start", true));

	// loop start point field
	containerEnvelopes->addControl(new PPStaticText(STATICTEXT_ENVELOPE_LOOPSTARTPT, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 6*8, y4 + 2 + 18*2 + 10 + 12), "00", false));

	button = new PPButton(BUTTON_ENVELOPE_LOOPSTART_PLUS, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 28 + 40, y4 + 2 + 18*2 + 10 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);

	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_LOOPSTART_MINUS, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 28 + 40 + 11, y4 + 2 + 18*2 + 10 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);

	containerEnvelopes->addControl(button);

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + envelopeEditorControl->getSize().width + 4, y4 + 2 + 18*2 + 10 + 12 + 10), "End", true));

	// loop end point field
	containerEnvelopes->addControl(new PPStaticText(STATICTEXT_ENVELOPE_LOOPENDPT, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 6*8, y4 + 2 + 18*2 + 10 + 12 + 10), "00", false));

	button = new PPButton(BUTTON_ENVELOPE_LOOPEND_PLUS, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 28 + 40, y4 + 2 + 18*2 + 10 + 12 + 9), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);

	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_LOOPEND_MINUS, screen, this, PPPoint(x + envelopeEditorControl->getSize().width + 4 + 28 + 40 + 11, y4 + 2 + 18*2 + 10 + 12 + 9), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);

	containerEnvelopes->addControl(button);

	pp_int32 ty = button->getLocation().y + button->getSize().height + 5;
	pp_int32 tx = x + envelopeEditorControl->getSize().width + 4;

	containerEnvelopes->addControl(new PPSeperator(0, screen, PPPoint(tx-2, ty), 8*5*2+8+4, containerEnvelopes->getColor(), true));

	ty+=4;

	button = new PPButton(BUTTON_ENVELOPE_SCALEX, screen, this, PPPoint(tx+3, ty), PPSize(8*5+1, 13));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Scale X");
	containerEnvelopes->addControl(button);		

	tx+=button->getSize().width+1;
	button = new PPButton(BUTTON_ENVELOPE_SCALEY, screen, this, PPPoint(tx+3, ty), PPSize(8*5+2, 13));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Scale Y");
	containerEnvelopes->addControl(button);	
	
	ty+=button->getSize().height+2;

	tx = x + envelopeEditorControl->getSize().width + 4;
	
	containerEnvelopes->addControl(new PPSeperator(0, screen, PPPoint(tx-2, ty), 8*5*2+8+4, containerEnvelopes->getColor(), true));

	ty+=5;

	button = new PPButton(BUTTON_ENVELOPE_ZOOMIN, screen, this, PPPoint(tx+3, ty), PPSize(8*5+1, 10));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Zoom in");
	containerEnvelopes->addControl(button);	
	
	tx+=button->getSize().width+1;
	button = new PPButton(BUTTON_ENVELOPE_ZOOMOUT, screen, this, PPPoint(tx+3, ty), PPSize(8*5+2, 10));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Zoom out");
	containerEnvelopes->addControl(button);	

	// ----------------- instrument info ----------------- 
	//y+=containerEnvelopes->getSize().height;
	x+=containerEnvelopes->getSize().width;
	
	PPContainer* container = new PPContainer(CONTAINER_INSTRUMENTS_INFO1, screen, this, PPPoint(x, y), PPSize(w4,34+4), false);
	containerEntire->addControl(container);
	containerSampleSlider = container;
	
	container->setColor(TrackerConfig::colorThemeMain);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4), "Volume", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VOLUME, screen, this, PPPoint(x + 4 + 8*9, y + 4), "FF", false));	

	//PPSlider* slider = new PPSlider(SLIDER_SAMPLE_VOLUME, screen, this, PPPoint(x + 4 + 8*7+2, y + 2), 51, true);
	PPSlider* slider = new PPSlider(SLIDER_SAMPLE_VOLUME, screen, this, PPPoint(x + 4 + 8*11+2, y + 2), 68, true);
	slider->setMaxValue(64);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 12), "Panning", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_PANNING, screen, this, PPPoint(x + 4 + 8*9, y + 4 + 12), "FF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_PANNING, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + 12), 68, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 24), "F.tune", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_FINETUNE, screen, this, PPPoint(x + 4 + 8*7, y + 4 + 24), "-128", false));	

	slider = new PPSlider(SLIDER_SAMPLE_FINETUNE, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + 24), 68, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	pp_int32 height = container->getSize().height;

	// exit 'n stuff
	y4=y;
	pp_int32 nx = x + container->getSize().width;

	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO4, screen, this, PPPoint(nx, y), PPSize(w3,38), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_EXIT, screen, &tracker, PPPoint(nx + 2, y + 2), PPSize(34, 34));
	button->setText("Exit");

	container->addControl(button);

	// load & save 
	y+=container->getSize().height;

	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO5, screen, this, PPPoint(nx, y), PPSize(w3,39), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_CLEAR, screen, this, PPPoint(nx + 2, y + 2), PPSize(34, 11));
	button->setText("Zap");
	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_LOAD, screen, this, PPPoint(nx + 2, y + 2+12), PPSize(34, 11));
	button->setText("Load");
	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_SAVE, screen, this, PPPoint(nx + 2, y + 2+12*2), PPSize(34, 11));
	button->setText("Save");
	container->addControl(button);

	// copy & paste
	y+=container->getSize().height;

	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO6, screen, this, PPPoint(nx, y), PPSize(w3,27), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_COPY, screen, this, PPPoint(nx + 2, y + 2), PPSize(34, 11));
	//button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Copy");
	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_SWAP, screen, this, PPPoint(nx + 2, y + 2+12), PPSize(34, 11));
	//button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Swap");
	container->addControl(button);

	y+=container->getSize().height;	

	y = y4;

	// autovibrato etc.
	y+=height;

	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO3, screen, this, PPPoint(x, y), PPSize(w4,66), false);
	containerEntire->addControl(container);
	containerInstrumentSlider = container;
	container->setColor(TrackerConfig::colorThemeMain);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4), "Fadeout", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VOLFADE, screen, this, PPPoint(x + 4 + 8*8, y + 4), "FFF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VOLFADE, screen, this, PPPoint(x + 4 + 8*11+2, y + 2), 68, true);
	slider->setMaxValue(0x1000);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 12), "Vibspeed", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VIBSPEED, screen, this, PPPoint(x + 4 + 8*9, y + 4 + 12), "FF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VIBSPEED, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + 12), 68, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 12*2), "Vibdepth", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VIBDEPTH, screen, this, PPPoint(x + 4 + 8*10, y + 4 + 12*2), "F", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VIBDEPTH, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + 12*2), 68, true);
	slider->setMaxValue(0xf);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 12*3), "Vibsweep", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VIBSWEEP, screen, this, PPPoint(x + 4 + 8*9, y + 4 + 12*3), "FF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VIBSWEEP, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + 12*3), 68, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 12*4), "Type:", true));	
	
	PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SAMPLE_VIBTYPE, screen, this, PPPoint(x + 2 + 5*8, y + 4 + 12*3 + 8), PPSize(w4, 20));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->setHorizontal(true);
	radioGroup->addItem("\xfc\xfb");
	radioGroup->addItem("\xfa\xf9");
	radioGroup->addItem("\xf8\xf7");
	radioGroup->addItem("\xf6\xf5");

	container->addControl(radioGroup);

	height = container->getSize().height;

	// relative note
	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO2, screen, this, PPPoint(x, y+height), PPSize(w4+39,36+2), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	y+=height;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 2), "Relative note:", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_RELNOTE, screen, this, PPPoint(x + 4 + 15*8 - 4, y + 2), "C-4", false));	

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_OCTUP, screen, this, PPPoint(x + 4, y + 1 + 12), PPSize(78+19, 11));
	button->setText("Octave up");
	container->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_NOTEUP, screen, this, PPPoint(x + 4 + 79+19, y + 1 + 12), PPSize(78+19, 11));
	button->setText("Note up");
	container->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_OCTDN, screen, this, PPPoint(x + 4, y + 1 + 12 + 12), PPSize(78+19, 11));
	button->setText("Octave dn");
	container->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_NOTEDN, screen, this, PPPoint(x + 4 + 79+19, y + 1 + 24), PPSize(78+19, 11));
	button->setText("Note dn");
	container->addControl(button);

	// piano
	y+=container->getSize().height;
	
	PPContainer* pianoContainer = new PPContainer(CONTAINER_PIANO, screen, this, PPPoint(0, y), PPSize(screen->getWidth(),25*2+SCROLLBUTTONSIZE+4), false);
	containerEntire->addControl(pianoContainer);
	pianoContainer->setColor(TrackerConfig::colorThemeMain);

	// piano test
	pp_int32 pianoWidth = screen->getWidth()-2-40;
	if (pianoWidth > 898)
		pianoWidth = 898;
	
	pp_int32 dx = 0;
		
	pianoControl = new PianoControl(PIANO_CONTROL, screen, this, PPPoint(1+40+dx, y+1), PPSize(pianoWidth, 25*2+12), ModuleEditor::MAX_NOTE); 
	// show C-2
	pianoControl->assureNoteVisible(12*2);
	pianoControl->setBorderColor(TrackerConfig::colorThemeMain);
	pianoContainer->addControl(pianoControl);
	
	button = new PPButton(BUTTON_PIANO_PLAY, screen, this, PPPoint(1, y+1), PPSize(38+dx, 20), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Play");

	pianoContainer->addControl(button);

	button = new PPButton(BUTTON_PIANO_EDIT, screen, this, PPPoint(1, y+1+20), PPSize(38+dx, 20), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Edit");

	pianoContainer->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_PLAY_STOP, screen, this, PPPoint(2, y+1+2*20), PPSize(36+dx, 20));
	button->setText("Stop");
	pianoContainer->addControl(button);		
#else
	// envelope stuff
	containerEnvelopes = new PPContainer(CONTAINER_ENVELOPES, screen, this, PPPoint(x, y), PPSize(320,69+4), false);
	containerEntire->addControl(containerEnvelopes);
	containerEnvelopes->setColor(TrackerConfig::colorThemeMain);

	envelopeEditorControl = new EnvelopeEditorControl(CONTAINER_ENVELOPES, screen, this, PPPoint(x + 2, y + 2 + 10), PPSize(224,59));
	envelopeEditorControl->attachEnvelopeEditor(tracker.getEnvelopeEditor());
	envelopeEditorControl->setBorderColor(TrackerConfig::colorThemeMain);
	
	containerEnvelopes->addControl(envelopeEditorControl);

	PPButton* button = new PPButton(BUTTON_ENVELOPE_VOLUME, screen, this, PPPoint(x + 2, y + 1), PPSize(32, 11), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Vol");
	containerEnvelopes->addControl(button);
	
	button = new PPButton(BUTTON_ENVELOPE_PANNING, screen, this, PPPoint(x + 2 + 32, y + 1), PPSize(32, 11), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Pan");
	containerEnvelopes->addControl(button);

	containerEnvelopes->addControl(new PPCheckBox(CHECKBOX_ENVELOPE_ON, screen, this, PPPoint(x + 224 + 4, y + 2)));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 224 + 4 + 13, y + 3), "On", true));

	button = new PPButton(BUTTON_ENVELOPE_UNDO, screen, this, PPPoint(button->getLocation().x + button->getSize().width+4, y + 2), PPSize(22, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Undo");
	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_REDO, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y + 2), PPSize(22, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Redo");
	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_COPY, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y + 2), PPSize(22, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Copy");
	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_PASTE, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y + 2), PPSize(27, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Paste");
	containerEnvelopes->addControl(button);

	PPStaticText* statict = new PPStaticText(0, NULL, NULL, PPPoint(button->getLocation().x + button->getSize().width+3, y + 4), "Zoom", true);
	statict->setFont(PPFont::getFont(PPFont::FONT_TINY));
	statict->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	containerEnvelopes->addControl(statict);

	button = new PPButton(BUTTON_ENVELOPE_ZOOMIN, screen, this, PPPoint(statict->getLocation().x + 5*5, y + 2), PPSize(12, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	containerEnvelopes->addControl(button);	

	button = new PPButton(BUTTON_ENVELOPE_ZOOMOUT, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y + 2), PPSize(12, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	containerEnvelopes->addControl(button);	

	button = new PPButton(BUTTON_ENVELOPE_ADD, screen, this, PPPoint(x + 224 + 36, y + 2), PPSize(28, 11));
	button->setText("Add");
	containerEnvelopes->addControl(button);
	
	button = new PPButton(BUTTON_ENVELOPE_DELETE, screen, this, PPPoint(x + 224 + 4 + 61, y + 2), PPSize(28, 11));
	button->setText("Del");
	containerEnvelopes->addControl(button);

	// sustain
	containerEnvelopes->addControl(new PPCheckBox(CHECKBOX_ENVELOPE_SUSTAIN, screen, this, PPPoint(x + 224 + 4, y + 2 + 14)));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 224 + 4 + 13, y + 2 + 15), "Sustain:", true));
	
	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 224 + 4, y + 2 + 15 + 12), "Point", true));

	// sustain point field
	containerEnvelopes->addControl(new PPStaticText(STATICTEXT_ENVELOPE_SUSTAINPT, screen, this, PPPoint(x + 224 + 4 + 6*8, y + 2 + 15 + 12), "00", false));

	button = new PPButton(BUTTON_ENVELOPE_SUSTAIN_PLUS, screen, this, PPPoint(x + 224 + 4 + 28 + 40, y + 2 + 15 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);

	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_SUSTAIN_MINUS, screen, this, PPPoint(x + 224 + 4 + 28 + 40 + 11, y + 2 + 15 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);

	containerEnvelopes->addControl(button);

	// loop
	containerEnvelopes->addControl(new PPCheckBox(CHECKBOX_ENVELOPE_LOOP, screen, this, PPPoint(x + 224 + 4, y + 2 + 14*2 + 9)));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 224 + 4 + 13, y + 2 + 14*2 + 10), "Loop:", true));

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 224 + 4, y + 2 + 14*2 + 10 + 12), "Start", true));

	// loop start point field
	containerEnvelopes->addControl(new PPStaticText(STATICTEXT_ENVELOPE_LOOPSTARTPT, screen, this, PPPoint(x + 224 + 4 + 6*8, y + 2 + 14*2 + 10 + 12), "00", false));

	button = new PPButton(BUTTON_ENVELOPE_LOOPSTART_PLUS, screen, this, PPPoint(x + 224 + 4 + 28 + 40, y + 2 + 14*2 + 10 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);

	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_LOOPSTART_MINUS, screen, this, PPPoint(x + 224 + 4 + 28 + 40 + 11, y + 2 + 14*2 + 10 + 11), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);

	containerEnvelopes->addControl(button);

	containerEnvelopes->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 224 + 4, y + 2 + 14*2 + 10 + 12 + 10), "End", true));

	// loop end point field
	containerEnvelopes->addControl(new PPStaticText(STATICTEXT_ENVELOPE_LOOPENDPT, screen, this, PPPoint(x + 224 + 4 + 6*8, y + 2 + 14*2 + 10 + 12 + 10), "00", false));

	button = new PPButton(BUTTON_ENVELOPE_LOOPEND_PLUS, screen, this, PPPoint(x + 224 + 4 + 28 + 40, y + 2 + 14*2 + 10 + 12 + 9), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);

	containerEnvelopes->addControl(button);

	button = new PPButton(BUTTON_ENVELOPE_LOOPEND_MINUS, screen, this, PPPoint(x + 224 + 4 + 28 + 40 + 11, y + 2 + 14*2 + 10 + 12 + 9), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);

	containerEnvelopes->addControl(button);

	// --------------- instrument info --------------- 
	y+=containerEnvelopes->getSize().height;
	
	PPContainer* container = new PPContainer(CONTAINER_INSTRUMENTS_INFO1, screen, this, PPPoint(x, y), PPSize(116,34+4), false);
	containerEntire->addControl(container);
	containerSampleSlider = container;
	container->setColor(TrackerConfig::colorThemeMain);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4), "Vol.", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VOLUME, screen, this, PPPoint(x + 4 + 8*5, y + 4), "FF", false));	

	PPSlider* slider = new PPSlider(SLIDER_SAMPLE_VOLUME, screen, this, PPPoint(x + 4 + 8*7+2, y + 2), 51, true);
	slider->setMaxValue(64);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 12), "Pan.", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_PANNING, screen, this, PPPoint(x + 4 + 8*5, y + 4 + 12), "FF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_PANNING, screen, this, PPPoint(x + 4 + 8*7+2, y + 2 + 12), 51, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 24), "Ft.", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_FINETUNE, screen, this, PPPoint(x + 4 + 8*3, y + 4 + 12*2), "-128", false));	

	slider = new PPSlider(SLIDER_SAMPLE_FINETUNE, screen, this, PPPoint(x + 4 + 8*7+2, y + 2 + 24), 51, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	pp_int32 width = container->getSize().width;
	pp_int32 height = container->getSize().height;

	// relative note
	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO2, screen, this, PPPoint(0, y + height), PPSize(116,34+4), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + 37), "Rel.note:", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_RELNOTE, screen, this, PPPoint(x + 4 + 16*5 - 4, y + 4 + 37), "C-4", false));	

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_OCTUP, screen, this, PPPoint(x + 4, y + 3 + 36 + 12), PPSize(56, 10));
	button->setText("Octave up");
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	container->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_NOTEUP, screen, this, PPPoint(x + 4 + 55, y + 3 + 36 + 12), PPSize(54, 10));
	button->setText("Note up");
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	container->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_OCTDN, screen, this, PPPoint(x + 4, y + 3 + 36 + 12 + 11), PPSize(56, 10));
	button->setText("Octave dn");
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	container->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_RELNOTENUM_NOTEDN, screen, this, PPPoint(x + 4 + 55, y + 3 + 36 + 23), PPSize(54, 10));
	button->setText("Note dn");
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	container->addControl(button);

	// autovibrato etc.
	x+=width;

	const pp_int32 dy = 15;

	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO3, screen, this, PPPoint(x, y), PPSize(165,76), false);
	containerEntire->addControl(container);
	containerInstrumentSlider = container;
	container->setColor(TrackerConfig::colorThemeMain);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4), "Fadeout", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VOLFADE, screen, this, PPPoint(x + 4 + 8*8, y + 4), "FFF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VOLFADE, screen, this, PPPoint(x + 4 + 8*11+2, y + 2), 68, true);
	slider->setMaxValue(0x1000);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + dy), "Vibspeed", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VIBSPEED, screen, this, PPPoint(x + 4 + 8*9, y + 4 + dy), "FF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VIBSPEED, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + dy), 68, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + dy*2), "Vibdepth", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VIBDEPTH, screen, this, PPPoint(x + 4 + 8*10, y + 4 + dy*2), "F", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VIBDEPTH, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + dy*2), 68, true);
	slider->setMaxValue(0xf);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + dy*3), "Vibsweep", true));	
	container->addControl(new PPStaticText(STATICTEXT_SAMPLE_VIBSWEEP, screen, this, PPPoint(x + 4 + 8*9, y + 4 + dy*3), "FF", false));	

	slider = new PPSlider(SLIDER_SAMPLE_VIBSWEEP, screen, this, PPPoint(x + 4 + 8*11+2, y + 2 + dy*3), 68, true);
	slider->setBarSize(16384);
	container->addControl(slider);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y + 4 + dy*4), "Type:", true));	
	
	PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SAMPLE_VIBTYPE, screen, this, PPPoint(x + 2 + 5*8, y + 4 + dy*4 - 4), PPSize(204, 20));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->setHorizontal(true);
	radioGroup->addItem("\xfc\xfb");
	radioGroup->addItem("\xfa\xf9");
	radioGroup->addItem("\xf8\xf7");
	radioGroup->addItem("\xf6\xf5");

	container->addControl(radioGroup);

	// sucks
	pp_int32 nx = x + container->getSize().width;

	// load & save 
	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO5, screen, this, PPPoint(nx, y), PPSize(39,23), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_LOAD, screen, this, PPPoint(nx + 2, y + 2), PPSize(34, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Load");

	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_SAVE, screen, this, PPPoint(nx + 2, y + 2+10), PPSize(34, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Save");

	container->addControl(button);

	// copy & paste
	y+=container->getSize().height;

	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO6, screen, this, PPPoint(nx, y), PPSize(39,23), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_COPY, screen, this, PPPoint(nx + 2, y + 2), PPSize(34, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Copy");

	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_SWAP, screen, this, PPPoint(nx + 2, y + 2+10), PPSize(34, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Swap");

	container->addControl(button);

	y+=container->getSize().height;

	// buoah
	container = new PPContainer(CONTAINER_INSTRUMENTS_INFO4, screen, this, PPPoint(nx, y), PPSize(39,30), false);
	containerEntire->addControl(container);
	container->setColor(TrackerConfig::colorThemeMain);

	button = new PPButton(MAINMENU_SMPEDIT, screen, &tracker, PPPoint(nx + 2, y + 2), PPSize(34, 12));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Smp.Ed.");
	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_EXIT, screen, &tracker, PPPoint(nx + 2, y + 2+13), PPSize(34, 12));
	button->setText("Exit");
	container->addControl(button);
	
	// piano
	y+=container->getSize().height;
	
	PPContainer* pianoContainer = new PPContainer(CONTAINER_PIANO, screen, this, PPPoint(0, y), PPSize(screen->getWidth(),25+SCROLLBUTTONSIZE+4), false);
	containerEntire->addControl(pianoContainer);
	pianoContainer->setColor(TrackerConfig::colorThemeMain);

	// piano test
	pianoControl = new PianoControl(PIANO_CONTROL, screen, this, 
									PPPoint(1+32, y+1), PPSize(screen->getWidth()-2-32, 25+12), ModuleEditor::MAX_NOTE); 
	// show C-3
	pianoControl->assureNoteVisible(12*4);
	pianoControl->setBorderColor(TrackerConfig::colorThemeMain);
	pianoControl->setMode(PianoControl::ModePlay);
	
	pianoContainer->addControl(pianoControl);

	button = new PPButton(BUTTON_PIANO_PLAY, screen, this, PPPoint(1, y+1), PPSize(30, 12), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Play");

	pianoContainer->addControl(button);

	button = new PPButton(BUTTON_PIANO_EDIT, screen, this, PPPoint(1, y+1+12), PPSize(30, 12), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Edit");

	pianoContainer->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_PLAY_STOP, screen, this, PPPoint(2, y+1+2*12), PPSize(28, 11));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Stop");
	pianoContainer->addControl(button);
	
#endif
	containerEntire->adjustContainerSize();
	screen->addControl(containerEntire);

	initialised = true;
	
	showSection(false);	
}

void SectionInstruments::realign()
{
	pp_uint32 maxShould = tracker.MAXEDITORHEIGHT();
	pp_uint32 maxIs = containerEntire->getLocation().y + containerEntire->getSize().height;
	
	if (maxIs != maxShould)
	{
		pp_int32 offset = maxShould - maxIs;
		containerEntire->move(PPPoint(0, offset));
	}
	
	PatternEditorControl* control = tracker.getPatternEditorControl();
	PPScreen* screen = tracker.screen;
	
	if (visible)
	{
		control->setSize(PPSize(screen->getWidth(),
							tracker.MAXEDITORHEIGHT()-tracker.INSTRUMENTSECTIONDEFAULTHEIGHT()-tracker.UPPERSECTIONDEFAULTHEIGHT()));
	}
	else
	{
		control->setSize(PPSize(screen->getWidth(),tracker.MAXEDITORHEIGHT()-tracker.UPPERSECTIONDEFAULTHEIGHT()));
	}
}

void SectionInstruments::show(bool bShow)
{
#ifdef __LOWRES__
	PPScreen* screen = tracker.screen;
	screen->pauseUpdate(true);
#endif
	SectionAbstract::show(bShow);
	
	visible = bShow;
	containerEntire->show(bShow);
	
	if (!initialised)
	{
		init();
	}

	if (initialised)
	{
		PatternEditorControl* control = tracker.getPatternEditorControl();
		
#ifndef __LOWRES__
		realign();
#endif
		
		if (bShow)
		{
			if (control)
			{
#ifdef __LOWRES__
				control->show(false);				
				replaceInstrumentListBoxes(true);
#endif
				tracker.hideInputControl();				
			}
			
			update(false);
		}
		else if (control)
		{
#ifdef __LOWRES__
			control->show(true);
			replaceInstrumentListBoxes(false);
#endif
		}
		
		showSection(bShow);
	}	

#ifdef __LOWRES__
	// If instrument section is shown (bShow = true)
	// set focus to the Instrumentlist container (instrument listbox)
	// but before disable screen updates to prevent flickering
	if (bShow)
	{
		screen->setFocus(screen->getControlByID(CONTAINER_INSTRUMENTLIST));
	}
	screen->pauseUpdate(false);
	if (!bShow)
	{
		screen->update();
	}
#endif
}

void SectionInstruments::updateSampleSliders(bool repaint/* = true*/)
{
	if (!initialised || !visible)
		return;

	PPScreen* screen = tracker.screen;
	PPContainer* container2 = containerSampleSlider;

	SampleEditor* sampleEditor = tracker.getSampleEditor();

	static_cast<PPSlider*>(container2->getControlByID(SLIDER_SAMPLE_VOLUME))->setCurrentValue(sampleEditor->getFT2Volume());	
	static_cast<PPStaticText*>(container2->getControlByID(STATICTEXT_SAMPLE_VOLUME))->setHexValue(sampleEditor->getFT2Volume(), 2);	

	static_cast<PPSlider*>(container2->getControlByID(SLIDER_SAMPLE_PANNING))->setCurrentValue(sampleEditor->getPanning());	
	static_cast<PPStaticText*>(container2->getControlByID(STATICTEXT_SAMPLE_PANNING))->setHexValue(sampleEditor->getPanning(), 2);	

	mp_sint32 ft = sampleEditor->getFinetune();

	static_cast<PPSlider*>(container2->getControlByID(SLIDER_SAMPLE_FINETUNE))->setCurrentValue((mp_uint32)(ft+128));	
	static_cast<PPStaticText*>(container2->getControlByID(STATICTEXT_SAMPLE_FINETUNE))->setIntValue(ft, 4, true);	

	screen->paintControl(container2, repaint);
}

void SectionInstruments::updateInstrumentSliders(bool repaint/* = true*/)
{
	if (!initialised || !visible)
		return;

	PPScreen* screen = tracker.screen;
	ModuleEditor* moduleEditor = tracker.moduleEditor;

	PPContainer* container4 = containerInstrumentSlider;
	ModuleEditor::TEditorInstrument* ins = moduleEditor->getInstrumentInfo(tracker.listBoxInstruments->getSelectedIndex());

	if (ins->volfade <= 0xFFF)
	{
		static_cast<PPSlider*>(container4->getControlByID(SLIDER_SAMPLE_VOLFADE))->setCurrentValue(ins->volfade);
		static_cast<PPStaticText*>(container4->getControlByID(STATICTEXT_SAMPLE_VOLFADE))->setHexValue(ins->volfade, 3);
	}
	else
	{
		static_cast<PPSlider*>(container4->getControlByID(SLIDER_SAMPLE_VOLFADE))->setCurrentValue(0x1000);
		static_cast<PPStaticText*>(container4->getControlByID(STATICTEXT_SAMPLE_VOLFADE))->setText("cut");
	}

	static_cast<PPSlider*>(container4->getControlByID(SLIDER_SAMPLE_VIBSPEED))->setCurrentValue(ins->vibrate);
	static_cast<PPStaticText*>(container4->getControlByID(STATICTEXT_SAMPLE_VIBSPEED))->setHexValue(ins->vibrate, 2);	

	static_cast<PPSlider*>(container4->getControlByID(SLIDER_SAMPLE_VIBDEPTH))->setCurrentValue(ins->vibdepth);
	static_cast<PPStaticText*>(container4->getControlByID(STATICTEXT_SAMPLE_VIBDEPTH))->setHexValue(ins->vibdepth, 1);	

	static_cast<PPSlider*>(container4->getControlByID(SLIDER_SAMPLE_VIBSWEEP))->setCurrentValue(ins->vibsweep);
	static_cast<PPStaticText*>(container4->getControlByID(STATICTEXT_SAMPLE_VIBSWEEP))->setHexValue(ins->vibsweep, 2);	

	static_cast<PPRadioGroup*>(container4->getControlByID(RADIOGROUP_SAMPLE_VIBTYPE))->setChoice(ins->vibtype);	

	screen->paintControl(container4, repaint);
}

void SectionInstruments::notifyTabSwitch()
{
	if (isVisible())
		realign();
	
	updateEnvelopeEditor(false, true);
	resetPianoAssignment();
}

void SectionInstruments::notifySampleSelect(pp_int32 index)
{
	PianoControl* pianoControl = getPianoControl();				
	if (pianoControl)
		pianoControl->setSampleIndex(index);
}

void SectionInstruments::update(bool repaint)
{
	if (!initialised || !visible)
		return;

	PPScreen* screen = tracker.screen;
	ModuleEditor* moduleEditor = tracker.moduleEditor;

	updateEnvelopeEditor(false, true);

	// volume/panning etc.
	updateSampleSliders(false);

	// relative note number
	PPContainer* container3 = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INSTRUMENTS_INFO2));

	char noteName[40];
	SampleEditor* sampleEditor = tracker.getSampleEditor();

#ifndef __LOWRES__ 
	if (sampleEditor->getRelNoteNum() > 0)
		sprintf(noteName,"    (+%i)",(pp_int32)sampleEditor->getRelNoteNum());
	else
		sprintf(noteName,"    (%i)",(pp_int32)sampleEditor->getRelNoteNum());
	PatternTools::getNoteName(noteName, sampleEditor->getRelNoteNum() + 4*12 + 1, false);
#else
	PatternTools::getNoteName(noteName, sampleEditor->getRelNoteNum() + 4*12 + 1);
#endif

	static_cast<PPStaticText*>(container3->getControlByID(STATICTEXT_SAMPLE_RELNOTE))->setText(noteName);		

	pianoControl->setSampleTable(moduleEditor->getSampleTable(tracker.listBoxInstruments->getSelectedIndex()));

	// volfade/autovobrato
	updateInstrumentSliders(false);

//#ifdef __LOWRES__
	PPContainer* container5 = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_PIANO));
	{
		PPButton* editButton = static_cast<PPButton*>(container5->getControlByID(BUTTON_PIANO_EDIT));
		PPButton* playButton = static_cast<PPButton*>(container5->getControlByID(BUTTON_PIANO_PLAY));

		editButton->setPressed(false);
		playButton->setPressed(false);
		switch (getPianoControl()->getMode())
		{
			case PianoControl::ModeEdit:
				editButton->setPressed(true);
				break;
			case PianoControl::ModePlay:
				playButton->setPressed(true);
				break;
		}
	}
	screen->paintControl(container5, false);
//#endif

	screen->paintControl(container3, false);
	screen->paintControl(pianoControl, false);
	if (repaint)
		screen->update();
}

void SectionInstruments::updateAfterLoad()
{
	tracker.updateInstrumentsListBox(false);
	tracker.updateSamplesListBox(false);
	tracker.updateSampleEditorAndInstrumentSection(false);
}

void SectionInstruments::updateEnvelopeWindow(bool repaint/* = true*/)
{
	tracker.screen->paintControl(envelopeEditorControl, repaint);
}

void SectionInstruments::updateEnvelopeEditor(bool repaint/* = true*/, bool reAttach/* = false*/)
{
	EnvelopeEditor* envelopeEditor = getEnvelopeEditor();

	if (!initialised || !visible)
	{
		// clear out envelope from editor if nothing is visible at all
		envelopeEditor->attachEnvelope(NULL, NULL);
		return;
	}

	PPScreen* screen = tracker.screen;
	
	PPContainer* container = containerEnvelopes;

	pp_int32 envIndex = -1;
	if (currentEnvelopeType == EnvelopeEditor::EnvelopeTypeVolume)
	{
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_VOLUME))->setPressed(true);
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_PANNING))->setPressed(false);		
		envIndex = 0;
		envelopeEditorControl->setShowVCenter(false);
	}
	else if (currentEnvelopeType == EnvelopeEditor::EnvelopeTypePanning)
	{
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_VOLUME))->setPressed(false);
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_PANNING))->setPressed(true);
		envIndex = 1;
		envelopeEditorControl->setShowVCenter(true);
	}

	if (reAttach && envIndex != -1)
		tracker.moduleEditor->reloadEnvelope(tracker.listBoxInstruments->getSelectedIndex(),
											 tracker.listBoxSamples->getSelectedIndex(), envIndex);		
	
#ifdef __LOWRES__
	// these buttons are always there 
	static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_UNDO))->setClickable(envelopeEditor->canUndo());
	static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_REDO))->setClickable(envelopeEditor->canRedo());
	static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_COPY))->setClickable(envelopeEditor->canCopy());
	static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_PASTE))->setClickable(envelopeEditor->canPaste());
#else
	// these buttons only exist with screen width >= 800
	if (screen->getWidth() >= 800)
	{
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_UNDO))->setClickable(envelopeEditor->canUndo());
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_REDO))->setClickable(envelopeEditor->canRedo());
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_COPY))->setClickable(envelopeEditor->canCopy());
		static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_PASTE))->setClickable(envelopeEditor->canPaste());
	}
#endif

	static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_ON))->enable(envelopeEditor->isValidEnvelope());
	static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_SUSTAIN))->enable(envelopeEditor->isValidEnvelope());
	static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_LOOP))->enable(envelopeEditor->isValidEnvelope());

	if (envelopeEditor->isValidEnvelope())
	{
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_ON))->checkIt(envelopeEditor->isEnvelopeEnabled());
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_SUSTAIN))->checkIt(envelopeEditor->isSustainEnabled());
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_LOOP))->checkIt(envelopeEditor->isLoopEnabled());
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ENVELOPE_SUSTAINPT))->setIntValue(envelopeEditor->getSustainPtIndex(), 2);
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ENVELOPE_LOOPSTARTPT))->setIntValue(envelopeEditor->getLoopStartPtIndex(), 2);
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ENVELOPE_LOOPENDPT))->setIntValue(envelopeEditor->getLoopEndPtIndex(), 2);
	}
	else
	{
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_ON))->checkIt(false);
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_SUSTAIN))->checkIt(false);
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_ENVELOPE_LOOP))->checkIt(false);
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ENVELOPE_SUSTAINPT))->setIntValue(0, 2);
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ENVELOPE_LOOPSTARTPT))->setIntValue(0, 2);
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ENVELOPE_LOOPENDPT))->setIntValue(0, 2);
	}	

	PPButton* button = static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_SCALEX));
	// button is only present in the hires version, NULL will be returned otherwise
	if (button)
		button->setClickable(!envelopeEditor->isEmptyEnvelope());
	button = static_cast<PPButton*>(container->getControlByID(BUTTON_ENVELOPE_SCALEY));
	// see above
	if (button)
		button->setClickable(!envelopeEditor->isEmptyEnvelope());

	screen->paintControl(container, repaint);	
}

void SectionInstruments::resetEnvelopeEditor()
{
	if (envelopeEditorControl)
		envelopeEditorControl->reset();
}

void SectionInstruments::resetPianoAssignment()
{
	pianoControl->setSampleIndex(0);
}

bool SectionInstruments::isEnvelopeVisible()
{
	if (containerEnvelopes)
		return containerEnvelopes->isVisible();
		
	return false;
}

pp_int32 SectionInstruments::getNumPredefinedEnvelopes()
{
	return TrackerConfig::numPredefinedEnvelopes;
}
	
PPString SectionInstruments::getEncodedEnvelope(EnvelopeTypes type, pp_int32 index)
{
	switch (type)
	{
		case EnvelopeTypeVolume:
			return EnvelopeContainer::encodeEnvelope(*predefinedVolumeEnvelopes->restore(index));
		case EnvelopeTypePanning:
			return EnvelopeContainer::encodeEnvelope(*predefinedPanningEnvelopes->restore(index));
	}
	
	ASSERT(false);
	return "";
}
	
void SectionInstruments::setEncodedEnvelope(EnvelopeTypes type, pp_int32 index, const PPString& str)
{
	TEnvelope e = EnvelopeContainer::decodeEnvelope(str); 

	switch (type)
	{
		case EnvelopeTypeVolume:
			predefinedVolumeEnvelopes->store(index, e);
			break;
		case EnvelopeTypePanning:
			predefinedPanningEnvelopes->store(index, e);
			break;
		default:
			ASSERT(false);
	}	
}

void SectionInstruments::handleZapInstrument()
{
	showMessageBox(MESSAGEBOX_ZAPINSTRUMENT, "Zap instrument?");
}

void SectionInstruments::zapInstrument()
{
	tracker.moduleEditor->zapInstrument(tracker.listBoxInstruments->getSelectedIndex());
	resetEnvelopeEditor();
	tracker.sectionSamples->resetSampleEditor();
	updateAfterLoad();
}

