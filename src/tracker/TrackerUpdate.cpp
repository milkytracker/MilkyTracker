/*
 *  tracker/TrackerUpdate.cpp
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

#include "Tracker.h"
#include "TabManager.h"
#include "TrackerConfig.h"
#include "PlayerController.h"
#include "PlayerMaster.h"
#include "ModuleEditor.h"
#include "ModuleServices.h"
#include "TabTitleProvider.h"
#include "EnvelopeEditor.h"
#include "PatternTools.h"
#include "PPUIConfig.h"
#include "Container.h"
#include "ListBox.h"
#include "StaticText.h"
#include "PatternEditorControl.h"
#include "EnvelopeEditorControl.h"
#include "PianoControl.h"
#include "PeakLevelControl.h"
#include "ScopesControl.h"
#include "SampleEditorControl.h"
#include "TrackerSettingsDatabase.h"
#include "SectionInstruments.h"
#include "SectionSamples.h"
#include "SectionHDRecorder.h"
#include "SectionQuickOptions.h"
#include "TabHeaderControl.h"
#include "PPOpenPanel.h"
#include "TitlePageManager.h"

#include "ControlIDs.h"

void Tracker::updateAboutToggleButton(pp_int32 id, bool b, bool repaint/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_ABOUT));
	ASSERT(container);
	PPButton* button = static_cast<PPButton*>(container->getControlByID(id));
	if (button == NULL)
		return;
	button->setPressed(b);	
	
	if (repaint)
		screen->paintControl(button);
}

bool Tracker::updatePianoControl(PianoControl* pianoControl)
{
	if (!pianoControl)
		return false;

	if (!pianoControl->isVisible())
		return false;

	pp_int32 ins = listBoxInstruments->getSelectedIndex() + 1;

	pp_int32 i;

	bool tab[ModuleEditor::MAX_NOTE];

	for (i = 0; i < ModuleEditor::MAX_NOTE; i++)
	{
		tab[i] = pianoControl->getNoteState(i);
		pianoControl->pressNote(i, false);
	}

	for (i = 0; i < playerController->getPlayerNumPlayingChannels(); i++)
	{
		pp_int32 note;
		bool muted = false;
		if (playerController->isNotePlaying(ins, i, note, muted))
			pianoControl->pressNote(note - 1, true, muted);
	}

	bool update = false;
	for (i = 0; i < ModuleEditor::MAX_NOTE; i++)
	{
		if (pianoControl->getNoteState(i) != tab[i])
		{
			update = true;
			break;
		}
	}

	if (update)
		screen->paintControl(pianoControl, false);

	return update;
}

bool Tracker::updatePeakLevelControl()
{
	const pp_int32 maxPeakThreshold = 32700*2;

	if (!peakLevelControl)
		return false;
	
	if (!peakLevelControl->isVisible())
		return false;

	bool bUpdateL = false;
	bool bUpdateR = false;

	bool bUpdateEntire = false;

	pp_int32 left, right;
	playerMaster->getCurrentSamplePeak(left, right);

	// Left channel
	pp_int32 oldPeak = peakLevelControl->getPeak(0);
	pp_int32 newPeak = left << 1;
	if (newPeak >= maxPeakThreshold)
	{
		TitlePageManager titlePageManager(*screen);
		titlePageManager.setPeakControlHeadingColor(TrackerConfig::colorPeakClipIndicator, false);
		bUpdateEntire = true;
	}
	if (newPeak < oldPeak)
	{
		newPeak = oldPeak;
		newPeak -= 2048;
		if (newPeak < 0)
			newPeak = 0;
	}
	if (oldPeak != newPeak)
	{
		peakLevelControl->setPeak(0, newPeak);
		bUpdateL = true;
	}

	// Right channel
	oldPeak = peakLevelControl->getPeak(1);
	newPeak = right << 1;
	if (newPeak >= maxPeakThreshold)
	{
		TitlePageManager titlePageManager(*screen);
		titlePageManager.setPeakControlHeadingColor(TrackerConfig::colorRecordModeButtonText, false);
		bUpdateEntire = true;
	}
	if (newPeak < oldPeak)
	{
		newPeak = oldPeak;
		newPeak -= 2048;
		if (newPeak < 0)
			newPeak = 0;
	}
	if (oldPeak != newPeak)
	{
		peakLevelControl->setPeak(1, newPeak);
		bUpdateR = true;
	}
	
	if (bUpdateEntire)
	{
		screen->paintControl(screen->getControlByID(CONTAINER_ABOUT), false);
		return true;
	}
	else if (bUpdateL || bUpdateR)
	{
		screen->paintControl(peakLevelControl, false);
		return true;
	}

	return false;
}

bool Tracker::updatePlayTime()
{
	if (!playTimeText)
		return false;

	if (!playTimeText->isVisible())
		return false;
	
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_ABOUT));

	char buffer[100], buffer2[100];
	
	pp_int32 playtime = (pp_int32)playerController->getPlayTime();
	
	if (!playerController->isPlaying())
		playtime = 0;
		
	pp_int32 seconds = playtime % 60;
	pp_int32 minutes = (playtime / 60) % 60;
	pp_int32 hours = (playtime / 3600) % 100;

	sprintf(buffer,"%02i:%02i:%02i", hours, minutes, seconds);

	playtime = moduleEditor->getModuleServices()->getEstimatedSongLength();
	if (playtime == -1)
	{
		// strcpy(buffer2,"(--:--:--)");
		// believe it or not, the VC 6.0 compiler
		// fucks up the above strcpy, now providing some awesome workaround ;)
		buffer2[0] = '(';
		buffer2[1] = '-';
		buffer2[2] = '-';
		buffer2[3] = ':';
		buffer2[4] = '-';
		buffer2[5] = '-';
		buffer2[6] = ':';
		buffer2[7] = '-';
		buffer2[8] = '-';
		buffer2[9] = ')';
		buffer2[10] = 0;
	}
	else
	{
		pp_int32 seconds = playtime % 60;
		pp_int32 minutes = (playtime / 60) % 60;
		pp_int32 hours = (playtime / 3600) % 100;
		sprintf(buffer2,"(%02i:%02i:%02i)", hours, minutes, seconds);
	}
	
	strcat(buffer, buffer2);
	
	if (strcmp(playTimeText->getText(), buffer) != 0)
	{
		playTimeText->setText(buffer);
		screen->paintControl(container, false);
		return true;
	}
	
	return false;
}

///////////////////////////////////////////
// update song title
///////////////////////////////////////////
void Tracker::updateSongTitle(bool repaint)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_ABOUT));
	
	PPListBox* listBox = static_cast<PPListBox*>(container->getControlByID(LISTBOX_SONGTITLE));
	
	listBox->clear();
	char str[MP_MAXTEXT+1];
	moduleEditor->getTitle(str, ModuleEditor::MAX_TITLETEXT);

	listBox->addItem(str);
	listBox->setMaxEditSize(ModuleEditor::MAX_TITLETEXT);

	screen->paintControl(container, repaint);
}

#ifdef __LOWRES__
void Tracker::updateJamMenuOrder(bool repaint/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_LOWRES_JAMMENU));
	PPStaticText* staticText = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_JAMMENU_CURORDER));
	staticText->setHexValue(getOrderListBoxIndex(), 2);
	if (container->isVisible())
		screen->paintControl(container, repaint);
}
#endif

void Tracker::updateOrderlist(bool repaint/* = true*/)
{
	// First of all: update orderlist (something might have been removed/added)
	updateSongLength(false);
	// Now update patterns
	moduleEditor->setCurrentPatternIndex(moduleEditor->getOrderPosition(getOrderListBoxIndex()));
	moduleEditor->setCurrentOrderIndex(getOrderListBoxIndex());

	updatePattern(false);

	if (repaint)
		screen->update();
}

