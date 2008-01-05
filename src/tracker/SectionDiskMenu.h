/*
 *  SectionDiskMenu.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.07.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SECTIONDISKMENU__H
#define SECTIONDISKMENU__H

#include "BasicTypes.h"
#include "SimpleVector.h"
#include "Event.h"
#include "SectionUpperLeft.h"

class PPControl;
class Tracker;
class PPListBoxFileBrowser;

class SectionDiskMenu : public SectionUpperLeft
{
private:
	enum ClassicViewStates
	{
		BrowseAll,
		BrowseModules,
		BrowseInstruments,
		BrowseSamples,
		BrowsePatterns,
		BrowseTracks,
		
		BrowseLAST // needs to be last
	};

	bool diskMenuVisible;

	PPSimpleVector<PPControl>* normalViewControls;
	PPSimpleVector<PPControl>* classicViewControls;
	ClassicViewStates classicViewState;
	bool classicViewVisible;
	
	bool forceClassicBrowser;
	bool moduleTypeAdjust;
	bool sortAscending;
	bool storePath;

	PPControl* lastFocusedControl;

	PPListBoxFileBrowser* listBoxFiles;
	class PPListBox* editFieldCurrentFile;
	class PPRadioGroup* currentActiveRadioGroup;

	PPPoint radioGroupLocations[5];
	
	// respond listener
	class RespondMessageBox* respondMessageBox;
	class MessageBoxResponderDisk* messageBoxResponder;

	PPSystemString* file;
	PPSystemString* fileFullPath;
	PPSystemString currentPath;

#ifdef __LOWRES__
	pp_int32 lastSIPOffsetMove;
	PPPoint lastPatternEditorControlLocation;
	PPSize lastPatternEditorControlSize;
#endif

	PPSize fileBrowserExtent;
	
	class ColorQueryListener* colorQueryListener;

public:
	SectionDiskMenu(Tracker& tracker);
	virtual ~SectionDiskMenu();

	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	virtual void init() { SectionUpperLeft::init(); }
	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow);
	virtual void update(bool repaint = true);

	pp_int32 getCurrentSelectedSampleSaveType();
	
	bool isActiveEditing();
	bool isFileBrowserVisible();
	bool fileBrowserHasFocus();
	
	void setFileBrowserShowFocus(bool showFocus);

	void selectSaveType(pp_uint32 type);

	static pp_uint32 getDefaultConfigUInt32();
	pp_uint32 getConfigUInt32();
	void setConfigUInt32(pp_uint32 config);

	PPSystemString getCurrentPath();
	void setCurrentPath(const PPSystemString& path, bool reload = true);
	PPString getCurrentPathASCII();

	void setModuleTypeAdjust(bool moduleTypeAdjust) { this->moduleTypeAdjust = moduleTypeAdjust; }
	
	bool isDiskMenuVisible() { return diskMenuVisible; }
	void resizeInstrumentContainer();
	
	void setCycleFilenames(bool cycleFilenames);
	
	PPListBoxFileBrowser* getListBoxFiles() { return listBoxFiles; }

private:
	void prepareSection();

	void showNormalView(bool bShow);
	
	void updateClassicView(bool repaint = true);
	
	void showClassicView(bool bShow);
	
	bool isNormalViewVisible();
	bool isClassicViewVisible();
	
	void flip();
	
	void updateFilenameEditFieldExtension(ClassicViewStates viewState);
	void updateFilenameEditField(ClassicViewStates viewState);
	void updateFilenameEditField(const PPSystemString& fileName);
	void updateFilenameEditFieldFromBrowser();
	
	void handleLoadOrStep();
	void loadCurrentSelectedFile();
	
	void showOverwriteMessageBox();
	void prepareSave();
	void saveCurrent();
	void showDeleteMessageBox();
	void deleteCurrent();

	void updateButtonStates(bool repaint = true);
	void next(bool repaint = true);
	void prev(bool repaint = true);
	void parent(bool repaint = true);
	void root(bool repaint = true);
	void home(bool repaint = true);
	void reload(bool repaint = true);

	void updateFilter(bool repaint = true);
	
	void switchState(ClassicViewStates viewState);

	void resizeBrowserVertically();
	void resizeBrowserHorizontally();
	void fitDirButtons();
	
	PPString getKeyFromPredefPathButton(PPControl* button);
	
	void assureExtension();
	
	// Responder should be friend
	friend class MessageBoxResponderDisk;		
	
	friend class Tracker;
};

#endif
