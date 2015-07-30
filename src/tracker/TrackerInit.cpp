/*
 *  tracker/TrackerInit.cpp
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
#include "TrackerConfig.h"
#include "TabManager.h"
#include "PlayerController.h"
#include "ModuleEditor.h"
#include "PatternTools.h"
#include "Tools.h"

#include "PPUIConfig.h"
#include "ListBox.h"
#include "StaticText.h"
#include "Seperator.h"
#include "MessageBoxContainer.h"
#include "PatternEditorControl.h"
#include "TransparentContainer.h"
#include "PianoControl.h"
#include "PeakLevelControl.h"
#include "ScopesControl.h"
#include "TabHeaderControl.h"
#include "TitlePageManager.h"

// Sections
#include "SectionSwitcher.h"
#include "SectionTranspose.h"
#include "SectionAdvancedEdit.h"
#include "SectionDiskMenu.h"
#include "SectionHDRecorder.h"
#include "SectionSettings.h"
#include "SectionInstruments.h"
#include "SectionSamples.h"
#include "SectionQuickOptions.h"
#include "SectionOptimize.h"
#include "SectionAbout.h"

#include "InputControlListener.h"

#include "ControlIDs.h"
#include "SIPButtons.h"

void Tracker::initUI()
{
	pp_int32 c;
	PPButton* button = NULL;
	
	// ---------- initialise sections --------
	for (pp_int32 i = 0; i < sections->size(); i++)
		sections->get(i)->init();

#ifdef __LOWRES__
	pp_int32 height2 = screen->getHeight()-UPPERSECTIONDEFAULTHEIGHT();

	PPContainer* containerAbout = new PPContainer(CONTAINER_ABOUT, screen, this, PPPoint(116-2, height2), PPSize((306-116+2)+14,24), false);
	containerAbout->setColor(TrackerConfig::colorThemeMain);

	// Song title edit field
	PPListBox* listBox = new PPListBox(LISTBOX_SONGTITLE, screen, this, PPPoint(116-2+2, height2+2+8), PPSize(200+2,12), true, true, false);
	listBox->showSelection(false);
	listBox->setSingleButtonClickEdit(true);
	listBox->setBorderColor(TrackerConfig::colorThemeMain);

	char str[MP_MAXTEXT+1];
	moduleEditor->getTitle(str, ModuleEditor::MAX_TITLETEXT);

	listBox->addItem(str);
	listBox->setMaxEditSize(ModuleEditor::MAX_TITLETEXT);

	containerAbout->addControl(listBox);

	PPStaticText* staticText = playTimeText = new PPStaticText(STATICTEXT_ABOUT_TIME, screen, this, PPPoint(116+2, height2+2+8+3), "", false);
	containerAbout->addControl(staticText);
	
	button = new PPButton(BUTTON_ABOUT_ESTIMATESONGLENGTH, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - 6*8-4, height2+2+8+2), PPSize(6*8, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Estimate");
	containerAbout->addControl(button);

	peakLevelControl = new PeakLevelControl(PEAKLEVEL_CONTROL, screen, this, PPPoint(116-2+2, height2+10), PPSize(200+2,12));
	peakLevelControl->setBorderColor(TrackerConfig::colorThemeMain);
	peakLevelControl->hide(true);
	containerAbout->addControl(peakLevelControl);

	// switch to Peak level
	pp_int32 aboutButtonOffset = -33 - 30 - 30 - 23*3 - 14;

	button = new PPButton(BUTTON_ABOUT_SHOWTITLE, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset, height2+1), PPSize(29, 9), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Title");
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	containerAbout->addControl(button);

	button = new PPButton(BUTTON_ABOUT_SHOWTIME, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset + 29, height2+1), PPSize(24, 9), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Time");
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	containerAbout->addControl(button);
	
	button = new PPButton(BUTTON_ABOUT_SHOWPEAK, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset + 29+24, height2+1), PPSize(24, 9), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Peak");
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	containerAbout->addControl(button);

	button = new PPButton(MAINMENU_PLAY_SONG, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset + (29+24+24) + 2, height2+1), PPSize(23, 9), false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Play");
	containerAbout->addControl(button);

	button = new PPButton(MAINMENU_PLAY_PATTERN, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset + (29+24+24) + 2 + 23, height2+1), PPSize(23, 9), false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Pat");
	containerAbout->addControl(button);

	button = new PPButton(MAINMENU_STOP, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset + (29+24+24) + 2 + 23*2, height2+1), PPSize(23, 9), false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Stop");
	containerAbout->addControl(button);

	button = new PPButton(BUTTON_ABOUT_FOLLOWSONG, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset + (29+24+24) + 2 + 23*3, height2+1), PPSize(12, 9), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("F");
	containerAbout->addControl(button);

	button = new PPButton(BUTTON_ABOUT_LIVESWITCH, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width + aboutButtonOffset + (29+24+24) + 2 + 23*3 + 12, height2+1), PPSize(12, 9), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("L");
	containerAbout->addControl(button);
	
	staticText = new PPStaticText(STATICTEXT_ABOUT_HEADING, screen, this, PPPoint(116, height2+3), "Title:", true);
	staticText->setFont(PPFont::getFont(PPFont::FONT_TINY));
	containerAbout->addControl(staticText);

	screen->addControl(containerAbout);

	// small sections
	initSectionOrderlist(0,0+height2);

	initSectionSpeed(116-2,24+height2);

	initSectionPattern(116-4+99,24+height2);

	// Main options
	initSectionMainOptions(0, 0+height2);
	initListboxesSection(0, 0+height2);

	// add scopes (hidden by default)
	scopesControl = new ScopesControl(SCOPES_CONTROL, screen, this, 
									  PPPoint(0, 0 + height2), 
									  PPSize(SCOPESWIDTH(), SCOPESHEIGHT()));
	scopesControl->setBorderColor(TrackerConfig::colorThemeMain);
	scopesControl->show(false);
	screen->addControl(scopesControl);

	// add sub menu selection buttons (pages)
	{
		PPButton* button;
	
		PPContainer* container = new PPContainer(CONTAINER_SCOPECONTROL, screen, this, PPPoint(scopesControl->getSize().width, 0 + height2), PPSize(screen->getWidth()-scopesControl->getSize().width,SCOPESHEIGHT()), false);
		container->setColor(TrackerConfig::colorThemeMain);
		
		pp_int32 dy = (container->getSize().height-2) / 5;
		button = new PPButton(BUTTON_SCOPECONTROL_MUTE, screen, this, PPPoint(container->getLocation().x + 1, container->getLocation().y + 2), PPSize(container->getSize().width-3, dy), false, true, false);
		button->setText("Mute");
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		container->addControl(button);
		
		button = new PPButton(BUTTON_SCOPECONTROL_SOLO, screen, this, PPPoint(container->getLocation().x + 1, container->getLocation().y + 2 + dy), PPSize(container->getSize().width-3, dy), false, true, false);
		button->setText("Solo");
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		container->addControl(button);

		button = new PPButton(BUTTON_SCOPECONTROL_REC, screen, this, PPPoint(container->getLocation().x + 1, container->getLocation().y + 2 + dy*2), PPSize(container->getSize().width-3, dy), false, true, false);
		button->setText("Rec.");
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		container->addControl(button);

		button = new PPButton(MAINMENU_PLAY_SONG, screen, this, PPPoint(container->getLocation().x + 1, container->getLocation().y + 2 + dy*3), PPSize(container->getSize().width-3, dy), false);
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Play");
		container->addControl(button);

		button = new PPButton(MAINMENU_STOP, screen, this, PPPoint(container->getLocation().x + 1, container->getLocation().y + 2 + dy*4), PPSize(container->getSize().width-3, dy), false);
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Stop");
		container->addControl(button);

		container->show(false);
		screen->addControl(container);

		pp_int32 y = height2 + 64;

		container = new PPContainer(CONTAINER_LOWRES_MENUSWITCH, screen, this, PPPoint(0, y), PPSize(screen->getWidth(),16), false);
		container->setColor(TrackerConfig::colorThemeMain);
		
		const pp_int32 numSubMenus = NUMSUBMENUS();
		
		const char* subMenuTexts[] = {"Main","Song","Ins.","Scopes","Jam"};	
		const pp_int32 subMenuIDs[] = {BUTTON_0, BUTTON_0+1, BUTTON_0+2, BUTTON_0+3, BUTTON_0+4};

		pp_int32 dx = (screen->getWidth() - (4+38))/numSubMenus;
		
		for (pp_int32 i = 0; i < numSubMenus; i++)
		{
			button = new PPButton(subMenuIDs[i], screen, this, PPPoint(0 + 2+i*dx, y+1), PPSize(dx, 13), false, true, false);
			button->setColor(TrackerConfig::colorThemeMain);
			button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
			button->setText(subMenuTexts[i]);
			container->addControl(button);	
		}
		
		button = new PPButton(BUTTON_APP_EXIT, screen, this, PPPoint(0 + 6+numSubMenus*dx, y+2), PPSize(35,11));
		button->setText("Exit");
		container->addControl(button);
		
		screen->addControl(container);
	}

	sectionSwitcher->updateSubMenusButtons(false);
	sectionSwitcher->showCurrentSubMenu(false);

	pp_int32 peCtrlHeight = screen->getHeight()-UPPERSECTIONDEFAULTHEIGHT()-INPUTCONTAINERHEIGHT_DEFAULT;
	
	patternEditorControl = new PatternEditorControl(PATTERN_EDITOR, screen, this, 
													PPPoint(0,0), 
													PPSize(screen->getWidth(),peCtrlHeight));

	initInputContainerDefault(0, screen->getHeight()-UPPERSECTIONDEFAULTHEIGHT()-INPUTCONTAINERHEIGHT_DEFAULT);
	initInputContainerExtended(0, screen->getHeight()-UPPERSECTIONDEFAULTHEIGHT()-INPUTCONTAINERHEIGHT_EXTENDED);
#else
	PPContainer* containerAbout = new PPContainer(CONTAINER_ABOUT, screen, this, PPPoint(116-2, 0), PPSize((306-116+2)+14,24), false);
	containerAbout->setColor(TrackerConfig::colorThemeMain);

	// Song title edit field
	PPListBox* listBox = new PPListBox(LISTBOX_SONGTITLE, screen, this, PPPoint(116-2+2, 2+8), PPSize(200+2,12), true, true, false);
	listBox->showSelection(false);
	listBox->setSingleButtonClickEdit(true);
	listBox->setBorderColor(TrackerConfig::colorThemeMain);

	char str[MP_MAXTEXT+1];
	moduleEditor->getTitle(str, ModuleEditor::MAX_TITLETEXT);

	listBox->addItem(str);
	listBox->setMaxEditSize(ModuleEditor::MAX_TITLETEXT);

	containerAbout->addControl(listBox);
	
	PPStaticText* staticText = playTimeText = new PPStaticText(STATICTEXT_ABOUT_TIME, screen, this, PPPoint(116+2, 2+8+3), "", false);
	containerAbout->addControl(staticText);

	button = new PPButton(BUTTON_ABOUT_ESTIMATESONGLENGTH, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - 6*8-4, 2+8+2), PPSize(6*8, 9));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("estimate");
	containerAbout->addControl(button);

	peakLevelControl = new PeakLevelControl(PEAKLEVEL_CONTROL, screen, this, PPPoint(116-2+2, 2+8), PPSize(200+2,12));
	peakLevelControl->setBorderColor(TrackerConfig::colorThemeMain);
	containerAbout->addControl(peakLevelControl);

	staticText = new PPStaticText(STATICTEXT_ABOUT_HEADING, screen, this, PPPoint(116, 3), "Song title:", true);
	staticText->setFont(PPFont::getFont(PPFont::FONT_TINY));
	containerAbout->addControl(staticText);

	// switch to Peak level
	pp_int32 aboutButtonOffset = 51;

	button = new PPButton(BUTTON_ABOUT_FOLLOWSONG, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - aboutButtonOffset, 1), PPSize(12, 9), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("F");
	containerAbout->addControl(button);

	button = new PPButton(BUTTON_ABOUT_PROSPECTIVE, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - aboutButtonOffset + 12, 1), PPSize(12, 9), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("P");
	button->setPressed(true);
	containerAbout->addControl(button);

	button = new PPButton(BUTTON_ABOUT_WRAPCURSOR, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - aboutButtonOffset + 12*2, 1), PPSize(12, 9), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("W");
	containerAbout->addControl(button);

	button = new PPButton(BUTTON_ABOUT_LIVESWITCH, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - aboutButtonOffset + 12*3, 1), PPSize(12, 9), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("L");
	containerAbout->addControl(button);

	aboutButtonOffset+=34;

	button = new PPButton(BUTTON_ABOUT_SHOWPEAK, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - aboutButtonOffset, 1), PPSize(30, 9), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Peak");
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	containerAbout->addControl(button);

	aboutButtonOffset+=30;

	button = new PPButton(BUTTON_ABOUT_SHOWTIME, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - aboutButtonOffset, 1), PPSize(30, 9), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Time");
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	containerAbout->addControl(button);

	aboutButtonOffset+=30;

	button = new PPButton(BUTTON_ABOUT_SHOWTITLE, screen, this, PPPoint(containerAbout->getLocation().x + containerAbout->getSize().width - aboutButtonOffset, 1), PPSize(30, 9), false, true, false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Title");
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	containerAbout->addControl(button);

	screen->addControl(containerAbout);

	// small sections
	initSectionOrderlist(0,0);

	initSectionSpeed(116-2,24);

	initSectionPattern(116-4+99,24);

	// Main options
	initSectionMainOptions(0, 64);

	// ---------- Instrument & Sample listboxes ---------- 
	initListboxesSection(320, 0);
	
#ifndef __LOWRES__
	scopesControl = new ScopesControl(SCOPES_CONTROL, screen, this, 
									  PPPoint(0, UPPERSECTIONDEFAULTHEIGHTWOINS()), 
									  PPSize(screen->getWidth(), SCOPESHEIGHT()));
	scopesControl->setBorderColor(TrackerConfig::colorThemeMain);
	for (c = 0; c < TrackerConfig::MAXCHANNELS; c++)
		scopesControl->recordChannel(c, playerController->isChannelRecording(c));
	screen->addControl(scopesControl);
	screen->paintControl(scopesControl, false);

	PPContainer* containerOpenRemoveTabs = new PPContainer(CONTAINER_OPENREMOVETABS, screen, this, 
														   PPPoint(0, screen->getHeight()-TABHEADERHEIGHT()),
														   PPSize(TABHEADERHEIGHT()*2, TABHEADERHEIGHT()), 
														   false);
	containerOpenRemoveTabs->setColor(TrackerConfig::colorThemeMain);

	button = new PPButton(BUTTON_TAB_OPEN, screen, this, 
						  PPPoint(1, screen->getHeight()-TABHEADERHEIGHT()+1), 
						  PPSize(TABHEADERHEIGHT()-2, TABHEADERHEIGHT()-2), 
						  false);
	button->setText("+");
	containerOpenRemoveTabs->addControl(button);

	button = new PPButton(BUTTON_TAB_CLOSE, screen, this, 
						  PPPoint(TABHEADERHEIGHT(), screen->getHeight()-TABHEADERHEIGHT()+1), 
						  PPSize(TABHEADERHEIGHT()-2, TABHEADERHEIGHT()-2), 
						  false);
	button->setText("-");
	containerOpenRemoveTabs->addControl(button);
	screen->addControl(containerOpenRemoveTabs);
	
	TabHeaderControl* tabHeader = new TabHeaderControl(TABHEADER_CONTROL, screen, this, 
													   PPPoint(TABHEADERHEIGHT()*2, screen->getHeight() - TABHEADERHEIGHT()), 
													   PPSize(screen->getWidth()-TABHEADERHEIGHT()*2, TABHEADERHEIGHT()));
	tabHeader->setColor(TrackerConfig::colorThemeMain);
	screen->addControl(tabHeader);
	
#endif

	patternEditorControl = new PatternEditorControl(PATTERN_EDITOR, screen, this, 
													PPPoint(0,UPPERSECTIONDEFAULTHEIGHT()), 
													PPSize(screen->getWidth(),MAXEDITORHEIGHT()-UPPERSECTIONDEFAULTHEIGHT()));
#endif
	// first thing to do is, attach pattern editor
	patternEditorControl->attachPatternEditor(moduleEditor->getPatternEditor());
	patternEditorControl->setColor(TrackerConfig::colorPatternEditorBackground);
	patternEditorControl->setOrderlistIndex(getOrderListBoxIndex());
	
	screen->addControl(patternEditorControl);

	// ---------- update fields --------------
	updateSamplesListBox(false);
	updateSongLength(false);
	updateSongRepeat(false);

	//updateBPM(false);
	updateSpeed(false);                                                                      
	updatePatternAddAndOctave(false);

	updatePatternIndex(false);
	updatePatternLength(false);

	updatePatternEditorControl(false);

#ifdef __LOWRES__
	switchEditMode(EditModeMilkyTracker);
#else
	switchEditMode(EditModeFastTracker);
#endif

	screen->setFocus(patternEditorControl);
	
#ifdef __LOWRES__
	setInputControl(SIPDefault);
	updateJamMenuOrder(false);
#endif

	TitlePageManager titlePageManager(*screen);	
	titlePageManager.showSongTitleEditField(false);

	setFollowSong(true, false);
	setProspectiveMode(false, false);
	setCursorWrapAround(true, false);

	for (c = 0; c < TrackerConfig::MAXCHANNELS; c++)
	{
		scopesControl->recordChannel(c, playerController->isChannelRecording(c));
		getPatternEditorControl()->recordChannel(c, playerController->isChannelRecording(c));
	}

	patternEditorControl->setScrollMode(ScrollModeStayInCenter);
	
	scopesControl->attachSource(this->playerController);
	
	tabManager->openNewTab(this->playerController, this->moduleEditor);
}

////////////////////////////////////////////////////////////////////
// Build orderlist section
////////////////////////////////////////////////////////////////////
void Tracker::initSectionOrderlist(pp_int32 x, pp_int32 y)
{
	// setup controls
	PPContainer* containerOrderlist = new PPContainer(CONTAINER_ORDERLIST, screen, this, PPPoint(x, y), PPSize(116-2,64), false);
	containerOrderlist->setColor(TrackerConfig::colorThemeMain);

	PPButton* button = new PPButton(BUTTON_ORDERLIST_EXTENT, screen, this, PPPoint(x + 2 + 78 - 2 - 22, y + 2), PPSize(19, 10), false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText(TrackerConfig::stringButtonCollapsed);
	containerOrderlist->addControl(button);

	// DUP
	button = new PPButton(BUTTON_ORDERLIST_SEQENTRY, screen, this, PPPoint(x + 2 + 78 - 2 - 22, y+2+12), PPSize(18, 11));
	//button->setVerticalText(true);
	button->setXOffset(-1);
	button->setText("Seq");
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_CLNENTRY, screen, this, PPPoint(x + 2 + 78 - 2 - 22, y+2+12+12), PPSize(18, 11));
	//button->setVerticalText(true);
	button->setXOffset(-1);
	button->setText("Cln");
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_INSERT, screen, this, PPPoint(x+2 + 78-2, y+2), PPSize(33, 11));
	button->setText("Ins.");
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_NEXT, screen, this, PPPoint(x+2 + 78-2, y+2+12), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonPlus);
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_PREVIOUS, screen, this, PPPoint(x+2 + 78-2 + 17, y+2+12), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonMinus);
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_DELETE, screen, this, PPPoint(x+2 + 78-2, y+2+12+12), PPSize(33, 11));
	button->setText("Del");
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_SONGLENGTH_PLUS, screen, this, PPPoint(x+2 + 78-2, y+2+12+12+12), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonPlus);
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_SONGLENGTH_MINUS, screen, this, PPPoint(x+2 + 78-2 + 17, y+2+12+12+12), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonMinus);
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_REPEAT_PLUS, screen, this, PPPoint(x+2 + 78-2, y+2+12+12+12+12), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonPlus);
	containerOrderlist->addControl(button);

	button = new PPButton(BUTTON_ORDERLIST_REPEAT_MINUS, screen, this, PPPoint(x+2 + 78-2 + 17, y+2+12+12+12+12), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonMinus);
	containerOrderlist->addControl(button);

	PPStaticText* staticText = new PPStaticText(0, NULL, NULL, PPPoint(x+2, y+2+12+12+12+2), "Length", true);
	containerOrderlist->addControl(staticText);

	staticText = new PPStaticText(1, NULL, NULL, PPPoint(x+2 + 54, y+2+12+12+12+12+2), "Len.", true);
	containerOrderlist->addControl(staticText);

	// actual Song Length field
	staticText = new PPStaticText(STATICTEXT_ORDERLIST_SONGLENGTH, screen, NULL, PPPoint(x+2 + 8*7, y+2+12+12+12+2), "", false);
	containerOrderlist->addControl(staticText);	

	staticText = new PPStaticText(2, NULL, NULL, PPPoint(x+2, y+2+12+12+12+12+2), "Repeat", true);
	containerOrderlist->addControl(staticText);

	// actual Song repeat field
	staticText = new PPStaticText(STATICTEXT_ORDERLIST_REPEAT, screen, NULL, PPPoint(x+2 + 8*7, y+2+12+12+12+12+2), "", false);
	containerOrderlist->addControl(staticText);	

	listBoxOrderList = new PPListBox(LISTBOX_ORDERLIST, screen, this, PPPoint(x+2, y+2), PPSize(51,36), true, false, true, true);
	listBoxOrderList->setAutoHideVScroll(false);
	listBoxOrderList->setBorderColor(TrackerConfig::colorThemeMain);
	listBoxOrderList->setCenterSelection(true);
	listBoxOrderList->setSelectOnScroll(true);

	containerOrderlist->addControl(listBoxOrderList);

	screen->addControl(containerOrderlist);	
	
	expandOrderlist(false);
}

////////////////////////////////////////////////////////////////////
// Build speed section
////////////////////////////////////////////////////////////////////
void Tracker::initSectionSpeed(pp_int32 x, pp_int32 y)
{
	PPContainer* containerSpeed = new PPContainer(CONTAINER_SPEED, screen, this, PPPoint(x, y), PPSize(99-2,40), false);
	containerSpeed->setColor(TrackerConfig::colorThemeMain);

	PPStaticText* staticText = new PPStaticText(STATICTEXT_SPEED_BPM_DESC, NULL, NULL, PPPoint(x+2, y+2+2), "BPM", true);
	containerSpeed->addControl(staticText);	

	// actual BPM field
	staticText = new PPStaticText(STATICTEXT_SPEED_BPM, screen, NULL, PPPoint(x+2 + 4*8 - 5, y+2+2), "", false);
	containerSpeed->addControl(staticText);	

	// Octave text field goes at the same place but hidden by default
	staticText = new PPStaticText(STATICTEXT_SPEED_OCTAVE_DESC, NULL, NULL, PPPoint(x+2, y+2+2), "Oct", true);
	staticText->hide(true);
	containerSpeed->addControl(staticText);	

	// actual octave field, hidden by default
	staticText = new PPStaticText(STATICTEXT_SPEED_OCTAVE, screen, NULL, PPPoint(x+2 + 5*8 - 5, y+2+2), "", false);
	staticText->hide(true);
	containerSpeed->addControl(staticText);	

	staticText = new PPStaticText(STATICTEXT_SPEED_SPEED_DESC, NULL, NULL, PPPoint(x+2, y+2 + 2 + 12), "Spd", true);
	containerSpeed->addControl(staticText);	

	// actual speed field
	staticText = new PPStaticText(STATICTEXT_SPEED_SPEED, screen, NULL, PPPoint(x+2 + 5*8 - 5, y+2 + 2 + 12), "", false);
	containerSpeed->addControl(staticText);	

	staticText = new PPStaticText(STATICTEXT_SPEED_PATTERNADD_DESC, NULL, NULL, PPPoint(x+2, y+2 + 2 + 12 + 12), "Add", true);
	containerSpeed->addControl(staticText);	

	// actual speed field
	staticText = new PPStaticText(STATICTEXT_SPEED_PATTERNADD, screen, NULL, PPPoint(x+2 + 5*8 - 5, y+2 + 2 + 12 + 12), "", false);
	containerSpeed->addControl(staticText);	

	staticText = new PPStaticText(STATICTEXT_SPEED_MAINVOL_DESC, NULL, NULL, PPPoint(x+2, y+2 + 2 + 12 + 12), "Mainvol", true);
	staticText->hide(true);
	containerSpeed->addControl(staticText);	

	// actual speed field
	staticText = new PPStaticText(STATICTEXT_SPEED_MAINVOL, screen, NULL, PPPoint(x+2 + 8*8 - 3, y+2 + 2 + 12 + 12), "xx", false);
	staticText->hide(true);
	containerSpeed->addControl(staticText);	

	const pp_int32 bSize = 14;

	PPButton* button = new PPButton(BUTTON_BPM_PLUS, screen, this, PPPoint(x + 2 + 54, y+2), PPSize(bSize, 11));
	button->setText(TrackerConfig::stringButtonPlus);
	containerSpeed->addControl(button);

	// octave plus button, hidden by default
	button = new PPButton(BUTTON_OCTAVE_PLUS, screen, this, PPPoint(x + 2 + 54, y+2), PPSize(bSize, 11));
	button->setText(TrackerConfig::stringButtonPlus);
	button->hide(true);
	containerSpeed->addControl(button);

	button = new PPButton(BUTTON_BPM_MINUS, screen, this, PPPoint(x + 2 + 54 + bSize+1, y+2), PPSize(bSize-1, 11));
	button->setText(TrackerConfig::stringButtonMinus);
	containerSpeed->addControl(button);

	// octave minus button, hidden by default
	button = new PPButton(BUTTON_OCTAVE_MINUS, screen, this, PPPoint(x + 2 + 54 + bSize+1, y+2), PPSize(bSize-1, 11));
	button->setText(TrackerConfig::stringButtonMinus);
	button->hide(true);
	containerSpeed->addControl(button);

	button = new PPButton(BUTTON_SPEED_PLUS, screen, this, PPPoint(x + 2 + 54, y+2 + 12), PPSize(bSize, 11));
	button->setText(TrackerConfig::stringButtonPlus);
	containerSpeed->addControl(button);

	button = new PPButton(BUTTON_SPEED_MINUS, screen, this, PPPoint(x + 2 + 54 + bSize+1, y+2 + 12), PPSize(bSize-1, 11));
	button->setText(TrackerConfig::stringButtonMinus);
	containerSpeed->addControl(button);

	button = new PPButton(BUTTON_ADD_PLUS, screen, this, PPPoint(x + 2 + 54, y+2 + 12 + 12), PPSize(bSize, 11));
	button->setText(TrackerConfig::stringButtonPlus);
	containerSpeed->addControl(button);

	button = new PPButton(BUTTON_ADD_MINUS, screen, this, PPPoint(x + 2 + 54 + bSize+1, y+2 + 12 + 12), PPSize(bSize-1, 11));
	button->setText(TrackerConfig::stringButtonMinus);
	containerSpeed->addControl(button);

	button = new PPButton(BUTTON_SPEEDCONTAINERFLIP, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y+1), PPSize(10, 37), false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Flip");
	button->setVerticalText(true);
	containerSpeed->addControl(button);
	
	screen->addControl(containerSpeed);
}

////////////////////////////////////////////////////////////////////
// Build pattern section
////////////////////////////////////////////////////////////////////
void Tracker::initSectionPattern(pp_int32 x, pp_int32 y)
{
	PPContainer* containerPattern = new PPContainer(CONTAINER_PATTERN, screen, this, PPPoint(x, y), PPSize(91+14+4,40), false);
	containerPattern->setColor(TrackerConfig::colorThemeMain);

	PPStaticText* staticText = new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y+2 + 2), "Patn.", true);
	containerPattern->addControl(staticText);	

	// actual pattern index field
	staticText = new PPStaticText(STATICTEXT_PATTERN_INDEX, screen, NULL, PPPoint(x + 2 + 4 * 8 + 18, y+2 + 2), "", false);
	containerPattern->addControl(staticText);	

	staticText = new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y+2 + 2 + 12), "Len.", true);
	containerPattern->addControl(staticText);	

	// actual pattern length field
	staticText = new PPStaticText(STATICTEXT_PATTERN_LENGTH, screen, NULL, PPPoint(x + 2 + 3 * 8 + 18, y+2 + 2 + 12), "", false);
	containerPattern->addControl(staticText);	

	PPButton* button = new PPButton(BUTTON_PATTERN_PLUS, screen, this, PPPoint(x + 2 + 52+18, y+2), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonPlus);

	containerPattern->addControl(button);

	button = new PPButton(BUTTON_PATTERN_MINUS, screen, this, PPPoint(x + 2 + 52 + 17+18, y+2), PPSize(17, 11));
	button->setText(TrackerConfig::stringButtonMinus);

	containerPattern->addControl(button);

	button = new PPButton(BUTTON_PATTERN_SIZE_PLUS, screen, this, PPPoint(x + 2 + 52+18, y+2 + 12), PPSize(16, 11));
	button->setText(TrackerConfig::stringButtonPlus);

	containerPattern->addControl(button);

	button = new PPButton(BUTTON_PATTERN_SIZE_MINUS, screen, this, PPPoint(x + 2 + 52 + 17+18, y+2 + 12), PPSize(17, 11));
	button->setText(TrackerConfig::stringButtonMinus);

	containerPattern->addControl(button);

	button = new PPButton(BUTTON_PATTERN_EXPAND, screen, this, PPPoint(x + 3, y+2 + 12 + 12), PPSize(51, 11));
	button->setText("Expand");

	containerPattern->addControl(button);

	button = new PPButton(BUTTON_PATTERN_SHRINK, screen, this, PPPoint(x + 3 + 52, y+2 + 12 +12), PPSize(51, 11));
	button->setText("Shrink");

	containerPattern->addControl(button);

	screen->addControl(containerPattern);
}

/////////////////////////////////////////////////////////////
// Build main options section
/////////////////////////////////////////////////////////////
void Tracker::initSectionMainOptions(pp_int32 x, pp_int32 y)
{
	pp_int32 i,j;

#ifndef __LOWRES__
	pp_int32 bHeight = 12;
	PPSize size(320, 54);
#else
	pp_int32 bHeight = 14;
	PPSize size(320, 64);
#endif

	PPContainer* container = new PPContainer(CONTAINER_MENU, screen, this, PPPoint(x, y), size, false);
	container->setColor(TrackerConfig::colorThemeMain);

#ifdef __LOWRES__
	y+=2;
#endif
	
	PPButton* button;
	
	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < 4; i++)
		{
			if (j * 4 + i < 15)
			{
				button = new PPButton(BUTTON_MENU_ITEM_0 + j*4+i, screen, this, PPPoint(x+4 + i*78, y + 3 + j*bHeight), PPSize(77, bHeight-1));
				button->setText("Unused");
			
				container->addControl(button);
			}
		}
		
	}

	static_cast<PPButton*>(container->getControlByID(MAINMENU_PLAY_SONG))->setText("Play Sng");	
	static_cast<PPButton*>(container->getControlByID(MAINMENU_PLAY_PATTERN))->setText("Play Pat");
	//static_cast<PPButton*>(container->getControlByID(MAINMENU_STOP))->setText("Stop");
	// Setup "Stop" PPButton
	button = static_cast<PPButton*>(container->getControlByID(MAINMENU_STOP));
	button->setText("Stop");
	button->setSize(PPSize(77>>1, bHeight-1));
	// Add "Edit" button
	button = new PPButton(MAINMENU_EDIT, screen, this, 
						PPPoint(button->getLocation().x + button->getSize().width+1, button->getLocation().y), 
						PPSize(77>>1, bHeight-1), true, true, false);
	
	button->setText("Rec");
	
	container->addControl(button);

	static_cast<PPButton*>(container->getControlByID(MAINMENU_ZAP))->setText("Zap");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_LOAD))->setText("Load");
	// Setup "Save" button
	button = static_cast<PPButton*>(container->getControlByID(MAINMENU_SAVE));
	button->setText("Save");
	button->setSize(PPSize(77>>1, bHeight-1));
	// Add "Save As" button
	button = new PPButton(MAINMENU_SAVEAS, screen, this, 
						PPPoint(button->getLocation().x + button->getSize().width+1, button->getLocation().y), 
						PPSize(77>>1, bHeight-1));
	
	button->setText("As" PPSTR_PERIODS);
	
	container->addControl(button);
	
	//static_cast<PPButton*>(container->getControlByID(MAINMENU_SAVE));
	static_cast<PPButton*>(container->getControlByID(MAINMENU_DISKMENU))->setText("Disk Op.");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_INSEDIT))->setText("Ins. Ed.");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_SMPEDIT))->setText("Smp. Ed.");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_ADVEDIT))->setText("Adv. Edit");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_TRANSPOSE))->setText("Transpose");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_CONFIG))->setText("Config");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_QUICKOPTIONS))->setText("Options");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_OPTIMIZE))->setText("Optimize");
	static_cast<PPButton*>(container->getControlByID(MAINMENU_ABOUT))->setText("About");

	// add/subtract channels
	button = new PPButton(BUTTON_MENU_ITEM_ADDCHANNELS, screen, this, PPPoint(x+4 + 3*78, y + 3 + 3*bHeight), PPSize((77>>1) - 1, bHeight-1));
	button->setText("Add");
	container->addControl(button);

	button = new PPButton(BUTTON_MENU_ITEM_SUBCHANNELS, screen, this, PPPoint(x+4 + 3*78 + (77>>1), y + 3 + 3*bHeight), PPSize((77>>1)+1, bHeight-1));
	button->setText("Sub");
	container->addControl(button);

	button = static_cast<PPButton*>(container->getControlByID(MAINMENU_PLAY_PATTERN));
	button->setText("Pat");
	button->setSize(PPSize((77>>1)-1, bHeight-1));

	PPPoint p = button->getLocation();
	p.x+=button->getSize().width+1;
	
	button = new PPButton(MAINMENU_PLAY_POSITION, screen, this, p, PPSize((77>>1)+1, bHeight-1));
	button->setText("Pos");
	container->addControl(button);

	screen->addControl(container);	
}

void Tracker::initListboxesSection(pp_int32 x, pp_int32 y)
{
	pp_int32 size = (screen->getWidth()-x) / 2 - 4;

	if (size > 236)
		size = 236;

#ifndef __LOWRES__
	const pp_int32 tinyButtonHeight = 10;
	const pp_int32 tinyButtonOffset = -1;
	pp_int32 height = 118;
	pp_int32 dy = 4;
#else
	const pp_int32 tinyButtonHeight = 9;
	const pp_int32 tinyButtonOffset = -1;
	pp_int32 height = 64;
	pp_int32 dy = 3;
#endif

	PPButton* button;

	// Crippled main menu & jam menu
#ifdef __LOWRES__
	pp_int32 myDx = 55;
	{
		pp_int32 bHeight = 14;
		
		pp_int32 x2 = x+4;
		pp_int32 y2 = y+2+3;

		PPContainer* container = new PPContainer(CONTAINER_LOWRES_TINYMENU, screen, this, PPPoint(x, y), PPSize((size-myDx)+7,height), false);
		container->setColor(TrackerConfig::colorThemeMain);
		
		button = new PPButton(MAINMENU_PLAY_SONG, screen, this, PPPoint(x2, y2), PPSize(73, bHeight-1));
		button->setText("Play Sng");		
		container->addControl(button);
		
		button = new PPButton(MAINMENU_PLAY_PATTERN, screen, this, PPPoint(x2, y2 + 1*bHeight), PPSize(73, bHeight-1));
		button->setText("Play Pat");		
		container->addControl(button);
		
		/*button = new PPButton(MAINMENU_STOP, screen, this, PPPoint(x2, y2 + 2*bHeight), PPSize(73, bHeight-1));
		button->setText("Stop");		
		container->addControl(button);*/

		button = new PPButton(MAINMENU_STOP, screen, this, PPPoint(x2, y2 + 2*bHeight), PPSize((73>>1) - 1, bHeight-1));
		button->setText("Stop");
		container->addControl(button);
		
		button = new PPButton(MAINMENU_EDIT, screen, this, PPPoint(x2 + (73>>1), y2 + 2*bHeight), PPSize((73>>1)+1, bHeight-1), true, true, false);
		button->setText("Rec");
		container->addControl(button);
		
		button = new PPButton(BUTTON_MENU_ITEM_ADDCHANNELS, screen, this, PPPoint(x2, y2 + 3*bHeight), PPSize((73>>1) - 1, bHeight-1));
		button->setText("Add");
		container->addControl(button);
		
		button = new PPButton(BUTTON_MENU_ITEM_SUBCHANNELS, screen, this, PPPoint(x2 + (73>>1), y2 + 3*bHeight), PPSize((73>>1)+1, bHeight-1));
		button->setText("Sub");
		container->addControl(button);

		x2+=74;

		button = new PPButton(MAINMENU_INSEDIT, screen, this, PPPoint(x2, y2), PPSize(26, bHeight-1));
		button->setText("Ins");		
		container->addControl(button);
		
		button = new PPButton(MAINMENU_SMPEDIT, screen, this, PPPoint(x2, y2 + 1*bHeight), PPSize(26, bHeight-1));
		button->setText("Smp");		
		container->addControl(button);
		
		button = new PPButton(MAINMENU_ADVEDIT, screen, this, PPPoint(x2, y2 + 2*bHeight), PPSize(26, bHeight-1));
		button->setText("Adv");		
		container->addControl(button);

		button = new PPButton(MAINMENU_TRANSPOSE, screen, this, PPPoint(x2, y2 + 3*bHeight), PPSize(26, bHeight-1));
		button->setText("Trn");		
		container->addControl(button);
		
		// play pattern/position
		button = static_cast<PPButton*>(container->getControlByID(MAINMENU_PLAY_PATTERN));
		button->setText("Pat");
		button->setSize(PPSize((73>>1)-1, bHeight-1));
		
		PPPoint p = button->getLocation();
		p.x+=button->getSize().width+1;
		
		button = new PPButton(MAINMENU_PLAY_POSITION, screen, this, p, PPSize((73>>1)+1, bHeight-1));
		button->setText("Pos");
		container->addControl(button);
		
		screen->addControl(container);
		
		x+=(size-myDx)+7;
	}

	size-=4;

	{
		pp_int32 height2 = height + 39 + 14;
		pp_int32 y3 = y - 39 - 14;
		
		pp_int32 x2 = 0+4;

		PPContainer* container = new PPContainer(CONTAINER_LOWRES_JAMMENU, screen, this, PPPoint(0, y3), PPSize(screen->getWidth(),height2), false);
		container->setColor(TrackerConfig::colorThemeMain);

		pp_int32 bHeight = 12;
		pp_int32 y2 = y3 + 1;

		PianoControl* pianoControl = new PianoControl(PIANO_CONTROL, screen, inputControlListener, 
													  PPPoint(container->getLocation().x+2, y2), PPSize(screen->getWidth() - 4, 25*3+12), ModuleEditor::MAX_NOTE); 
		// show C-3
		pianoControl->setBorderColor(TrackerConfig::colorThemeMain);
		pianoControl->setMode(PianoControl::ModePlay);
		pianoControl->setxScale(6);
		pianoControl->setyScale(3);		
		pianoControl->assureNoteVisible(12*4);
		
		container->addControl(pianoControl);
		
		x2 = 0+4;
		y2+=pianoControl->getSize().height+1;
		
		PPStaticText* staticText = new PPStaticText(0, NULL, NULL, PPPoint(x2, y2+2), "Pos", true);
		container->addControl(staticText);	
		
		staticText = new PPStaticText(STATICTEXT_JAMMENU_CURORDER, NULL, NULL, PPPoint(x2 + 3*8+4, y2+2), "FF");
		container->addControl(staticText);	

		button = new PPButton(BUTTON_JAMMENU_NEXTORDERLIST, screen, this, PPPoint(x2 + 5*8+4 + 1, y2), PPSize(12, 11));
		button->setText(TrackerConfig::stringButtonPlus);
		container->addControl(button);
		
		button = new PPButton(BUTTON_JAMMENU_PREVORDERLIST, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y2), PPSize(12, 11));
		button->setText(TrackerConfig::stringButtonMinus);
		container->addControl(button);
		
		pp_int32 x3 = button->getLocation().x + button->getSize().width+3;
		
		staticText = new PPStaticText(0, NULL, NULL, PPPoint(x3, y2+2), "Pat", true);
		container->addControl(staticText);	

		staticText = new PPStaticText(STATICTEXT_JAMMENU_CURPATTERN, NULL, NULL, PPPoint(x3 + 3*8+4, y2+2), "FF");
		container->addControl(staticText);	

		button = new PPButton(BUTTON_PATTERN_PLUS, screen, this, PPPoint(x3 + 5*8+4 + 1, y2), PPSize(12, 11));
		button->setText(TrackerConfig::stringButtonPlus);
		container->addControl(button);
		
		button = new PPButton(BUTTON_PATTERN_MINUS, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y2), PPSize(12, 11));
		button->setText(TrackerConfig::stringButtonMinus);
		container->addControl(button);

		x3 = button->getLocation().x + button->getSize().width+3;
		
		staticText = new PPStaticText(0, NULL, NULL, PPPoint(x3, y2+2), "Ins", true);
		container->addControl(staticText);	

		staticText = new PPStaticText(STATICTEXT_JAMMENU_CURINSTRUMENT, NULL, NULL, PPPoint(x3 + 3*8+4, y2+2), "FF");
		container->addControl(staticText);	

		button = new PPButton(BUTTON_JAMMENU_NEXTINSTRUMENT, screen, this, PPPoint(x3 + 5*8+4 + 1, y2), PPSize(12, 11));
		button->setText(TrackerConfig::stringButtonUp);
		container->addControl(button);
		
		button = new PPButton(BUTTON_JAMMENU_PREVINSTRUMENT, screen, this, PPPoint(button->getLocation().x + button->getSize().width+1, y2), PPSize(12, 11));
		button->setText(TrackerConfig::stringButtonDown);
		container->addControl(button);

		button = new PPButton(INPUT_BUTTON_INS, screen, inputControlListener, PPPoint(button->getLocation().x + button->getSize().width+1 + 2, y2), PPSize(17, 11));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Ins");
		container->addControl(button);

		button = new PPButton(INPUT_BUTTON_DEL, screen, inputControlListener, PPPoint(button->getLocation().x + button->getSize().width+1, y2), PPSize(17, 11));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Del");
		container->addControl(button);

		button = new PPButton(INPUT_BUTTON_BACK, screen, inputControlListener, PPPoint(button->getLocation().x + button->getSize().width+1, y2), PPSize(22, 11));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Back");
		container->addControl(button);

		button = new PPButton(INPUT_BUTTON_KEYOFF, screen, inputControlListener, PPPoint(button->getLocation().x + button->getSize().width+1, y2), PPSize(17, 11));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Off");
		container->addControl(button);

		button = new PPButton(BUTTON_JAMMENU_TOGGLEPIANOSIZE, screen, this, PPPoint(button->getLocation().x + button->getSize().width+3, y2+1), PPSize(12, 11), false);
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		button->setText(TrackerConfig::stringButtonCollapsed);
		container->addControl(button);

		x2 = 0+4;
		y2+=14;
		
		button = new PPButton(MAINMENU_PLAY_SONG, screen, this, PPPoint(x2, y2), PPSize(77, bHeight-1));
		button->setText("Play Sng");		
		container->addControl(button);
		
		x2+=button->getSize().width+1;
		button = new PPButton(MAINMENU_PLAY_PATTERN, screen, this, PPPoint(x2, y2), PPSize((77>>1)-1, bHeight-1));
		button->setText("Pat");		
		container->addControl(button);

		x2+=button->getSize().width+1;
		button = new PPButton(MAINMENU_PLAY_POSITION, screen, this, PPPoint(x2, y2), PPSize((77>>1)+1, bHeight-1));
		button->setText("Pos");
		container->addControl(button);
		
		x2+=button->getSize().width+1;
		button = new PPButton(MAINMENU_STOP, screen, this, PPPoint(x2, y2), PPSize((77>>1), bHeight-1));
		button->setText("Stop");		
		container->addControl(button);
		
		// Add "Edit" button
		x2+=button->getSize().width+1;
		button = new PPButton(MAINMENU_EDIT, screen, this, PPPoint(x2, y2), PPSize((77>>1), bHeight-1), true, true, false);
		button->setText("Rec");
		container->addControl(button);	
		
		x2+=button->getSize().width+1;
		button = new PPButton(BUTTON_MENU_ITEM_ADDCHANNELS, screen, this, PPPoint(x2, y2), PPSize((77>>1) - 1, bHeight-1));
		button->setText("Add");
		container->addControl(button);
		
		x2+=button->getSize().width+1;
		button = new PPButton(BUTTON_MENU_ITEM_SUBCHANNELS, screen, this, PPPoint(x2, y2), PPSize((77>>1)+1, bHeight-1));
		button->setText("Sub");
		container->addControl(button);		

		screen->addControl(container);		
	}