///////////////////////////////////////////
// update song length field + orderlist
///////////////////////////////////////////
void Tracker::updateSongLength(bool repaint)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_ORDERLIST));
	
	static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ORDERLIST_SONGLENGTH))->setHexValue(moduleEditor->getNumOrders(), 2);

	listBoxOrderList->setShowIndex(true);
	listBoxOrderList->setIndexBaseCount(0);

	listBoxOrderList->saveState();

	bool update = true;
	if (listBoxOrderList->getNumItems() != moduleEditor->getNumOrders())
	{
		listBoxOrderList->clear();
		update = false;
	}

	for (pp_int32 i = 0; i < moduleEditor->getNumOrders(); i++)
	{
		char orderlistEntry[4];
		
		PatternTools::convertToHex(orderlistEntry, moduleEditor->getOrderPosition(i), 2);

		if (update)
			listBoxOrderList->updateItem(i, orderlistEntry);
		else
			listBoxOrderList->addItem(orderlistEntry);
	}

	if (!update)
	{
		listBoxOrderList->restoreState();
		listBoxOrderList->setSelectedIndex(moduleEditor->getCurrentOrderIndex());
	}

	screen->paintControl(container, repaint);
}

///////////////////////////////////////////
// update song restart position field
///////////////////////////////////////////
void Tracker::updateSongRepeat(bool repaint)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_ORDERLIST));
	static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ORDERLIST_REPEAT))->setHexValue(moduleEditor->getRepeatPos(), 2);
	
	screen->paintControl(container, repaint);
}

