/*
 *  tracker/SectionOptimize.cpp
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
 *  SectionOptimize.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 17.11.05.
 *
 */

#include "SectionOptimize.h"
#include "SectionInstruments.h"
#include "Tracker.h"
#include "SectionSamples.h"
#include "TrackerConfig.h"
#include "ModuleEditor.h"
#include "PatternEditorControl.h"
#include "SampleEditorControl.h"
#include "StaticText.h"
#include "Seperator.h"
#include "Container.h"
#include "CheckBox.h"
#include "Font.h"
#include "DialogListBox.h"
#include "ListBox.h"

#include "ControlIDs.h"

enum CheckBoxBitPositions
{
	// Group 2 ---------------------------------------
	BITPOS_CHECKBOX_REMOVE = 0,
	BITPOS_CHECKBOX_REMOVE_PATTERNS,
	BITPOS_CHECKBOX_REMOVE_INSTRUMENTS,
	BITPOS_CHECKBOX_REMOVE_SAMPLES,
	BITPOS_CHECKBOX_REARRANGE,
	BITPOS_CHECKBOX_MINIMIZEALL,
	BITPOS_CHECKBOX_CONVERTALL,
	BITPOS_CHECKBOX_CRUNCHHEADER,
	// -----------------------------------------------

	// Group 2 ---------------------------------------
	BITPOS_CHECKBOX_OPERANDCONTROL_1xx = 0,
	BITPOS_CHECKBOX_OPERANDCONTROL_2xx,
	BITPOS_CHECKBOX_OPERANDCONTROL_3xx,
	BITPOS_CHECKBOX_OPERANDCONTROL_4xx,
	BITPOS_CHECKBOX_OPERANDCONTROL_A56xx,
	BITPOS_CHECKBOX_OPERANDCONTROL_Hxx,
	BITPOS_CHECKBOX_OPERANDCONTROL_7xx,
	BITPOS_CHECKBOX_OPERANDCONTROL_E1x,
	BITPOS_CHECKBOX_OPERANDCONTROL_E2x,
	BITPOS_CHECKBOX_OPERANDCONTROL_EAx,
	BITPOS_CHECKBOX_OPERANDCONTROL_EBx,
	BITPOS_CHECKBOX_OPERANDCONTROL_Pxx,
	BITPOS_CHECKBOX_OPERANDCONTROL_Rxx,
	BITPOS_CHECKBOX_OPERANDCONTROL_X1x,
	BITPOS_CHECKBOX_OPERANDCONTROL_X2x,

	BITPOS_CHECKBOX_EFFECTRELOCATION_3xx,
	BITPOS_CHECKBOX_EFFECTRELOCATION_4xx,
	BITPOS_CHECKBOX_EFFECTRELOCATION_8xx,
	BITPOS_CHECKBOX_EFFECTRELOCATION_Cxx,
	BITPOS_CHECKBOX_EFFECTRELOCATION_Axx,
	BITPOS_CHECKBOX_EFFECTRELOCATION_EAx,
	BITPOS_CHECKBOX_EFFECTRELOCATION_EBx,
	BITPOS_CHECKBOX_EFFECTRELOCATION_Pxx
	// -----------------------------------------------
};

enum ControlIDs
{
	CONTAINER_OPTIMIZE					= 8000,
	OPTIMIZE_BUTTON_EXIT,

	OPTIMIZE_CHECKBOX_REMOVE,
	OPTIMIZE_CHECKBOX_REMOVE_PATTERNS,
	OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS,
	OPTIMIZE_CHECKBOX_REMOVE_SAMPLES,
	OPTIMIZE_STATICTEXT_REMOVE_PATTERNS,
	OPTIMIZE_STATICTEXT_REMOVE_INSTRUMENTS,
	OPTIMIZE_STATICTEXT_REMOVE_SAMPLES,

	OPTIMIZE_CHECKBOX_REARRANGE,
	OPTIMIZE_STATICTEXT_REARRANGE,

	OPTIMIZE_CHECKBOX_MINIMIZEALL,

	OPTIMIZE_CHECKBOX_CONVERTALL,

