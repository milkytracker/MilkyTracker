/*
 *  ppui/ListBoxFileBrowser.h
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
 *  ListBoxFileBrowser.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 19.10.06.
 *
 */
#ifndef __LISTBOXFILEBROWSER_H__
#define __LISTBOXFILEBROWSER_H__

#include "ListBox.h"
#include "SimpleVector.h"
#include "UndoStack.h"

class PPListBoxFileBrowser : public PPListBox
{
public:
	enum SortTypes
	{
		SortByName,
		SortBySize,
		SortByExtension,
		NumSortRules
	};

private:
	class PPPath* currentPath;
	PPSystemString* initialPath;
	PPSystemString* fileFullPath;
	PPSimpleVector<class PPPathEntry> pathEntries;
	PPUndoStack<PPSystemString> history;	

	PPString filePrefix, fileSuffix;
	PPString directoryPrefix, directorySuffix;

	bool sortAscending;
	bool cycleFilenames;

	struct Descriptor
	{
		PPSystemString extension;
		PPSystemString description;

		Descriptor(const PPSystemString& ext, const PPSystemString& desc) :
			extension(ext), description(desc)
		{}
	};

	PPSimpleVector<Descriptor> items;

	SortTypes sortType;

public:
	PPListBoxFileBrowser(pp_int32 id, 
						 PPScreen* parentScreen, 
						 EventListenerInterface* eventListener, 
						 PPPoint location, 
						 PPSize size);
	
	virtual ~PPListBoxFileBrowser();
	
	virtual pp_int32 dispatchEvent(PPEvent* event);

	virtual bool receiveTimerEvent() { return false; }	
	
	void refreshFiles();
	
	void setSortAscending(bool sortAscending) { this->sortAscending = sortAscending; }
	void setCycleFilenames(bool cycleFilenames) { this->cycleFilenames = cycleFilenames; }
	void cycleSorting() { sortType = (SortTypes)(((pp_int32)sortType+1) % NumSortRules); }
	void setSortType(SortTypes sortType) 
	{ 
		this->sortType = sortType; 
		if (this->sortType >= NumSortRules)
			this->sortType = SortByName;
	}
	SortTypes getSortType() { return sortType; }
	
	void clearExtensions();	
	// must contain pairs of extensions / description
	// terminated by TWO NULL pointers
	void addExtensions(const char* extensions[]);
	void addExtension(const PPSystemString& ext, const PPSystemString& desc);
	
	PPPath& getCurrentPath() { return *currentPath; }
	PPSystemString getCurrentPathAsString();
	PPString getCurrentPathAsASCIIString();
	PPPathEntry* getPathEntry(pp_int32 index);
	PPPathEntry* getCurrentSelectedPathEntry() { return getPathEntry(PPListBox::getSelectedIndex()); }
	PPSimpleVector<class PPPathEntry>& getPathEntries() { return pathEntries; }

	bool canGotoHome();
	void gotoHome();
	bool canGotoRoot();
	void gotoRoot();
	bool canGotoParent();
	void gotoParent();

	bool currentSelectionIsFile();
	bool stepIntoCurrentSelection();
	bool stepInto(const PPPathEntry& entry);
	bool gotoPath(const PPSystemString& path, bool reload = true);
	
	bool canPrev();
	void prev();
	bool canNext();
	void next();

	void setFilePrefix(const PPString& prefix);
	void setFileSuffix(const PPString& suffix);
	void setDirectoryPrefix(const PPString& prefix);
	void setDirectorySuffix(const PPString& suffix);	
	void setDirectorySuffixPathSeperator();
	
private:
	void iterateFilesInFolder();
	void buildFileList();
	void sortFileList();
	void cycle(char chr);
	static void appendFileSize(PPString& name, const PPPathEntry& entry);
	
	bool checkExtension(const PPPathEntry& entry);
};

#endif	// __LISTBOXFILEBROWSER_H__

