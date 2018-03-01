/*
 *  ppui/DialogFileSelector.cpp
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
 *  DialogFileSelector.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#include "DialogFileSelector.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "ListBox.h"
#include "ListBoxFileBrowser.h"
#include "CheckBox.h"
#include "CheckBoxLabel.h"
#include "PPPathFactory.h"

enum ControlIDs
{
	DISKMENU_STATICTEXT_FILTEREXTENSIONS = PP_MESSAGEBOX_BUTTON_USER6,
	DISKMENU_CHECKBOX_FILTEREXTENSIONS,
	
	DISKMENU_STATICTEXT_SORTBY,
	DISKMENU_BUTTON_SORTBY,
	DISKMENU_BUTTON_SORTORDER,
};

pp_int32 DialogFileSelector::OverwritePromptResponder::ActionOkay(PPObject* sender)
{
	PPEvent event(eCommand);
	return dialogFileSelector.baseClassHandleEvent(dialogFileSelector.getMessageBoxContainer()->getControlByID(PP_MESSAGEBOX_BUTTON_OK), &event);
}
		
pp_int32 DialogFileSelector::OverwritePromptResponder::ActionCancel(PPObject* sender)
{
	dialogFileSelector.show();
	// avoid removal of modal dialog box after we're out of here
	return 1;
}


DialogFileSelector::DialogFileSelector(PPScreen* screen, 
									   DialogResponder* responder,
									   pp_int32 id,
									   const PPString& caption,
									   bool editfileName/* = false*/,
									   bool overwritePrompt/* = false*/,
									   bool selectDirectories/* = false*/) :
	PPDialogBase(),
	doOverwritePrompt(overwritePrompt),
	allowSelectDirectories(selectDirectories),
	allowEditFileName(editfileName),
	sortAscending(true)
{
	initDialog(screen, responder, id, caption, 310, 230, 26, "Ok", "Cancel");

	PPControl* text = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION);
	text->setLocation(PPPoint(text->getLocation().x, text->getLocation().y - 4));

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	
	pp_int32 width = getMessageBoxContainer()->getSize().width;
	pp_int32 height = getMessageBoxContainer()->getSize().height;

	PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_YES));	
	pp_int32 y2 = button->getLocation().y;
	pp_int32 x2 = x + width / 2 - 65;
	button->setLocation(PPPoint(x2+5,y2));

	button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_CANCEL));
	x2 = x + width / 2 + 5;
	button->setLocation(PPPoint(x2+5,y2));
	
	y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y + 14 + 16;
	pp_int32 listBoxWidth = (240*width)>>8;
	x2 = x + width / 2 - (listBoxWidth>>1);

	pp_uint32 x3 = x2+3;
	pp_uint32 size = 6*8+4;
	pp_uint32 space = 6;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER4, screen, this, PPPoint(x3, y2 - 16), PPSize(size, 12));
	button->setText("Prev.");
	messageBoxContainerGeneric->addControl(button);
	x3+=button->getSize().width + space;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER5, screen, this, PPPoint(x3, y2 - 16), PPSize(size, 12));
	button->setText("Next");
	messageBoxContainerGeneric->addControl(button);
	x3+=button->getSize().width + space;
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER2, screen, this, PPPoint(x3, y2 - 16), PPSize(size, 12));
	button->setText("Parent");
	messageBoxContainerGeneric->addControl(button);
	x3+=button->getSize().width + space;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER3, screen, this, PPPoint(x3, y2 - 16), PPSize(size, 12));
	button->setText("Root");
	messageBoxContainerGeneric->addControl(button);
	x3+=button->getSize().width + space;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_USER1, screen, this, PPPoint(x3, y2 - 16), PPSize(size+2, 12));
	button->setText("Home");
	messageBoxContainerGeneric->addControl(button);
	x3+=button->getSize().width + space;

	{
		pp_uint32 x3 = x2+3;
		pp_uint32 y3 = y2 + 3;
	
		PPControl* ctrl;
		
		ctrl = new PPStaticText(DISKMENU_STATICTEXT_SORTBY, NULL, NULL, PPPoint(x3, y3), "Sort by:", true);
		messageBoxContainerGeneric->addControl(ctrl);	
		x3+=8*8;
		
		button = new PPButton(DISKMENU_BUTTON_SORTBY, screen, this, PPPoint(x3, y3-1), PPSize(51, 11), false);
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		button->setText("Extension");
		messageBoxContainerGeneric->addControl(button);
		
		button = new PPButton(DISKMENU_BUTTON_SORTORDER, screen, this, PPPoint(x3+button->getSize().width, y3-1), PPSize(13, 11), false);
		button->setText(sortAscending ? "\xfd" : "\xfe");
		messageBoxContainerGeneric->addControl(button);
		
		x3+=114;		
		PPCheckBox* checkBox = new PPCheckBox(DISKMENU_CHECKBOX_FILTEREXTENSIONS, screen, this, PPPoint(x3 + 12 * 8 + 2, y3 - 1));
		messageBoxContainerGeneric->addControl(checkBox);
		messageBoxContainerGeneric->addControl(new PPCheckBoxLabel(DISKMENU_STATICTEXT_FILTEREXTENSIONS, screen, this, PPPoint(x3, y3), "Type filter:", checkBox, true));
	}
	
	y2 = getMessageBoxContainer()->getControlByID(DISKMENU_STATICTEXT_SORTBY)->getLocation().y + 14;
	
	listBoxFiles	= new PPListBoxFileBrowser(MESSAGEBOX_LISTBOX_USER1, screen, this, PPPoint(x2, y2), PPSize(listBoxWidth, height - (122)));
	listBoxFiles->setFilePrefix("");
	listBoxFiles->setDirectoryPrefix("");
	listBoxFiles->setDirectorySuffixPathSeperator();
	listBoxFiles->setSortAscending(sortAscending);
	messageBoxContainerGeneric->addControl(listBoxFiles);

	editFieldCurrentFile = new PPListBox(MESSAGEBOX_LISTBOX_USER1+1, screen, this, PPPoint(x2, y2+listBoxFiles->getSize().height + 4), PPSize(listBoxWidth,12), true, editfileName, false);
	editFieldCurrentFile->setBorderColor(messageBoxContainerGeneric->getColor());
	editFieldCurrentFile->showSelection(false);
	editFieldCurrentFile->setMaxEditSize(4096);
	editFieldCurrentFile->addItem("");
	editFieldCurrentFile->setSingleButtonClickEdit(true);
	messageBoxContainerGeneric->addControl(editFieldCurrentFile);	

	overwritePrompResponder = new OverwritePromptResponder(*this);
	overwritePromptMessageBox = new PPDialogBase(screen, overwritePrompResponder, PP_DEFAULT_ID, "Overwrite existing file?");

	fileFullPath = new PPSystemString();
	initialPath = new PPSystemString(listBoxFiles->getCurrentPathAsString());
}