///////////////////////////////////////////
// update speed field
///////////////////////////////////////////
bool Tracker::updateSpeed(bool repaint)
{
	mp_sint32 speed, bpm, mainvol;

	playerController->getSpeed(bpm, speed);
	mainvol = playerController->getSongMainVolume();

	if (bpm != lastBPM || 
		lastSpeed != speed ||
		lastMainVol != mainvol)
	{
		lastBPM = bpm;
		lastSpeed = speed;
		lastMainVol = mainvol;
		PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_SPEED));
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SPEED_SPEED))->setIntValue(speed, 2);		
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SPEED_BPM))->setIntValue(bpm, 3);
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SPEED_MAINVOL))->setHexValue((mainvol*64)/255, 2);
		
		screen->paintControl(container, repaint);
		return true;
	}
	return false;
}

///////////////////////////////////////////
// update pattern add field
///////////////////////////////////////////
void Tracker::updatePatternAddAndOctave(bool repaint)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_SPEED));

	pp_int32 currentPatternAdd = getPatternEditorControl()->getRowInsertAdd();
	static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SPEED_PATTERNADD))->setIntValue(currentPatternAdd, 2);
	pp_int32 currentOctave = getPatternEditor()->getCurrentOctave();
	static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SPEED_OCTAVE))->setIntValue(currentOctave-1, 2);
	
	screen->paintControl(container, repaint);
}


///////////////////////////////////////////
// update current pattern index
///////////////////////////////////////////
void Tracker::updatePatternIndex(bool repaint)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_PATTERN));	
	static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_PATTERN_INDEX))->setHexValue(moduleEditor->getCurrentPatternIndex(), 2);
	
#ifdef __LOWRES__
	{
		PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_LOWRES_JAMMENU));
		PPStaticText* staticText = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_JAMMENU_CURPATTERN));
		staticText->setHexValue(moduleEditor->getCurrentPatternIndex(), 2);
		if (container->isVisible())
			screen->paintControl(container, repaint);
	}
#endif
	screen->paintControl(container, repaint);
}

///////////////////////////////////////////
// update current pattern's length
///////////////////////////////////////////
void Tracker::updatePatternLength(bool repaint)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_PATTERN));
	static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_PATTERN_LENGTH))->setHexValue(getPatternEditor()->getNumRows(),3);
	
	screen->paintControl(container, repaint);
}

///////////////////////////////////////////
// update pattern and it's length and index
///////////////////////////////////////////
void Tracker::updatePattern(bool repaint)
{
	updatePatternEditorControl(false);
	updatePatternIndex(false);
	updatePatternLength(false);
	
	if (repaint)
		screen->update();
}

///////////////////////////////////////////
// update samples listbox
///////////////////////////////////////////
void Tracker::updateSamplesListBox(bool repaint)
{	
	listBoxSamples->saveState();
				
	listBoxSamples->clear();
	
	fillSampleListBox(listBoxSamples, listBoxInstruments->getSelectedIndex());			
				
	listBoxSamples->restoreState();

	// check for visibility of parent container before updating
	if (static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INSTRUMENTLIST))->isVisible())
		screen->paintControl(listBoxSamples, repaint);

#ifdef __LOWRES__
	{
		pp_int32 i = listBoxInstruments->getSelectedIndex();

		PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_LOWRES_JAMMENU));
		PPStaticText* staticText = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_JAMMENU_CURINSTRUMENT));
		staticText->setHexValue(i+1, 2);
		if (container->isVisible())
			screen->paintControl(container, repaint);

		container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INSTRUMENTLIST));
		staticText = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER2));
		staticText->setHexValue(i+1, 2);
		if (container->isVisible())
			screen->paintControl(container, repaint);
	}
#endif
}

