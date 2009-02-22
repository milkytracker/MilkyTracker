/*
 *  tracker/TabManager.h
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
 *  TabManager.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.12.07.
 *
 */

#ifndef __TABMANAGER_H__
#define __TABMANAGER_H__

#include "BasicTypes.h"

template<class Type>
class PPSimpleVector;
	
class ModuleEditor;
class PlayerController;
	
class TabManager
{
public:
	enum StopTabsBehaviours
	{
		StopTabsBehaviourNone,
		StopTabsBehaviourOnTabSwitch,
		StopTabsBehaviourOnPlayback
	};

private:
	class Tracker& tracker;
	
	struct Document
	{
		ModuleEditor* moduleEditor;
		PlayerController* playerController;

		Document(ModuleEditor* moduleEditor, PlayerController* playerController);
		~Document();
	};
	PPSimpleVector<Document>* documents;
	Document* currentDocument;
	
	bool stopOnTabSwitch;
	bool resumeOnTabSwitch;
	
	class TabHeaderControl* getTabHeaderControl();
	void applyPlayerDefaults(PlayerController* playerController);

	void selectModuleEditor(Document* document);

public:
	TabManager(Tracker& tracker);
	~TabManager();

	ModuleEditor* createModuleEditor();
	PlayerController* createPlayerController();	

	void setStopOnTabSwitch(bool stopOnTabSwitch) { this->stopOnTabSwitch = stopOnTabSwitch; }
	bool getStopOnTabSwitch() const { return stopOnTabSwitch; }

	void setResumeOnTabSwitch(bool resumeOnTabSwitch) { this->resumeOnTabSwitch = resumeOnTabSwitch; }
	bool getResumeOnTabSwitch() const { return resumeOnTabSwitch; }

	void openNewTab(PlayerController* playerController = NULL, ModuleEditor* moduleEditor = NULL);	
	void switchToTab(pp_uint32 index);
	void closeTab(pp_int32 index = -1);
	ModuleEditor* getModuleEditorFromTabIndex(pp_int32 index);
	PlayerController* getPlayerControllerFromTabIndex(pp_int32 index);
	pp_uint32 getSelectedTabIndex();
	
	void cycleTab(pp_int32 offset);
	
	pp_int32 getNumTabs() const;
};

#endif