DialogFileSelector::~DialogFileSelector()
{
	delete initialPath;
	delete fileFullPath;
	delete overwritePromptMessageBox;
}

pp_int32 DialogFileSelector::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eKeyDown ||
		event->getID() == eKeyUp ||
		event->getID() == eKeyChar) 
	{	
		return processKeys(sender, event);
	}
	else if (event->getID() == eCommand)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case PP_MESSAGEBOX_BUTTON_OK:
			{
				return confirm();
			}

			case PP_MESSAGEBOX_BUTTON_USER1:
			{
				gotoHome();
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER2:
			{
				gotoParent();
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER3:
			{
				gotoRoot();
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER4:
			{
				prev();
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER5:
			{
				next();
				break;
			}
			
			case DISKMENU_BUTTON_SORTBY:
				listBoxFiles->cycleSorting();
				refresh();
				break;
				
			case DISKMENU_BUTTON_SORTORDER:
				sortAscending = !sortAscending;
				listBoxFiles->setSortAscending(sortAscending);
				refresh();
				break;
				
			case DISKMENU_CHECKBOX_FILTEREXTENSIONS:
				updateFilter();
				break;

			default:
				return PPDialogBase::handleEvent(sender, event);
		}
	}
	else if (reinterpret_cast<PPControl*>(sender) == listBoxFiles && event->getID() == eConfirmed)
	{
		pp_int32 index = *((pp_int32*)event->getDataPtr());
		
		return confirm(*listBoxFiles->getPathEntry(index));
	}
	else if (reinterpret_cast<PPControl*>(sender) == listBoxFiles && event->getID() == eSelection)
	{
		updateSelection(*((pp_int32*)event->getDataPtr()));
		return PPDialogBase::handleEvent(sender, event);
	}
	else if (event->getID() == eValueChanged)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case MESSAGEBOX_LISTBOX_USER1+1:
			{
				const PPString* str = *(reinterpret_cast<PPString* const*>(event->getDataPtr()));
				*fileFullPath = listBoxFiles->getCurrentPathAsString();
				PPSystemString newStr(*str);
				fileFullPath->append(newStr);			
				break;
			}
		}
	}
	else
		return PPDialogBase::handleEvent(sender, event);
		
	return 0;
}