#else
	pp_int32 myDx = 0;
#endif

	PPContainer* container = new PPContainer(CONTAINER_INSTRUMENTLIST, screen, this, PPPoint(x, y), PPSize(screen->getWidth()-x,height), false);
	container->setColor(TrackerConfig::colorThemeMain);
	
	// Instruments
#ifndef __LOWRES__
	button = new PPButton(BUTTON_INSTRUMENT, screen, this, PPPoint(x+2, y+dy-2), PPSize(screen->getWidth() < 800 ? 3*8+4 : 11*8+4, 12), false, true, false);
	button->setText(screen->getWidth() < 800 ? "Ins" : "Instruments");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setPressed(true);
	//PPStaticText* staticText = new PPStaticText(0, NULL, NULL, PPPoint(x+3, y+dy), screen->getWidth() < 800 ? "Ins" : "Instruments", true);
#else
	button = new PPButton(BUTTON_INSTRUMENT, screen, this, PPPoint(x+2, y+dy-2), PPSize(11*8+4, 11), false, true, false);
	button->setText("Instruments");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setPressed(true);
	{
		PPStaticText* staticText = new PPStaticText(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER, screen, this, PPPoint(x+3, y+dy), "Samples / Ins:", true);
		staticText->hide(true);
		container->addControl(staticText);
		
		staticText = new PPStaticText(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER2, screen, this, PPPoint(x+3 + 14*8, y+dy), "xx", false);
		staticText->hide(true);
		container->addControl(staticText);
	}