	OPTIMIZE_BUTTON_ANALYZE,
	OPTIMIZE_BUTTON_OPTIMIZE,
	
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_1xx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_2xx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_3xx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_4xx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_A56xx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_Hxx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_7xx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_E2x,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_EAx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_EBx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_Pxx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_Rxx,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_X1x,
	OPTIMIZE_CHECKBOX_OPERANDCONTROL_X2x,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_1xx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_2xx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_3xx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_4xx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_A56xx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_Hxx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_7xx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_E1x,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_E2x,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_EAx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_EBx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_Pxx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_Rxx,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_X1x,
	OPTIMIZE_STATICTEXT_OPERANDCONTROL_X2x,

	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_3xx,
	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_4xx,
	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_8xx,
	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Cxx,
	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Axx,
	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EAx,
	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EBx,
	OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Pxx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_3xx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_4xx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_8xx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_Cxx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_Axx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_EAx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_EBx,
	OPTIMIZE_STATICTEXT_EFFECTRELOCATION_Pxx,

	OPTIMIZE_BUTTON_ZERO_OPERANDS_TRACK,
	OPTIMIZE_BUTTON_ZERO_OPERANDS_PATTERN,
	OPTIMIZE_BUTTON_ZERO_OPERANDS_SONG,
	OPTIMIZE_BUTTON_ZERO_OPERANDS_BLOCK,

	OPTIMIZE_BUTTON_FILL_OPERANDS_TRACK,
	OPTIMIZE_BUTTON_FILL_OPERANDS_PATTERN,
	OPTIMIZE_BUTTON_FILL_OPERANDS_SONG,
	OPTIMIZE_BUTTON_FILL_OPERANDS_BLOCK,

	OPTIMIZE_BUTTON_RELOCATE_FX_TRACK,
	OPTIMIZE_BUTTON_RELOCATE_FX_PATTERN,
	OPTIMIZE_BUTTON_RELOCATE_FX_SONG,
	OPTIMIZE_BUTTON_RELOCATE_FX_BLOCK,

	OPTIMIZE_STATICTEXT_CRUNCHHEADER,
	OPTIMIZE_CHECKBOX_CRUNCHHEADER
};

void SectionOptimize::refresh()
{
	tracker.updateOrderlist(false);
	tracker.updateInstrumentsListBox(false);
	tracker.updateSamplesListBox(false);
	// update instrument/sample editor
	tracker.updateSampleEditorAndInstrumentSection(false);
	tracker.updateWindowTitle();
	
	tracker.screen->paint();
}

void SectionOptimize::optimize(bool evaluate/* = false*/)
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}

	PPContainer* container = static_cast<PPContainer*>(sectionContainer);

	dialog = new DialogListBox(tracker.screen, 
													 NULL, 
													 PP_DEFAULT_ID, 
													 "Result Log");

	PPListBox* listBox = static_cast<DialogListBox*>(dialog)->getListBox();
	listBox->setShowIndex(false);

	if (!evaluate)
		tracker.ensureSongStopped(false, true);

	bool remove = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE))->isChecked();
	bool removePatterns = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_PATTERNS))->isChecked() && remove;
	bool removeInstruments = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS))->isChecked() && remove;
	bool remapInstruments = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_REARRANGE))->isChecked() && remove;
	bool removeSamples = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_SAMPLES))->isChecked() && remove;
	bool convertSamples = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_CONVERTALL))->isChecked();
	bool minimizeSamples = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_MINIMIZEALL))->isChecked();

	char buffer[1024];

	tracker.signalWaitState(true);

	if (removePatterns)
	{
		pp_int32 res = tracker.moduleEditor->removeUnusedPatterns(evaluate);
		sprintf(buffer, "Unused patterns: %i", res);
		listBox->addItem(buffer);
	}
	if (removeInstruments)
	{
		pp_int32 res = tracker.moduleEditor->removeUnusedInstruments(evaluate, remapInstruments);
		sprintf(buffer, "Unused instruments: %i", res);
		listBox->addItem(buffer);
	}
	if (removeSamples)
	{
		pp_int32 res = tracker.moduleEditor->removeUnusedSamples(evaluate);
		sprintf(buffer, "Unused samples: %i", res);
		listBox->addItem(buffer);
	}

	if (convertSamples || minimizeSamples)
	{
		OptimizeSamplesResult result = optimizeSamples(convertSamples, minimizeSamples, evaluate);
		
		if (minimizeSamples)
		{
			sprintf(buffer, "Minimized samples: %i", result.numMinimizedSamples);
			listBox->addItem(buffer);
		}

		if (convertSamples)
		{
			sprintf(buffer, "Converted samples: %i", result.numConvertedSamples);
			listBox->addItem(buffer);
		}
	}

	// Update all panels like we have loaded a new file
	if (!evaluate)
		tracker.updateAfterLoad(true, false, false);

	tracker.signalWaitState(false);

	if (listBox->getNumItems())
	{
		dialog->show();
		if (!evaluate)
			refresh();
	}
	else
		tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Nothing to do.", Tracker::MessageBox_OK);		

	if (!evaluate)
		tracker.ensureSongPlaying(true);
}

