/*
 *  tracker/TitlePageManager.cpp
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

#include "TitlePageManager.h"
#include "Screen.h"
#include "Container.h"
#include "StaticText.h"
#include "ListBox.h"
#include "PeakLevelControl.h"
#include "PPUIConfig.h"
#include "ControlIDs.h"

TitlePageManager::TitlePageManager(PPScreen& screen) :
	screen(screen)
{
}

TitlePageManager::Pages TitlePageManager::getCurrentTitlePage()
{
	PPContainer* container = static_cast<PPContainer*>(screen.getControlByID(CONTAINER_ABOUT));
	PPButton* buttonShowPeak = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWPEAK));
	PPButton* buttonShowTime = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTIME));
	//PPButton* buttonShowTitle = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTITLE));

	if (buttonShowPeak->isPressed())
		return PagePeak;
	else if (buttonShowTime->isPressed())
		return PageTime;
		
	return PageTitle;
}

void TitlePageManager::showTitlePage(Pages page, bool update/* = true*/)
{
	switch (page)
	{
		case PageTitle:
			showSongTitleEditField(update);
			break;
		case PageTime:
			showTimeCounter(update);
			break;
		case PagePeak:
			showPeakControl(update);
			break;
	}
}

void TitlePageManager::showSongTitleEditField(bool update/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(screen.getControlByID(CONTAINER_ABOUT));
	PeakLevelControl* peakLevelControl = static_cast<PeakLevelControl*>(screen.getControlByID(PEAKLEVEL_CONTROL));
	PPButton* buttonShowPeak = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWPEAK));
	PPButton* buttonShowTime = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTIME));
	PPButton* buttonShowTitle = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTITLE));
	PPButton* buttonTimeEstimate = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_ESTIMATESONGLENGTH));
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ABOUT_HEADING));
	PPStaticText* text2 = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ABOUT_TIME));
	
	static_cast<PPListBox*>(container->getControlByID(LISTBOX_SONGTITLE))->hide(false);
	peakLevelControl->hide(true);
	text2->hide(true);
	buttonTimeEstimate->hide(true);

	buttonShowPeak->setPressed(false);
	buttonShowTime->setPressed(false);
	buttonShowTitle->setPressed(true);
#ifdef __LOWRES__
	text->setText("Title:");
#else
	text->setText("Song Title:");
#endif
	text->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));

	if (update)
		screen.paintControl(container);
}

void TitlePageManager::showTimeCounter(bool update/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(screen.getControlByID(CONTAINER_ABOUT));
	PeakLevelControl* peakLevelControl = static_cast<PeakLevelControl*>(screen.getControlByID(PEAKLEVEL_CONTROL));
	PPButton* buttonShowPeak = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWPEAK));
	PPButton* buttonShowTime = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTIME));
	PPButton* buttonShowTitle = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTITLE));
	PPButton* buttonTimeEstimate = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_ESTIMATESONGLENGTH));
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ABOUT_HEADING));
	PPStaticText* text2 = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ABOUT_TIME));
	
	static_cast<PPListBox*>(container->getControlByID(LISTBOX_SONGTITLE))->hide(true);
	peakLevelControl->hide(true);
	text2->hide(false);
	buttonTimeEstimate->hide(false);

	buttonShowPeak->setPressed(false);
	buttonShowTime->setPressed(true);
	buttonShowTitle->setPressed(false);
	text->setText("Time:");
	text->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));

	if (update)
		screen.paintControl(container);
}

void TitlePageManager::showPeakControl(bool update/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(screen.getControlByID(CONTAINER_ABOUT));
	PeakLevelControl* peakLevelControl = static_cast<PeakLevelControl*>(screen.getControlByID(PEAKLEVEL_CONTROL));
	PPButton* buttonShowPeak = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWPEAK));
	PPButton* buttonShowTime = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTIME));
	PPButton* buttonShowTitle = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_SHOWTITLE));
	PPButton* buttonTimeEstimate = static_cast<PPButton*>(container->getControlByID(BUTTON_ABOUT_ESTIMATESONGLENGTH));
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ABOUT_HEADING));
	PPStaticText* text2 = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ABOUT_TIME));
	
	static_cast<PPListBox*>(container->getControlByID(LISTBOX_SONGTITLE))->hide(true);
	peakLevelControl->hide(false);
	text2->hide(true);
	buttonTimeEstimate->hide(true);

	buttonShowPeak->setPressed(true);
	buttonShowTime->setPressed(false);
	buttonShowTitle->setPressed(false);
#ifdef __LOWRES__
	text->setText("Peak:");
#else
	text->setText("Peak level:");
#endif
	text->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));

	if (update)
		screen.paintControl(container);
}

void TitlePageManager::setPeakControlHeadingColor(const PPColor& color, bool update/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(screen.getControlByID(CONTAINER_ABOUT));
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_ABOUT_HEADING));
	text->setColor(color);
	
	if (update)
		screen.paintControl(container);
}