#endif
	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTS_PLUS, screen, this, PPPoint(x+button->getSize().width+4, y+dy+tinyButtonOffset), PPSize(15, tinyButtonHeight));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	
	button = new PPButton(BUTTON_INSTRUMENTS_MINUS, screen, this, PPPoint(button->getLocation().x + 16, y+dy+tinyButtonOffset), PPSize(15, tinyButtonHeight));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

#ifndef __LOWRES__
	button = new PPButton(BUTTON_INSTRUMENTEDITOR_CLEAR, screen, sectionInstruments, PPPoint(x+2 + size - 2 - 92, y+dy+tinyButtonOffset), PPSize(30, tinyButtonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Zap");
	container->addControl(button);	

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_LOAD, screen, sectionInstruments, PPPoint(x+2 + size - 2 - 61, y+dy+tinyButtonOffset), PPSize(30, tinyButtonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Load");
	container->addControl(button);

	button = new PPButton(BUTTON_INSTRUMENTEDITOR_SAVE, screen, sectionInstruments, PPPoint(x+2 + size - 2 - 30, y+dy+tinyButtonOffset), PPSize(30, tinyButtonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Save");
	container->addControl(button);
#else
	button = new PPButton(BUTTON_INSTRUMENTS_FLIP, screen, this, PPPoint(container->getLocation().x + container->getSize().width - 27, y+dy+tinyButtonOffset - 1), PPSize(24, 11), false);
	button->setText("Flip");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	container->addControl(button);

	button = new PPButton(BUTTON_JAMMENU_PREVINSTRUMENT, screen, this, PPPoint(button->getLocation().x - 16, y+dy+tinyButtonOffset), PPSize(15, tinyButtonHeight));
	button->setText(TrackerConfig::stringButtonUp);
	container->addControl(button);
	
	button = new PPButton(BUTTON_JAMMENU_NEXTINSTRUMENT, screen, this, PPPoint(button->getLocation().x - 16, y+dy+tinyButtonOffset), PPSize(15, tinyButtonHeight));
	button->setText(TrackerConfig::stringButtonDown);
	container->addControl(button);
#endif

	listBoxInstruments = new PPListBox(LISTBOX_INSTRUMENTS, screen, this, PPPoint(x+2, y + 7 + dy + dy), PPSize(size+myDx,height-(10+2*dy)), true, true, true, true);
	listBoxInstruments->setBorderColor(TrackerConfig::colorThemeMain);
	listBoxInstruments->setShowIndex(true);
	listBoxInstruments->setMaxEditSize(ModuleEditor::MAX_INSTEXT);
	//listBoxInstruments->setSelectOnScroll(true);
	
	fillInstrumentListBox(listBoxInstruments);

	container->addControl(listBoxInstruments);

	// Samples
#ifndef __LOWRES__
	button = new PPButton(BUTTON_SAMPLE_EDIT_CLEAR, screen, sectionSamples, PPPoint(x+2 + size*2 + 4 - 2 - 92, y+dy+tinyButtonOffset), PPSize(30, tinyButtonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Clear");
	container->addControl(button);	

	button = new PPButton(BUTTON_SAMPLE_LOAD, screen, sectionSamples, PPPoint(x+2 + size*2 + 4 - 2 - 61, y+dy+tinyButtonOffset), PPSize(30, tinyButtonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Load");
	container->addControl(button);

	button = new PPButton(BUTTON_SAMPLE_SAVE, screen, sectionSamples, PPPoint(x+2 + size*2 + 4 - 2 - 30, y+dy+tinyButtonOffset), PPSize(30, tinyButtonHeight));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Save");
	container->addControl(button);
#endif

	PPStaticText* staticText = new PPStaticText(STATICTEXT_SAMPLEHEADER, NULL, NULL, PPPoint(x+size+myDx+4+3, y+dy), "Samples", true);
	container->addControl(staticText);

	listBoxSamples = new PPListBox(LISTBOX_SAMPLES, screen, this, PPPoint(x+2 + size+4, y + 7 + dy + dy), PPSize(size,height-(10+2*dy)), true, true, true, true);
	listBoxSamples->setBorderColor(TrackerConfig::colorThemeMain);
	listBoxSamples->setShowIndex(true);
	listBoxSamples->setMaxEditSize(ModuleEditor::MAX_SMPTEXT);
	listBoxSamples->setIndexBaseCount(0);

	container->addControl(listBoxSamples);

#ifdef __LOWRES__
	listBoxSamples->hide(true);
#endif

	screen->addControl(container);
}

void Tracker::showMessageBox(pp_int32 id, const PPString& caption, MessageBoxTypes type, bool update/* = true*/)
{
	showMessageBoxSized(id, caption, type, 290, 74, update);
}

void Tracker::showMessageBoxSized(pp_int32 id, const PPString& caption, MessageBoxTypes type, pp_int32 width/* = -1*/, pp_int32 height/* = -1*/, bool update/* = true*/)
{
	if (messageBoxContainerGeneric)
	{
		delete messageBoxContainerGeneric;
		messageBoxContainerGeneric = NULL;
	}

	PPSimpleVector<PPString>* stringList = PPTools::extractStringList(caption);

	if (height == -1)
		height = 62 + stringList->size()*12;

	if (width == -1)
		width = 290;
			
	pp_int32 x = screen->getWidth() / 2 - width/2;
	pp_int32 y = screen->getHeight() / 2 - height/2;

	PPMessageBoxContainer* container = new PPMessageBoxContainer(id, screen, this, PPPoint(x, y), PPSize(width,height), "System request");

	pp_int32 x2;
	pp_int32 y2; 
	
	y2 = y + 28;

	for (pp_int32 i = 0; i < stringList->size(); i++)
	{
		PPString* str = stringList->get(i);
	
		x2 = x + width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(*str) / 2);

		container->addControl(new PPStaticText(1 + i, screen, this, PPPoint(x2, y2), *str, true));
	
		y2+=12;
	}

	delete stringList;

	y2 = y + height - 46;

	if (type == MessageBox_OK)
	{
		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width/2-30, y2 + 20), PPSize(60, 11));
		button->setText("Okay");
		container->addControl(button);
	}
	else if (type == MessageBox_YESNO)
	{
		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width/2-65, y2 + 20), PPSize(60, 11));
		button->setText("Yes");
		container->addControl(button);
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_NO, screen, this, PPPoint(x+width/2+5, y2 + 20), PPSize(60, 11));
		button->setText("No");
		container->addControl(button);
	}
	else if (type == MessageBox_YESNOCANCEL)
	{
		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width/2-100, y2 + 20), PPSize(60, 11));
		button->setText("Yes");
		container->addControl(button);
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_NO, screen, this, PPPoint(x+width/2-30, y2 + 20), PPSize(60, 11));
		button->setText("No");
		container->addControl(button);

		button = new PPButton(PP_MESSAGEBOX_BUTTON_CANCEL, screen, this, PPPoint(x+width/2+40, y2 + 20), PPSize(60, 11));
		button->setText("Cancel");
		container->addControl(button);
	}

	messageBoxContainerGeneric = container;

	screen->setModalControl(messageBoxContainerGeneric, update);
}