PatternEditorTools::OperandOptimizeParameters SectionOptimize::getOptimizeParameters()
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);

	PatternEditorTools::OperandOptimizeParameters optimizeParameters;
	optimizeParameters.command_1xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_1xx))->isChecked();
	optimizeParameters.command_2xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_2xx))->isChecked();
	optimizeParameters.command_3xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_3xx))->isChecked();
	optimizeParameters.command_4xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_4xx))->isChecked();
	optimizeParameters.command_56Axx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_A56xx))->isChecked();
	optimizeParameters.command_7xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_7xx))->isChecked();
	optimizeParameters.command_E1x = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x))->isChecked();
	optimizeParameters.command_E2x = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_E2x))->isChecked();
	optimizeParameters.command_EAx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_EAx))->isChecked();
	optimizeParameters.command_EBx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_EBx))->isChecked();
	optimizeParameters.command_Hxx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Hxx))->isChecked();
	optimizeParameters.command_Pxx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Pxx))->isChecked();
	optimizeParameters.command_Rxx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Rxx))->isChecked();
	optimizeParameters.command_X1x = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_X1x))->isChecked();
	optimizeParameters.command_X2x = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_OPERANDCONTROL_X2x))->isChecked();

	return optimizeParameters;
}

void SectionOptimize::zeroOperandsTrack()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.getPatternEditor()->zeroOperandsTrack(optimizeParameters, false);
	refresh();
}

void SectionOptimize::zeroOperandsPattern()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.getPatternEditor()->zeroOperandsPattern(optimizeParameters, false);
	refresh();
}

void SectionOptimize::zeroOperandsSong()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.moduleEditor->zeroOperands(optimizeParameters, false);
	refresh();
}

void SectionOptimize::zeroOperandsBlock()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.getPatternEditor()->zeroOperandsSelection(optimizeParameters, false);
	refresh();
}

void SectionOptimize::fillOperandsTrack()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.getPatternEditor()->fillOperandsTrack(optimizeParameters, false);
	refresh();
}

void SectionOptimize::fillOperandsPattern()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.getPatternEditor()->fillOperandsPattern(optimizeParameters, false);
	refresh();
}

void SectionOptimize::fillOperandsSong()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.moduleEditor->fillOperands(optimizeParameters, false);
	refresh();
}

void SectionOptimize::fillOperandsBlock()
{
	PatternEditorTools::OperandOptimizeParameters optimizeParameters = getOptimizeParameters();
	tracker.getPatternEditor()->fillOperandsSelection(optimizeParameters, false);
	refresh();
}

PatternEditorTools::RelocateParameters SectionOptimize::getRelocateParameters()
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);

	PatternEditorTools::RelocateParameters relocateParameters;
	relocateParameters.command_3xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_3xx))->isChecked();
	relocateParameters.command_4xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_4xx))->isChecked();
	relocateParameters.command_8xx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_8xx))->isChecked();
	relocateParameters.command_Axx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Axx))->isChecked();
	relocateParameters.command_Cxx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Cxx))->isChecked();
	relocateParameters.command_EAx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EAx))->isChecked();
	relocateParameters.command_EBx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EBx))->isChecked();
	relocateParameters.command_Pxx = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Pxx))->isChecked();

	return relocateParameters;
}

void SectionOptimize::relocateCommandsTrack()
{
	PatternEditorTools::RelocateParameters relocateParameters = getRelocateParameters();
	tracker.getPatternEditor()->relocateCommandsTrack(relocateParameters, false);
	refresh();
}

void SectionOptimize::relocateCommandsPattern()
{
	PatternEditorTools::RelocateParameters relocateParameters = getRelocateParameters();
	tracker.getPatternEditor()->relocateCommandsPattern(relocateParameters, false);
	refresh();
}

void SectionOptimize::relocateCommandsSong()
{
	PatternEditorTools::RelocateParameters relocateParameters = getRelocateParameters();
	tracker.moduleEditor->relocateCommands(relocateParameters, false);
	refresh();
}

void SectionOptimize::relocateCommandsBlock()
{
	PatternEditorTools::RelocateParameters relocateParameters = getRelocateParameters();
	tracker.getPatternEditor()->relocateCommandsSelection(relocateParameters, false);
	refresh();
}

