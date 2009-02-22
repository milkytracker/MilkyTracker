/*
 *  ppui/ListBoxFileBrowser.cpp
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
 *  ListBoxFileBrowser.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 19.10.06.
 *
 */
 
#include "ListBoxFileBrowser.h"
#include "Screen.h"
#include "PPPathFactory.h"

PPListBoxFileBrowser::PPListBoxFileBrowser(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
										   const PPPoint& location, const PPSize& size) :
	PPListBox(id, parentScreen, eventListener, location, size, true, false, true, true),
	filePrefix("<FILE> "), fileSuffix(""),
	directoryPrefix("<DIR>  "), directorySuffix(""),
	sortAscending(true),
	cycleFilenames(true),
	sortType(SortByName)
{
	setRightButtonConfirm(true);
	currentPath = PPPathFactory::createPath();
}

PPListBoxFileBrowser::~PPListBoxFileBrowser()
{
	delete currentPath;
}

pp_int32 PPListBoxFileBrowser::dispatchEvent(PPEvent* event)
{
	if (event->getID() == eKeyChar && cycleFilenames)
	{	
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());		
			
		if (keyCode < 255)
			cycle((char)keyCode);
	}
	return PPListBox::dispatchEvent(event);
}

void PPListBoxFileBrowser::clearExtensions()
{
	items.clear();
}

// must contain pairs of extensions / description
// terminated by TWO NULL pointers
void PPListBoxFileBrowser::addExtensions(const char* const extensions[])
{
	for (pp_uint32 i = 0; extensions[i] != NULL; i+=2)
		addExtension(extensions[i], extensions[i+1]);
}

void PPListBoxFileBrowser::addExtension(const PPString& ext, const PPString& desc)
{
	Descriptor* d = new Descriptor(ext, desc);
	items.add(d);
}


PPSystemString PPListBoxFileBrowser::getCurrentPathAsString() const
{ 
	return currentPath->getCurrent(); 
}

PPString PPListBoxFileBrowser::getCurrentPathAsASCIIString() const
{
	char* nameASCIIZ = currentPath->getCurrent().toASCIIZ();	
	PPString result(nameASCIIZ);
	delete[] nameASCIIZ;
	return result;
}

void PPListBoxFileBrowser::refreshFiles()
{
	iterateFilesInFolder();
}

const PPPathEntry* PPListBoxFileBrowser::getPathEntry(pp_int32 index) const
{
	return pathEntries.get(index);
}

bool PPListBoxFileBrowser::canGotoHome() const
{
	return currentPath->canGotoHome();
}

void PPListBoxFileBrowser::gotoHome()
{
	PPSystemString before = currentPath->getCurrent();
	currentPath->gotoHome();
	PPSystemString after = currentPath->getCurrent();
	
	if (after.compareTo(before) != 0)
	{
		history.Push(before); 
		history.Push(after); 
		history.Pop(); 
	} 
	
	refreshFiles();
}

bool PPListBoxFileBrowser::canGotoRoot() const
{
	return currentPath->canGotoRoot();
}

void PPListBoxFileBrowser::gotoRoot()
{
	PPSystemString before = currentPath->getCurrent();
	currentPath->gotoRoot();
	PPSystemString after = currentPath->getCurrent();
	
	if (after.compareTo(before) != 0)
	{
		history.Push(before); 
		history.Push(after); 
		history.Pop(); 
	} 

	refreshFiles();
}

bool PPListBoxFileBrowser::canGotoParent() const
{
	return currentPath->canGotoParent();
}

void PPListBoxFileBrowser::gotoParent()
{
	PPSystemString before = currentPath->getCurrent();
	currentPath->gotoParent();
	PPSystemString after = currentPath->getCurrent();

	if (after.compareTo(before) != 0)
	{
		history.Push(before); 
		history.Push(after); 
		history.Pop(); 
	} 
	
	refreshFiles();
}

bool PPListBoxFileBrowser::currentSelectionIsFile()
{
	const PPPathEntry* entry = getPathEntry(PPListBox::getSelectedIndex());
	return entry ? entry->isFile() : false;
}

bool PPListBoxFileBrowser::stepIntoCurrentSelection()
{
	const PPPathEntry* entry = getPathEntry(PPListBox::getSelectedIndex());
	
	return entry ? stepInto(*entry) : false;
}

bool PPListBoxFileBrowser::stepInto(const PPPathEntry& entry)
{
	if (entry.isDirectory())
	{
		PPSystemString before = currentPath->getCurrent();
		// check if we can actually change to this directory
		if (!currentPath->stepInto(entry.getName()))
			return false;
		PPSystemString after = currentPath->getCurrent();
		
		if (after.compareTo(before) != 0)
		{
			history.Push(before); 
			history.Push(after); 
			history.Pop(); 
		} 
		
		refreshFiles();
		return true;
	}
	
	return false;
}

bool PPListBoxFileBrowser::gotoPath(const PPSystemString& path, bool reload/* = true*/)
{
	bool res = currentPath->change(path);
	if (res && reload) 
		refreshFiles(); 
	return res;
}

bool PPListBoxFileBrowser::canPrev() const
{
	return !history.IsEmpty();
}

// undo last changes
void PPListBoxFileBrowser::prev()
{
	if (history.IsEmpty()) return;
	
	gotoPath(*history.Pop());
}

bool PPListBoxFileBrowser::canNext() const
{
	return !history.IsTop();
}

// redo last changes
void PPListBoxFileBrowser::next()
{
	if (history.IsTop()) return;
	
	gotoPath(*history.Advance());
}

void PPListBoxFileBrowser::setFilePrefix(const PPString& prefix)
{
	filePrefix = prefix;
}