void Tracker::showQuitMessageBox(const char* caption, const char* captionOk, const char* captionCancel)
{
	showMessageBoxSized(MESSAGEBOX_REALLYQUIT, caption, MessageBox_YESNO, -1, -1, false);
	
	if (captionOk)
	{
		PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_YES));
		button->setText(captionOk);
	}

	if (captionCancel)
	{
		PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_NO));
		button->setText(captionCancel);
	}
	
	screen->paint();
}

void Tracker::initInstrumentChooser(pp_int32 id, const PPString& buttonText1, const PPString& buttonText2, const PPString& caption, 
									const PPString& userString1, const PPString& userString2, 
									pp_int32 srcSelection/* = -1*/, pp_int32 srcSelection2/* = -1*/, pp_int32 srcSelection3/* = -1*/)
{
	if (instrumentChooser)
	{
		delete instrumentChooser;
		instrumentChooser = NULL;
	}

	const pp_uint32 spacer = 10;

	PPString buttonText1_2 = buttonText1;
	buttonText1_2.append("++");

	PPString buttonText2_2 = buttonText2;
	buttonText2_2.append("++");

	const pp_int32 height = (screen->getHeight() >= 480) ? 380 : screen->getHeight();
	const pp_int32 width = (screen->getWidth()-10) > 480 ? 480 : screen->getWidth();

	pp_int32 x = screen->getWidth() / 2 - width/2;
	pp_int32 y = screen->getHeight() / 2 - height/2;

	PPMessageBoxContainer* container = new PPMessageBoxContainer(id, screen, this, PPPoint(x, y), PPSize(width,height), caption);

	// ------------- source listboxes ---------------------------------
	pp_int32 x2 = x + 5;
	pp_int32 y2 = y + 18;

	pp_int32 lBoxHeight = ((height-100)/2) & ~1;

	PPString str;
	PPListBox* listBoxInstrumentsSrc = NULL;
	PPListBox* listBoxSamplesSrc = NULL;
	PPListBox* listBoxModulesSrc = NULL;
	PPListBox* listBoxInstrumentsDst = NULL;
	PPListBox* listBoxSamplesDst = NULL;
	PPListBox* listBoxModulesDst = NULL;

	if (tabManager->getNumTabs() > 1)
	{
		listBoxModulesSrc = new PPListBox(INSTRUMENT_CHOOSER_LIST_SRC3, screen, this, 
													 PPPoint(x2-2, y2 + 10), PPSize((width-10)/2,lBoxHeight), true, false, true, true);
		listBoxModulesSrc->setShowIndex(true);

		fillModuleListBox(listBoxModulesSrc);

		if (srcSelection3 >= 0)
			listBoxModulesSrc->setSelectedIndex(srcSelection3, false);

		container->addControl(listBoxModulesSrc);

		str = "From Module:";
	
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));

		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_USER5, screen, this, 
										PPPoint(x2 - 2 + (width-10) / 2 - 50 - 1, y2-1),
										PPSize(50, 10));

		button->setText("From<->To");
		container->addControl(button);

		x2+=3+(width-10)/2+2;
		
		pp_int32 temp = lBoxHeight;
		lBoxHeight = lBoxHeight / 2 - 6;

		str = "From Ins:";
		
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER10, screen, this, 
							  PPPoint(x2 + 9*8+4, y2-1),
							  PPSize(14, 10));
		
		button->setText("+");
		container->addControl(button);		
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER11, screen, this, 
							  PPPoint(x2 + 9*8+4 + 14 + 1, y2-1),
							  PPSize(14, 10));
		
		button->setText("-");
		container->addControl(button);		

		listBoxInstrumentsSrc = new PPListBox(INSTRUMENT_CHOOSER_LIST_SRC, screen, this, 
											  PPPoint(x2-2, y2 + 10), PPSize((width-10)/2-1,lBoxHeight), true, false, true, true);
		listBoxInstrumentsSrc->setShowIndex(true);
		
		fillInstrumentListBox(listBoxInstrumentsSrc, 
							  tabManager->getModuleEditorFromTabIndex(listBoxModulesSrc->getSelectedIndex()));
		
		if (srcSelection >= 0)
			listBoxInstrumentsSrc->setSelectedIndex(srcSelection, false);
		
		container->addControl(listBoxInstrumentsSrc);
		
		y2+=listBoxInstrumentsSrc->getSize().height+12;
		
		str = "From Smp:";
		
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER6, screen, this, 
							  PPPoint(x2 + 9*8+4, y2-1),
							  PPSize(24, 10));
		
		button->setText("Play");
		container->addControl(button);		

		
		listBoxSamplesSrc = new PPListBox(INSTRUMENT_CHOOSER_LIST_SRC2, screen, this, 
													 PPPoint(x2-2, y2 + 10), PPSize((width-10)/2-1,lBoxHeight), true, false, true, true);
		listBoxSamplesSrc->setShowIndex(true);
		listBoxSamplesSrc->setIndexBaseCount(0);
		
		fillSampleListBox(listBoxSamplesSrc, listBoxInstrumentsSrc->getSelectedIndex(), 
						  tabManager->getModuleEditorFromTabIndex(listBoxModulesSrc->getSelectedIndex()));
		
		if (srcSelection2 >= 0)
			listBoxSamplesSrc->setSelectedIndex(srcSelection2, false);
		
		container->addControl(listBoxSamplesSrc);
		
		lBoxHeight = temp;
	}
	else
	{
		str = "From Ins:";
		
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));

		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_USER10, screen, this, 
										PPPoint(x2 + 9*8+4, y2-1),
										PPSize(14, 10));

		button->setText("+");
		container->addControl(button);		

		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER11, screen, this, 
										PPPoint(x2 + 9*8+4 + 14 + 1, y2-1),
										PPSize(14, 10));

		button->setText("-");
		container->addControl(button);		
		
		listBoxInstrumentsSrc = new PPListBox(INSTRUMENT_CHOOSER_LIST_SRC, screen, this, 
														 PPPoint(x+3, y2 + 10), PPSize((width-10)/2,lBoxHeight), true, false, true, true);
		listBoxInstrumentsSrc->setShowIndex(true);
		
		fillInstrumentListBox(listBoxInstrumentsSrc);
		
		if (srcSelection >= 0)
			listBoxInstrumentsSrc->setSelectedIndex(srcSelection, false);
		
		container->addControl(listBoxInstrumentsSrc);
		
		y2 = y+18;
		lBoxHeight = ((height-100)/2) & ~1;
		
		str = "From Smp:";
		
		x2+=3+(width-10)/2+2;
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER6, screen, this, 
							  PPPoint(x2 + 9*8+4, y2-1),
							  PPSize(24, 10));
		
		button->setText("Play");
		container->addControl(button);		

		listBoxSamplesSrc = new PPListBox(INSTRUMENT_CHOOSER_LIST_SRC2, screen, this, 
													 PPPoint(x+3+(width-10)/2+4, y2 + 10), PPSize((width-10)/2-1,lBoxHeight), true, false, true, true);
		listBoxSamplesSrc->setShowIndex(true);
		listBoxSamplesSrc->setIndexBaseCount(0);
		
		fillSampleListBox(listBoxSamplesSrc, listBoxInstrumentsSrc->getSelectedIndex());
		
		if (srcSelection2 >= 0)
			listBoxSamplesSrc->setSelectedIndex(srcSelection2, false);
		
		container->addControl(listBoxSamplesSrc);
	}
	
	y2+=listBoxSamplesSrc->getSize().height+12 + 3;

	pp_int32 temp = PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(userString1) + 10 +
		PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText1) + 4 + spacer +
		PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText1_2) + 4 + spacer;
	x2 = x + width / 2 - (temp / 2);
	pp_int32 y3 = y2;
	
	PPStaticText* text = new PPStaticText(INSTRUMENT_CHOOSER_USERSTR1, screen, this, PPPoint(x2, y2), userString1, true);
	container->addControl(text);

	PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_USER1, screen, this, 
									PPPoint(text->getLocation().x + text->getSize().width+spacer+10, y2-2), 
									PPSize(PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText1) + 4, 11));
	button->setText(buttonText1);
	container->addControl(button);

	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER3, screen, this, 
									PPPoint(button->getLocation().x + button->getSize().width+spacer, y2-2), 
									PPSize(PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText1_2) + 4, 11));
	button->setText(buttonText1_2);
	container->addControl(button);
	
	// ------------- destination listboxes ---------------------------------
	y2+=12;
	
	container->addControl(new PPSeperator(0, screen, PPPoint(x+4, y2), width-8, container->getColor(), true));
	
	y2+=4;
	temp = y2;
	pp_int32 temp2 = lBoxHeight;

	x2 = x + 5;

	if (tabManager->getNumTabs() > 1)
	{
		listBoxModulesDst = new PPListBox(INSTRUMENT_CHOOSER_LIST_DST3, screen, this, 
													 PPPoint(x2-2, y2 + 10), PPSize((width-10)/2,lBoxHeight), true, false, true, true);
		listBoxModulesDst->setShowIndex(true);
		fillModuleListBox(listBoxModulesDst);
		container->addControl(listBoxModulesDst);

		str = "To Module:";
	
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));

		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_USER5, screen, this, 
										PPPoint(x2 - 2 + (width-10) / 2 - 50 - 1, y2-1),
										PPSize(50, 10));

		button->setText("To<->From");
		container->addControl(button);

		x2+=3+(width-10)/2+2;
		
		lBoxHeight = lBoxHeight / 2 - 6;

		str = "To Ins:";
		
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));

		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER12, screen, this, 
										PPPoint(x2 + 7*8+4, y2-1),
										PPSize(14, 10));

		button->setText("+");
		container->addControl(button);		

		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER13, screen, this, 
										PPPoint(x2 + 7*8+4 + 14 + 1, y2-1),
										PPSize(14, 10));

		button->setText("-");
		container->addControl(button);		
		
		listBoxInstrumentsDst = new PPListBox(INSTRUMENT_CHOOSER_LIST_DST, screen, this, 
														 PPPoint(x2-2, y2 + 10), PPSize((width-10)/2-1,lBoxHeight), true, false, true, true);
		listBoxInstrumentsDst->setShowIndex(true);
		
		fillInstrumentListBox(listBoxInstrumentsDst, 
							  tabManager->getModuleEditorFromTabIndex(listBoxModulesDst->getSelectedIndex()));
		
		container->addControl(listBoxInstrumentsDst);
		
		y2+=listBoxInstrumentsDst->getSize().height+12;
		
		str = "To Smp:";
		
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER7, screen, this, 
							  PPPoint(x2 + 7*8+4, y2-1),
							  PPSize(24, 10));
		
		button->setText("Play");
		container->addControl(button);		

		listBoxSamplesDst = new PPListBox(INSTRUMENT_CHOOSER_LIST_DST2, screen, this, 
													 PPPoint(x2-2, y2 + 10), PPSize((width-10)/2-1,lBoxHeight), true, false, true, true);
		listBoxSamplesDst->setShowIndex(true);
		listBoxSamplesDst->setIndexBaseCount(0);
		
		fillSampleListBox(listBoxSamplesDst, listBoxInstrumentsDst->getSelectedIndex(),
						  tabManager->getModuleEditorFromTabIndex(listBoxModulesDst->getSelectedIndex()));
		
		container->addControl(listBoxSamplesDst);
	}
	else
	{
		str = "To Ins:";
		
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));

		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_USER12, screen, this, 
										PPPoint(x2 + 7*8+4, y2-1),
										PPSize(14, 10));

		button->setText("+");
		container->addControl(button);		

		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER13, screen, this, 
										PPPoint(x2 + 7*8+4 + 14 + 1, y2-1),
										PPSize(14, 10));

		button->setText("-");
		container->addControl(button);		
		
		// put in here
		listBoxInstrumentsDst = new PPListBox(INSTRUMENT_CHOOSER_LIST_DST, screen, this, 
														 PPPoint(x+3, y2 + 10), PPSize((width-10)/2,lBoxHeight), true, false, true, true);
		listBoxInstrumentsDst->setShowIndex(true);
		
		fillInstrumentListBox(listBoxInstrumentsDst);
		
		container->addControl(listBoxInstrumentsDst);
		
		y2 = temp;
		lBoxHeight = temp2;
		
		str = "To Smp:";
		
		//x2 = x + width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(str2) / 2);
		//y2+=listBoxInstruments->getSize().height+16;
		
		x2+=3+(width-10)/2+2;
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_USER7, screen, this, 
							  PPPoint(x2 + 7*8+4, y2-1),
							  PPSize(24, 10));
		
		button->setText("Play");
		container->addControl(button);		

		listBoxSamplesDst = new PPListBox(INSTRUMENT_CHOOSER_LIST_DST2, screen, this, 
													 PPPoint(x+3+(width-10)/2+4, y2 + 10), PPSize((width-10)/2-1,lBoxHeight), true, false, true, true);
		listBoxSamplesDst->setShowIndex(true);
		listBoxSamplesDst->setIndexBaseCount(0);
		
		fillSampleListBox(listBoxSamplesDst, listBoxInstrumentsDst->getSelectedIndex());
		
		container->addControl(listBoxSamplesDst);
	}
	
	y2+=listBoxSamplesDst->getSize().height+12 + 3;

	temp = PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(userString2) + 10 +
		PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText2) + 4 + spacer +
		PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText2_2) + 4 + spacer;
	x2 = x + width / 2 - (temp / 2);
	y3 = y2;
	
	text = new PPStaticText(INSTRUMENT_CHOOSER_USERSTR2, screen, this, PPPoint(x2, y2), userString2, true);
	container->addControl(text);
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER2, screen, this, 
						  PPPoint(text->getLocation().x + text->getSize().width+spacer+10, y2-2), 
						  PPSize(PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText2) + 4, 11));
	button->setText(buttonText2);
	container->addControl(button);

	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER4, screen, this, 
									PPPoint(button->getLocation().x + button->getSize().width+spacer, y2-2), 
									PPSize(PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(buttonText2_2) + 4, 11));
	button->setText(buttonText2_2);
	container->addControl(button);

	// Buttons
	y2+=12;
	
	container->addControl(new PPSeperator(0, screen, PPPoint(x+4, y2), width-8, container->getColor(), true));

	y2+=7;

	//PPButton* button = new PPButton(MESSAGEBOX_BUTTON_USER1, screen, this, PPPoint(x+width/2-75-35, y2), PPSize(70, 11));
	//button->setText(buttonText1);
	//container->addControl(button);

	//button = new PPButton(MESSAGEBOX_BUTTON_USER2, screen, this, PPPoint(x+width/2-35, y2), PPSize(70, 11));
	//button->setText(buttonText2);
	//container->addControl(button);
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_CANCEL, screen, this, PPPoint(x+width/2-35, y2), PPSize(70, 11));
	button->setText("Cancel");
	container->addControl(button);
		
	instrumentChooser = container;
}

