/*
 *  ppui/RespondMessageBoxFileSelector.h
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
 *  RespondMessageBoxFileSelector.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.03.06.
 *
 */

#ifndef __RESPONDMESSAGEBOXFILESELECTOR_H__
#define __RESPONDMESSAGEBOXFILESELECTOR_H__

#include "RespondMessageBox.h"
#include "SimpleVector.h"
//#include "UndoStack.h"

class PPListBoxFileBrowser;
class PPListBox;
class PPPath;

class PPPathEntry;
class PPSystemString;

class RespondMessageBoxFileSelector : public RespondMessageBox
{
private:
	PPListBoxFileBrowser* listBoxFiles;
	PPListBox* editFieldCurrentFile;
	PPSystemString* initialPath;
	PPSystemString* fileFullPath;
	
	RespondMessageBox* overwritePromptMessageBox;

	bool doOverwritePrompt;
	bool allowSelectDirectories;
	bool allowEditFileName;
	bool sortAscending;

	class OverwritePromptResponder : public RespondListenerInterface
	{
	private:
		RespondMessageBoxFileSelector& respondMessageBoxFileSelector;
		
	public:
		OverwritePromptResponder(RespondMessageBoxFileSelector& responder) :
			respondMessageBoxFileSelector(responder)
		{
		}		
		
		virtual pp_int32 ActionOkay(PPObject* sender);
		virtual pp_int32 ActionCancel(PPObject* sender);
	};
	
	OverwritePromptResponder* overwritePrompResponder;

	struct Descriptor
	{
		PPSystemString extension;
		PPSystemString description;

		Descriptor(const PPSystemString& ext, const PPSystemString& desc) :
			extension(ext), description(desc)
		{}
	};
	
	PPSimpleVector<Descriptor> extensions;
	
public:
	RespondMessageBoxFileSelector(PPScreen* screen, 
								  RespondListenerInterface* responder,
								  pp_int32 id,
								  const PPString& caption,
								  bool editfileName = false,
								  bool overwritePrompt = false,
								  bool selectDirectories = false);

	~RespondMessageBoxFileSelector();

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	

	virtual void show();

	const PPSystemString& getSelectedPathFull();

	void setCurrentEditFileName(const PPSystemString& name);	
	
	void addExtension(const PPSystemString& ext, const PPSystemString& desc);

private:	
	void updateButtonStates(bool repaint = true);

	pp_int32 baseClassHandleEvent(PPObject* sender, PPEvent* event);

	pp_int32 processKeys(PPObject* sender, PPEvent* event);

	pp_int32 confirm(const PPPathEntry& entry);
	pp_int32 confirm();

	pp_int32 discard();
	
	void refresh(bool repaint = true);
	void refreshCurrentFileEditField(bool repaint = true);
	
	void updateSelection(pp_int32 index, bool repaint = true);
	
	void gotoHome();
	void gotoRoot();
	void gotoParent();	
	bool stepInto(const PPPathEntry& entry);
	void gotoPath(const PPSystemString& path);
	
	void prev();
	void next();

	void refreshExtensions();
	
	void updateFilter();

	friend class OverwritePromptResponder;
};

#endif
