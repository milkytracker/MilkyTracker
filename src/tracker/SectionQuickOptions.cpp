/*
 *  SectionQuickOptions.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.05.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SectionQuickOptions.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "PlayerController.h"
#include "PlayerMaster.h"
#include "PlayerSTD.h"
#include "CheckBox.h"
#include "RadioGroup.h"
#include "StaticText.h"
#include "Seperator.h"
#include "Container.h"
#include "PanningSettingsContainer.h"

#include "ControlIDs.h"

enum ControlIDs
{
	CONTAINER_QUICKOPTIONS					= 7000,
	QUICKOPTIONS_BUTTON_EXIT,
	QUICKOPTIONS_RADIOGROUP_PLAYBACKMODE,

	QUICKOPTIONS_STATICTEXT_ALLOW8XX,
	QUICKOPTIONS_CHECKBOX_ALLOW8XX,
	QUICKOPTIONS_STATICTEXT_ALLOWE8X,
	QUICKOPTIONS_CHECKBOX_ALLOWE8X,
	QUICKOPTIONS_STATICTEXT_PTPERIODRANGE,
	QUICKOPTIONS_CHECKBOX_PTPERIODRANGE,
	QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING,
	QUICKOPTIONS_BUTTON_SETDEFAULTPANNING,
	QUICKOPTIONS_STATICTEXT_KEEPOPTIONS,
	QUICKOPTIONS_CHECKBOX_KEEPOPTIONS,

	QUICKOPTIONS_CHECKBOX_FOLLOWSONG
};

SectionQuickOptions::SectionQuickOptions(Tracker& theTracker) :
	SectionUpperLeft(theTracker),
	checkBoxKeepSettings(NULL),
	panningSettingsContainer(NULL),
	oldPanning(NULL)
{
}

SectionQuickOptions::~SectionQuickOptions()
{
	delete[] oldPanning;
	delete panningSettingsContainer;
}

pp_int32 SectionQuickOptions::handleEvent(PPObject* sender, PPEvent* event)
{
	PPScreen* screen = tracker.screen;

	bool b;

	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case QUICKOPTIONS_BUTTON_EXIT:
				if (event->getID() != eCommand)
					break;

				show(false);
				break;

			case QUICKOPTIONS_CHECKBOX_ALLOW8XX:
				if (event->getID() != eCommand)
					break;

				b = tracker.playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanning8xx);
				tracker.playerController->enablePlayModeOption(PlayerController::PlayModeOptionPanning8xx, !b);
				break;

			case QUICKOPTIONS_CHECKBOX_ALLOWE8X:
				if (event->getID() != eCommand)
					break;

				b = tracker.playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanningE8x);
				tracker.playerController->enablePlayModeOption(PlayerController::PlayModeOptionPanningE8x, !b);
				break;

			case QUICKOPTIONS_CHECKBOX_PTPERIODRANGE:
				if (event->getID() != eCommand)
					break;

				b = tracker.playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionForcePTPitchLimit);
				tracker.playerController->enablePlayModeOption(PlayerController::PlayModeOptionForcePTPitchLimit, !b);
				break;
			
			case QUICKOPTIONS_BUTTON_SETDEFAULTPANNING:
			{
				if (event->getID() != eCommand)
					break;
				
				saveOldPanning();
				
				for (pp_int32 i = 0; i < TrackerConfig::numPlayerChannels; i++)
					panningSettingsContainer->setPanning((pp_uint8)i, tracker.playerController->getPanning((pp_uint8)i), false);
					
				panningSettingsContainer->show(true);
				break;
			}
				
			case QUICKOPTIONS_CHECKBOX_KEEPOPTIONS:
				if (event->getID() != eCommand)
					break;

				if (checkBoxKeepSettings->isChecked())
					tracker.showMessageBoxSized(MESSAGEBOX_UNIVERSAL, "Play mode auto-switching is now OFF\nRemember, these settings will now\napply to all loaded modules.", Tracker::MessageBox_OK);
				break;
		}
		
	}
	else if (event->getID() == eSelection)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case QUICKOPTIONS_RADIOGROUP_PLAYBACKMODE:
				switch (reinterpret_cast<PPRadioGroup*>(sender)->getChoice())
				{
					case 0:
						tracker.playerController->switchPlayMode(PlayerController::PlayMode_FastTracker2, !keepSettings());
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE)->enable(false);
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_PTPERIODRANGE)->enable(false);
						
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING)->enable(false);
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_BUTTON_SETDEFAULTPANNING)->enable(false);
						break;
					case 1:
						tracker.playerController->switchPlayMode(PlayerController::PlayMode_ProTracker2, !keepSettings());						
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE)->enable(true);
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_PTPERIODRANGE)->enable(true);

						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING)->enable(true);
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_BUTTON_SETDEFAULTPANNING)->enable(true);
						break;
					case 2:
						tracker.playerController->switchPlayMode(PlayerController::PlayMode_ProTracker3, !keepSettings());						
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE)->enable(true);
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_PTPERIODRANGE)->enable(true);

						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING)->enable(true);
						static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_BUTTON_SETDEFAULTPANNING)->enable(true);
						break;
					default:
						ASSERT(false);
				}
				
				update();
				break;
		}
		
	}
	else if (event->getID() == eValueChanged && 
			 reinterpret_cast<PanningSettingsContainer*>(sender) == panningSettingsContainer)
	{
		pp_uint32 i = *(reinterpret_cast<pp_uint32*>(event->getDataPtr()));
		
		tracker.playerController->setPanning((pp_uint8)i, panningSettingsContainer->getPanning(i));
	}
	else if (event->getID() == eCanceled)
	{
		if (reinterpret_cast<PanningSettingsContainer*>(sender) == panningSettingsContainer)
		{	
			restoreOldPanning();
		}
	}

	return 0;
}

void SectionQuickOptions::init(pp_int32 px, pp_int32 py)
{
	PPCheckBox* checkBox;
	PPScreen* screen = tracker.screen;

	if (panningSettingsContainer == NULL)
		panningSettingsContainer = new PanningSettingsContainer(screen, this, TrackerConfig::numPlayerChannels);

	PPContainer* container = new PPContainer(CONTAINER_QUICKOPTIONS, tracker.screen, this, PPPoint(px, py), PPSize(320,UPPERLEFTSECTIONHEIGHT), false);
	container->setColor(TrackerConfig::colorThemeMain);	
	tracker.screen->addControl(container);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(px + 2, py + 2), "Quick Options (experts only)", true, true));

	PPSize size = container->getSize();

	pp_int32 buttonWidth = 8*4+4;
	pp_int32 buttonHeight = 11;
	
	pp_int32 x = px+container->getSize().width-(buttonWidth+4);
	pp_int32 y = py+container->getSize().height-(buttonHeight+4);

	container->addControl(new PPSeperator(0, screen, PPPoint(x - 6, y - 4), 4 + buttonHeight + 3, TrackerConfig::colorThemeMain, false));
	container->addControl(new PPSeperator(0, screen, PPPoint(px + 2, y - 4), container->getSize().width - 4, TrackerConfig::colorThemeMain, true));

	PPButton* button = new PPButton(QUICKOPTIONS_BUTTON_EXIT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Exit");
	container->addControl(button);

	pp_int32 x2 = px+4;
	pp_int32 y2 = py+4+12;

	y+=2;
	container->addControl(new PPStaticText(QUICKOPTIONS_STATICTEXT_KEEPOPTIONS, NULL, NULL, PPPoint(x2 + 2, y), "Keep settings (auto-adjust OFF)", true));
	checkBoxKeepSettings = new PPCheckBox(QUICKOPTIONS_CHECKBOX_KEEPOPTIONS, screen, this, PPPoint(x2 + 2 + 31*8 + 4, y-1));
	checkBoxKeepSettings->checkIt(false);
	container->addControl(checkBoxKeepSettings);	

	// add playback modes
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "Playback mode:", true));

	y2+=10;

	PPRadioGroup* radioGroup = new PPRadioGroup(QUICKOPTIONS_RADIOGROUP_PLAYBACKMODE, screen, this, PPPoint(x2+2, y2), PPSize(17*8, 3*14));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem("Fasttracker 2.x");
	radioGroup->addItem("Protracker 2.x");
	radioGroup->addItem("Protracker 3.x");

	container->addControl(radioGroup);		

	y2+=radioGroup->getSize().height;

	x2 += radioGroup->getSize().width+6;
	y2 = py + 16;
	container->addControl(new PPSeperator(0, screen, PPPoint(x2 - 4, y2 - 2), container->getLocation().y + container->getSize().height - y2 - 17, TrackerConfig::colorThemeMain, false));

	y2 = py+4+12;
	x2+=2;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2, y2), "Advanced:", true));

	y2+=15;

	container->addControl(new PPStaticText(QUICKOPTIONS_STATICTEXT_ALLOW8XX, NULL, NULL, PPPoint(x2, y2), "Allow 8xx panning", true));
	checkBox = new PPCheckBox(QUICKOPTIONS_CHECKBOX_ALLOW8XX, screen, this, PPPoint(x2 + 19*8 + 4, y2-1));
	container->addControl(checkBox);

	y2+=13;

	container->addControl(new PPStaticText(QUICKOPTIONS_STATICTEXT_ALLOWE8X, NULL, NULL, PPPoint(x2, y2), "Allow E8x panning", true));
	checkBox = new PPCheckBox(QUICKOPTIONS_CHECKBOX_ALLOWE8X, screen, this, PPPoint(x2 + 19*8 + 4, y2-1));
	container->addControl(checkBox);

	y2+=13;

	container->addControl(new PPStaticText(QUICKOPTIONS_STATICTEXT_PTPERIODRANGE, NULL, NULL, PPPoint(x2, y2), "PT 3 octaves limit", true));
	checkBox = new PPCheckBox(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE, screen, this, PPPoint(x2 + 19*8 + 4, y2-1));
	container->addControl(checkBox);

	y2+=13;

	container->addControl(new PPStaticText(QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING, NULL, NULL, PPPoint(x2, y2), "Default panning", true));

	button = new PPButton(QUICKOPTIONS_BUTTON_SETDEFAULTPANNING, screen, this, PPPoint(x2 + 19*8 - 18, y2-1), PPSize(4*8,buttonHeight));
	button->setText("Set");
	container->addControl(button);

	sectionContainer = container;
	
	initialised = true;

	showSection(false);
}

void SectionQuickOptions::updateControlStates()
{
	switch (tracker.playerController->getPlayMode())
	{
		case PlayerBase::PlayMode_ProTracker2:
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_PTPERIODRANGE)->enable(true);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE)->enable(true);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING)->enable(true);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_BUTTON_SETDEFAULTPANNING)->enable(true);
			break;
			
		case PlayerBase::PlayMode_ProTracker3:
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_PTPERIODRANGE)->enable(true);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE)->enable(true);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING)->enable(true);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_BUTTON_SETDEFAULTPANNING)->enable(true);
			break;
			
		case PlayerBase::PlayMode_FastTracker2:
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_PTPERIODRANGE)->enable(false);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE)->enable(false);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_STATICTEXT_SETDEFAULTPANNING)->enable(false);
			static_cast<PPContainer*>(sectionContainer)->getControlByID(QUICKOPTIONS_BUTTON_SETDEFAULTPANNING)->enable(false);
			break;
			
		default:
			ASSERT(false);
	}
}

void SectionQuickOptions::show(bool bShow) 
{ 
	if (bShow)
	{
		updateControlStates();
	}
	
	SectionUpperLeft::show(bShow); 	
}

void SectionQuickOptions::update(bool repaint/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	PPRadioGroup* radioGroup = static_cast<PPRadioGroup*>(container->getControlByID(QUICKOPTIONS_RADIOGROUP_PLAYBACKMODE));
	ASSERT(radioGroup);
	
	switch (tracker.playerController->getPlayMode())
	{
		case PlayerBase::PlayMode_ProTracker2:
			radioGroup->setChoice(1);
			break;

		case PlayerBase::PlayMode_ProTracker3:
			radioGroup->setChoice(2);
			break;

		case PlayerBase::PlayMode_FastTracker2:
			radioGroup->setChoice(0);
			break;
			
		default:
			ASSERT(false);
	}
	
	PPCheckBox* checkBox = static_cast<PPCheckBox*>(container->getControlByID(QUICKOPTIONS_CHECKBOX_ALLOW8XX));
	checkBox->checkIt(tracker.playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanning8xx));

	checkBox = static_cast<PPCheckBox*>(container->getControlByID(QUICKOPTIONS_CHECKBOX_ALLOWE8X));
	checkBox->checkIt(tracker.playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanningE8x));

	checkBox = static_cast<PPCheckBox*>(container->getControlByID(QUICKOPTIONS_CHECKBOX_PTPERIODRANGE));
	checkBox->checkIt(tracker.playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionForcePTPitchLimit));

	//checkBoxFollowSong->checkIt(tracker.getFollowSong());

	if (repaint)
		tracker.screen->paintControl(container);
}

void SectionQuickOptions::notifyTabSwitch()
{
	if (sectionContainer->isVisible())
	{
		updateControlStates();
		update(false);
	}
}

bool SectionQuickOptions::setKeepSettings(bool b)
{
	if (checkBoxKeepSettings)
	{
		checkBoxKeepSettings->checkIt(b);
		return true;
	}
	
	return false;
}

bool SectionQuickOptions::keepSettings()
{
	if (checkBoxKeepSettings)
	{
		return checkBoxKeepSettings->isChecked();
	}
	
	return false;
}

void SectionQuickOptions::saveOldPanning()
{
	if (oldPanning == NULL)
		oldPanning = new pp_uint8[TrackerConfig::numPlayerChannels];

	for (pp_int32 i = 0; i < TrackerConfig::numPlayerChannels; i++)
		oldPanning[i] = tracker.playerController->getPanning((pp_uint8)i);
}

void SectionQuickOptions::restoreOldPanning()
{
	if (oldPanning == NULL)
		return;
		
	for (pp_int32 i = 0; i < TrackerConfig::numPlayerChannels; i++)	
		tracker.playerController->setPanning((pp_uint8)i, oldPanning[i]);
}