void Tracker::initAdvEdit()
{	
	const pp_int32 buttonIDs[4] =  {PP_MESSAGEBOX_BUTTON_USER1, 
									PP_MESSAGEBOX_BUTTON_USER2, 
									PP_MESSAGEBOX_BUTTON_USER3, 
									PP_MESSAGEBOX_BUTTON_USER4};

	const char* buttonTexts[4] =  {"Track",
								   "Pattern",
								   "Song",
								   "Block"};
									   

	char buffer[100];

	sprintf(buffer, "Remap ins. %x to %x", listBoxInstruments->getSelectedIndex()+1, 1);

	//initInstrumentChooser(MESSAGEBOX_INSREMAP, "", "Instrument remapping", "",buffer, "",listBoxInstruments->getSelectedIndex());
	// old instrument chooser / just inserted ;)
	{
		PPString userString = buffer;
		pp_int32 srcSelection = listBoxInstruments->getSelectedIndex();

		if (instrumentChooser)
		{
			delete instrumentChooser;
			instrumentChooser = NULL;
		}
		
		const pp_int32 height = 222;
		const pp_int32 width = (screen->getWidth()-10) > 480 ? 480 : (screen->getWidth()-10);
		
		pp_int32 x = screen->getWidth() / 2 - width/2;
		pp_int32 y = screen->getHeight() / 2 - height/2;
		
		PPMessageBoxContainer* container = new PPMessageBoxContainer(MESSAGEBOX_INSREMAP, screen, this, PPPoint(x, y), PPSize(width,height), "Instrument remapping");
		
		const PPString str = "From:";
		
		pp_int32 x2 = x + 5;
		pp_int32 y2 = y + 20;
		
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str, true));
		
		const pp_int32 lBoxHeight = height-90;
		
		PPListBox* listBoxInstruments = new PPListBox(INSTRUMENT_CHOOSER_LIST_SRC, screen, this, PPPoint(x+3, y2 + 12), PPSize((width-10)/2,lBoxHeight), true, false, true, true);
		listBoxInstruments->setShowIndex(true);
		
		fillInstrumentListBox(listBoxInstruments);
		
		if (srcSelection >= 0)
			listBoxInstruments->setSelectedIndex(srcSelection, false);
		
		container->addControl(listBoxInstruments);
		
		PPString str2 = "To:";
		
		x2+=3+(width-10)/2+2;
		container->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2), str2, true));
		
		listBoxInstruments = new PPListBox(INSTRUMENT_CHOOSER_LIST_DST, screen, this, PPPoint(x+3+(width-10)/2+4, y2 + 12), PPSize((width-10)/2,lBoxHeight), true, false, true, true);
		listBoxInstruments->setShowIndex(true);
		
		fillInstrumentListBox(listBoxInstruments);
		
		container->addControl(listBoxInstruments);
		
		y2+=listBoxInstruments->getSize().height+12 + 6 + 30;
		
		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_USER1, screen, this, PPPoint(x+width/2-65, y2), PPSize(60, 11));
		button->setText("");
		container->addControl(button);
		
		button = new PPButton(PP_MESSAGEBOX_BUTTON_CANCEL, screen, this, PPPoint(x+width/2+5, y2), PPSize(60, 11));
		button->setText("Cancel");
		container->addControl(button);
		
		x2 = x + width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(userString) / 2);
		
		container->addControl(new PPStaticText(INSTRUMENT_CHOOSER_USERSTR1, screen, this, PPPoint(x2, y2 - 21), userString, true));
		
		instrumentChooser = container;
	}

	// modify current instrument chooser dialog
	if (instrumentChooser)
	{
		const pp_int32 spacer = 10;
	
		PPControl* ctrl = instrumentChooser->getControlByID(PP_MESSAGEBOX_BUTTON_USER1);
		pp_int32 y2 = ctrl->getLocation().y - 16;

		pp_int32 buttonWidth = ctrl->getSize().width;		
		pp_int32 buttonHeight = ctrl->getSize().height;		 
		pp_int32 cx = ((buttonWidth) * 4 + spacer*3)/2;
	
		bool b = instrumentChooser->removeControl(ctrl);
		ASSERT(b);
		b = instrumentChooser->removeControl(instrumentChooser->getControlByID(PP_MESSAGEBOX_BUTTON_CANCEL));
		ASSERT(b);
		
		pp_int32 width = instrumentChooser->getSize().width;
	
		pp_int32 x = instrumentChooser->getLocation().x + width/2 - cx;
		
		for (pp_int32 i = 0; i < 4; i++)
		{
			PPButton* button = new PPButton(buttonIDs[i], screen, this, PPPoint(x, y2), PPSize(buttonWidth, buttonHeight));
			button->setText(buttonTexts[i]);
			instrumentChooser->addControl(button);
			
			x+= buttonWidth + spacer;
		}
		
		x-=buttonWidth + spacer;
		y2+=6 + buttonHeight;
		
		PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_CANCEL, screen, this, PPPoint(x, y2), PPSize(buttonWidth, buttonHeight));
		button->setText("Cancel");
		instrumentChooser->addControl(button);
		
		ctrl = instrumentChooser->getControlByID(INSTRUMENT_CHOOSER_USERSTR1);
		y2 = ctrl->getLocation().y - 11;
		x = ctrl->getLocation().x;
		ctrl->setLocation(PPPoint(x, y2));
	
	}
	
	screen->setModalControl(instrumentChooser);				
}