void PPListBoxFileBrowser::setFileSuffix(const PPString& suffix)
{
	fileSuffix = suffix;
}

void PPListBoxFileBrowser::setDirectoryPrefix(const PPString& prefix)
{
	directoryPrefix = prefix;
}

void PPListBoxFileBrowser::setDirectorySuffix(const PPString& suffix)
{
	directorySuffix = suffix;
}

void PPListBoxFileBrowser::setDirectorySuffixPathSeperator()
{
	directorySuffix = currentPath->getPathSeparatorAsASCII();
}

void PPListBoxFileBrowser::iterateFilesInFolder()
{
	pathEntries.clear();
	
	const PPPathEntry* entry = currentPath->getFirstEntry();
	while (entry)
	{
		if (!entry->isHidden() && checkExtension(*entry))
		{
			pathEntries.add(entry->clone());			
		}
		entry = currentPath->getNextEntry();
	}
	
	sortFileList();
	
	buildFileList();
}

void PPListBoxFileBrowser::buildFileList()
{
	PPListBox::clear();
	
	for (pp_int32 i = 0; i < pathEntries.size(); i++)
	{
		const PPPathEntry* entry = pathEntries.get(i);
		char* nameASCIIZ = entry->getName().toASCIIZ();
		PPString str(entry->isDirectory() ? directoryPrefix : filePrefix);
		str.append(nameASCIIZ);
		str.append(entry->isDirectory() ? directorySuffix : fileSuffix);
		
		appendFileSize(str, *entry);
		
		PPListBox::addItem(str);
		delete[] nameASCIIZ;
	}
}

void PPListBoxFileBrowser::appendFileSize(PPString& name, const PPPathEntry& entry)
{
	if (entry.isFile())
	{
		pp_int64 size = entry.getSize();

		char buffer[1024];
		
		if (size < 1024)
			sprintf(buffer, " (%db)", (pp_int32)size);
		else if (size < 1024*1024)
		{
			size>>=10;
			sprintf(buffer, " (%dkb)", (pp_int32)size);
		}
		else
		{
			size>>=20;
			sprintf(buffer, " (%dmb)", (pp_int32)size);
		}
	
		name.append(buffer);
	}
}

void PPListBoxFileBrowser::sortFileList()
{
	pp_int32 i;

	PPPathEntry** tempEntries;
	PPPathEntry** drives;
	PPPathEntry** parents;
	
	pp_int32 numEntries = pathEntries.size();
	tempEntries = new PPPathEntry*[numEntries];	
	drives = new PPPathEntry*[numEntries];	
	parents = new PPPathEntry*[numEntries];	
	//for (i = 0; i < numEntries; i++)
	//	tempEntries[i] = pathEntries.get(i)->clone();
	
	pp_int32 numDrives = 0;
	pp_int32 numParents = 0;
	pp_int32 content = 0;
	for (i = 0; i < numEntries; i++)
	{
		if (pathEntries.get(i)->isParent())
		{
			parents[numParents] = pathEntries.get(i)->clone();			
			numParents++;
		}
		else if (pathEntries.get(i)->isDrive())
		{
			drives[numDrives] = pathEntries.get(i)->clone();			
			numDrives++;
		}
		else 
		{
			tempEntries[content] = pathEntries.get(i)->clone();
			content++;
		}
	}
	
	
	PPPathEntry::PathSortRuleInterface* sortRules[NumSortRules];
	
	PPPathEntry::PathSortByFileRule sortByFileRule;
	PPPathEntry::PathSortBySizeRule sortBySizeRule;
	PPPathEntry::PathSortByExtRule sortByExtRule;
	
	sortRules[0] = &sortByFileRule;
	sortRules[1] = &sortBySizeRule;
	sortRules[2] = &sortByExtRule;
	
	if (content)
		PPPathEntry::sort(tempEntries, 0, content-1, *sortRules[sortType], !sortAscending);
	if (numDrives)
		PPPathEntry::sort(drives, 0, numDrives-1, *sortRules[0], false);
	
	pathEntries.clear();
	
	for (i = 0; i < numParents; i++)
	{
		pathEntries.add(parents[i]->clone());
		delete parents[i];
	}

	for (i = 0; i < content; i++)
	{
		pathEntries.add(tempEntries[i]->clone());
		delete tempEntries[i];
	}
	
	for (i = 0; i < numDrives; i++)
	{
		pathEntries.add(drives[i]->clone());
		delete drives[i];
	}
	
	delete[] parents;
	delete[] drives;
	delete[] tempEntries;
}

void PPListBoxFileBrowser::cycle(char chr)
{
	pp_int32 currentIndex = PPListBox::getSelectedIndex();
	
	PPSystemString prefix(chr);
	prefix.toUpper();
	
	pp_uint32 j = currentIndex+1;
	for (pp_int32 i = 0; i < pathEntries.size(); i++, j++)
	{
		PPSystemString str = pathEntries.get(j % pathEntries.size())->getName();
		str.toUpper();
		
		if (str.startsWith(prefix))
		{
			PPListBox::setSelectedIndex(j % pathEntries.size(), false);
			
			pp_int32 selectionIndex = PPListBox::getSelectedIndex();
			PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			
			parentScreen->paintControl(this);
			break;
		}
	}
}

bool PPListBoxFileBrowser::checkExtension(const PPPathEntry& entry)
{
	if (items.size() == 0 || entry.isDirectory())
		return true;

	bool res = false;
	
	for (pp_int32 i = 0; i < items.size(); i++)
	{
		PPSystemString sysStr(items.get(i)->extension);
		if (entry.getName().compareToExtension(sysStr))
		{
			res = true;
			break;
		}
	}

	return res;
}