///////////////////////////////////////////
// update instruments listbox
///////////////////////////////////////////
void Tracker::updateInstrumentsListBox(bool repaint)
{
	listBoxInstruments->saveState();
				
	listBoxInstruments->clear();
	
	fillInstrumentListBox(listBoxInstruments);
				
	listBoxInstruments->restoreState();

	// check for visibility of parent container before updating
	if (!static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INSTRUMENTLIST))->isVisible())
		return;

	screen->paintControl(listBoxInstruments, repaint);
}

///////////////////////////////////////////
// update pattern editor
///////////////////////////////////////////
void Tracker::updatePatternEditorControl(/*TXMPattern* pattern, */bool repaint/* = true*/, bool fast/* = false*/)
{
	PatternEditorControl* patternEditorCtrl = getPatternEditorControl();
	
	moduleEditor->reloadCurrentPattern();
	
	TXMPattern* pattern = getPatternEditor()->getPattern();
	
	if (pattern)
	{
		if (isEditingCurrentOrderlistPattern())
			patternEditorCtrl->setOrderlistIndex(getOrderListBoxIndex());
		else
			patternEditorCtrl->setOrderlistIndex(-1);
		
		//patternEditorCtrl->attachPattern(pattern, moduleEditor->getModule());
	}

	if (!fast)
		screen->paintControl(patternEditorCtrl, repaint);
}

///////////////////////////////////////////
// update instrument editor
///////////////////////////////////////////
void Tracker::updateSampleEditor(bool repaint, bool force)
{
	sectionSamples->realUpdate(repaint, force, true);
}

void Tracker::updateSampleEditorAndInstrumentSection(bool repaint/* = true*/)
{
	// update instrument/sample editor
	// important: sample editor first => will reload sample into sample editor
	updateSampleEditor(repaint);
	sectionInstruments->update(repaint);
}

void Tracker::updateSongInfo(bool repaint/* = true*/)
{
	updateSongTitle(repaint);
				
	updateSongLength(repaint);
	updateSongRepeat(repaint);
				
	//updateBPM(repaint);
	updateSpeed(repaint);                                                                      
	updatePatternAddAndOctave(repaint);
				
	updatePatternIndex(repaint);
	updatePatternLength(repaint);
				
	updateInstrumentsListBox(repaint);
	updateSamplesListBox(repaint);				
				
	getPatternEditorControl()->reset();
	getPatternEditorControl()->unmuteAll();
	sectionInstruments->resetEnvelopeEditor();
	sectionInstruments->updateEnvelopeEditor(false, true);
	sectionInstruments->resetPianoAssignment();
	sectionSamples->resetSampleEditor();

	setNumChannels(moduleEditor->getNumChannels(), false);

	updatePatternEditorControl(repaint);
				
	updateSampleEditorAndInstrumentSection(repaint);

	updateWindowTitle(moduleEditor->getModuleFileName());

	if (repaint)
		screen->update();
}

void Tracker::updateInstrumentChooser(bool repaint/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getModalControl());

	PPListBox* listBoxSrc = static_cast<PPListBox*>(container->getControlByID(INSTRUMENT_CHOOSER_LIST_SRC));
	PPListBox* listBoxSrcModule = static_cast<PPListBox*>(container->getControlByID(INSTRUMENT_CHOOSER_LIST_SRC3));
	PPListBox* listBoxDst = static_cast<PPListBox*>(container->getControlByID(INSTRUMENT_CHOOSER_LIST_DST));
	PPListBox* listBoxDstModule = static_cast<PPListBox*>(container->getControlByID(INSTRUMENT_CHOOSER_LIST_DST3));

	ModuleEditor* src = listBoxSrcModule ? tabManager->getModuleEditorFromTabIndex(listBoxSrcModule->getSelectedIndex()) : this->moduleEditor;
	ModuleEditor* dst = listBoxDstModule ? tabManager->getModuleEditorFromTabIndex(listBoxDstModule->getSelectedIndex()) : this->moduleEditor;

	listBoxSrc->saveState();
	listBoxSrc->clear();	
	fillInstrumentListBox(listBoxSrc, src);
	listBoxSrc->restoreState();
	
	listBoxDst->saveState();
	listBoxDst->clear();
	fillInstrumentListBox(listBoxDst, dst);
	listBoxDst->restoreState();

	mp_sint32 srcInsIndex = listBoxSrc->getSelectedIndex();
	mp_sint32 dstInsIndex = listBoxDst->getSelectedIndex();

	listBoxSrc = static_cast<PPListBox*>(container->getControlByID(INSTRUMENT_CHOOSER_LIST_SRC2));
	listBoxDst = static_cast<PPListBox*>(container->getControlByID(INSTRUMENT_CHOOSER_LIST_DST2));

	listBoxSrc->saveState();
	listBoxSrc->clear();
	
	fillSampleListBox(listBoxSrc, srcInsIndex, src);	

	listBoxSrc->restoreState();
	
	listBoxDst->saveState();
	listBoxDst->clear();
	
	fillSampleListBox(listBoxDst, dstInsIndex, dst);

	listBoxDst->restoreState();
	
	if (repaint)
		screen->update();
}