SectionOptimize::OptimizeSamplesResult SectionOptimize::optimizeSamples(bool convertTo8Bit, bool minimize, bool evaluate)
{
	OptimizeSamplesResult result;
	tracker.moduleEditor->optimizeSamples(convertTo8Bit, minimize, result.numConvertedSamples, result.numMinimizedSamples, evaluate);
	refresh();
	return result;
}

SectionOptimize::SectionOptimize(Tracker& theTracker) :
	SectionUpperLeft(theTracker)
{
}

SectionOptimize::~SectionOptimize()
{
}

pp_int32 SectionOptimize::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case OPTIMIZE_BUTTON_EXIT:
				show(false);
				break;
			
			case OPTIMIZE_CHECKBOX_REMOVE:
			case OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS:
			case OPTIMIZE_CHECKBOX_REMOVE_SAMPLES:
				update();
				break;

			case OPTIMIZE_BUTTON_OPTIMIZE:
				optimize();
				break;

			case OPTIMIZE_BUTTON_ANALYZE:
				optimize(true);
				break;

			case OPTIMIZE_BUTTON_ZERO_OPERANDS_TRACK:
				zeroOperandsTrack();
				break;
			case OPTIMIZE_BUTTON_ZERO_OPERANDS_PATTERN:
				zeroOperandsPattern();
				break;
			case OPTIMIZE_BUTTON_ZERO_OPERANDS_SONG:
				zeroOperandsSong();
				break;
			case OPTIMIZE_BUTTON_ZERO_OPERANDS_BLOCK:
				zeroOperandsBlock();
				break;

			case OPTIMIZE_BUTTON_FILL_OPERANDS_TRACK:
				fillOperandsTrack();
				break;
			case OPTIMIZE_BUTTON_FILL_OPERANDS_PATTERN:
				fillOperandsPattern();
				break;
			case OPTIMIZE_BUTTON_FILL_OPERANDS_SONG:
				fillOperandsSong();
				break;
			case OPTIMIZE_BUTTON_FILL_OPERANDS_BLOCK:
				fillOperandsBlock();
				break;

			case OPTIMIZE_BUTTON_RELOCATE_FX_TRACK:
				relocateCommandsTrack();
				break;
			case OPTIMIZE_BUTTON_RELOCATE_FX_PATTERN:
				relocateCommandsPattern();
				break;
			case OPTIMIZE_BUTTON_RELOCATE_FX_SONG:
				relocateCommandsSong();
				break;
			case OPTIMIZE_BUTTON_RELOCATE_FX_BLOCK:
				relocateCommandsBlock();
				break;
		}
		
	}

	return 0;
}

