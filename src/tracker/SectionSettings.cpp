/*
 *  tracker/SectionSettings.cpp
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
 *  SectionSettings.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 13 2005.
 *
 */

#include "SectionSettings.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "ModuleEditor.h"
#include "PlayerMaster.h"
#include "ResamplerHelper.h"
#include "PlayerController.h"
#include "SystemMessage.h"

#include "PPUIConfig.h"
#include "CheckBox.h"
#include "CheckBoxLabel.h"
#include "ListBox.h"
#include "RadioGroup.h"
#include "Seperator.h"
#include "Slider.h"
#include "StaticText.h"
#include "TransparentContainer.h"
#include "PatternEditorControl.h"
#include "DialogWithValues.h"
#include "DialogListBox.h"
#include "Graphics.h"

#include "PatternTools.h"
#include "TrackerSettingsDatabase.h"
#include "SectionSamples.h"
#include "ColorPaletteContainer.h"
#include "ColorExportImport.h"
// OS Interface
#include "PPOpenPanel.h"
#include "PPSavePanel.h"

#include "FileExtProvider.h"

#include "ControlIDs.h"

#ifdef __LOWRES__
#define SECTIONHEIGHT		148
#else
#define SECTIONHEIGHT		118
#endif
#define UPPERFRAMEHEIGHT	118

// small custom button class which will be used to show a color preview
class PPColPrevButton : public PPButton
{
public:
	PPColPrevButton(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size) :
		PPButton(id, parentScreen, eventListener, location, size, false, false, false)
	{
	}

	virtual void paint(PPGraphicsAbstract* g)
	{
		PPButton::paint(g);

		PPFont* font = PPFont::getFont(PPFont::FONT_TINY);
		g->setFont(font);

		PPPoint p = this->getLocation();
		p.x+=1;
		p.y+=2;

		g->setColor(255^getColor()->r, 255^getColor()->g, 255^getColor()->b);

		char buffer[100];

		sprintf(buffer, "R:%02X", getColor()->r);
		g->drawString(buffer, p.x, p.y, false);
		p.y+=font->getCharHeight();
		sprintf(buffer, "G:%02X", getColor()->g);
		g->drawString(buffer, p.x, p.y, false);
		p.y+=font->getCharHeight();
		sprintf(buffer, "B:%02X", getColor()->b);
		g->drawString(buffer, p.x, p.y, false);
	}
};

// settings
enum ControlIDs
{
	PAGE_BUTTON_0						= 50,
	PAGE_BUTTON_1,
	PAGE_BUTTON_2,
	PAGE_BUTTON_3,
	PAGE_BUTTON_4,
	PAGE_BUTTON_5,
	PAGE_BUTTON_6,
	PAGE_BUTTON_7,

	SUBPAGE_BUTTON_LEFT_0,
	SUBPAGE_BUTTON_LEFT_1,
	SUBPAGE_BUTTON_LEFT_2,
	SUBPAGE_BUTTON_LEFT_3,
	SUBPAGE_BUTTON_LEFT_4,
	SUBPAGE_BUTTON_LEFT_5,
	SUBPAGE_BUTTON_LEFT_6,
	SUBPAGE_BUTTON_LEFT_7,

	SUBPAGE_BUTTON_RIGHT_0,
	SUBPAGE_BUTTON_RIGHT_1,
	SUBPAGE_BUTTON_RIGHT_2,
	SUBPAGE_BUTTON_RIGHT_3,
	SUBPAGE_BUTTON_RIGHT_4,
	SUBPAGE_BUTTON_RIGHT_5,
	SUBPAGE_BUTTON_RIGHT_6,
	SUBPAGE_BUTTON_RIGHT_7,

	RADIOGROUP_SETTINGS_PAGE,

	BUTTON_SETTINGS_OK,
	BUTTON_SETTINGS_APPLY,
	BUTTON_SETTINGS_CANCEL,

	// Page I
	RADIOGROUP_SETTINGS_FREQTAB,
	STATICTEXT_SETTINGS_BUFFERSIZE,
	STATICTEXT_SETTINGS_MIXERVOL,
	SLIDER_SETTINGS_BUFFERSIZE,
	SLIDER_SETTINGS_MIXERVOL,
	STATICTEXT_SETTINGS_FORCEPOWER2BUFF,
	CHECKBOX_SETTINGS_FORCEPOWER2BUFF,
	RADIOGROUP_SETTINGS_AMPLIFY,
	BUTTON_SETTINGS_RESAMPLING,
	CHECKBOX_SETTINGS_RAMPING,
	RADIOGROUP_SETTINGS_MIXFREQ,
	BUTTON_SETTINGS_CHOOSEDRIVER,
    RADIOGROUP_SETTINGS_XMCHANNELLIMIT,

	// PAGE I (2)
	CHECKBOX_SETTINGS_VIRTUALCHANNELS,
	BUTTON_SETTINGS_VIRTUALCHANNELS_PLUS,
	BUTTON_SETTINGS_VIRTUALCHANNELS_MINUS,
	STATICTEXT_SETTINGS_VIRTUALCHANNELS,
	CHECKBOX_SETTINGS_MULTICHN_RECORD,
	CHECKBOX_SETTINGS_MULTICHN_KEYJAZZ,
	CHECKBOX_SETTINGS_MULTICHN_EDIT,
	CHECKBOX_SETTINGS_MULTICHN_RECORDKEYOFF,
	CHECKBOX_SETTINGS_MULTICHN_RECORDNOTEDELAY,

	// Page II
	CHECKBOX_SETTINGS_HEXCOUNT,
	CHECKBOX_SETTINGS_SHOWZEROEFFECT,
	CHECKBOX_SETTINGS_FULLSCREEN,
	LISTBOX_SETTINGS_RESOLUTIONS,
	BUTTON_RESOLUTIONS_CUSTOM,
	BUTTON_RESOLUTIONS_FULL,
	STATICTEXT_SETTINGS_MAGNIFY,
	RADIOGROUP_SETTINGS_MAGNIFY,
	LISTBOX_COLORS,
	SLIDER_COLOR_RED,
	SLIDER_COLOR_GREEN,
	SLIDER_COLOR_BLUE,
	BUTTON_COLOR,
	BUTTON_COLOR_PREDEF_STORE,

	BUTTON_COLOR_EXPORT,
	BUTTON_COLOR_IMPORT,

	BUTTON_COLOR_PREVIEW,
	BUTTON_COLOR_COPY,
	BUTTON_COLOR_PASTE,
	BUTTON_COLOR_DARKER,
	BUTTON_COLOR_BRIGHTER,
	BUTTON_COLOR_PREDEF_0,
	BUTTON_COLOR_PREDEF_1,
	BUTTON_COLOR_PREDEF_2,
	BUTTON_COLOR_PREDEF_3,
	BUTTON_COLOR_PREDEF_4,
	BUTTON_COLOR_PREDEF_5,
	BUTTON_COLOR_RESTORE,
	STATICTEXT_SPACING,
	SLIDER_SPACING,
	STATICTEXT_MUTEFADE,
	SLIDER_MUTEFADE,

	STATICTEXT_HIGHLIGHTMODULO1,
	BUTTON_HIGHLIGHTMODULO1_PLUS,
	BUTTON_HIGHLIGHTMODULO1_MINUS,
	CHECKBOX_HIGHLIGHTMODULO1_FULLROW,

	STATICTEXT_HIGHLIGHTMODULO2,
	BUTTON_HIGHLIGHTMODULO2_PLUS,
	BUTTON_HIGHLIGHTMODULO2_MINUS,
	CHECKBOX_HIGHLIGHTMODULO2_FULLROW,

	// Page III
	RADIOGROUP_SETTINGS_PATTERNFONT,
	LISTBOX_SETTINGS_FONTFAMILIES,
	LISTBOX_SETTINGS_FONTENTRIES,

	// PAGE VI
	CHECKBOX_SETTINGS_INSTRUMENTBACKTRACE,
	CHECKBOX_SETTINGS_WRAPCURSOR,
	CHECKBOX_SETTINGS_PROSPECTIVE,
	//CHECKBOX_SETTINGS_FOLLOWSONG,
	CHECKBOX_SETTINGS_TABTONOTE,
	CHECKBOX_SETTINGS_AUTORESIZE,
	CHECKBOX_SETTINGS_CLICKTOCURSOR,
	RADIOGROUP_SETTINGS_EDITMODE,
	RADIOGROUP_SETTINGS_SCROLLMODE,

	CHECKBOX_SETTINGS_SAMPLEEDITORUNDO,
	CHECKBOX_SETTINGS_AUTOMIXDOWNSAMPLES,
	CHECKBOX_SETTINGS_INTERNALDISKBROWSER,
	CHECKBOX_SETTINGS_AUTOESTPLAYTIME,
	CHECKBOX_SETTINGS_SHOWSPLASH,
	CHECKBOX_SETTINGS_SCOPES,

	STATICTEXT_SETTINGS_SCOPESAPPEARANCE,
	RADIOGROUP_SETTINGS_SCOPESAPPEARANCE,

	CHECKBOX_SETTINGS_INVERTMWHEELZOOM,
	
	// Page V (only on desktop version)
	RADIOGROUP_SETTINGS_STOPBACKGROUNDBEHAVIOUR,
	CHECKBOX_SETTINGS_TABSWITCHRESUMEPLAY,
	STATICTEXT_SETTINGS_TABSWITCHRESUMEPLAY,
	CHECKBOX_SETTINGS_LOADMODULEINNEWTAB,

	PAGE_IO_1,
	PAGE_IO_2,
	PAGE_IO_3,
    PAGE_IO_4,

	PAGE_LAYOUT_1,
	PAGE_LAYOUT_2,
	PAGE_LAYOUT_3,
	PAGE_LAYOUT_4,

	PAGE_FONTS_1,
	PAGE_FONTS_2,
	PAGE_FONTS_3,

	PAGE_MISC_1,
	PAGE_MISC_2,
	PAGE_MISC_3,
	PAGE_MISC_4,

	PAGE_TABS_1,
	PAGE_TABS_2,
	PAGE_TABS_3,

	RESPONDMESSAGEBOX_CUSTOMRESOLUTION,
	RESPONDMESSAGEBOX_RESTOREPALETTES,
	RESPONDMESSAGEBOX_SELECTAUDIODRV,
	RESPONDMESSAGEBOX_SELECTRESAMPLER
};

struct TScreenRes
{
	pp_int32 width, height;
	const char* name;
};

#define NUMRESOLUTIONS	13
#define MINWIDTH		640
#define MINHEIGHT		480

static TScreenRes resolutions[NUMRESOLUTIONS] =
{
	{640, 480, "640x480"},
	{720, 480, "720x480"},
	{800, 480, "800x480"},
	{800, 500, "800x500"},
	{800, 600, "800x600"},
	{1024, 768, "1024x768"},
	{1152, 864, "1152x864"},
	{1280, 768, "1280x768"},
	{1280, 800, "1280x800"},
	{1280, 854, "1280x854"},
	{1280, 1024, "1280x1024"},
	{1680, 1050, "1680x1050"},
	{-1, -1, "<Custom>"}
};

// Class which responds to message box clicks
class DialogResponderSettings : public DialogResponder
{
private:
	SectionSettings& section;

public:
	DialogResponderSettings(SectionSettings& section) :
		section(section)
	{
	}

	virtual pp_int32 ActionOkay(PPObject* sender)
	{
		switch (reinterpret_cast<PPDialogBase*>(sender)->getID())
		{
			case RESPONDMESSAGEBOX_CUSTOMRESOLUTION:
			{
				section.storeCustomResolution();
				break;
			}

			case RESPONDMESSAGEBOX_RESTOREPALETTES:
				section.restorePalettes();
				break;

			case RESPONDMESSAGEBOX_SELECTAUDIODRV:
			{
				PPListBox* listBox = reinterpret_cast<DialogListBox*>(sender)->getListBox();
				section.storeAudioDriver(listBox->getItem(listBox->getSelectedIndex()));
				break;
			}

			case RESPONDMESSAGEBOX_SELECTRESAMPLER:
			{
				PPListBox* listBox = reinterpret_cast<DialogListBox*>(sender)->getListBox();
				section.storeResampler(listBox->getSelectedIndex());
				break;
			}
		}
		return 0;
	}
};

class TabPage : public EventListenerInterface
{
protected:
	pp_uint32 id;
	SectionSettings& sectionSettings;
	PPTransparentContainer* container;
	bool visible;