void Tracker::updateTabTitle()
{
	TabHeaderControl* tabHeader = static_cast<TabHeaderControl*>(screen->getControlByID(TABHEADER_CONTROL));
	
	if (tabHeader != NULL)
	{
		TabTitleProvider tabTitleProvider(*moduleEditor);
		PPString tabTitle = tabTitleProvider.getTabTitle();
		if (moduleEditor->hasChanged())
			tabTitle.append("*");
		
		bool refresh = false;
		
		for (pp_int32 i = 0; i < (signed)tabHeader->getNumTabs(); i++)
		{
			if (tabManager->getModuleEditorFromTabIndex(i) == this->moduleEditor)
			{
				if (tabHeader->getTab(i)->text.compareTo(tabTitle) != 0)
				{					
					tabHeader->setTabHeaderText(i, tabTitle);
					refresh = true;
				}
			}
		}
		
		if (refresh)
			screen->paintControl(tabHeader);
	}
}

void Tracker::updateWindowTitle()
{
	if (moduleEditor->hasChanged() != lastState)
	{
		PPSystemString title = "MilkyTracker - ";

		title.append(currentFileName);

		if (moduleEditor->hasChanged())
			title.append(" (modified)");

		screen->setTitle(title);
		
		updateTabTitle();
		
		lastState = moduleEditor->hasChanged();
	}
}

void Tracker::updateWindowTitle(const PPSystemString& fileName)
{
	lastState = !moduleEditor->hasChanged();

	currentFileName = fileName.stripPath();

	updateWindowTitle();
}

bool Tracker::updateSongPosition(pp_int32 pos/* = -1*/, pp_int32 row/* = -1*/, bool fast/* = false*/)
{
	mp_sint32 thePos, theRow;
	playerController->getPosition(thePos, theRow);
	
#if 0
	static int counter = 0;
	if (thePos == 0 && theRow == 63 && (theRow != lastRow || thePos != lastPos))
		counter++;
		
	if (counter == 2)
	{
		int i = 0;
		i++;
		i--;
	}
#endif
	
	if (pos == -1)
		pos = thePos;
	if (row == -1)
		row = theRow;
	
	bool redraw = false;
	
	// I'm now trying to understand what happens here ------>
	// When the current row is different from the last row
	// or the current order position is different from the last one
	// we're doing some screen refreshing
	if (row != lastRow || pos != lastPos)
	{
		// if we're not playing a single pattern, we're most likely playing the entire song
		// and in that case if we're instructed to follow the song while playing we're doing some
		// order list position updates and stuff ------>
		if (!playerController->isPlayingPattern() && shouldFollowSong())
		{
			// if the current order list index is different from the current 
			// order list position from the player UPDATE ------>
			if (getOrderListBoxIndex() != pos)
			{
				listBoxOrderList->setSelectedIndex(pos, false);	
				moduleEditor->setCurrentOrderIndex(pos);		
				// in low res mode we also need to update
				// the order position in the "Jam"-section
#ifdef __LOWRES__
				if (!fast)
					updateJamMenuOrder(false);
#endif
				// now tell the module editor that we're editing another pattern
				moduleEditor->setCurrentPatternIndex(moduleEditor->getOrderPosition(pos));
				// update pattern editor with current order list pattern
				// but don't redraw/update, just get it from the module editor and attach it to the pattern editor
				updatePatternEditorControl(false, true);			
			}
			
			if (!fast)
			{
				// update pattern property fields
				updateSongLength(false);
				updatePatternIndex(false);
				updatePatternLength(false);
			}

			// only follow song if the pattern index matches the one from the
			// order list, otherwise the user has probably selected a different
			// pattern while playing takes place
			if (moduleEditor->getCurrentPatternIndex() == moduleEditor->getOrderPosition(pos))
			{			
				// song positon is only used in non-follow mode
				getPatternEditorControl()->setSongPosition(-1, -1);
				// but we should probably update the current row in the pattern editor as well
				// as long as the user is not currently dragging the scroll bars in the sample editor
				getPatternEditorControl()->setRow(row, fast ? false : !getPatternEditorControl()->isDraggingVertical());
			}
		}
		else if (playerController->isPlayingPattern() && !shouldFollowSong())
		{
			// we're playing the current pattern and don't follow the song
			// just update the current playing row in the pattern editor
			if (playerController->isPlayingPattern(moduleEditor->getCurrentPatternIndex()))
				getPatternEditorControl()->setSongPosition(-1, row);
			// we're playing a pattern different from the one in the editor
			// don't show any positions
			else
				getPatternEditorControl()->setSongPosition(-1, -1);
		}
		else if (playerController->isPlayingPattern() && shouldFollowSong())
		{
			getPatternEditorControl()->setSongPosition(-1, -1);
			getPatternEditorControl()->setRow(row, fast ? false : !getPatternEditorControl()->isDraggingVertical());
		}
		else		
		{
			getPatternEditorControl()->setSongPosition(pos, row);
		}
		
		updatePatternEditorControl(false, fast);
	
		redraw = true;

		lastRow = row;
		lastPos = pos;
	}
	
	if (!fast)
		return updateSpeed(false) || redraw;
	else
		return redraw;
}