void SectionOptimize::init(pp_int32 px, pp_int32 py)
{
	PPScreen* screen = tracker.screen;

	PPContainer* container = new PPContainer(CONTAINER_OPTIMIZE, tracker.screen, this, PPPoint(px, py), PPSize(320,UPPERLEFTSECTIONHEIGHT), false);
	container->setColor(TrackerConfig::colorThemeMain);	
	tracker.screen->addControl(container);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(px + 2, py + 2), "Optimize", true, true));

	PPSize size = container->getSize();

	pp_int32 buttonWidth = 8*4+4;
	pp_int32 buttonHeight = 11;
	
	pp_int32 x = px+container->getSize().width-(buttonWidth+4);
	pp_int32 y = py+container->getSize().height-(buttonHeight+4);

	pp_int32 x2, y2;

	container->addControl(new PPSeperator(0, screen, PPPoint(x - 6, y - 4), 4 + buttonHeight + 3, TrackerConfig::colorThemeMain, false));
	container->addControl(new PPSeperator(0, screen, PPPoint(px + 2, y - 4), container->getSize().width - 4, TrackerConfig::colorThemeMain, true));

	pp_int32 space;

	PPButton* button = new PPButton(OPTIMIZE_BUTTON_EXIT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Exit");
	container->addControl(button);

	y+=2;
	x = px+4;
	PPStaticText* staticText = new PPStaticText(OPTIMIZE_STATICTEXT_CRUNCHHEADER, NULL, NULL, PPPoint(x, y), "Crunch headers (XM format only)", true);
	staticText->enable(false);
	container->addControl(staticText);
	PPCheckBox* checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_CRUNCHHEADER, screen, this, PPPoint(x + 32*8, y-1));
	checkBox->checkIt(false);
	checkBox->enable(false);
	container->addControl(checkBox);

	// ----------------------------- "remove"
	space = 5*8;
	x = px+4;
	y = py+4+11;	

	pp_int32 seperatorWidth = (x + 14*8+2) - (px+4) + 4 + 2;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x, y), "Remove unused", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_REMOVE, screen, this, PPPoint(x + 13*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	y+=12;

	x = px+4;
	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_REMOVE_PATTERNS, NULL, NULL, PPPoint(x, y), "Pat", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_REMOVE_PATTERNS, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_REMOVE_INSTRUMENTS, NULL, NULL, PPPoint(x, y), "Ins", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_REMOVE_SAMPLES, NULL, NULL, PPPoint(x, y), "Smp", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_REMOVE_SAMPLES, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	y+=12;
	x = px+4;
	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_REARRANGE, NULL, NULL, PPPoint(x, y), "Remap instr.", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_REARRANGE, screen, this, PPPoint(x + 13*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	// ----------------------------- "seperator"
	y+=12;

	container->addControl(new PPSeperator(0, screen, PPPoint(px + 2, y), seperatorWidth, TrackerConfig::colorThemeMain, true));
	y+=4;

	// ----------------------------- "minimize all samples"
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x, y), "Min. all smp.", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_MINIMIZEALL, screen, this, PPPoint(x + 13*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	// ----------------------------- "samples to 8 bit"
	y+=12;
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x, y), "Smp. to 8 bit", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_CONVERTALL, screen, this, PPPoint(x + 13*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	y+=12;
	container->addControl(new PPSeperator(0, screen, PPPoint(x-1, y), seperatorWidth, TrackerConfig::colorThemeMain, true));
	y+=4;

	buttonWidth = 8*7 + 1;
	buttonHeight = 14;

	button = new PPButton(OPTIMIZE_BUTTON_ANALYZE, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Analyze");
	container->addControl(button);
	x+=buttonWidth+2;

	button = new PPButton(OPTIMIZE_BUTTON_OPTIMIZE, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Optimize");
	container->addControl(button);

	// ----------------------------- "operand control"
	y = py+4+11 - 12;	
	x = x2 = px + 4 + seperatorWidth + 1;
	space = 5*8-1;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_1xx, NULL, NULL, PPPoint(x, y), "1xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_1xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_2xx, NULL, NULL, PPPoint(x, y), "2xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_2xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_3xx, NULL, NULL, PPPoint(x, y), "3xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_3xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_4xx, NULL, NULL, PPPoint(x, y), "4xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_4xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_7xx, NULL, NULL, PPPoint(x, y), "7xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_7xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	
	y+=12;
	x = x2;
	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_A56xx, NULL, NULL, PPPoint(x+4, y), "A/5/6xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_A56xx, screen, this, PPPoint(x + 8*8+1, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space*2;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_Hxx, NULL, NULL, PPPoint(x, y), "Hxx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Hxx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_Pxx, NULL, NULL, PPPoint(x, y), "Pxx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Pxx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_Rxx, NULL, NULL, PPPoint(x, y), "Rxx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Rxx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	y+=12;
	x = x2;
	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_E1x, NULL, NULL, PPPoint(x, y), "E1x", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_E2x, NULL, NULL, PPPoint(x, y), "E2x", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_E2x, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_EAx, NULL, NULL, PPPoint(x, y), "EAx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_EAx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_EBx, NULL, NULL, PPPoint(x, y), "EBx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_EBx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_X1x, NULL, NULL, PPPoint(x, y), "X1x", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_X1x, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	y+=12;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_OPERANDCONTROL_X2x, NULL, NULL, PPPoint(x, y), "X2x", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_OPERANDCONTROL_X2x, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	y+=1;
	x = x2+1;
	buttonWidth = container->getSize().width/10-5;
	buttonHeight = 8;
	
	staticText = new PPStaticText(0, NULL, NULL, PPPoint(x-1, y+1), "Zero ops", true);
	staticText->setFont(PPFont::getFont(PPFont::FONT_TINY));
	container->addControl(staticText);

	x+=8*5+1;
	button = new PPButton(OPTIMIZE_BUTTON_ZERO_OPERANDS_TRACK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Track");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_ZERO_OPERANDS_PATTERN, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Pat.");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_ZERO_OPERANDS_SONG, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Song");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_ZERO_OPERANDS_BLOCK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Block");
	container->addControl(button);
	
	y+=buttonHeight+1;
	x = x2+1;
		
	staticText = new PPStaticText(0, NULL, NULL, PPPoint(x-1, y+1), "Fill ops", true);
	staticText->setFont(PPFont::getFont(PPFont::FONT_TINY));
	container->addControl(staticText);

	x+=8*5+1;
	button = new PPButton(OPTIMIZE_BUTTON_FILL_OPERANDS_TRACK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Track");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_FILL_OPERANDS_PATTERN, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Pat.");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_FILL_OPERANDS_SONG, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Song");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_FILL_OPERANDS_BLOCK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Block");
	container->addControl(button);

	y+=buttonHeight+2;
	x = x2;
	container->addControl(new PPSeperator(0, screen, PPPoint(x-2, y), container->getSize().width - seperatorWidth - 5, TrackerConfig::colorThemeMain, true));
	y+=4;

	// ----------------------------- "effect relocation"
	x = x2;
	space = 5*8-1;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_3xx, NULL, NULL, PPPoint(x, y), "3xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_3xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_4xx, NULL, NULL, PPPoint(x, y), "4xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_4xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_8xx, NULL, NULL, PPPoint(x, y), "8xx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_8xx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_Axx, NULL, NULL, PPPoint(x, y), "Axx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Axx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_Cxx, NULL, NULL, PPPoint(x, y), "Cxx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Cxx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	x = x2;
	y+=12;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_EAx, NULL, NULL, PPPoint(x, y), "EAx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EAx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_EBx, NULL, NULL, PPPoint(x, y), "EBx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EBx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);
	x+=space;

	container->addControl(new PPStaticText(OPTIMIZE_STATICTEXT_EFFECTRELOCATION_Pxx, NULL, NULL, PPPoint(x, y), "Pxx", true));
	checkBox = new PPCheckBox(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Pxx, screen, this, PPPoint(x + 3*8+2, y));
	checkBox->checkIt(true);
	container->addControl(checkBox);

	y+=12;
	x = x2;

	// ----------------------------- buttons for fx relocation
	y++;
	x = x2+1;

	buttonHeight = 9;
	buttonWidth = container->getSize().width/8-7;

	staticText = new PPStaticText(0, NULL, NULL, PPPoint(x-1, y+2), "Relocate FX", true);
	staticText->setFont(PPFont::getFont(PPFont::FONT_TINY));
	container->addControl(staticText);

	x+=11*5+1;
	button = new PPButton(OPTIMIZE_BUTTON_RELOCATE_FX_TRACK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Track");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_RELOCATE_FX_PATTERN, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Pat.");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_RELOCATE_FX_SONG, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Song");
	container->addControl(button);
	x+=buttonWidth+1;

	button = new PPButton(OPTIMIZE_BUTTON_RELOCATE_FX_BLOCK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Block");
	container->addControl(button);

	// ----------------------------- vertical seperator
	x2 = px + 2 + seperatorWidth;
	y2 = py + 2;
	container->addControl(new PPSeperator(0, screen, PPPoint(x2, y2 - 2), container->getLocation().y + container->getSize().height - y2 - 17, TrackerConfig::colorThemeMain, false));

	sectionContainer = container;

	initialised = true;

	showSection(false);
}

void SectionOptimize::update(bool repaint/* = true*/)
{
	PPScreen* screen = tracker.screen;

	PPContainer* container = static_cast<PPContainer*>(sectionContainer);

	PPCheckBox* checkBox = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE));

	if (checkBox->isChecked())
	{
		container->getControlByID(OPTIMIZE_STATICTEXT_REMOVE_PATTERNS)->enable(true);
		container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_PATTERNS)->enable(true);

		container->getControlByID(OPTIMIZE_STATICTEXT_REMOVE_INSTRUMENTS)->enable(true);
		container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS)->enable(true);

		container->getControlByID(OPTIMIZE_STATICTEXT_REMOVE_SAMPLES)->enable(true);
		container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_SAMPLES)->enable(true);
	
		bool b = static_cast<PPCheckBox*>(container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS))->isChecked();

		container->getControlByID(OPTIMIZE_STATICTEXT_REARRANGE)->enable(b);
		container->getControlByID(OPTIMIZE_CHECKBOX_REARRANGE)->enable(b);
	}
	else
	{
		container->getControlByID(OPTIMIZE_STATICTEXT_REMOVE_PATTERNS)->enable(false);
		container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_PATTERNS)->enable(false);

		container->getControlByID(OPTIMIZE_STATICTEXT_REMOVE_INSTRUMENTS)->enable(false);
		container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS)->enable(false);

		container->getControlByID(OPTIMIZE_STATICTEXT_REMOVE_SAMPLES)->enable(false);
		container->getControlByID(OPTIMIZE_CHECKBOX_REMOVE_SAMPLES)->enable(false);
		
		container->getControlByID(OPTIMIZE_STATICTEXT_REARRANGE)->enable(false);
		container->getControlByID(OPTIMIZE_CHECKBOX_REARRANGE)->enable(false);
	}


	screen->paintControl(container, repaint);
}