void DialogFileSelector::show(bool b/* = true*/)
{
	if (b)
	{
		refreshExtensions();
		refresh(false);
	}
	
	PPDialogBase::show(b);
}

const PPSystemString& DialogFileSelector::getSelectedPathFull() 
{ 
	editFieldCurrentFile->commitChanges();

	PPString str = editFieldCurrentFile->getItem(0);
	*fileFullPath = listBoxFiles->getCurrentPathAsString();
	PPSystemString newStr(str);
	fileFullPath->append(newStr);			
	
	return *fileFullPath; 
}

void DialogFileSelector::setCurrentEditFileName(const PPSystemString& name)
{
	editFieldCurrentFile->clear();
	
	char* nameASCIIZ = name.toASCIIZ();
	PPString str(nameASCIIZ);
	editFieldCurrentFile->addItem(str);
	delete[] nameASCIIZ;

	*fileFullPath = listBoxFiles->getCurrentPathAsString();
	fileFullPath->append(name);			
}

void DialogFileSelector::updateButtonStates(bool repaint/* = true*/)
{
	// Get home button
	PPButton* buttonHome = static_cast<PPButton*>(getMessageBoxContainer()->getControlByID(PP_MESSAGEBOX_BUTTON_USER1));
	buttonHome->setClickable(listBoxFiles->canGotoHome());

	// Get previous button
	PPButton* buttonPrev = static_cast<PPButton*>(getMessageBoxContainer()->getControlByID(PP_MESSAGEBOX_BUTTON_USER4));
	buttonPrev->setClickable(listBoxFiles->canPrev());

	// Get next button
	PPButton* buttonNext = static_cast<PPButton*>(getMessageBoxContainer()->getControlByID(PP_MESSAGEBOX_BUTTON_USER5));
	buttonNext->setClickable(listBoxFiles->canNext());

	if (repaint)
	{
		parentScreen->paintControl(buttonHome);
		parentScreen->paintControl(buttonPrev);
		parentScreen->paintControl(buttonNext);
	}

	{
		PPButton* button;
		
		const char* stateText = sortAscending ? "\xfd" : "\xfe";
		button = static_cast<PPButton*>(getMessageBoxContainer()->getControlByID(DISKMENU_BUTTON_SORTORDER));
		if (button->getText().compareTo(stateText) != 0)
		{
			button->setText(stateText);
			if (repaint)
				parentScreen->paintControl(button);		
		}
		
		static const char* sortTypes[PPListBoxFileBrowser::NumSortRules] = {"Name", "Size", "Extension"};
		stateText = sortTypes[listBoxFiles->getSortType()];
		button = static_cast<PPButton*>(getMessageBoxContainer()->getControlByID(DISKMENU_BUTTON_SORTBY));
		if (button->getText().compareTo(stateText) != 0)
		{
			button->setText(stateText);
			if (repaint)
				parentScreen->paintControl(button);		
		}
	}
}

pp_int32 DialogFileSelector::baseClassHandleEvent(PPObject* sender, PPEvent* event)
{
	return PPDialogBase::handleEvent(sender, event);
}

pp_int32 DialogFileSelector::processKeys(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eKeyDown)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
		//pp_uint16 scanCode = *(((pp_uint16*)event->getDataPtr())+1);
		
		switch (keyCode)
		{
			case VK_ESCAPE:
				if (editFieldCurrentFile->isEditing())
					goto callBaseClass;
				return discard();
				break;
				
			case VK_RETURN:
				if (editFieldCurrentFile->isEditing())
					goto callBaseClass;
				return confirm();			
				break;
				
			default:
callBaseClass:
				return baseClassHandleEvent(sender, event);
		}		
	}
	else if (event->getID() == eKeyUp)
	{
		return baseClassHandleEvent(sender, event);
	}
	
	return baseClassHandleEvent(sender, event);
}