void Tracker::updateRecordButton(PPContainer* container, const PPColor& pColor)
{
	ASSERT(container);
	
	// now if the parent container is visible, 
	// we're going to update the record button
	if (container->isVisible())
	{
		// get button from container
		PPButton* button = static_cast<PPButton*>(container->getControlByID(MAINMENU_EDIT));
		ASSERT(button);
		
		// if it's visible and the color is not what we're going to set
		// set new text color and repaint
		if (button->isVisible() && (&pColor) != button->getTextColor())
		{
			button->setTextColor(pColor);
			screen->paintControl(button);
		}
	}
}

void Tracker::doFollowSong()
{
	// check if we need to update the record button
	// this is done periodically and only in MilkyTracker mode
	if (editMode == EditModeMilkyTracker)
	{
		// If the pattern editor has got the focus
		// we're in record mode, so get the record button text color
		const PPColor& pColor = getPatternEditorControl()->gotFocus() ? TrackerConfig::colorRecordModeButtonText : PPUIConfig::getInstance()->getColor(PPUIConfig::ColorDefaultButtonText);
		
		// we're going to update the record button
		updateRecordButton(static_cast<PPContainer*>(screen->getControlByID(CONTAINER_MENU)), pColor);
		
#ifdef __LOWRES__
		// in low-res mode, the record buttons appears 
		// also in some other containers (e.g. TINYMENU) ...
		updateRecordButton(static_cast<PPContainer*>(screen->getControlByID(CONTAINER_LOWRES_TINYMENU)), pColor);

		// ... also the JAMMENU
		updateRecordButton(static_cast<PPContainer*>(screen->getControlByID(CONTAINER_LOWRES_JAMMENU)), pColor);
#endif
	}
	
	// check if the piano has been updated
	bool updatePiano = updatePianoControl(sectionInstruments->getPianoControl());
	
	// check if the play time has been updated
	bool updatePlayTime = this->updatePlayTime();

#ifdef __LOWRES__
	// in low-res mode a piano might be embedded into other containers as well
	// => check for updates
	if (inputContainerCurrent->isVisible())
	{
		PianoControl* pianoControl = static_cast<PianoControl*>(inputContainerCurrent->getControlByID(PIANO_CONTROL));
		updatePiano |= updatePianoControl(pianoControl);
	}
	else if (screen->getControlByID(CONTAINER_LOWRES_JAMMENU)->isVisible())
	{
		PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_LOWRES_JAMMENU));
		PianoControl* pianoControl = static_cast<PianoControl*>(container->getControlByID(PIANO_CONTROL));
		updatePiano |= updatePianoControl(pianoControl);
	}