pp_uint32 SectionOptimize::getNumFlagGroups()
{
	return 2;
}

pp_uint32 SectionOptimize::getDefaultFlags(pp_uint32 groupIndex)
{
	pp_uint32 value = 0;

	switch (groupIndex)
	{
		case 0:
			value = (1 << BITPOS_CHECKBOX_REMOVE) |
				(1 << BITPOS_CHECKBOX_REMOVE_PATTERNS) |
				(1 << BITPOS_CHECKBOX_REMOVE_INSTRUMENTS) |
				(1 << BITPOS_CHECKBOX_REMOVE_SAMPLES) |
				(1 << BITPOS_CHECKBOX_REARRANGE) |
				(1 << BITPOS_CHECKBOX_MINIMIZEALL) |
				(1 << BITPOS_CHECKBOX_CONVERTALL) |
				(1 << BITPOS_CHECKBOX_CRUNCHHEADER);
			break;

		case 1:
			value = (1 << BITPOS_CHECKBOX_OPERANDCONTROL_1xx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_2xx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_3xx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_4xx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_A56xx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_Hxx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_7xx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_E1x) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_E2x) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_EAx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_EBx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_Pxx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_Rxx) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_X1x) |
				(1 << BITPOS_CHECKBOX_OPERANDCONTROL_X2x) |

				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_3xx) |
				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_4xx) |
				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_8xx) |
				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_Cxx) |
				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_Axx) |
				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_EAx) |
				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_EBx) |
				(1 << BITPOS_CHECKBOX_EFFECTRELOCATION_Pxx);
			break;
	}

	return value;
}