pp_int32 DialogFileSelector::confirm()
{
	pp_int32 i = listBoxFiles->getSelectedIndex();
	
	PPSystemString currentSelectedName = listBoxFiles->getPathEntry(i)->getName();

	PPString str = editFieldCurrentFile->getItem(0);
	PPSystemString currentEnteredName(str);

	if (currentEnteredName.compareTo(currentSelectedName) == 0 ||
		currentEnteredName.length() == 0)
		return confirm(*listBoxFiles->getPathEntry(i));
	
	pp_int32 res;
	
	PPPathEntry* newPathEntry = PPPathFactory::createPathEntry();
	newPathEntry->create(listBoxFiles->getCurrentPathAsString(), currentEnteredName);
	res = confirm(*newPathEntry);
	delete newPathEntry;
	return res;
}

pp_int32 DialogFileSelector::confirm(const PPPathEntry& entry)
{
	if (!allowSelectDirectories && entry.isDirectory())
	{
		stepInto(entry);
		return 0;
	}
	else if (doOverwritePrompt &&
			 entry.isFile() && 
			 listBoxFiles->getCurrentPath().fileExists(getSelectedPathFull()))
	{
		overwritePromptMessageBox->show();
		return 0;
	}
	
	// fake OK press
	PPEvent event(eCommand);
	return PPDialogBase::handleEvent(reinterpret_cast<PPObject*>(getMessageBoxContainer()->getControlByID(PP_MESSAGEBOX_BUTTON_OK)), &event);
}

pp_int32 DialogFileSelector::discard()
{
	// fake cancel button press
	PPEvent event(eCommand);
	return PPDialogBase::handleEvent(reinterpret_cast<PPObject*>(getMessageBoxContainer()->getControlByID(PP_MESSAGEBOX_BUTTON_CANCEL)), &event);
}

void DialogFileSelector::refresh(bool repaint/* = true*/)
{
	listBoxFiles->refreshFiles();
	
	updateButtonStates(false);

	refreshCurrentFileEditField(false);
	
	if (repaint)
		parentScreen->paintControl(messageBoxContainerGeneric);
}

void DialogFileSelector::refreshCurrentFileEditField(bool repaint/* = true*/)
{
	const PPPathEntry* entry = listBoxFiles->getCurrentSelectedPathEntry();

	if (!entry || (allowEditFileName && !entry->isFile()))
		return;

	editFieldCurrentFile->clear();
	
	char* nameASCIIZ = entry->getName().toASCIIZ();
	PPString str(nameASCIIZ);
	editFieldCurrentFile->addItem(str);
	delete[] nameASCIIZ;
	
	if (repaint)
		parentScreen->paintControl(editFieldCurrentFile);
}

void DialogFileSelector::updateSelection(pp_int32 index, bool repaint/* = true*/)
{
	refreshCurrentFileEditField(repaint);		
	*fileFullPath = listBoxFiles->getCurrentPathAsString();
	fileFullPath->append(listBoxFiles->getPathEntry(index)->getName());			
}

void DialogFileSelector::gotoHome()
{
	listBoxFiles->gotoHome();
	
	refresh();
}

void DialogFileSelector::gotoRoot()
{
	listBoxFiles->gotoRoot();

	refresh();
}

void DialogFileSelector::gotoParent()
{
	listBoxFiles->gotoParent();
	
	refresh();
}

bool DialogFileSelector::stepInto(const PPPathEntry& entry)
{
	if (listBoxFiles->stepInto(entry))
	{
		refresh();
		return true;
	}
	
	return false;
}

void DialogFileSelector::gotoPath(const PPSystemString& path)
{
	listBoxFiles->gotoPath(path);
	
	refresh();
}

// undo last changes
void DialogFileSelector::prev()
{
	listBoxFiles->prev();
	
	refresh();
}

// redo last changes
void DialogFileSelector::next()
{
	listBoxFiles->next();
	
	refresh();
}

void DialogFileSelector::addExtension(const PPString& ext, const PPString& desc)
{
	extensions.add(new Descriptor(ext, desc));
}

void DialogFileSelector::refreshExtensions()
{
	listBoxFiles->clearExtensions();

	for (pp_int32 i = 0; i < extensions.size(); i++)
		listBoxFiles->addExtension(extensions.get(i)->extension, extensions.get(i)->description);
		
}

void DialogFileSelector::updateFilter()
{
	listBoxFiles->clearExtensions();
	
	if (static_cast<PPCheckBox*>(messageBoxContainerGeneric->getControlByID(DISKMENU_CHECKBOX_FILTEREXTENSIONS))->isChecked())
	{
		refreshExtensions();
	}
	
	refresh();
}