#endif

	bool importantRefresh = false;

	// now for updating the sample editor control
	SampleEditorControl* sampleEditorControl = sectionSamples->getSampleEditorControl(false);

	if (sampleEditorControl)
	{
		bool showMarksVisibleOld = sampleEditorControl->isVisible() ? sampleEditorControl->showMarksVisible() : false;
		bool updateSample = false;
	
		TXMSample* smp = getSampleEditor()->getSample();

		for (pp_int32 i = 0; i < playerController->getAllNumPlayingChannels(); i++)
		{
			pp_int32 pos, vol, pan;
			if (playerController->isSamplePlaying(*smp, i, pos, vol, pan))
			{
				// it is there 
				// => set the position mark
				sampleEditorControl->setShowMark(i, pos, vol, pan);
				updateSample = true;
			}
			else
			{
				// it is not there
				// => clear the position mark
				if (sampleEditorControl->getShowMarkPos(i) != -1)
				{
					sampleEditorControl->setShowMark(i, -1, 0);
					updateSample = true;
				}
			}
		}
		
		if (updateSample && sampleEditorControl->isVisible())
		{
			bool showMarksVisible = sampleEditorControl->showMarksVisible();
			
			// if either show marks HAVE BEEN visible and they're no longer visble OR
			// no show marks have been visible and now they're some visible
			// => redraw
			if (showMarksVisible || showMarksVisibleOld)
			{
				sectionSamples->updateSampleWindow(false);
				importantRefresh = true;
			}
		}
		
	}
	
	if (sectionInstruments->isEnvelopeVisible())
	{
		EnvelopeEditorControl* eeCtrl = sectionInstruments->getEnvelopeEditorControl();
		
		bool redrawEnvelopeEditor = false;
		
		if (eeCtrl->hasShowMarks())
		{
			eeCtrl->clearShowMarks();
			redrawEnvelopeEditor = true;
		}	

		EnvelopeEditor* ee = getEnvelopeEditor();

		for (pp_int32 i = 0; i < playerController->getPlayerNumPlayingChannels(); i++)
		{	
			pp_int32 pos;
			if (playerController->isEnvelopePlaying(*ee->getEnvelope(), sectionInstruments->getVisibleEnvelopeType(), i, pos))
			{
				eeCtrl->setShowMark(i, pos);
				
				if (pos != -1)
					redrawEnvelopeEditor = true;
			}				
		}
		
		if (redrawEnvelopeEditor)
		{
			sectionInstruments->updateEnvelopeWindow(false);
			importantRefresh = true;
		}

	}
	
	if (playerController->isPlaying() && 
		!playerController->isPlayingRowOnly()/* && getFollowSong()*/)
	{	
		importantRefresh |= updateSongPosition();
	}
	
	bool updatePeak = updatePeakLevelControl();
	
	bool updateScopes = false;
	if (scopesControl && scopesControl->isVisible())
	{
		updateScopes = scopesControl->needsUpdate();
		if (updateScopes && !updatePiano && !importantRefresh && !updatePlayTime && !updatePeak)
		{
			screen->paintControl(scopesControl);
			return;
		}
		else if (updateScopes)
			screen->paintControl(scopesControl, false);			
	}
	
	importantRefresh |= updatePiano;
	
	if (!importantRefresh)
	{
		if (updatePeak || updatePlayTime)
		{
			screen->updateControl(screen->getControlByID(CONTAINER_ABOUT));
		}
		
		if (updateScopes)
		{
			screen->updateControl(scopesControl);
		}
	}
	else
	{
		screen->update();
	}
}

