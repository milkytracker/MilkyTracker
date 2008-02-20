/*
 *  tracker/SectionSettings.h
 *
 *  Copyright 2008 Peter Barth
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
 *  SectionSettings.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 13 2005.
 *
 */

#ifndef SECTIONSETTINGS__H
#define SECTIONSETTINGS__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionAbstract.h"
#include "GlobalColorConfig.h"

#ifdef __LOWRES__
#define NUMSETTINGSPAGES	4
#else
#define NUMSETTINGSPAGES	5
#endif

class PPControl;
class PPContainer;
class PPListBox;
struct TColorPalette;
struct TMixerSettings;
class ColorPaletteContainer;

class SectionSettings : public SectionAbstract
{
private:
	struct TColorDescriptor
	{
		const char* readableDecription;
		PPColor colorCopy;
	};
	
	PPContainer* sectionContainer;
	PPContainer* currentActivePageContainer;

	PPListBox* listBoxColors;
	PPListBox* listBoxFontFamilies;
	PPListBox* listBoxFontEntries;
	PPColor currentColor, *colorCopy;

	bool visible;
	
	pp_int32 currentActivePageNum;
	
	PPSimpleVector<PPContainer>* pages[NUMSETTINGSPAGES];
	pp_int32 numSubPages[NUMSETTINGSPAGES];
	pp_int32 currentActiveSubPageNum[NUMSETTINGSPAGES];
	
	TColorDescriptor colorDescriptors[GlobalColorConfig::ColorLast];
	pp_int32 colorMapping[GlobalColorConfig::ColorLast];
	
	// Hold 5 predefined palettes
	ColorPaletteContainer* predefinedColorPalettes;
	TColorPalette* palette;
	bool storePalette;
	// backup of the current mixer settings, will be restored on cancel
	TMixerSettings* mixerSettings;
	
	void showPage(pp_int32 page, pp_int32 subPage = 0);
	
	void initPage_I(PPContainer* container, pp_int32 x, pp_int32 y);
	void initPage_I_2(PPContainer* container, pp_int32 x, pp_int32 y);

	void initPage_II(PPContainer* container, pp_int32 x, pp_int32 y);

	void initPage_III(PPContainer* container, pp_int32 x, pp_int32 y);

	void initPage_IV(PPContainer* container, pp_int32 x, pp_int32 y);
	void initPage_IV_2(PPContainer* container, pp_int32 x, pp_int32 y);

	void initPage_V(PPContainer* container, pp_int32 x, pp_int32 y);

	void updatePage_I();
	void updatePage_I_2();

	void updatePage_II();

	void updatePage_III();

	void updatePage_IV();
	void updatePage_IV_2();

	void updatePage_V();

	void showRestartMessageBox();
	
	void initColorDescriptors();
	void updateColors();
	
	pp_int32 getColorIndex();

protected:
	virtual void showSection(bool bShow);
	
public:
	SectionSettings(Tracker& tracker);
	virtual ~SectionSettings();

	// PPEvent listener
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void init();

	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow);
	virtual void update(bool repaint = true);
	
	void cancelSettings();
	
	bool isVisible() { return visible; }
	
private:
	// color palette handling
	pp_int32 getNumPredefinedColorPalettes();
	PPString getEncodedPalette(pp_int32 index);
	void setEncodedPalette(pp_int32 index, const PPString& str);
	
	void storeCurrentPaletteToDatabase();	
	
	void saveCurrentGlobalPalette();
	void restoreCurrentGlobalPalette();	
	
	void updateCurrentColors();
	void restorePalettes();

	// mixer settings handling
	void saveCurrentMixerSettings(TMixerSettings& settings);
	void restoreCurrentMixerSettings();

	// font face handling
	void enumerateFontFacesInListBox(pp_uint32 fontID);
	
	// Custom resolution
	void storeCustomResolution();
	
	// Show message box with custom resolutions
	void showCustomResolutionMessageBox();

	// Message box which asks if default palettes should
	// be restored
	void showRestorePaletteMessageBox();

	// Message box which asks for an audio driver
	void showSelectDriverMessageBox();

	// Message box with list of resampler
	void showResamplerMessageBox();

	void storeAudioDriver(const char* driverName);
	void storeResampler(pp_uint32 resampler);

	// Responder should be friend
	friend class DialogResponderSettings;	

	friend class Tracker;
};

#endif