#define BITFROMCHECKBOX(ID, POS) \
	(static_cast<PPCheckBox*>(container->getControlByID((ID)))->isChecked() ? 1 : 0) << (POS)

pp_uint32 SectionOptimize::getOptimizeCheckBoxFlags(pp_uint32 groupIndex)
{
	pp_uint32 value = 0;

	PPContainer* container = static_cast<PPContainer*>(sectionContainer);

	switch (groupIndex)
	{
		case 0:
		{
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_REMOVE, BITPOS_CHECKBOX_REMOVE);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_REMOVE_PATTERNS, BITPOS_CHECKBOX_REMOVE_PATTERNS);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS, BITPOS_CHECKBOX_REMOVE_INSTRUMENTS);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_REMOVE_SAMPLES, BITPOS_CHECKBOX_REMOVE_SAMPLES);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_REARRANGE, BITPOS_CHECKBOX_REARRANGE);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_MINIMIZEALL, BITPOS_CHECKBOX_MINIMIZEALL);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_CONVERTALL, BITPOS_CHECKBOX_CONVERTALL);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_CRUNCHHEADER, BITPOS_CHECKBOX_CRUNCHHEADER);
			break;
		}

		case 1:
		{
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_1xx, BITPOS_CHECKBOX_OPERANDCONTROL_1xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_2xx, BITPOS_CHECKBOX_OPERANDCONTROL_2xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_3xx, BITPOS_CHECKBOX_OPERANDCONTROL_3xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_4xx, BITPOS_CHECKBOX_OPERANDCONTROL_4xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_A56xx, BITPOS_CHECKBOX_OPERANDCONTROL_A56xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Hxx, BITPOS_CHECKBOX_OPERANDCONTROL_Hxx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_7xx, BITPOS_CHECKBOX_OPERANDCONTROL_7xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x, BITPOS_CHECKBOX_OPERANDCONTROL_E1x);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_E2x, BITPOS_CHECKBOX_OPERANDCONTROL_E2x);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_EAx, BITPOS_CHECKBOX_OPERANDCONTROL_EAx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_EBx, BITPOS_CHECKBOX_OPERANDCONTROL_EBx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Pxx, BITPOS_CHECKBOX_OPERANDCONTROL_Pxx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_Rxx, BITPOS_CHECKBOX_OPERANDCONTROL_Rxx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_X1x, BITPOS_CHECKBOX_OPERANDCONTROL_X1x);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_OPERANDCONTROL_X2x, BITPOS_CHECKBOX_OPERANDCONTROL_X2x);

			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_3xx, BITPOS_CHECKBOX_EFFECTRELOCATION_3xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_4xx, BITPOS_CHECKBOX_EFFECTRELOCATION_4xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_8xx, BITPOS_CHECKBOX_EFFECTRELOCATION_8xx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Cxx, BITPOS_CHECKBOX_EFFECTRELOCATION_Cxx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Axx, BITPOS_CHECKBOX_EFFECTRELOCATION_Axx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EAx, BITPOS_CHECKBOX_EFFECTRELOCATION_EAx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EBx, BITPOS_CHECKBOX_EFFECTRELOCATION_EBx);
			value |= BITFROMCHECKBOX(OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Pxx, BITPOS_CHECKBOX_EFFECTRELOCATION_Pxx);
			break;
		}
	}

	return value;
}