void Tracker::updateAfterLoad(bool loadResult, bool wasPlaying, bool wasPlayingPattern)
{
	ASSERT(settingsDatabase->restore("AUTOESTPLAYTIME"));
	if (loadResult && settingsDatabase->restore("AUTOESTPLAYTIME")->getIntValue())
		estimateSongLength();
	else
		moduleEditor->getModuleServices()->resetEstimatedSongLength();
	
	// special updates
	listBoxOrderList->setSelectedIndex(0);
	moduleEditor->setCurrentOrderIndex(0);
	
	moduleEditor->setCurrentPatternIndex(moduleEditor->getOrderPosition(0));
	
	listBoxInstruments->setSelectedIndex(0);
	moduleEditor->setCurrentInstrumentIndex(0);
	listBoxSamples->setSelectedIndex(0);
	moduleEditor->setCurrentSampleIndex(0);
	
	updateSongInfo(false);
	
	playerController->resetMainVolume();
	getPatternEditorControl()->setChannel(0,0);
	getPatternEditorControl()->setCurrentInstrument(1);
	
	if (wasPlaying)
	{
		if (!wasPlayingPattern && shouldFollowSong())
		{
			getPatternEditorControl()->setSongPosition(-1, -1);
			getPatternEditorControl()->setRow(0);
		}
		else if (wasPlayingPattern && !shouldFollowSong())
		{
			getPatternEditorControl()->setSongPosition(-1, 0);
		}
		else if (wasPlayingPattern && shouldFollowSong())
		{
			getPatternEditorControl()->setSongPosition(-1, -1);
			getPatternEditorControl()->setRow(0);
		}
	}
	else		
	{
		getPatternEditorControl()->setSongPosition(-1, -1);
		getPatternEditorControl()->setRow(0);
	}
	
	if (loadResult)
	{
		playerController->resetPlayTimeCounter();
		
		sectionHDRecorder->resetCurrentFileName();
		sectionHDRecorder->adjustOrders();
		
		updateWindowTitle(moduleEditor->getModuleFileName());					
		
		// !!! Remember this !!!
		// It's important to apply new speed settings before the playmode is switched
		playerController->setSpeed(moduleEditor->getSongBPM(), moduleEditor->getSongTickSpeed());

		if (!sectionQuickOptions->keepSettings())
		{
			switch (moduleEditor->getSaveType())
			{
				case ModuleEditor::ModSaveTypeMOD:
					if (moduleEditor->getNumChannels() == 4)
						playerController->switchPlayMode(PlayerController::PlayMode_ProTracker2);
					else
						playerController->switchPlayMode(PlayerController::PlayMode_ProTracker3);
					break;
				case ModuleEditor::ModSaveTypeXM:
					playerController->switchPlayMode(PlayerController::PlayMode_FastTracker2);
					break;
				default:
					ASSERT(false);
			}
		}

		if (wasPlaying && !wasPlayingPattern)
		{
			playerController->restartPlaying();
		}
		else if (wasPlaying && wasPlayingPattern)
		{
			eventKeyDownBinding_Stop();
			eventKeyDownBinding_PlayPattern();
		}
	}
	
	updateSongInfo(false);
}

void Tracker::updateAfterTabSwitch()
{
	pp_int32 i;
	
	getPatternEditorControl()->attachPatternEditor(moduleEditor->getPatternEditor());
	sectionSamples->getSampleEditorControl(false)->attachSampleEditor(moduleEditor->getSampleEditor());
	sectionInstruments->getEnvelopeEditorControl()->attachEnvelopeEditor(moduleEditor->getEnvelopeEditor());
	// -----------------------------------------------------------------------
	sectionHDRecorder->resetCurrentFileName();
	sectionHDRecorder->adjustOrders();
	
	updateSongTitle(false);
				
	updateSongLength(false);
	updateSongRepeat(false);
				
	updateSpeed(false);                                                                      
	updatePatternAddAndOctave(false);
				
	updatePatternIndex(false);
	updatePatternLength(false);
				
	updateInstrumentsListBox(false);
	updateSamplesListBox(false);				
				
	// -----------------------------------------------------------------------
	// restore old positions
	listBoxInstruments->setSelectedIndex(moduleEditor->getCurrentInstrumentIndex(), false);
	listBoxSamples->setSelectedIndex(moduleEditor->getCurrentSampleIndex(), false);

	if (!playerController->isPlaying())
	{
		listBoxOrderList->setSelectedIndex(moduleEditor->getCurrentOrderIndex());
		updatePatternEditorControl(false, true);
		getPatternEditor()->setCursor(moduleEditor->getCurrentCursorPosition());
	}

	getPatternEditorControl()->setSize(PPSize(screen->getWidth(),MAXEDITORHEIGHT()-UPPERSECTIONDEFAULTHEIGHT()));

	// let tabs handle their own update
	for (i = 0; i < sections->size(); i++)
		sections->get(i)->notifyTabSwitch();

	setNumChannels(moduleEditor->getNumChannels(), false);
	scopesControl->attachSource(playerController);
	updateSampleEditorAndInstrumentSection(false);
	updateWindowTitle(moduleEditor->getModuleFileName());
	
	// apply muting from playercontroller
	const pp_int32 numChannels = TrackerConfig::numPlayerChannels;
	for (i = 0; i < numChannels; i++)
	{
		bool b = playerController->isChannelMuted(i);
		muteChannels[i] = b ? 1 : 0;
		getPatternEditorControl()->muteChannel(i, b);
		scopesControl->muteChannel(i, b);
		
		b = playerController->isChannelRecording(i);
		scopesControl->recordChannel(i, b);
	}

	if (playerController->isPlaying())
	{
		screen->pauseUpdate(true);
		doFollowSong();
		screen->pauseUpdate(false);
	}
	screen->paint(true);
}