	enum
	{
		PageWidth = 160,
		PageHeight = UPPERFRAMEHEIGHT
	};

public:
	TabPage(pp_uint32 id, SectionSettings& sectionSettings) :
		id(id),
		sectionSettings(sectionSettings),
		container(NULL),
		visible(false)
	{
	}

	virtual ~TabPage() {}

	PPTransparentContainer* getContainer() const { return container; }

	static pp_int32 getWidth() { return PageWidth; }
	static pp_int32 getHeight() { return PageHeight; }

	void setVisible(bool visible) { this->visible = visible; }
	bool isVisible() const { return visible; }

	virtual void init(PPScreen* screen) = 0;
	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor) = 0;

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event)
	{
		return sectionSettings.handleEvent(sender, event);
	}

	PPPoint getLocation() { return container->getLocation(); }

	void setLocation(const PPPoint& p)
	{
		if (container->getLocation().x != p.x || container->getLocation().y != p.y)
		{
			pp_int32 dx = p.x - container->getLocation().x;
			pp_int32 dy = p.y - container->getLocation().y;
			container->move(PPPoint(dx, dy));
		}
	}
};

class TabPageIO_1 : public TabPage
{
public:
	TabPageIO_1(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y2 + 4), "Driver:", true));

		PPButton* button = new PPButton(BUTTON_SETTINGS_CHOOSEDRIVER, screen, this, PPPoint(x + 4 + 7*8 + 4, y2 + 3), PPSize(90, 11));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Select Driver" PPSTR_PERIODS);
		container->addControl(button);

		y2+=4;

		PPStaticText* text = new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y2 + 2 + 11), "Buffer:", true);
		//text->setFont(PPFont::getFont(PPFont::FONT_TINY));
		container->addControl(text);
		container->addControl(new PPStaticText(STATICTEXT_SETTINGS_BUFFERSIZE, NULL, NULL, PPPoint(x + 4 + 7*8, y2 + 2 + 11), "000ms(xx)", false));

		PPSlider* slider = new PPSlider(SLIDER_SETTINGS_BUFFERSIZE, screen, this, PPPoint(x + 4, y2 + 2 + 11*2-1), 151, true);
		slider->setMaxValue(511);
		slider->setBarSize(8192);
		container->addControl(slider);

		y2++;

		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_FORCEPOWER2BUFF, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 + 2 + 11 * 3 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(STATICTEXT_SETTINGS_FORCEPOWER2BUFF, NULL, this, PPPoint(x + 4, y2 + 2 + 11*3), "Force 2^n sizes:", checkBox, true));
		
		y2+=12;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y2 + 2 + 11*3), "Mixervol:", true));
		container->addControl(new PPStaticText(STATICTEXT_SETTINGS_MIXERVOL, NULL, NULL, PPPoint(x + 4 + 8*9, y2 + 2 + 11*3), "100%", false));

		slider = new PPSlider(SLIDER_SETTINGS_MIXERVOL, screen, this, PPPoint(x + 4, y2 + 2 + 11*4-1), 151, true);
		slider->setMaxValue(256);
		slider->setBarSize(8192);
		container->addControl(slider);

		y2-=1;
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y2 + 2 + 11*5 + 4), "Amp:", true));

		PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_AMPLIFY, screen, this, PPPoint(x + 2 + 5*8, y2 + 2 + 11*5 + 1), PPSize(120, 16));
		radioGroup->setColor(TrackerConfig::colorThemeMain);
		radioGroup->setFont(PPFont::getFont(PPFont::FONT_TINY));

		radioGroup->setHorizontal(true);
		radioGroup->addItem("25%");
		radioGroup->addItem("50%");
		radioGroup->addItem("100%");

		container->addControl(radioGroup);

		y2 += 2 + 11*7 - 4;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y2), "Resampling:", true));
		button = new PPButton(BUTTON_SETTINGS_RESAMPLING, screen, this, PPPoint(x + 4 + 11*8 + 4, y2-2), PPSize(6*9 + 4, 11));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Select" PPSTR_PERIODS);
		container->addControl(button);

		y2+=12;

		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_RAMPING, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2), "Volume ramping:", checkBox, true));
		
		//container->addControl(new PPSeperator(0, screen, PPPoint(x + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SETTINGS_BUFFERSIZE));
		PPSlider* slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_SETTINGS_BUFFERSIZE));

		char buffer[100];
		char buffer2[100];

		// buffersize
		bool forcePowerOfTwo = settingsDatabase->restore("FORCEPOWEROFTWOBUFFERSIZE")->getBoolValue();
		pp_int32 v = settingsDatabase->restore("BUFFERSIZE")->getIntValue();
		pp_int32 v2 = v;

		if (forcePowerOfTwo)
			v = PlayerMaster::roundToNearestPowerOfTwo(v);

		float fv = PlayerMaster::convertBufferSizeToMillis(settingsDatabase->restore("MIXERFREQ")->getIntValue(),
														   v);

		pp_int32 fixed = (pp_int32)fv;
		pp_int32 decimal = (pp_int32)(fv*10.0f) % 10;
		if (v >= 1000)
		{
			if (fixed >= 100)
			{
				if (fixed >= 1000)
					sprintf(buffer, "%i.%is(%i.%ik)", fixed / 1000, (fixed / 100) % 10, v / 1000, (v / 100) % 10);
				else
					sprintf(buffer, "%ims(%i.%ik)", fixed, v / 1000, (v / 100) % 10);
			}
			else
				sprintf(buffer, "%i.%ims(%i.%ik)", fixed, decimal, v / 1000, (v / 100) % 10);
		}
		else
		{
			if (fixed >= 1000)
				sprintf(buffer, "%i.%is(%i)", fixed / 1000, (fixed / 100) % 10, v);
			else
				sprintf(buffer, "%i.%ims(%i)", fixed, decimal, v);
		}

		if (strlen(buffer) < 9)
		{
			memset(buffer2, 32, sizeof(buffer2));
			strcpy(buffer2 + 9-strlen(buffer), buffer);
			strcpy(buffer, buffer2);
		}

		text->setText(buffer);

		slider->setCurrentValue((v2 >> 5) - 1);

		// force 2^n buffer size
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_FORCEPOWER2BUFF))->checkIt(forcePowerOfTwo);

		// mixervolume
		text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SETTINGS_MIXERVOL));
		slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_SETTINGS_MIXERVOL));

		v = settingsDatabase->restore("MIXERVOLUME")->getIntValue();

		sprintf(buffer, "%i%%", (v*100)/256);

		if (strlen(buffer) < 4)
		{
			memset(buffer2, 32, sizeof(buffer2));
			strcpy(buffer2 + 4-strlen(buffer), buffer);
			strcpy(buffer, buffer2);
		}

		text->setText(buffer);

		slider->setCurrentValue(v);

		// amplify
		v = settingsDatabase->restore("MIXERSHIFT")->getIntValue();

		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_AMPLIFY))->setChoice(v);

		// checkboxes
		v = settingsDatabase->restore("RAMPING")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_RAMPING))->checkIt(v!=0);
	}

};

class TabPageIO_2 : public TabPage
{
public:
	TabPageIO_2(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		// frequency table
		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Mixer Resolution", true, true));

		pp_int32 j;
		PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_MIXFREQ, screen, this, PPPoint(x2, y2+2+11), PPSize(160, TrackerConfig::numMixFrequencies*14));
		radioGroup->setColor(TrackerConfig::colorThemeMain);

		for (j = 0; j < TrackerConfig::numMixFrequencies; j++)
		{
			char buffer[32];
			sprintf(buffer, "%i Hz", TrackerConfig::mixFrequencies[j]);
			radioGroup->addItem(buffer);
		}

		container->addControl(radioGroup);

		y2+=j*14+14;

		container->addControl(new PPSeperator(0, screen, PPPoint(x2, y2), 158, TrackerConfig::colorThemeMain, true));

		y2+=4;

		// module frequencies
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Frequency Table", true, true));

		radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_FREQTAB, screen, this, PPPoint(x2, y2+2+11), PPSize(160, 30));
		radioGroup->setColor(TrackerConfig::colorThemeMain);

		radioGroup->addItem("Amiga frequencies");
		radioGroup->addItem("Linear frequencies");

		container->addControl(radioGroup);

		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		// mixer resolution
		pp_int32 v = settingsDatabase->restore("MIXERFREQ")->getIntValue();

		pp_int32 i;
		for (i = 0; i < TrackerConfig::numMixFrequencies; i++)
			if (v == TrackerConfig::mixFrequencies[i])
			{
				static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_MIXFREQ))->setChoice(i);
				break;
			}

		// frequency table
		v = moduleEditor.getFrequency();

		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_FREQTAB))->setChoice(v);
	}

};