void Tracker::initInputContainerDefault(pp_int32 x, pp_int32 y)
{
	PPContainer* container = new PPContainer(CONTAINER_INPUTDEFAULT, screen, this, PPPoint(0, y), PPSize(screen->getWidth(),INPUTCONTAINERHEIGHT_DEFAULT), false);
	container->setColor(TrackerConfig::colorThemeMain);

	PPButton* button;

	pp_int32 bWidth = 11;
	pp_int32 bHeight = 12;
	pp_int32 j,i;
	for (j = 0; j < 2; j++)
		for (i = 0; i < 8; i++)
		{
			char buffer[4];
			PPTools::convertToHex(buffer, j*8+i, 1);
			button = new PPButton(INPUT_BUTTON_0 + j*8+i, screen, inputControlListener, PPPoint(x+2 + i*(bWidth), y + 2 + j*(bHeight)), PPSize(bWidth, bHeight), false, true, true);
			button->setText(buffer);
			
			container->addControl(button);
		}
	
	pp_int32 y2 = y + /*2*(bHeight+1)+*/2;
	pp_int32 x2 = x + 3 + 8*(bWidth);
				
	bWidth = 22;

	pp_int32 x3 = x2-1;
	button = new PPButton(INPUT_BUTTON_DEL, screen, inputControlListener, PPPoint(x3, y2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Del");
	container->addControl(button);
	y2+=button->getSize().height;

	button = new PPButton(INPUT_BUTTON_INS, screen, inputControlListener, PPPoint(x3, y2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Ins");
	container->addControl(button);
	y2+=button->getSize().height;

	button = new PPButton(INPUT_BUTTON_BACK, screen, inputControlListener, PPPoint(x3, y2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Back");
	container->addControl(button);

	x2+=button->getSize().width;

	x3+=button->getSize().width;

	x3 = x+2;
	
	bWidth = 44;
	button = new PPButton(INPUT_BUTTON_INSLINE, screen, inputControlListener, PPPoint(x3, y2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Insline");
	container->addControl(button);	

	x3+=button->getSize().width;
	button = new PPButton(INPUT_BUTTON_BACKLINE, screen, inputControlListener, PPPoint(x3, y2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Backline");
	container->addControl(button);	

	// piano test
	PianoControl* pianoControl = new PianoControl(PIANO_CONTROL, screen, inputControlListener, 
												  PPPoint(x2, y+1), PPSize(screen->getWidth() - 2 - x2 - (21+2), 25+12), ModuleEditor::MAX_NOTE); 
	// show C-3
	pianoControl->assureNoteVisible(12*4);
	pianoControl->setBorderColor(TrackerConfig::colorThemeMain);
	pianoControl->setMode(PianoControl::ModePlay);
	
	container->addControl(pianoControl);
	
	x2+=1+pianoControl->getSize().width;
	
	button = new PPButton(INPUT_BUTTON_KEYOFF, screen, inputControlListener, PPPoint(x2, y+2), PPSize(22, 18), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Off");
	container->addControl(button);

	button = new PPButton(INPUT_BUTTON_EXPAND, screen, inputControlListener, PPPoint(x2, y+2+18), PPSize(22, 17), false, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("Exp.");
	container->addControl(button);

	inputContainerDefault = inputContainerCurrent = container;
	
	screen->addControl(container);
}

void Tracker::initInputContainerExtended(pp_int32 x, pp_int32 y)
{
	PPContainer* container = new PPContainer(CONTAINER_INPUTEXTENDED, screen, this, PPPoint(0, y), PPSize(screen->getWidth(),INPUTCONTAINERHEIGHT_EXTENDED), false);
	container->setColor(TrackerConfig::colorThemeMain);

	// First line of keys
	PPButton* button;

	pp_int32 bWidth = 11;
	pp_int32 bHeight = 12;
	pp_int32 j,i, offset = 0;
	for (i = 0; i < (signed)sizeof(keyLine_0_lowerCase); i++)
	{
		button = new PPButton(keyLineIDs_0[i], screen, inputControlListener, PPPoint(x+2 + i*(bWidth)+offset, y + 2), PPSize(bWidth, bHeight), false, true, true);
		button->setText(keyLine_0_lowerCase[i]);
		container->addControl(button);
	}

	offset = bWidth+(bWidth>>1)-1;
	pp_int32 y2 = y+bHeight;
	j = x + 2 + offset;
	for (i = 0; i < (signed)sizeof(keyLine_1_lowerCase); i++)
	{
		button = new PPButton(keyLineIDs_1[i], screen, inputControlListener, PPPoint(j, y2 + 2), PPSize(keyLineSizes_1[i], bHeight), false, true, true);
		button->setText(keyLine_1_lowerCase[i]);
		container->addControl(button);
		j+=keyLineSizes_1[i];
	}
	y2+=bHeight;

	bHeight++;
	offset = (bWidth*2)-4;
	for (i = 0; i < (signed)sizeof(keyLine_2_lowerCase); i++)
	{
		button = new PPButton(keyLineIDs_2[i], screen, inputControlListener, PPPoint(x+2 + i*(bWidth)+offset, y2 + 2), PPSize(bWidth, bHeight), false, true, true);
		button->setText(keyLine_2_lowerCase[i]);
		container->addControl(button);
	}

	y2+=bHeight;

	bHeight--;
	offset = (bWidth+2)+1;
	for (i = 0; i < (signed)sizeof(keyLine_3_lowerCase); i++)
	{
		button = new PPButton(keyLineIDs_3[i], screen, inputControlListener, PPPoint(x+2 + i*(bWidth)+offset, y2 + 2), PPSize(bWidth, bHeight), false, true, true);
		button->setText(keyLine_3_lowerCase[i]);
		container->addControl(button);
	}

	y2 = y + bHeight;
	bWidth = 15;
	offset = 0;
	button = new PPButton(INPUT_BUTTON_TAB, screen, inputControlListener, PPPoint(x+2 + offset, y2 + 2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("->");
	container->addControl(button);

	y2 += bHeight;
	bHeight++;
	bWidth = 18;
	offset = 0;
	button = new PPButton(INPUT_BUTTON_CAPSLOCK, screen, inputControlListener, PPPoint(x+2 + offset, y2 + 2), PPSize(bWidth, bHeight), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Cap");
	container->addControl(button);
	y2 += bHeight;
	bHeight--;

	bWidth = 14;
	offset = 0;
	button = new PPButton(INPUT_BUTTON_LSHIFT, screen, inputControlListener, PPPoint(x+2 + offset, y2 + 2), PPSize(bWidth, bHeight), false, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Sh");
	container->addControl(button);

	bWidth = 23;
	pp_int32 x3 = x + 145;
	y2 = y;
	button = new PPButton(INPUT_BUTTON_BACK, screen, inputControlListener, PPPoint(x3, y2+2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Back");
	container->addControl(button);

	y2+=bHeight;

	x3+=7;
	bWidth = 16;
	bHeight*=2;
	bHeight+=1;

	button = new PPButton(INPUT_BUTTON_ENTER, screen, inputControlListener, PPPoint(x3, y2+2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Ret");
	container->addControl(button);

	y2+=bHeight;
	bHeight = 12;
	bWidth = 31;
	x3 = x + 135;
	button = new PPButton(INPUT_BUTTON_SPACE, screen, inputControlListener, PPPoint(x3+2 + offset, y2 + 2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Space");
	container->addControl(button);

	x3 += bWidth+2;

	PianoControl* pianoControl = new PianoControl(PIANO_CONTROL, screen, inputControlListener, 
												  PPPoint(x3+2, y+1), PPSize(screen->getWidth() - 3 - x3, 25+12), ModuleEditor::MAX_NOTE); 
	// show C-3
	pianoControl->assureNoteVisible(12*4);
	pianoControl->setBorderColor(TrackerConfig::colorThemeMain);
	pianoControl->setMode(PianoControl::ModePlay);
	
	container->addControl(pianoControl);

	// more stuff
	bWidth = 18;

	button = new PPButton(INPUT_BUTTON_DEL, screen, inputControlListener, PPPoint(x3+2, y2+2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Del");
	container->addControl(button);
	x3+=button->getSize().width;

	button = new PPButton(INPUT_BUTTON_INS, screen, inputControlListener, PPPoint(x3+2, y2+2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Ins");
	container->addControl(button);
	x3+=button->getSize().width;

	bWidth = 37;
	button = new PPButton(INPUT_BUTTON_INSLINE, screen, inputControlListener, PPPoint(x3+2, y2+2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Insline");
	container->addControl(button);	

	bWidth = 41;
	x3+=button->getSize().width;
	button = new PPButton(INPUT_BUTTON_BACKLINE, screen, inputControlListener, PPPoint(x3+2, y2+2), PPSize(bWidth, bHeight), false, true, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Backline");
	container->addControl(button);		

	x3+=button->getSize().width+1;
	bWidth = 33;
	button = new PPButton(INPUT_BUTTON_SHRINK, screen, inputControlListener, PPPoint(x3+2, y2+2), PPSize(bWidth, bHeight), false, true);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Shrink");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	container->addControl(button);		

	container->hide(true);

	inputContainerExtended = container;
	
	screen->addControl(container);
}

void Tracker::moveInputControls(pp_uint32 deltay)
{
	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INPUTDEFAULT));
	ASSERT(container);
	
	PPPoint p(0, deltay);
	container->move(p);

	container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INPUTEXTENDED));
	ASSERT(container);

	container->move(p);

	rearrangePatternEditorControlOrInstrumentContainer();
}

void Tracker::hideInputControl(bool bHide/* = true*/)
{
	if (inputContainerCurrent && bHide)
	{
		inputContainerCurrent->show(false);
	}
	else if (inputContainerCurrent)
	{
		inputContainerCurrent->show(true);
	}
}