#define BITTOCHECKBOX(VAL, ID, POS) \
	static_cast<PPCheckBox*>(container->getControlByID((ID)))->checkIt((((VAL) >> POS) & 1) != 0)

void SectionOptimize::setOptimizeCheckBoxFlags(pp_uint32 groupIndex, pp_uint32 flags)
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);

	switch (groupIndex)
	{
		case 0:
		{
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_REMOVE, BITPOS_CHECKBOX_REMOVE);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_REMOVE_PATTERNS, BITPOS_CHECKBOX_REMOVE_PATTERNS);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_REMOVE_INSTRUMENTS, BITPOS_CHECKBOX_REMOVE_INSTRUMENTS);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_REMOVE_SAMPLES, BITPOS_CHECKBOX_REMOVE_SAMPLES);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_REARRANGE, BITPOS_CHECKBOX_REARRANGE);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_MINIMIZEALL, BITPOS_CHECKBOX_MINIMIZEALL);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_CONVERTALL, BITPOS_CHECKBOX_CONVERTALL);
			//BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_CRUNCHHEADER, BITPOS_CHECKBOX_CRUNCHHEADER);
			break;
		}

		case 1:
		{
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_1xx, BITPOS_CHECKBOX_OPERANDCONTROL_1xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_2xx, BITPOS_CHECKBOX_OPERANDCONTROL_2xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_3xx, BITPOS_CHECKBOX_OPERANDCONTROL_3xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_4xx, BITPOS_CHECKBOX_OPERANDCONTROL_4xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_A56xx, BITPOS_CHECKBOX_OPERANDCONTROL_A56xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_Hxx, BITPOS_CHECKBOX_OPERANDCONTROL_Hxx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_7xx, BITPOS_CHECKBOX_OPERANDCONTROL_7xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x, BITPOS_CHECKBOX_OPERANDCONTROL_E1x);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x, BITPOS_CHECKBOX_OPERANDCONTROL_E1x);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x, BITPOS_CHECKBOX_OPERANDCONTROL_E1x);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_E1x, BITPOS_CHECKBOX_OPERANDCONTROL_E1x);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_E2x, BITPOS_CHECKBOX_OPERANDCONTROL_E2x);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_EAx, BITPOS_CHECKBOX_OPERANDCONTROL_EAx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_EBx, BITPOS_CHECKBOX_OPERANDCONTROL_EBx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_Pxx, BITPOS_CHECKBOX_OPERANDCONTROL_Pxx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_Rxx, BITPOS_CHECKBOX_OPERANDCONTROL_Rxx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_X1x, BITPOS_CHECKBOX_OPERANDCONTROL_X1x);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_OPERANDCONTROL_X2x, BITPOS_CHECKBOX_OPERANDCONTROL_X2x);

			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_3xx, BITPOS_CHECKBOX_EFFECTRELOCATION_3xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_4xx, BITPOS_CHECKBOX_EFFECTRELOCATION_4xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_8xx, BITPOS_CHECKBOX_EFFECTRELOCATION_8xx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Cxx, BITPOS_CHECKBOX_EFFECTRELOCATION_Cxx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Axx, BITPOS_CHECKBOX_EFFECTRELOCATION_Axx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EAx, BITPOS_CHECKBOX_EFFECTRELOCATION_EAx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_EBx, BITPOS_CHECKBOX_EFFECTRELOCATION_EBx);
			BITTOCHECKBOX(flags, OPTIMIZE_CHECKBOX_EFFECTRELOCATION_Pxx, BITPOS_CHECKBOX_EFFECTRELOCATION_Pxx);
			break;
		}
	}

	update(false);
}