class TabPageIO_3 : public TabPage
{
public:
	TabPageIO_3(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y2 + 2), "Instrument Playback", true, true));

		y2+=15;
		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_VIRTUALCHANNELS, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2), "Jam channels:", checkBox, true));

		y2+=12;
		container->addControl(new PPStaticText(STATICTEXT_SETTINGS_VIRTUALCHANNELS, NULL, NULL, PPPoint(x + 4, y2), "Use xx channels", false));

		PPButton* button = new PPButton(BUTTON_SETTINGS_VIRTUALCHANNELS_PLUS, screen, this, PPPoint(x + 4 + 15*8 + 4, y2), PPSize(12, 9));
		button->setText(TrackerConfig::stringButtonPlus);
		container->addControl(button);

		button = new PPButton(BUTTON_SETTINGS_VIRTUALCHANNELS_MINUS, screen, this, PPPoint(x + 4 + 15*8 + 4 + 13, y2), PPSize(13, 9));
		button->setText(TrackerConfig::stringButtonMinus);

		container->addControl(button);

		y2+=14;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y2 + 2), "Multichannel", true, true));

		y2+=15;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_MULTICHN_RECORD, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2), "Recording:", checkBox, true));

		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_MULTICHN_KEYJAZZ, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2), """Keyjazzing"":", checkBox, true));

		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_MULTICHN_EDIT, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2), "Editing:", checkBox, true));

		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_MULTICHN_RECORDKEYOFF, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2), "Record key off:", checkBox, true));

		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_MULTICHN_RECORDNOTEDELAY, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2), "Rec. note delays:", checkBox, true));

		//container->addControl(new PPSeperator(0, screen, PPPoint(x + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SETTINGS_VIRTUALCHANNELS));

		char buffer[100];

		// buffersize
		pp_int32 v = settingsDatabase->restore("VIRTUALCHANNELS")->getIntValue();

		sprintf(buffer, "Use %02i", v);

		text->setText(buffer);

		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_VIRTUALCHANNELS))->checkIt(v>0);

		v = settingsDatabase->restore("MULTICHN_RECORD")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_MULTICHN_RECORD))->checkIt(v!=0);
		v = settingsDatabase->restore("MULTICHN_KEYJAZZ")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_MULTICHN_KEYJAZZ))->checkIt(v!=0);
		v = settingsDatabase->restore("MULTICHN_EDIT")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_MULTICHN_EDIT))->checkIt(v!=0);
		v = settingsDatabase->restore("MULTICHN_RECORDKEYOFF")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_MULTICHN_RECORDKEYOFF))->checkIt(v!=0);
		v = settingsDatabase->restore("MULTICHN_RECORDNOTEDELAY")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_MULTICHN_RECORDNOTEDELAY))->checkIt(v!=0);
	}

};

class TabPageIO_4 : public TabPage
{
public:
    TabPageIO_4(pp_uint32 id, SectionSettings& sectionSettings) :
    TabPage(id, sectionSettings)
    {
    }
    
    virtual void init(PPScreen* screen)
    {
        pp_int32 x = 0;
        pp_int32 y = 0;
        
        container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));
        
        pp_int32 x2 = x;
        pp_int32 y2 = y;
        
        container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "XM channel limit", true, true));
        
        PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_XMCHANNELLIMIT, screen, this, PPPoint(x2, y2+2+11), PPSize(160, 3*14));
        radioGroup->setColor(TrackerConfig::colorThemeMain);
        radioGroup->addItem("32");
        radioGroup->addItem("64");
        radioGroup->addItem("128");
        
        container->addControl(radioGroup);
    }
    
    virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
    {
        // mixer resolution
        pp_int32 v = settingsDatabase->restore("XMCHANNELLIMIT")->getIntValue();
        
        switch (v) {
            case 32:
                static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_XMCHANNELLIMIT))->setChoice(0);
                break;
            case 64:
                static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_XMCHANNELLIMIT))->setChoice(1);
                break;
            case 128:
                static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_XMCHANNELLIMIT))->setChoice(2);
                break;
                
        }
    }
    
};

class TabPageLayout_1 : public TabPage
{
public:
	TabPageLayout_1(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y2 + 2), "Pattern Editor", true, true));

		y2+=14;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Spacing:", true));
		container->addControl(new PPStaticText(STATICTEXT_SPACING, screen, this, PPPoint(x2 + 9*8, y2), "0px"));

		PPSlider* slider = new PPSlider(SLIDER_SPACING, screen, this, PPPoint(x2 + 13*8 + 2, y2-1), 49, true);
		slider->setMaxValue(31);
		slider->setBarSize(16384);
		container->addControl(slider);

		y2+=11;
		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_HEXCOUNT, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 2, y2), "Hex count:", checkBox, true));
		
		y2+=11;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_SHOWZEROEFFECT, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 2, y2), "Show zero effect:", checkBox, true));		

		y2+=11;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_PROSPECTIVE, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Prospective:", checkBox, true));

		y2+=11;
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Muting opacity:", true));

		y2+=11;
		container->addControl(new PPStaticText(STATICTEXT_MUTEFADE, screen, this, PPPoint(x2 + 4, y2), "100%"));

		slider = new PPSlider(SLIDER_MUTEFADE, screen, this, PPPoint(x2 + 4 + 8*4 + 4, y2-1), 113, true);
		slider->setMaxValue(100);
		slider->setBarSize(8192);
		container->addControl(slider);

		y2+=12;
		PPStaticText* staticText = new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Row highlight spacing:", true);
		staticText->setFont(PPFont::getFont(PPFont::FONT_TINY));
		container->addControl(staticText);

		y2+=11;
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "1st:", true));
		container->addControl(new PPStaticText(STATICTEXT_HIGHLIGHTMODULO1, NULL, NULL, PPPoint(x2 + 2 + 4*8 + 2, y2), "xx"));

		PPButton* button = new PPButton(BUTTON_HIGHLIGHTMODULO1_PLUS, screen, this, PPPoint(x + 2 + 7*8, y2-1), PPSize(12, 9));
		button->setText(TrackerConfig::stringButtonPlus);
		container->addControl(button);
		button = new PPButton(BUTTON_HIGHLIGHTMODULO1_MINUS, screen, this, PPPoint(x + 2 + 7*8 + 13, y2-1), PPSize(13, 9));
		button->setText(TrackerConfig::stringButtonMinus);
		container->addControl(button);
		checkBox = new PPCheckBox(CHECKBOX_HIGHLIGHTMODULO1_FULLROW, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 2 + 7*8 + 13 + 20, y2), "Full:", checkBox, true));

		y2+=11;
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "2nd:", true));
		container->addControl(new PPStaticText(STATICTEXT_HIGHLIGHTMODULO2, NULL, NULL, PPPoint(x2 + 2 + 4*8 + 2, y2), "xx"));
		button = new PPButton(BUTTON_HIGHLIGHTMODULO2_PLUS, screen, this, PPPoint(x + 2 + 7*8, y2-1), PPSize(12, 9));
		button->setText(TrackerConfig::stringButtonPlus);
		container->addControl(button);
		button = new PPButton(BUTTON_HIGHLIGHTMODULO2_MINUS, screen, this, PPPoint(x + 2 + 7*8 + 13, y2-1), PPSize(13, 9));
		button->setText(TrackerConfig::stringButtonMinus);
		container->addControl(button);
		checkBox = new PPCheckBox(CHECKBOX_HIGHLIGHTMODULO2_FULLROW, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 2 + 7 * 8 + 13 + 20, y2), "Full:", checkBox, true));

		// vertical separator
		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		// spacing slider
		PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SPACING));
		PPSlider* slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_SPACING));

		pp_int32 v = settingsDatabase->restore("SPACING")->getIntValue();
		char buffer[100], buffer2[100];
		sprintf(buffer,"%ipx", v);
		text->setText(buffer);
		slider->setCurrentValue(v);

		// mute fade strength slider
		text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_MUTEFADE));
		slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_MUTEFADE));

		v = settingsDatabase->restore("MUTEFADE")->getIntValue();
		sprintf(buffer, "%i%%", v);
		// right align
		if (strlen(buffer) < 4)
		{
			memset(buffer2, 32, sizeof(buffer2));
			strcpy(buffer2 + 4-strlen(buffer), buffer);
			strcpy(buffer, buffer2);
		}
		text->setText(buffer);
		slider->setCurrentValue(v);

		// update primary pattern row highlight
		text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_HIGHLIGHTMODULO1));
		v = settingsDatabase->restore("HIGHLIGHTMODULO1")->getIntValue();
		sprintf(buffer, "%02i ", v);
		text->setText(buffer);

		// update secondary pattern row highlight
		text = static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_HIGHLIGHTMODULO2));
		v = settingsDatabase->restore("HIGHLIGHTMODULO2")->getIntValue();
		sprintf(buffer, "%02i ", v);
		text->setText(buffer);

		v = settingsDatabase->restore("HIGHLIGHTROW1")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_HIGHLIGHTMODULO1_FULLROW))->checkIt(v!=0);

		v = settingsDatabase->restore("HIGHLIGHTROW2")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_HIGHLIGHTMODULO2_FULLROW))->checkIt(v!=0);

		v = settingsDatabase->restore("HEXCOUNT")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_HEXCOUNT))->checkIt(v!=0);

		v = settingsDatabase->restore("SHOWZEROEFFECT")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_SHOWZEROEFFECT))->checkIt(v!=0);

		v = settingsDatabase->restore("PROSPECTIVE")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_PROSPECTIVE))->checkIt(v!=0);
	}

};

class TabPageLayout_2 : public TabPage
{
public:
	TabPageLayout_2(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 i = 0;

		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;

		// Colors
		pp_int32 y2 = y;

		pp_int32 lbheight = container->getSize().height - (y2 - y) - 66;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Colors", true, true));

		PPButton* button = new PPButton(BUTTON_COLOR_IMPORT, screen, this, PPPoint(x2 + 54, y2 + 2), PPSize(9, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("I");
		container->addControl(button);

		button = new PPButton(BUTTON_COLOR_EXPORT, screen, this, PPPoint(x2 + 54 + 10, y2 + 2), PPSize(9, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("E");
		container->addControl(button);

		button = new PPButton(BUTTON_COLOR_PREVIEW, screen, this, PPPoint(x2 + 115, y2 + 2), PPSize(39, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Preview");
		container->addControl(button);

		button = new PPButton(BUTTON_COLOR_RESTORE, screen, this, PPPoint(x2 + 115 - 40, y2 + 2), PPSize(39, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Restore");
		container->addControl(button);

		y2+=1+11;

		sectionSettings.listBoxColors = new PPListBox(LISTBOX_COLORS, screen, this, PPPoint(x2+2, y2+2), PPSize(153,lbheight), true, false, true, true);
		sectionSettings.listBoxColors->setBorderColor(TrackerConfig::colorThemeMain);
		sectionSettings.listBoxColors->setKeepsFocus(false);
		sectionSettings.listBoxColors->setShowFocus(false);

		for (i = 0; i < GlobalColorConfig::ColorLast; i++)
		{
			pp_int32 j = sectionSettings.colorMapping[i];
			if (sectionSettings.colorDescriptors[j].readableDecription == NULL)
				break;

			sectionSettings.listBoxColors->addItem(sectionSettings.colorDescriptors[j].readableDecription);
		}

#ifdef __LOWRES__
		y2+=sectionSettings.listBoxColors->getSize().height + 2;
#else
		y2+=sectionSettings.listBoxColors->getSize().height + 4;
#endif
		container->addControl(sectionSettings.listBoxColors);

		button = new PPColPrevButton(BUTTON_COLOR, screen, this, PPPoint(x2 + 88, y2 + 1), PPSize(31, 34));
		button->setFlat(true);
		button->setColor(sectionSettings.currentColor);
		container->addControl(button);

		button = new PPButton(BUTTON_COLOR_COPY, screen, this, PPPoint(x2 + 88 + 32 + 2, y2 + 2), PPSize(5*6+2, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Copy");
		container->addControl(button);

		button = new PPButton(BUTTON_COLOR_PASTE, screen, this, PPPoint(x2 + 88 + 32 + 2, y2 + 2+10), PPSize(5*6+2, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Paste");
		container->addControl(button);

		button = new PPButton(BUTTON_COLOR_DARKER, screen, this, PPPoint(x2 + 88 + 32 + 2, y2 + 2+23), PPSize(16, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("<<");
		container->addControl(button);

		button = new PPButton(BUTTON_COLOR_BRIGHTER, screen, this, PPPoint(x2 + 88 + 32 + 2 + 17, y2 + 2+23), PPSize(15, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText(">>");
		container->addControl(button);

		// Red slider
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "R", true));
		PPSlider* slider = new PPSlider(SLIDER_COLOR_RED, screen, this, PPPoint(x2 + 2 + 1*8 + 2, y2 + 1), 74, true);
		slider->setMaxValue(255);
		slider->setBarSize(16384);
		container->addControl(slider);

		y2+=12;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "G", true));
		slider = new PPSlider(SLIDER_COLOR_GREEN, screen, this, PPPoint(x2 + 2 + 1*8 + 2, y2 + 1), 74, true);
		slider->setMaxValue(255);
		slider->setBarSize(16384);
		container->addControl(slider);

		y2+=12;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "B", true));
		slider = new PPSlider(SLIDER_COLOR_BLUE, screen, this, PPPoint(x2 + 2 + 1*8 + 2, y2 + 1), 74, true);
		slider->setMaxValue(255);
		slider->setBarSize(16384);
		container->addControl(slider);

		y2+=12;

		// predefs
		pp_int32 px = x2 + 2;

		PPStaticText* staticText = new PPStaticText(0, NULL, NULL, PPPoint(px, y2 + 3), "Predef:", true);
		container->addControl(staticText);

		px+=7*8+2;

		// pre-defined envelopes
		for (i = 0; i < TrackerConfig::numPredefinedColorPalettes; i++)
		{
			button = new PPButton(BUTTON_COLOR_PREDEF_0+i, screen, this, PPPoint(px, y2 + 2), PPSize(9, 9));
			button->setFont(PPFont::getFont(PPFont::FONT_TINY));
			button->setText((char)('A'+i));
			container->addControl(button);
			px+=button->getSize().width+1;
		}
		px+=2;

		button = new PPButton(BUTTON_COLOR_PREDEF_STORE, screen, this, PPPoint(px, y2 + 2), PPSize(5*6+2, 9), true, true, false);
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Store");
		container->addControl(button);

		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		// Update color from sliders
		PPSlider* slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_COLOR_RED));
		slider->setCurrentValue(sectionSettings.currentColor.r);
		slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_COLOR_GREEN));
		slider->setCurrentValue(sectionSettings.currentColor.g);
		slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_COLOR_BLUE));
		slider->setCurrentValue(sectionSettings.currentColor.b);

		static_cast<PPButton*>(container->getControlByID(BUTTON_COLOR_PASTE))->setClickable(sectionSettings.colorCopy != NULL);
	}

};

class TabPageLayout_3 : public TabPage
{
public:
	TabPageLayout_3(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 i;

		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Resolutions", true, true));

		PPButton* button = new PPButton(BUTTON_RESOLUTIONS_CUSTOM, screen, this, PPPoint(x2 + 92, y2 + 2), PPSize(37, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Custom");
		container->addControl(button);

		button = new PPButton(BUTTON_RESOLUTIONS_FULL, screen, this, PPPoint(x2 + 92 + 37 + 2, y2 + 2), PPSize(23, 9));
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Full");
		container->addControl(button);

		y2+=12;

		pp_int32 lbheight = container->getSize().height - (y2 - y) - (18);
		PPListBox* listBox;
		listBox = new PPListBox(LISTBOX_SETTINGS_RESOLUTIONS, screen, this, PPPoint(x2+2, y2+2), PPSize(153,lbheight), true, false, true, true);
		listBox->setBorderColor(TrackerConfig::colorThemeMain);
		listBox->setKeepsFocus(false);
		listBox->setShowFocus(false);

		for (i = 0; i < NUMRESOLUTIONS; i++)
			listBox->addItem(resolutions[i].name);
		container->addControl(listBox);


		y2+=lbheight + 6;

		container->addControl(new PPStaticText(STATICTEXT_SETTINGS_MAGNIFY, NULL, NULL, PPPoint(x2 + 2, y2), "Scale:", true));

		PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_MAGNIFY, screen, this, PPPoint(x2 + 2 + 7*8, y2 - 4), PPSize(120, 16));
		radioGroup->setColor(TrackerConfig::colorThemeMain);
		radioGroup->setFont(PPFont::getFont(PPFont::FONT_TINY));

		radioGroup->setHorizontal(true);
		radioGroup->addItem("x1");
		radioGroup->addItem("x2");
		radioGroup->addItem("x4");
		radioGroup->addItem("x8");

		container->addControl(radioGroup);
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 width = settingsDatabase->restore("XRESOLUTION")->getIntValue();
		pp_int32 height = settingsDatabase->restore("YRESOLUTION")->getIntValue();

		bool found = false;
		for (pp_int32 i = 0; i < NUMRESOLUTIONS; i++)
		{
			if (resolutions[i].width == width && resolutions[i].height == height)
			{
				static_cast<PPListBox*>(container->getControlByID(LISTBOX_SETTINGS_RESOLUTIONS))->setSelectedIndex(i, false, false);
				found = true;
				break;
			}
		}

		if (!found)
			static_cast<PPListBox*>(container->getControlByID(LISTBOX_SETTINGS_RESOLUTIONS))->setSelectedIndex(NUMRESOLUTIONS-1, false, false);

		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SETTINGS_MAGNIFY))->enable(screen->supportsScaling());
		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_MAGNIFY))->enable(screen->supportsScaling());

		pp_int32 screenScaleFactor = settingsDatabase->restore("SCREENSCALEFACTOR")->getIntValue();
		pp_int32 index = 0;
		switch (screenScaleFactor)
		{
			case 2:
				index = 1;
				break;
			case 4:
				index = 2;
				break;
			case 8:
				index = 3;
				break;
		}

		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_MAGNIFY))->setChoice(index);
	}

};

class TabPageLayout_4 : public TabPage
{
public:
	TabPageLayout_4(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Global", true, true));

		y2+=4+11;
		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_FULLSCREEN, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 4, y2), "Fullscreen:", checkBox, true));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 v = settingsDatabase->restore("FULLSCREEN")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_FULLSCREEN))->checkIt(v!=0);
	}

};

class TabPageFonts_1 : public TabPage
{
public:
	TabPageFonts_1(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 i;
		const char* name = NULL;

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y2 + 2), "Pattern Editor", true, true));

		y2+=11;

		PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_PATTERNFONT, screen, this, PPPoint(x2, y2), PPSize(159, 0));
		radioGroup->setColor(TrackerConfig::colorThemeMain);

		name = PPFont::getFirstFontFamilyName();
		i = 0;
		while (name)
		{
			radioGroup->addItem(name);
			name = PPFont::getNextFontFamilyName();
			i++;
		}
		radioGroup->setSize(PPSize(radioGroup->getSize().width, 14*i));

		container->addControl(radioGroup);

		y2+=radioGroup->getSize().height;

		container->addControl(new PPSeperator(0, screen, PPPoint(x2, y2), 158, TrackerConfig::colorThemeMain, true));

		y2+=5;

		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 v = settingsDatabase->restore("PATTERNFONT")->getIntValue();
		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_PATTERNFONT))->setChoice(v);
	}

};

class TabPageFonts_2 : public TabPage
{
public:
	TabPageFonts_2(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Font face config", true, true));

		y2+=12;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Available sizes:", true));

		y2+=8;

		pp_int32 lbheight = (container->getSize().height - (y2-y+11*2+2)) / 2;
		sectionSettings.listBoxFontFamilies = new PPListBox(LISTBOX_SETTINGS_FONTFAMILIES, screen, this, PPPoint(x2+2, y2+2), PPSize(153,lbheight), true, false, true, true);
		sectionSettings.listBoxFontFamilies->setBorderColor(TrackerConfig::colorThemeMain);
		sectionSettings.listBoxFontFamilies->setKeepsFocus(false);
		sectionSettings.listBoxFontFamilies->setShowFocus(false);

		const char* name = PPFont::getFirstFontFamilyName();
		while (name)
		{
			sectionSettings.listBoxFontFamilies->addItem(name);
			name = PPFont::getNextFontFamilyName();
		}
		container->addControl(sectionSettings.listBoxFontFamilies);

		y2+=sectionSettings.listBoxFontFamilies->getSize().height+2;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Available faces:", true));

		y2+=8;
		lbheight+=7;

		sectionSettings.listBoxFontEntries = new PPListBox(LISTBOX_SETTINGS_FONTENTRIES, screen, this, PPPoint(x2+2, y2+2), PPSize(153,lbheight), true, false, true, true);
		sectionSettings.listBoxFontEntries->setBorderColor(TrackerConfig::colorThemeMain);
		sectionSettings.listBoxFontEntries->setKeepsFocus(false);
		sectionSettings.listBoxFontEntries->setShowFocus(false);

		sectionSettings.enumerateFontFacesInListBox(sectionSettings.listBoxFontFamilies->getSelectedIndex());
		container->addControl(sectionSettings.listBoxFontEntries);

		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		PPString str = settingsDatabase->restore(PPFont::getFamilyInternalName((PPFont::FontID)sectionSettings.listBoxFontFamilies->getSelectedIndex()))->getStringValue();
		sectionSettings.listBoxFontEntries->setSelectedIndexByItem(str, false);
	}

};

class TabPageMisc_1 : public TabPage
{
public:
	TabPageMisc_1(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Edit mode", true, true));

		PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_EDITMODE, screen, this, PPPoint(x2, y2+2+11), PPSize(160, 42));
		radioGroup->setColor(TrackerConfig::colorThemeMain);

		radioGroup->addItem("MilkyTracker");
		radioGroup->addItem("Fasttracker II");
		container->addControl(radioGroup);

		y2+=3*12+8;

		container->addControl(new PPSeperator(0, screen, PPPoint(x2, y2), 156, TrackerConfig::colorThemeMain, true));

		y2+=6;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Scrolling Style", true, true));

		radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_SCROLLMODE, screen, this, PPPoint(x2, y2+2+11), PPSize(160, 42));
		radioGroup->setColor(TrackerConfig::colorThemeMain);

		radioGroup->addItem("Scroll to end");
		radioGroup->addItem("Scroll to center");
		radioGroup->addItem("Always centered");
		container->addControl(radioGroup);

		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 v = settingsDatabase->restore("EDITMODE")->getIntValue();
		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_EDITMODE))->setChoice(v);

		v = settingsDatabase->restore("SCROLLMODE")->getIntValue();
		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_SCROLLMODE))->setChoice(v);
	}

};

class TabPageMisc_2 : public TabPage
{
public:
	TabPageMisc_2(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 y2 = y;
		pp_int32 x2 = x;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Pattern Editor", true, true));

		y2+=4+11;
		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_AUTORESIZE, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Paste autoresize:", checkBox, true));

		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_INSTRUMENTBACKTRACE, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Instr. backtrace:", checkBox, true));

		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_TABTONOTE, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "TAB to note:", checkBox, true));
		
		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_CLICKTOCURSOR, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Click to cursor:", checkBox, true));
		
		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_WRAPCURSOR, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Wrap cursor:", checkBox, true));

		y2+=11;

		//container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Follow song:", true));
		//container->addControl(new PPCheckBox(CHECKBOX_SETTINGS_FOLLOWSONG, screen, this, PPPoint(x2 + 4 + 17*8 + 4, y2-1)));

		// ------------------ sample editor -------------------
		container->addControl(new PPSeperator(0, screen, PPPoint(x2, y2), 158, TrackerConfig::colorThemeMain, true));

		y2+=3;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Sample Editor", true, true));
		y2+=4+11;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_SAMPLEEDITORUNDO, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Enable undo buff:", checkBox, true));

		y2+=14;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_AUTOMIXDOWNSAMPLES, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 3));
		container->addControl(checkBox);
		PPCheckBoxLabel* cbLabel = new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Auto-mixdown stereo samples:", checkBox, true);
		cbLabel->setFont(PPFont::getFont(PPFont::FONT_TINY));
		container->addControl(cbLabel);
		

		y2+=10;

		/*// ------------------ other -------------------
			container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Other", true, true));
		y2+=4+11;
		PPStaticText* statict = new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Estimate playtime after load", true);
		statict->setFont(PPFont::getFont(PPFont::FONT_TINY));
		container->addControl(statict);
		container->addControl(new PPCheckBox(CHECKBOX_SETTINGS_AUTOESTPLAYTIME, screen, this, PPPoint(x2 + 4 + 17*8 + 4, y2-3)));*/

		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 v = settingsDatabase->restore("PATTERNAUTORESIZE")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_AUTORESIZE))->checkIt(v!=0);

		v = settingsDatabase->restore("INSTRUMENTBACKTRACE")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_INSTRUMENTBACKTRACE))->checkIt(v!=0);

		v = settingsDatabase->restore("TABTONOTE")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_TABTONOTE))->checkIt(v!=0);

		v = settingsDatabase->restore("CLICKTOCURSOR")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_CLICKTOCURSOR))->checkIt(v!=0);

		//v = settingsDatabase->restore("WRAPAROUND")->getIntValue();
		//static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_WRAPCURSOR))->checkIt(v!=0);

		//v = settingsDatabase->restore("FOLLOWSONG")->getIntValue();
		//static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_FOLLOWSONG))->checkIt(v!=0);

		/*v = settingsDatabase->restore("AUTOESTPLAYTIME")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_AUTOESTPLAYTIME))->checkIt(v!=0);*/

		v = settingsDatabase->restore("SAMPLEEDITORUNDOBUFFER")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_SAMPLEEDITORUNDO))->checkIt(v!=0);

		v = settingsDatabase->restore("AUTOMIXDOWNSAMPLES")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_AUTOMIXDOWNSAMPLES))->checkIt(v!=0);
	}

};

class TabPageMisc_3 : public TabPage
{
public:
	TabPageMisc_3(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Other", true, true));
		y2+=4+11;		
		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_INTERNALDISKBROWSER, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Internal browser:", checkBox, true));

		y2+=12;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_SHOWSPLASH, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Splash screen:", checkBox, true));

		y2+=14;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_AUTOESTPLAYTIME, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 3));
		container->addControl(checkBox);
		PPCheckBoxLabel* cbLabel = new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Estimate playtime after load", checkBox, true);
		cbLabel->setFont(PPFont::getFont(PPFont::FONT_TINY));
		container->addControl(cbLabel);
		

		y2+=10;

#ifndef __LOWRES__
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_SCOPES, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Show scopes:", checkBox, true));
		y2+=12;
#endif

		container->addControl(new PPStaticText(STATICTEXT_SETTINGS_SCOPESAPPEARANCE, NULL, NULL, PPPoint(x2 + 2, y2), "Scope Style:", true));

		PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_SCOPESAPPEARANCE, screen, this, PPPoint(x2, y2+10), PPSize(160, 42));
		radioGroup->setColor(TrackerConfig::colorThemeMain);
		radioGroup->setFont(PPFont::getFont(PPFont::FONT_TINY));
		radioGroup->addItem("Dots");
		radioGroup->addItem("Solid");
		radioGroup->addItem("Smooth Lines");
		container->addControl(radioGroup);

		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 v = settingsDatabase->restore("AUTOESTPLAYTIME")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_AUTOESTPLAYTIME))->checkIt(v!=0);

		v = settingsDatabase->restore("INTERNALDISKBROWSER")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_INTERNALDISKBROWSER))->checkIt(v!=0);

		v = settingsDatabase->restore("SHOWSPLASH")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_SHOWSPLASH))->checkIt(v!=0);

		v = settingsDatabase->restore("SCOPES")->getIntValue();
#ifndef __LOWRES__
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_SCOPES))->checkIt(v & 1);

		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_SCOPESAPPEARANCE))->enable((v & 1) != 0);
		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SETTINGS_SCOPESAPPEARANCE))->enable((v & 1) != 0);
#endif
		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_SCOPESAPPEARANCE))->setChoice(v >> 1);
	}

};

class TabPageMisc_4 : public TabPage
{
public:
	TabPageMisc_4(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2), "Other", true, true));
		y2+=4+11;
		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_INVERTMWHEELZOOM, screen, this, PPPoint(x2 + 4 + 17 * 8 + 4, y2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x2 + 2, y2), "Inv. mwheel zoom:", checkBox, true));
		

		y2+=12;
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 v = settingsDatabase->restore("INVERTMWHEELZOOM")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_INVERTMWHEELZOOM))->checkIt(v!=0);
	}
};

class TabPageTabs_1 : public TabPage
{
public:
	TabPageTabs_1(pp_uint32 id, SectionSettings& sectionSettings) :
		TabPage(id, sectionSettings)
	{
	}

	virtual void init(PPScreen* screen)
	{
		pp_int32 x = 0;
		pp_int32 y = 0;

		container = new PPTransparentContainer(id, screen, this, PPPoint(x, y), PPSize(PageWidth,PageHeight));

		pp_int32 x2 = x;
		pp_int32 y2 = y;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Load module" PPSTR_PERIODS, true, true));

		y2+=12;
		PPCheckBox* checkBox = new PPCheckBox(CHECKBOX_SETTINGS_LOADMODULEINNEWTAB, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 + 2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(0, NULL, this, PPPoint(x + 4, y2 + 2), PPSTR_PERIODS"in new Tab", checkBox, true));
		
		y2+=12+3;

		container->addControl(new PPSeperator(0, screen, PPPoint(x2, y2), 158, TrackerConfig::colorThemeMain, true));

		y2+=3;

		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 2, y2 + 2), "Stop background", true, true));

		PPRadioGroup* radioGroup = new PPRadioGroup(RADIOGROUP_SETTINGS_STOPBACKGROUNDBEHAVIOUR, screen, this, PPPoint(x2, y2+2+11), PPSize(160, 42));
		radioGroup->setColor(TrackerConfig::colorThemeMain);

		radioGroup->addItem("Never");
		radioGroup->addItem("On Tab switch");
		radioGroup->addItem("On Playback");
		container->addControl(radioGroup);

		y2+=3*14 + 14;
		checkBox = new PPCheckBox(CHECKBOX_SETTINGS_TABSWITCHRESUMEPLAY, screen, this, PPPoint(x + 4 + 17 * 8 + 4, y2 + 2 - 1));
		container->addControl(checkBox);
		container->addControl(new PPCheckBoxLabel(STATICTEXT_SETTINGS_TABSWITCHRESUMEPLAY, NULL, this, PPPoint(x + 4, y2 + 2), "Tab-switch resume", checkBox, true));
				
		//container->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	}

	virtual void update(PPScreen* screen, TrackerSettingsDatabase* settingsDatabase, ModuleEditor& moduleEditor)
	{
		pp_int32 v = settingsDatabase->restore("TABS_STOPBACKGROUNDBEHAVIOUR")->getIntValue();
		static_cast<PPRadioGroup*>(container->getControlByID(RADIOGROUP_SETTINGS_STOPBACKGROUNDBEHAVIOUR))->setChoice(v);

		static_cast<PPStaticText*>(container->getControlByID(STATICTEXT_SETTINGS_TABSWITCHRESUMEPLAY))->enable(v != 0);
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_TABSWITCHRESUMEPLAY))->enable(v != 0);

		v = settingsDatabase->restore("TABS_TABSWITCHRESUMEPLAY")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_TABSWITCHRESUMEPLAY))->checkIt(v != 0);

		v = settingsDatabase->restore("TABS_LOADMODULEINNEWTAB")->getIntValue();
		static_cast<PPCheckBox*>(container->getControlByID(CHECKBOX_SETTINGS_LOADMODULEINNEWTAB))->checkIt(v != 0);
	}

};

SectionSettings::SectionSettings(Tracker& theTracker) :
	SectionAbstract(theTracker, NULL, new DialogResponderSettings(*this)),
	sectionContainer(NULL),
	listBoxColors(NULL),
	listBoxFontFamilies(NULL),
	listBoxFontEntries(NULL),
	currentActiveTabNum(0),
	currentActivePageStart(0),
	visible(false),
	palette(NULL),
	storePalette(false),
	colorCopy(NULL),
	lastColorFile(TrackerConfig::untitledSong)
{
	pp_int32 i;

	for (i = 0; i < NUMSETTINGSPAGES; i++)
	{
		tabPages.add(new PPSimpleVector<TabPage>());
		currentActiveSubPageNum[i] = 0;
	}

	initColorDescriptors();
	currentColor.set(128, 128, 128);

	palette = new TColorPalette();
	mixerSettings = new TMixerSettings();

	// predefined color palettes
	predefinedColorPalettes = new ColorPaletteContainer(TrackerConfig::numPredefinedColorPalettes);

	for (i = 0; i < getNumPredefinedColorPalettes(); i++)
	{
		/*TColorPalette pal;
		pal.numColors = GlobalColorConfig::ColorLast;
		for (j = 0; j < pal.numColors; j++)
		{
			pal.colors[j].set(rand()&255, rand()&255, rand()&255);
		}*/
		predefinedColorPalettes->store(i, ColorPaletteContainer::decodePalette(TrackerConfig::predefinedColorPalettes[i]));
	}

	for (i = 0; i < GlobalColorConfig::ColorLast; i++)
		colorMapping[i] = GlobalColorConfig::ColorLast-1;

	colorMapping[0] = GlobalColorConfig::ColorTheme;
	colorMapping[1] = GlobalColorConfig::ColorForegroundText;
	colorMapping[2] = GlobalColorConfig::ColorTextHighlited;
	colorMapping[3] = GlobalColorConfig::ColorRowHighlitedFirst;
	colorMapping[4] = GlobalColorConfig::ColorTextHighlitedSecond;
	colorMapping[5] = GlobalColorConfig::ColorRowHighlitedSecond;
	colorMapping[6] = GlobalColorConfig::ColorPatternNote;
	colorMapping[7] = GlobalColorConfig::ColorPatternInstrument;
	colorMapping[8] = GlobalColorConfig::ColorPatternVolume;
	colorMapping[9] = GlobalColorConfig::ColorPatternEffect;
	colorMapping[10] = GlobalColorConfig::ColorPatternOperand;
	colorMapping[11] = GlobalColorConfig::ColorCursor;
	colorMapping[12] = GlobalColorConfig::ColorCursorLine;
	colorMapping[13] = GlobalColorConfig::ColorCursorLineHighlighted;
	colorMapping[14] = GlobalColorConfig::ColorPatternSelection;
	colorMapping[15] = GlobalColorConfig::ColorButtons;
	colorMapping[16] = GlobalColorConfig::ColorButtonText;
	colorMapping[17] = GlobalColorConfig::ColorRecordModeButtonText;
	colorMapping[18] = GlobalColorConfig::ColorSelection;
	colorMapping[19] = GlobalColorConfig::ColorListBoxBackground;
	colorMapping[20] = GlobalColorConfig::ColorScrollBarBackground;
	colorMapping[21] = GlobalColorConfig::ColorScopes;
	colorMapping[22] = GlobalColorConfig::ColorScopesRecordIndicator;
	colorMapping[23] = GlobalColorConfig::ColorPeakClipIndicator;
	colorMapping[24] = GlobalColorConfig::ColorSampleEditorWaveform;
}

SectionSettings::~SectionSettings()
{
	delete predefinedColorPalettes;
	delete palette;
	delete mixerSettings;
	delete colorCopy;
}

pp_int32 SectionSettings::getColorIndex()
{
	return listBoxColors ? colorMapping[listBoxColors->getSelectedIndex()] : colorMapping[0];
}

void SectionSettings::showRestartMessageBox()
{
	if (tracker.settingsDatabase->restore("XRESOLUTION")->getIntValue() != tracker.settingsDatabaseCopy->restore("XRESOLUTION")->getIntValue() ||
		tracker.settingsDatabase->restore("YRESOLUTION")->getIntValue() != tracker.settingsDatabaseCopy->restore("YRESOLUTION")->getIntValue() ||
		tracker.settingsDatabase->restore("SCREENSCALEFACTOR")->getIntValue() != tracker.settingsDatabaseCopy->restore("SCREENSCALEFACTOR")->getIntValue())
	{
		SystemMessage message(*tracker.screen, SystemMessage::MessageResChangeRestart);
		message.show();
	}
}

pp_int32 SectionSettings::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case PAGE_BUTTON_0:
			case PAGE_BUTTON_1:
			case PAGE_BUTTON_2:
			case PAGE_BUTTON_3:
			case PAGE_BUTTON_4:
			{
				pp_int32 i = reinterpret_cast<PPControl*>(sender)->getID()-PAGE_BUTTON_0;
				showPage(i, currentActiveSubPageNum[i]);
				update();
				break;
			}

			case SUBPAGE_BUTTON_LEFT_0:
			case SUBPAGE_BUTTON_LEFT_1:
			case SUBPAGE_BUTTON_LEFT_2:
			case SUBPAGE_BUTTON_LEFT_3:
			case SUBPAGE_BUTTON_LEFT_4:
			{
				pp_int32 i = currentActiveSubPageNum[currentActiveTabNum] - 1;
				if (i>=0)
				{
					showPage(currentActiveTabNum, i);
					update();
				}
				break;
			}

			case SUBPAGE_BUTTON_RIGHT_0:
			case SUBPAGE_BUTTON_RIGHT_1:
			case SUBPAGE_BUTTON_RIGHT_2:
			case SUBPAGE_BUTTON_RIGHT_3:
			case SUBPAGE_BUTTON_RIGHT_4:
			{
				pp_int32 i = currentActiveSubPageNum[currentActiveTabNum] + 1;
				if (i<tabPages.get(currentActiveTabNum)->size())
				{
					showPage(currentActiveTabNum, i);
					update();
				}
				break;
			}

			case BUTTON_SETTINGS_OK:
			{
				if (event->getID() != eCommand)
					break;

				showRestartMessageBox();

				// Store new palette settings
				saveCurrentGlobalPalette();
				storeCurrentPaletteToDatabase();
				tracker.applySettings(tracker.settingsDatabase, tracker.settingsDatabaseCopy);

				// Store new mixer settings
				saveCurrentMixerSettings(*mixerSettings);

				show(false);
				tracker.sectionSamples->update(false);
				tracker.screen->paint();

				break;
			}

			case BUTTON_SETTINGS_APPLY:
			{
				if (event->getID() != eCommand)
					break;

				showRestartMessageBox();

				// Store new palette settings
				saveCurrentGlobalPalette();
				storeCurrentPaletteToDatabase();
				tracker.applySettings(tracker.settingsDatabase, tracker.settingsDatabaseCopy);

				// Store new mixer settings
				saveCurrentMixerSettings(*mixerSettings);

				update(false);
				tracker.sectionSamples->update(false);
				tracker.screen->paint();

				delete tracker.settingsDatabaseCopy;
				tracker.settingsDatabaseCopy = new TrackerSettingsDatabase(*tracker.settingsDatabase);
				break;
			}

			case BUTTON_SETTINGS_CANCEL:
			{
				if (event->getID() != eCommand)
					break;

				cancelSettings();
				break;
			}

			case BUTTON_SETTINGS_CHOOSEDRIVER:
			{
				if (event->getID() != eCommand)
					break;

				showSelectDriverMessageBox();
				break;
			}

			case CHECKBOX_SETTINGS_RAMPING:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("RAMPING", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case BUTTON_SETTINGS_RESAMPLING:
			{
				if (event->getID() != eCommand)
					break;

				showResamplerMessageBox();
				break;
			}

			case CHECKBOX_SETTINGS_FORCEPOWER2BUFF:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("FORCEPOWEROFTWOBUFFERSIZE", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_VIRTUALCHANNELS:
			{
				if (reinterpret_cast<PPCheckBox*>(sender)->isChecked())
					tracker.settingsDatabase->store("VIRTUALCHANNELS", 32);
				else
					tracker.settingsDatabase->store("VIRTUALCHANNELS", 0);

				update();
				break;
			}

			case BUTTON_SETTINGS_VIRTUALCHANNELS_PLUS:
			{
				pp_int32 v = tracker.settingsDatabase->restore("VIRTUALCHANNELS")->getIntValue() + 1;
				if (v > 99)
					v = 99;
				tracker.settingsDatabase->store("VIRTUALCHANNELS", v);
				update();
				break;
			}

			case BUTTON_SETTINGS_VIRTUALCHANNELS_MINUS:
			{
				pp_int32 v = tracker.settingsDatabase->restore("VIRTUALCHANNELS")->getIntValue() - 1;
				if (v < 0)
					v = 0;
				tracker.settingsDatabase->store("VIRTUALCHANNELS", v);
				update();
				break;
			}

			case CHECKBOX_SETTINGS_MULTICHN_RECORD:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("MULTICHN_RECORD", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_MULTICHN_KEYJAZZ:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("MULTICHN_KEYJAZZ", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_MULTICHN_EDIT:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("MULTICHN_EDIT", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_MULTICHN_RECORDKEYOFF:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("MULTICHN_RECORDKEYOFF", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_MULTICHN_RECORDNOTEDELAY:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("MULTICHN_RECORDNOTEDELAY", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_HEXCOUNT:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("HEXCOUNT", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_SHOWZEROEFFECT:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("SHOWZEROEFFECT", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_FULLSCREEN:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("FULLSCREEN", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_INSTRUMENTBACKTRACE:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("INSTRUMENTBACKTRACE", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_AUTORESIZE:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("PATTERNAUTORESIZE", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_TABTONOTE:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("TABTONOTE", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_CLICKTOCURSOR:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("CLICKTOCURSOR", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_WRAPCURSOR:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("WRAPAROUND", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_PROSPECTIVE:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("PROSPECTIVE", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case BUTTON_HIGHLIGHTMODULO1_PLUS:
			{
				pp_int32 v = tracker.settingsDatabase->restore("HIGHLIGHTMODULO1")->getIntValue() + 1;
				if (v > 99)
					v = 99;
				tracker.settingsDatabase->store("HIGHLIGHTMODULO1", v);
				update();
				break;
			}

			case BUTTON_HIGHLIGHTMODULO1_MINUS:
			{
				pp_int32 v = tracker.settingsDatabase->restore("HIGHLIGHTMODULO1")->getIntValue() - 1;
				if (v < 1)
					v = 1;
				tracker.settingsDatabase->store("HIGHLIGHTMODULO1", v);
				update();
				break;
			}

			case BUTTON_HIGHLIGHTMODULO2_PLUS:
			{
				pp_int32 v = tracker.settingsDatabase->restore("HIGHLIGHTMODULO2")->getIntValue() + 1;
				if (v > 99)
					v = 99;
				tracker.settingsDatabase->store("HIGHLIGHTMODULO2", v);
				update();
				break;
			}

			case CHECKBOX_HIGHLIGHTMODULO1_FULLROW:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("HIGHLIGHTROW1", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				break;
			}

			case BUTTON_HIGHLIGHTMODULO2_MINUS:
			{
				pp_int32 v = tracker.settingsDatabase->restore("HIGHLIGHTMODULO2")->getIntValue() - 1;
				if (v < 1)
					v = 1;
				tracker.settingsDatabase->store("HIGHLIGHTMODULO2", v);
				update();
				break;
			}

			case CHECKBOX_HIGHLIGHTMODULO2_FULLROW:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("HIGHLIGHTROW2", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				break;
			}

			case CHECKBOX_SETTINGS_SCOPES:
			{
				if (event->getID() != eCommand)
					break;

				mp_sint32 type = static_cast<PPRadioGroup*>(sectionContainer->getControlByID(RADIOGROUP_SETTINGS_SCOPESAPPEARANCE))->getChoice();
				mp_sint32 value = (reinterpret_cast<PPCheckBox*>(sender)->isChecked() ? 1 : 0) | (type << 1);
				tracker.settingsDatabase->store("SCOPES", value);
				update();
				break;
			}

			case CHECKBOX_SETTINGS_INVERTMWHEELZOOM:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("INVERTMWHEELZOOM", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				break;
			}

			//case CHECKBOX_SETTINGS_FOLLOWSONG:
			//{
			//	if (event->getID() != eCommand)
			//		break;

			//	tracker.settingsDatabase->store("FOLLOWSONG", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
			//	update();
			//	break;
			//}

			case CHECKBOX_SETTINGS_SAMPLEEDITORUNDO:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("SAMPLEEDITORUNDOBUFFER", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_AUTOMIXDOWNSAMPLES:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("AUTOMIXDOWNSAMPLES", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_AUTOESTPLAYTIME:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("AUTOESTPLAYTIME", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_SHOWSPLASH:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("SHOWSPLASH", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_INTERNALDISKBROWSER:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("INTERNALDISKBROWSER", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_TABSWITCHRESUMEPLAY:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("TABS_TABSWITCHRESUMEPLAY", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case CHECKBOX_SETTINGS_LOADMODULEINNEWTAB:
			{
				if (event->getID() != eCommand)
					break;

				tracker.settingsDatabase->store("TABS_LOADMODULEINNEWTAB", (pp_int32)reinterpret_cast<PPCheckBox*>(sender)->isChecked());
				update();
				break;
			}

			case BUTTON_COLOR_EXPORT:
			{
				if (event->getID() != eCommand)
					break;

				exportCurrentColorPalette();
				break;
			}

			case BUTTON_COLOR_IMPORT:
			{
				if (event->getID() != eCommand)
					break;

				importCurrentColorPalette();
				break;
			}

			case BUTTON_COLOR_PREVIEW:
			{
				if (event->getID() != eCommand)
					break;

				updateCurrentColors();
				break;
			}

			case BUTTON_COLOR_RESTORE:
			{
				if (event->getID() != eCommand)
					break;
				showRestorePaletteMessageBox();
				break;
			}

			// predefined color handling
			case BUTTON_COLOR_PREDEF_0:
			case BUTTON_COLOR_PREDEF_1:
			case BUTTON_COLOR_PREDEF_2:
			case BUTTON_COLOR_PREDEF_3:
			case BUTTON_COLOR_PREDEF_4:
			case BUTTON_COLOR_PREDEF_5:
			{
				if (event->getID() != eCommand)
					break;

				pp_int32 i = reinterpret_cast<PPControl*>(sender)->getID() - BUTTON_COLOR_PREDEF_0;

				if (storePalette)
				{
					TColorPalette pal;

					colorDescriptors[getColorIndex()].colorCopy = currentColor;

					pal.numColors = GlobalColorConfig::ColorLast;
					for (pp_int32 j = 0; j < pal.numColors; j++)
						pal.colors[j] = colorDescriptors[j].colorCopy;

					predefinedColorPalettes->store(i, pal);
					storePalette = !storePalette;

					PPButton* button = static_cast<PPButton*>(sectionContainer->getControlByID(BUTTON_COLOR_PREDEF_STORE));
					if (!button)
						break;
					button->setPressed(storePalette);
				}
				else
				{
					const TColorPalette* pal = predefinedColorPalettes->restore(i);

					for (i = 0; i < pal->numColors; i++)
					{
						if (i < GlobalColorConfig::ColorLast)
						{
							colorDescriptors[i].colorCopy = pal->colors[i];
						}
					}

					currentColor = colorDescriptors[getColorIndex()].colorCopy;
				}

				update();
				break;
			}

			case BUTTON_COLOR_PREDEF_STORE:
			{
				if (event->getID() != eCommand)
					break;

				storePalette = !storePalette;
				PPButton* button = reinterpret_cast<PPButton*>(sender);

				button->setPressed(storePalette);

				tracker.screen->paintControl(button);
				break;
			}

			case BUTTON_COLOR_COPY:
			{
				if (event->getID() != eCommand)
					break;

				if (colorCopy == NULL)
					colorCopy = new PPColor();

				*colorCopy = currentColor;
				update();
				break;
			}

			case BUTTON_COLOR_PASTE:
			{
				if (event->getID() != eCommand || colorCopy == NULL)
					break;

				currentColor = *colorCopy;
				colorDescriptors[getColorIndex()].colorCopy = currentColor;
				update();
				break;
			}

			case BUTTON_COLOR_DARKER:
			{
				currentColor.scale(0.5f);
				colorDescriptors[getColorIndex()].colorCopy = currentColor;
				update();
				break;
			}

			case BUTTON_COLOR_BRIGHTER:
			{
				currentColor.scale(2.0f);
				colorDescriptors[getColorIndex()].colorCopy = currentColor;
				update();
				break;
			}

			case BUTTON_RESOLUTIONS_CUSTOM:
			{
				if (event->getID() != eCommand)
					break;
				showCustomResolutionMessageBox();
				break;
			}

			case BUTTON_RESOLUTIONS_FULL:
			{
				if (event->getID() != eCommand)
					break;

				retrieveDisplayResolution();
				break;
			}

		}
	}
	else if (event->getID() == eValueChanged)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case SLIDER_SETTINGS_BUFFERSIZE:
			{
				pp_uint32 v = (reinterpret_cast<PPSlider*>(sender)->getCurrentValue()+1) << 5;
				tracker.settingsDatabase->store("BUFFERSIZE", v);
				update();
				break;
			}
			case SLIDER_SETTINGS_MIXERVOL:
			{
				pp_uint32 v = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				tracker.settingsDatabase->store("MIXERVOLUME", v);
				update();
				break;
			}

			case SLIDER_SPACING:
			{
				pp_uint32 v = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				tracker.settingsDatabase->store("SPACING", v);
				update();
				break;
			}

			case SLIDER_MUTEFADE:
			{
				pp_uint32 v = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				tracker.settingsDatabase->store("MUTEFADE", v);
				update();
				break;
			}

			// Color manipulation
			case SLIDER_COLOR_RED:
				currentColor.r = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				colorDescriptors[getColorIndex()].colorCopy = currentColor;
				update();
				break;
			case SLIDER_COLOR_GREEN:
				currentColor.g = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				colorDescriptors[getColorIndex()].colorCopy = currentColor;
				update();
				break;
			case SLIDER_COLOR_BLUE:
				currentColor.b = reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				colorDescriptors[getColorIndex()].colorCopy = currentColor;
				update();
				break;
		}
	}
	else if (event->getID() == eSelection)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			/*case RADIOGROUP_SETTINGS_PAGE:
			{
				showPage(reinterpret_cast<PPRadioGroup*>(sender)->getChoice());
				update();
				break;
			}*/


			case RADIOGROUP_SETTINGS_AMPLIFY:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
				tracker.settingsDatabase->store("MIXERSHIFT", v);
				update();
				break;
			}

			case RADIOGROUP_SETTINGS_MIXFREQ:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();

				ASSERT(v >= 0 && v < TrackerConfig::numMixFrequencies);
				tracker.settingsDatabase->store("MIXERFREQ", TrackerConfig::mixFrequencies[v]);
				update();
				break;
			}

			case RADIOGROUP_SETTINGS_FREQTAB:
			{
				// stop song without resetting main volume
				tracker.ensureSongStopped(false, false);
				pp_uint32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
				tracker.moduleEditor->setFrequency((ModuleEditor::Frequencies)v);
				update();
				tracker.ensureSongPlaying(true);
				break;
			}
                
            case RADIOGROUP_SETTINGS_XMCHANNELLIMIT:
            {
                pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
                
                ASSERT(v >= 0 && v < 3);
                tracker.settingsDatabase->store("XMCHANNELLIMIT", 1 << (v + 5));
                update();
                break;
            }

			case RADIOGROUP_SETTINGS_PATTERNFONT:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
				tracker.settingsDatabase->store("PATTERNFONT", v);
				update();
				break;
			}

			case RADIOGROUP_SETTINGS_EDITMODE:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
#ifdef __LOWRES__
				if (v != 0)
				{
					SystemMessage message(*tracker.screen, SystemMessage::MessageLimitedInput);
					message.show();
				}
#endif
				tracker.settingsDatabase->store("EDITMODE", v);
				update();
				break;
			}

			case RADIOGROUP_SETTINGS_SCROLLMODE:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
				tracker.settingsDatabase->store("SCROLLMODE", v);
				update();
				break;
			}

			case RADIOGROUP_SETTINGS_SCOPESAPPEARANCE:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
				pp_int32 res = (tracker.settingsDatabase->restore("SCOPES")->getIntValue() & 1) | (v << 1);
				tracker.settingsDatabase->store("SCOPES", res);
				update();
				break;
			}

			case RADIOGROUP_SETTINGS_MAGNIFY:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
				tracker.settingsDatabase->store("SCREENSCALEFACTOR", 1 << v);
				update();
				break;
			}

			case LISTBOX_SETTINGS_RESOLUTIONS:
			{
				pp_int32 v = *((pp_int32*)event->getDataPtr());
				if (v < NUMRESOLUTIONS-1)
				{
					tracker.settingsDatabase->store("XRESOLUTION", resolutions[v].width);
					tracker.settingsDatabase->store("YRESOLUTION", resolutions[v].height);
				}
				update();
			}

			case LISTBOX_COLORS:
			{
				currentColor = colorDescriptors[getColorIndex()].colorCopy;
				update();
				break;
			}

			case LISTBOX_SETTINGS_FONTFAMILIES:
			{
				enumerateFontFacesInListBox(*((pp_int32*)event->getDataPtr()));
				update();
				break;
			}

			case LISTBOX_SETTINGS_FONTENTRIES:
			{
				pp_int32 index = *((pp_int32*)event->getDataPtr());
				//PPFont::selectFontFace((PPFont::FontID)listBoxFontFamilies->getSelectedIndex(), listBoxFontEntries->getItem(index));
				tracker.settingsDatabase->store(PPFont::getFamilyInternalName((PPFont::FontID)listBoxFontFamilies->getSelectedIndex()), listBoxFontEntries->getItem(index));
				update();
				break;
			}

			case RADIOGROUP_SETTINGS_STOPBACKGROUNDBEHAVIOUR:
			{
				pp_int32 v = reinterpret_cast<PPRadioGroup*>(sender)->getChoice();
				tracker.settingsDatabase->store("TABS_STOPBACKGROUNDBEHAVIOUR", v);
				update();
				break;
			}
		}
	}

	return 0;
}

void SectionSettings::showSection(bool bShow)
{
	sectionContainer->show(bShow);
}

void SectionSettings::show(bool bShow)
{
	SectionAbstract::show(bShow);

	PPScreen* screen = tracker.screen;

	visible = bShow;

	if (!initialised)
	{
		init();
	}

	if (initialised)
	{
		PatternEditorControl* control = tracker.getPatternEditorControl();
		if (bShow)
		{
			tracker.showMainMenu(false, true);
			update(false);
			if (control)
			{
/*#ifndef
				control->setLocation(PPPoint(0, SECTIONHEIGHT));
				control->setSize(PPSize(screen->getWidth(),screen->getHeight()-SECTIONHEIGHT));*/
#ifdef __LOWRES__
				control->setLocation(PPPoint(0, 0));
				control->setSize(PPSize(screen->getWidth(),tracker.MAXEDITORHEIGHT()-SECTIONHEIGHT));
#endif
			}
			tracker.hideInputControl();

			// backup current palette
			saveCurrentGlobalPalette();
			updateColors();
			// backup current color
			currentColor = colorDescriptors[getColorIndex()].colorCopy;

			// backup current mixer settings
			saveCurrentMixerSettings(*mixerSettings);
		}
		else
		{
			tracker.showMainMenu(true, true);
			tracker.rearrangePatternEditorControl();
		}

		showSection(bShow);

		// why should we do that? just show the last active tab
		//showPage(0);

		screen->paint();
	}
}

void SectionSettings::cancelSettings()
{
	restoreCurrentGlobalPalette();
	restoreCurrentMixerSettings();

	if (tracker.settingsDatabaseCopy)
	{
		tracker.applySettings(tracker.settingsDatabaseCopy, tracker.settingsDatabase, false);
		delete tracker.settingsDatabase;
		tracker.settingsDatabase = tracker.settingsDatabaseCopy;
		tracker.settingsDatabaseCopy = NULL;
	}
	show(false);
}

void SectionSettings::init()
{
#ifndef __LOWRES__
	init(0,0);
#else
	init(0,tracker.screen->getHeight()-SECTIONHEIGHT);
#endif
}

void SectionSettings::init(pp_int32 x, pp_int32 y)
{
	pp_int32 i;

	PPScreen* screen = tracker.screen;

	pp_int32 y2 = y;

	sectionContainer = new PPContainer(CONTAINER_SETTINGS, screen, this, PPPoint(x, y2), PPSize(screen->getWidth(),SECTIONHEIGHT), false);
	static_cast<PPContainer*>(sectionContainer)->setColor(TrackerConfig::colorThemeMain);

#ifdef __LOWRES__
	pp_int32 x2 = 0;
#else
	pp_int32 x2 = 160;
#endif

	while (x2 < screen->getWidth())
	{
		sectionContainer->addControl(new PPSeperator(0, screen, PPPoint(x2 + 158, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));

		x2+=TabPage::getWidth();
	}

	tabPages.get(0)->add(new TabPageIO_1(PAGE_IO_1, *this));
	tabPages.get(0)->add(new TabPageIO_2(PAGE_IO_2, *this));
	tabPages.get(0)->add(new TabPageIO_3(PAGE_IO_3, *this));
#ifndef __LOWRES__
    tabPages.get(0)->add(new TabPageIO_4(PAGE_IO_4, *this));
#endif

	tabPages.get(1)->add(new TabPageLayout_1(PAGE_LAYOUT_1, *this));
	tabPages.get(1)->add(new TabPageLayout_2(PAGE_LAYOUT_2, *this));
#ifndef __LOWRES__
	tabPages.get(1)->add(new TabPageLayout_3(PAGE_LAYOUT_3, *this));
	tabPages.get(1)->add(new TabPageLayout_4(PAGE_LAYOUT_4, *this));
#endif

	tabPages.get(2)->add(new TabPageFonts_1(PAGE_FONTS_1, *this));
	tabPages.get(2)->add(new TabPageFonts_2(PAGE_FONTS_2, *this));

	tabPages.get(3)->add(new TabPageMisc_1(PAGE_MISC_1, *this));
	tabPages.get(3)->add(new TabPageMisc_2(PAGE_MISC_2, *this));
	tabPages.get(3)->add(new TabPageMisc_3(PAGE_MISC_3, *this));
	tabPages.get(3)->add(new TabPageMisc_4(PAGE_MISC_4, *this));

#ifndef __LOWRES__
	tabPages.get(4)->add(new TabPageTabs_1(PAGE_TABS_1, *this));
#endif

	for (i = 0; i < tabPages.size(); i++)
		for (pp_int32 j = 0; j < tabPages.get(i)->size(); j++)
		{
			tabPages.get(i)->get(j)->init(screen);
			sectionContainer->addControl(tabPages.get(i)->get(j)->getContainer());
		}

	PPButton* button;

	const pp_int32 numSettingsPages = NUMSETTINGSPAGES;

#ifndef __LOWRES__
	const char* subSettingsTexts[] = {"I/O","Layout","Fonts","Misc.","Tabs"};

	x2 = x;

	static_cast<PPContainer*>(sectionContainer)->addControl(new PPSeperator(0, screen, PPPoint(x2 + 156, y+4), SECTIONHEIGHT-8, TrackerConfig::colorThemeMain, false));
	static_cast<PPContainer*>(sectionContainer)->addControl(new PPSeperator(0, screen, PPPoint(x2 - 2, y+SECTIONHEIGHT-2-12-6), 157, TrackerConfig::colorThemeMain, true));

	pp_int32 bWidth = 140 - 14*2;
	pp_int32 bHeight = ((SECTIONHEIGHT - 2-12-6) - 8) / numSettingsPages;

	pp_int32 sx = x2 + 10;
	pp_int32 sy = y2 + 4;

	for (i = 0; i < numSettingsPages; i++)
	{
		button = new PPButton(PAGE_BUTTON_0+i, screen, this, PPPoint(sx, sy), PPSize(bWidth, bHeight), false, true, false);
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		button->setText(subSettingsTexts[i]);
		static_cast<PPContainer*>(sectionContainer)->addControl(button);

		button = new PPButton(SUBPAGE_BUTTON_LEFT_0+i, screen, this, PPPoint(sx+140-14*2+1, sy), PPSize(14, bHeight), false);
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		button->setText("<");
		static_cast<PPContainer*>(sectionContainer)->addControl(button);

		button = new PPButton(SUBPAGE_BUTTON_RIGHT_0+i, screen, this, PPPoint(sx+140-14+1, sy), PPSize(14, bHeight), false);
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		button->setText(">");
		static_cast<PPContainer*>(sectionContainer)->addControl(button);

		sy+=bHeight;
	}
	x2++;

#else
	const char* subSettingsTexts[] = {"I/O","Layout","Fonts","Misc."};

	x2 = screen->getWidth()-160 + 4;

	//static_cast<PPContainer*>(sectionContainer)->addControl(new PPSeperator(0, screen, PPPoint(x2 - 4, y+4), UPPERFRAMEHEIGHT-8, TrackerConfig::colorThemeMain, false));
	static_cast<PPContainer*>(sectionContainer)->addControl(new PPSeperator(0, screen, PPPoint(x + 2, y+UPPERFRAMEHEIGHT-4), screen->getWidth()-4, TrackerConfig::colorThemeMain, true));

	pp_int32 bWidth = (screen->getWidth()-8-26) / numSettingsPages;
	pp_int32 bHeight = 13;

	pp_int32 sx = x + 4;
	pp_int32 sy = y + UPPERFRAMEHEIGHT;

	for (i = 0; i < numSettingsPages; i++)
	{
		button = new PPButton(PAGE_BUTTON_0+i, screen, this, PPPoint(sx, sy), PPSize(bWidth, bHeight), false, true, false);
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		button->setText(subSettingsTexts[i]);
		static_cast<PPContainer*>(sectionContainer)->addControl(button);
		sx+=bWidth;
	}

	sx+=1;
	button = new PPButton(SUBPAGE_BUTTON_LEFT_0, screen, this, PPPoint(sx, sy), PPSize(13, bHeight), false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText("<");
	static_cast<PPContainer*>(sectionContainer)->addControl(button);

	button = new PPButton(SUBPAGE_BUTTON_RIGHT_0, screen, this, PPPoint(sx+13, sy), PPSize(13, bHeight), false);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	button->setText(">");
	static_cast<PPContainer*>(sectionContainer)->addControl(button);

	//static_cast<PPContainer*>(sectionContainer)->addControl(new PPSeperator(0, screen, PPPoint(x + 2, sy + bHeight + 1), screen->getWidth()-4, TrackerConfig::colorThemeMain, true));
#endif

	button = new PPButton(BUTTON_SETTINGS_OK, screen, this, PPPoint(x2+3, y+SECTIONHEIGHT-2-12), PPSize(46, 11));
	button->setText("Ok");

	static_cast<PPContainer*>(sectionContainer)->addControl(button);

	button = new PPButton(BUTTON_SETTINGS_APPLY, screen, this, PPPoint(x2+3+48, y+SECTIONHEIGHT-2-12), PPSize(48, 11));
	button->setText("Apply");

	static_cast<PPContainer*>(sectionContainer)->addControl(button);

	button = new PPButton(BUTTON_SETTINGS_CANCEL, screen, this, PPPoint(x2+3+48+50, y+SECTIONHEIGHT-2-12), PPSize(50, 11));
	button->setText("Cancel");

	static_cast<PPContainer*>(sectionContainer)->addControl(button);

	screen->addControl(sectionContainer);

	initialised = true;

	showPage(0, 0);

	showSection(false);
}

void SectionSettings::update(bool repaint/* = true*/)
{
	if (!initialised || !visible)
		return;

	pp_int32 i, j;

#ifdef __LOWRES__
	pp_int32 x = 0;
#else
	pp_int32 x = 160;
#endif
	pp_int32 y = sectionContainer->getLocation().y;

#ifdef __LOWRES__
	static_cast<PPButton*>(sectionContainer->getControlByID(SUBPAGE_BUTTON_LEFT_0))->enable(false);
	static_cast<PPButton*>(sectionContainer->getControlByID(SUBPAGE_BUTTON_RIGHT_0))->enable(false);
#endif

	// hide all tab pages first
	for (i = 0; i < tabPages.size(); i++)
	{
		for (j = 0; j < tabPages.get(i)->size(); j++)
		{
			tabPages.get(i)->get(j)->getContainer()->show(false);
			tabPages.get(i)->get(j)->setVisible(false);
		}

#ifndef __LOWRES__
		static_cast<PPButton*>(sectionContainer->getControlByID(SUBPAGE_BUTTON_LEFT_0+i))->enable(false);
		static_cast<PPButton*>(sectionContainer->getControlByID(SUBPAGE_BUTTON_RIGHT_0+i))->enable(false);
#endif
	}

	PPPoint location(x, y);

	pp_int32 lastVisiblePage = 0;

	for (j = 0; j < tabPages.get(currentActiveTabNum)->size(); j++)
	{
		if (j + currentActivePageStart < tabPages.get(currentActiveTabNum)->size())
		{
			TabPage* page = tabPages.get(currentActiveTabNum)->get(j + currentActivePageStart);
			page->setLocation(location);

			location.x+=TabPage::getWidth();

			if (location.x <= tracker.screen->getWidth())
			{
				page->getContainer()->show(true);
				page->setVisible(true);
				lastVisiblePage = j + currentActivePageStart;
			}
		}
		else
		{
			location.x+=TabPage::getWidth();
		}
	}

	i = currentActiveTabNum;
#ifdef __LOWRES__
	i = 0;
#endif
	PPButton* button = static_cast<PPButton*>(sectionContainer->getControlByID(SUBPAGE_BUTTON_RIGHT_0+i));
	button->enable(lastVisiblePage < tabPages.get(currentActiveTabNum)->size() - 1);

	button = static_cast<PPButton*>(sectionContainer->getControlByID(SUBPAGE_BUTTON_LEFT_0+i));
	button->enable(currentActivePageStart > 0);

	// update all visible pages
	for (i = 0; i < tabPages.size(); i++)
	{
		for (j = 0; j < tabPages.get(i)->size(); j++)
			if (tabPages.get(i)->get(j)-isVisible())
			{
				tabPages.get(i)->get(j)->update(tracker.screen, tracker.settingsDatabase, *tracker.moduleEditor);
			}

	}


	tracker.screen->paintControl(sectionContainer, false);

	if (repaint)
		tracker.screen->update();
}

void SectionSettings::showPage(pp_int32 page, pp_int32 subPage/* = 0*/)
{
	currentActiveTabNum = page;
	currentActivePageStart = subPage;

	currentActiveSubPageNum[currentActiveTabNum] = subPage;

	for (pp_int32 i = 0; i < NUMSETTINGSPAGES; i++)
		static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(PAGE_BUTTON_0+i))->setPressed(false);

	static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(PAGE_BUTTON_0+page))->setPressed(true);

	currentActiveSubPageNum[currentActiveTabNum] = subPage;
}

void SectionSettings::initColorDescriptors()
{
	for (pp_int32 i = 0; i < GlobalColorConfig::ColorLast; i++)
		colorDescriptors[i].readableDecription = GlobalColorConfig::getInstance()->getColorReadableDescription((GlobalColorConfig::GlobalColors)i);
}

void SectionSettings::updateColors()
{
	for (pp_int32 i = 0; i < GlobalColorConfig::ColorLast; i++)
		colorDescriptors[i].colorCopy = GlobalColorConfig::getInstance()->getColor((GlobalColorConfig::GlobalColors)i);
}

pp_int32 SectionSettings::getNumPredefinedColorPalettes()
{
	return TrackerConfig::numPredefinedColorPalettes;
}

PPString SectionSettings::getEncodedPalette(pp_int32 index)
{
	return ColorPaletteContainer::encodePalette(*predefinedColorPalettes->restore(index));
}

void SectionSettings::setEncodedPalette(pp_int32 index, const PPString& str)
{
	TColorPalette p = ColorPaletteContainer::decodePalette(str);

	predefinedColorPalettes->store(index, p);
}

void SectionSettings::storeCurrentPaletteToDatabase()
{
	TColorPalette pal;

	pal.numColors = GlobalColorConfig::ColorLast;
	for (pp_int32 j = 0; j < pal.numColors; j++)
		pal.colors[j] = colorDescriptors[j].colorCopy;

	tracker.settingsDatabase->store("ACTIVECOLORS", ColorPaletteContainer::encodePalette(pal));
}

void SectionSettings::saveCurrentGlobalPalette()
{
	palette->numColors = GlobalColorConfig::ColorLast;
	for (pp_int32 i = 0; i < GlobalColorConfig::ColorLast; i++)
	{
		if (i < palette->numColors)
			palette->colors[i] = GlobalColorConfig::getInstance()->getColor((GlobalColorConfig::GlobalColors)i);
	}
}

void SectionSettings::restoreCurrentGlobalPalette()
{
	for (pp_int32 i = 0; i < palette->numColors; i++)
	{
		if (i < GlobalColorConfig::ColorLast)
			GlobalColorConfig::getInstance()->setColor((GlobalColorConfig::GlobalColors)i, palette->colors[i]);
	}
}

void SectionSettings::updateCurrentColors()
{
	for (pp_int32 i = 0; i < GlobalColorConfig::ColorLast; i++)
		GlobalColorConfig::getInstance()->setColor((GlobalColorConfig::GlobalColors)i, colorDescriptors[i].colorCopy);

	tracker.screen->paint();
}

void SectionSettings::restorePalettes()
{
	pp_int32 i;
	for (i = 0; i < getNumPredefinedColorPalettes(); i++)
	{
		predefinedColorPalettes->store(i, ColorPaletteContainer::decodePalette(TrackerConfig::predefinedColorPalettes[i]));
	}

	*palette = ColorPaletteContainer::decodePalette(TrackerConfig::predefinedColorPalettes[0]);
	restoreCurrentGlobalPalette();

	for (i = 0; i < palette->numColors; i++)
		if (i < GlobalColorConfig::ColorLast)
			colorDescriptors[i].colorCopy = palette->colors[i];

	currentColor = colorDescriptors[getColorIndex()].colorCopy;

	tracker.screen->paint();
}

void SectionSettings::saveCurrentMixerSettings(TMixerSettings& settings)
{
	tracker.getMixerSettingsFromDatabase(settings, *tracker.settingsDatabase);
}

void SectionSettings::restoreCurrentMixerSettings()
{
	TMixerSettings newMixerSettings;
	tracker.getMixerSettingsFromDatabase(newMixerSettings, *tracker.settingsDatabase);

	if (*mixerSettings != newMixerSettings)
	{
		bool res = tracker.playerMaster->applyNewMixerSettings(*mixerSettings, true);
		if (!res)
		{
			SystemMessage message(*tracker.screen, SystemMessage::MessageSoundDriverInitFailed);
			message.show();
		}
	}
}

void SectionSettings::showCustomResolutionMessageBox()
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}

	dialog = new DialogWithValues(tracker.screen,
								  responder,
								  RESPONDMESSAGEBOX_CUSTOMRESOLUTION,
								  "Enter custom resolution" PPSTR_PERIODS,
								  DialogWithValues::ValueStyleEnterTwoValues);

	static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Width in pixels:");
	static_cast<DialogWithValues*>(dialog)->setValueTwoCaption("Height in pixels:");
	static_cast<DialogWithValues*>(dialog)->setValueOneRange(MINWIDTH, 10000.0f, 0);
	static_cast<DialogWithValues*>(dialog)->setValueTwoRange(MINHEIGHT, 10000.0f, 0);
	static_cast<DialogWithValues*>(dialog)->setValueOneIncreaseStep(1.0f);
	static_cast<DialogWithValues*>(dialog)->setValueTwoIncreaseStep(1.0f);

	pp_int32 width = tracker.settingsDatabase->restore("XRESOLUTION")->getIntValue();
	pp_int32 height = tracker.settingsDatabase->restore("YRESOLUTION")->getIntValue();

	static_cast<DialogWithValues*>(dialog)->setValueOne((float)width);
	static_cast<DialogWithValues*>(dialog)->setValueTwo((float)height);

	dialog->show();
}

void SectionSettings::showRestorePaletteMessageBox()
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}

	dialog = new PPDialogBase(tracker.screen,
							  responder,
							  RESPONDMESSAGEBOX_RESTOREPALETTES,
							  "Restore all default palettes?");
	dialog->show();
}

void SectionSettings::showSelectDriverMessageBox()
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}

	dialog = new DialogListBox(tracker.screen,
							   responder,
							   RESPONDMESSAGEBOX_SELECTAUDIODRV,
							   "Select audio driver",
							   true);
	PPListBox* listBox = static_cast<DialogListBox*>(dialog)->getListBox();

	mp_sint32 i = 0;
	mp_sint32 selectedIndex = -1;
	const char* name = tracker.playerMaster->getFirstDriverName();
	//const char* curDrvName = tracker.playerController->getCurrentDriverName();

	const char* curDrvName = tracker.settingsDatabase->restore("AUDIODRIVER")->getStringValue();

	while (name)
	{
		if (strcmp(name, curDrvName) == 0)
			selectedIndex = i;
		listBox->addItem(name);
		name = tracker.playerMaster->getNextDriverName();
		i++;
	}

	if (selectedIndex != -1)
		listBox->setSelectedIndex(selectedIndex, false);

	dialog->show();
}

void SectionSettings::showResamplerMessageBox()
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

	listBox->setSelectedIndex(tracker.settingsDatabase->restore("INTERPOLATION")->getIntValue(), false);

	dialog->show();
}

void SectionSettings::storeAudioDriver(const char* driverName)
{
	const char* curDrvName = tracker.playerMaster->getCurrentDriverName();
	if (strcmp(curDrvName, driverName) != 0)
	{
		tracker.settingsDatabase->store("AUDIODRIVER", driverName);

		TMixerSettings newMixerSettings;
		saveCurrentMixerSettings(newMixerSettings);
		bool res = tracker.playerMaster->applyNewMixerSettings(newMixerSettings, true);
		if (!res)
		{
			SystemMessage message(*tracker.screen, SystemMessage::MessageSoundDriverInitFailed);
			message.show();
		}
	}
	update();
}

void SectionSettings::storeResampler(pp_uint32 resampler)
{
	tracker.settingsDatabase->store("INTERPOLATION", resampler);

	TMixerSettings newMixerSettings;
	newMixerSettings.resampler = resampler;
	bool res = tracker.playerMaster->applyNewMixerSettings(newMixerSettings, true);
	if (!res)
	{
		SystemMessage message(*tracker.screen, SystemMessage::MessageSoundDriverInitFailed);
		message.show();
	}
}

void SectionSettings::enumerateFontFacesInListBox(pp_uint32 fontID)
{
	listBoxFontEntries->clear();
	const char* name = PPFont::getFirstFontEntryName((PPFont::FontID)fontID);
	while (name)
	{
		listBoxFontEntries->addItem(name);
		name = PPFont::getNextFontEntryName((PPFont::FontID)fontID);
	}
}

void SectionSettings::storeCustomResolution()
{
	pp_int32 width = (pp_int32)static_cast<DialogWithValues*>(dialog)->getValueOne();
	pp_int32 height = (pp_int32)static_cast<DialogWithValues*>(dialog)->getValueTwo();

	if (width < MINWIDTH)
		width = MINWIDTH;
	if (height < MINHEIGHT)
		height = MINHEIGHT;

	tracker.settingsDatabase->store("XRESOLUTION", width);
	tracker.settingsDatabase->store("YRESOLUTION", height);

	update();
}

void SectionSettings::importCurrentColorPalette()
{
	FileExtProvider fileExtProvider;

	PPOpenPanel panel(tracker.screen, "Open colors");

	panel.addExtensions(fileExtProvider.getColorExtensions());

	if (panel.runModal() == PPModalDialog::ReturnCodeOK)
	{
		TColorPalette pal;
		ColorExportImport exportImport(panel.getFileName());

		if (exportImport.importColorPalette(pal))
		{
			for (pp_int32 j = 0; j < pal.numColors; j++)
				colorDescriptors[j].colorCopy = pal.colors[j];

			currentColor = colorDescriptors[getColorIndex()].colorCopy;

			update();

			lastColorFile = panel.getFileName();
		}
		else
		{
			tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Unrecognized type/corrupt file", Tracker::MessageBox_OK);
		}
	}
}

void SectionSettings::exportCurrentColorPalette()
{
	FileExtProvider fileExtProvider;

	PPSystemString fileName = lastColorFile.stripPath().stripExtension();
	fileName.append(".");
	fileName.append(fileExtProvider.getColorExtension(FileExtProvider::ColorExtensionMCT));

	PPSavePanel panel(tracker.screen, "Save colors", fileName);

	panel.addExtensions(fileExtProvider.getColorExtensions());

	if (panel.runModal() == PPModalDialog::ReturnCodeOK)
	{
		TColorPalette pal;

		pal.numColors = GlobalColorConfig::ColorLast;
		for (pp_int32 j = 0; j < pal.numColors; j++)
			pal.colors[j] = colorDescriptors[j].colorCopy;

		ColorExportImport exportImport(panel.getFileName());

		if (!exportImport.exportColorPalette(pal))
		{
			tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Could not create file", Tracker::MessageBox_OK);
		}
	}
}

void SectionSettings::retrieveDisplayResolution()
{
	PPSize size = tracker.screen->getDisplayResolution();

	if (size.width > 0 && size.height > 0)
	{

		if (size.width < MINWIDTH)
			size.width = MINWIDTH;
		if (size.height < MINHEIGHT)
			size.height = MINHEIGHT;

		tracker.settingsDatabase->store("XRESOLUTION", size.width);
		tracker.settingsDatabase->store("YRESOLUTION", size.height);

		update();
	}
	else
	{
		tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Could not retrieve display resolution", Tracker::MessageBox_OK);
	}
}
