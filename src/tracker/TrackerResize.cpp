/*
 *  TrackerResize.cpp
 *  MilkyTracker
 *
 *  The UI laid out again while the tracker runs, for a window the user resizes and for Classic UX switched on the Config page.
 *
 *  Most of the UI does not depend on the screen's size: the song settings, the main menu and the sections that open over them (Disk Op., Transpose, Adv. Edit, Options, Optimize, HD Recorder, About) are a block of 320 x 118 in the top left corner, and they stay as they are. What depends on the size is moved or built again:
 *    - the scopes, the pattern editor and the tab bar are moved and resized
 *    - the instrument and sample lists, which span the rest of the top, and the instrument editor, the sample editor and the Config page, which lay themselves out across the width, are built again in the place the old ones had among the screen's controls (the order controls are hit and painted in), and what they showed is put back
 *
 *  Classic UX shapes the song settings, the main menu, the instrument and sample lists and the sample editor, which are built again the same way, and the pattern editor's menus.
 *
 */

#include "Tracker.h"
#include "TrackerConfig.h"
#include "TrackerSettingsDatabase.h"
#include "ModuleEditor.h"
#include "PatternEditorControl.h"
#include "ScopesControl.h"
#include "TabHeaderControl.h"
#include "SampleEditorControl.h"
#include "EnvelopeEditorControl.h"
#include "PianoControl.h"
#include "SectionSettings.h"
#include "SectionInstruments.h"
#include "SectionSamples.h"
#include "SectionDiskMenu.h"
#include "SectionHDRecorder.h"
#include "TitlePageManager.h"
#include "PlayerLogic.h"
#include "ControlIDs.h"

#include "PPUIConfig.h"
#include "Container.h"
#include "ListBox.h"
#include "Button.h"

#include <vector>

PPSize Tracker::getMinScreenSize()
{
	return PPSize(640, 480);
}

void Tracker::setClassic(bool classic)
{
	if (classic == screen->getClassic())
		return;

	screen->setClassic(classic);

	// at startup the UI is built for it afterwards
	if (screen->getControlByID(CONTAINER_MENU))
		layOut(PPSize(screen->getWidth(), screen->getHeight()), true);
}

#ifndef __LOWRES__

// Whether control is root or lies inside it
static bool lies(PPControl* control, PPControl* root)
{
	if (control == NULL || root == NULL)
		return false;

	if (control == root)
		return true;

	if (!root->isContainer())
		return false;

	PPSimpleVector<PPControl>& controls = static_cast<PPContainer*>(root)->getControls();
	for (pp_int32 i = 0; i < controls.size(); i++)
		if (lies(control, controls.get(i)))
			return true;

	return false;
}

// The part a build just added to the screen (the last of its controls) takes the place old had (at, from detachControl), and old goes. successors[k] is the part that took the place of parts[k]
static void takePlace(PPScreen* screen, PPControl* part, PPControl* old, pp_int32 at,
					  const std::vector<PPControl*>& parts, std::vector<PPControl*>& successors)
{
	screen->detachControl(part);
	screen->insertControl(part, at);

	for (size_t k = 0; k < parts.size(); k++)
		if (parts[k] == old)
			successors[k] = part;

	delete old;
}

void Tracker::setScreenSize(const PPSize& requested)
{
	const PPSize minSize = getMinScreenSize();
	const PPSize size(requested.width > minSize.width ? requested.width : minSize.width,
					  requested.height > minSize.height ? requested.height : minSize.height);

	if (size == PPSize(screen->getWidth(), screen->getHeight()))
		return;

	// The size is kept for the next start, unless it is the fullscreen one; in the Config page's backup too, so that its Cancel keeps it and its OK asks for no restart
	if (!screen->isFullScreen())
	{
		settingsDatabase->store("XRESOLUTION", size.width);
		settingsDatabase->store("YRESOLUTION", size.height);
		if (settingsDatabaseCopy)
		{
			settingsDatabaseCopy->store("XRESOLUTION", size.width);
			settingsDatabaseCopy->store("YRESOLUTION", size.height);
		}
	}

	layOut(size, false);
}

// The UI laid out for a screen of size, or, with classic, for Classic UX switched at the same size
void Tracker::layOut(const PPSize& size, bool classic)
{
	const PPSize oldSize(screen->getWidth(), screen->getHeight());

	// What is in flight ends first: an edit in a list, an open menu and a held mouse button. Classic UX is switched from the Config page's OK or Apply while it is held, and that button is kept
	commitListBoxChanges();
	screen->setContextMenuControl(NULL, false);
	if (!classic)
		screen->releaseCaughtControls();

	// What is built again: the parts that span the screen for a size, the song settings and the main menu for Classic UX, and the instrument and sample lists and the sample editor for both
	static const pp_int32 mainMenuIDs[] = {CONTAINER_ABOUT, CONTAINER_ORDERLIST, CONTAINER_SPEED, CONTAINER_PATTERN, CONTAINER_MENU};
	const pp_int32 numMainMenuParts = sizeof(mainMenuIDs) / sizeof(mainMenuIDs[0]);

	std::vector<PPControl*> parts;
	PPControl* const oldLists = screen->getControlByID(CONTAINER_INSTRUMENTLIST);
	PPControl* const oldSettings = !classic && sectionSettings->initialised ? sectionSettings->sectionContainer : NULL;
	PPControl* const oldInstruments = !classic && sectionInstruments->initialised ? sectionInstruments->containerEntire : NULL;
	PPControl* const oldSamples = sectionSamples->initialised ? sectionSamples->containerEntire : NULL;
	if (classic)
		for (pp_int32 i = 0; i < numMainMenuParts; i++)
			parts.push_back(screen->getControlByID(mainMenuIDs[i]));
	PPControl* const others[4] = {oldLists, oldSettings, oldInstruments, oldSamples};
	for (pp_int32 i = 0; i < 4; i++)
		if (others[i])
			parts.push_back(others[i]);
	std::vector<PPControl*> successors(parts.size(), (PPControl*)NULL);

	// The focus, and the controls the sections focus again as they close, may lie in a part that is built again: the control with the same ID in the new part takes their place, else the pattern editor
	struct Remembered { PPControl** control; pp_int32 part, id; };
	PPControl* focus = screen->getFocusedControl();
	std::vector<Remembered> remembered;
	remembered.push_back(Remembered{&focus, -1, 0});
	remembered.push_back(Remembered{&sectionDiskMenu->lastFocusedControl, -1, 0});
	for (pp_int32 j = 0; j < sections->size(); j++)
		remembered.push_back(Remembered{&sections->get(j)->lastFocusedControl, -1, 0});

	for (size_t j = 0; j < remembered.size(); j++)
		for (size_t k = 0; k < parts.size(); k++)
			if (lies(*remembered[j].control, parts[k]))
			{
				remembered[j].part = (pp_int32)k;
				remembered[j].id = (*remembered[j].control)->getID();
				*remembered[j].control = patternEditorControl;
				break;
			}
	screen->setFocus(NULL, false);

	screen->pauseUpdate(true);

	// The lists and the record button show the focus as the edit mode has it
	const bool showFocus = editMode == EditModeMilkyTracker;

	if (!classic)
	{
		screen->setSize(size);

		// Moved and resized: the scopes, the tab bar under everything and the pattern editor
		scopesControl->setSize(PPSize(size.width, SCOPESHEIGHT()));

		static_cast<PPContainer*>(screen->getControlByID(CONTAINER_OPENREMOVETABS))->move(PPPoint(0, size.height - oldSize.height));
		TabHeaderControl* tabHeader = static_cast<TabHeaderControl*>(screen->getControlByID(TABHEADER_CONTROL));
		tabHeader->setLocation(PPPoint(TABHEADERHEIGHT()*2, size.height - TABHEADERHEIGHT()));
		tabHeader->setSize(PPSize(size.width - TABHEADERHEIGHT()*2, TABHEADERHEIGHT()));

		patternEditorControl->setLocation(PPPoint(0, UPPERSECTIONDEFAULTHEIGHT()));
		patternEditorControl->setSize(PPSize(size.width, MAXEDITORHEIGHT() - UPPERSECTIONDEFAULTHEIGHT()));
	}

	// The song settings and the main menu: what they show is the song's, and their toggles the tracker's
	if (classic)
	{
		TitlePageManager titlePageManager(*screen);
		const TitlePageManager::Pages titlePage = titlePageManager.getCurrentTitlePage();
		const pp_int32 order = listBoxOrderList->getSelectedIndex();
		// the build collapses the order list and clears this
		const bool extended = extendedOrderlist;
		PPButton* edit = static_cast<PPButton*>(static_cast<PPContainer*>(screen->getControlByID(CONTAINER_MENU))->getControlByID(MAINMENU_EDIT));
		const PPColor* editTextColor = edit->getTextColor();
		const bool editPressed = edit->isPressed();

		PPControl* old[numMainMenuParts];
		pp_int32 at[numMainMenuParts];
		bool shown[numMainMenuParts];
		for (pp_int32 i = 0; i < numMainMenuParts; i++)
		{
			old[i] = parts[i];
			shown[i] = old[i]->isVisible();
			at[i] = screen->detachControl(old[i]);
		}

		initMainMenu();

		// the last taken out goes back first, so that each goes back to the index it was taken from
		for (pp_int32 i = numMainMenuParts - 1; i >= 0; i--)
		{
			PPControl* part = screen->getControlByID(mainMenuIDs[i]);
			takePlace(screen, part, old[i], at[i], parts, successors);
			part->show(shown[i]);
		}

		// the speed fields are written only as they change, and the new ones are empty
		resetStateMemories();
		updateSongTitle(false);
		updateSongLength(false);
		listBoxOrderList->setSelectedIndex(order, true, false);
		updateSongRepeat(false);
		updateSpeed(false);
		updatePatternAddAndOctave(false);
		updatePatternIndex(false);
		updatePatternLength(false);
		expandOrderlist(extended);

		setFollowSong(followSong, false);
		setProspectiveMode(getProspectiveMode(), false);
		setCursorWrapAround(getCursorWrapAround(), false);
		setLiveSwitch(playerLogic->getLiveSwitch(), false);
		titlePageManager.showTitlePage(titlePage, false);

		edit = static_cast<PPButton*>(static_cast<PPContainer*>(screen->getControlByID(CONTAINER_MENU))->getControlByID(MAINMENU_EDIT));
		edit->setTextColor(*editTextColor);
		edit->setPressed(editPressed);

		listBoxOrderList->setShowFocus(showFocus);
		static_cast<PPListBox*>(static_cast<PPContainer*>(screen->getControlByID(CONTAINER_ABOUT))->getControlByID(LISTBOX_SONGTITLE))->setShowFocus(showFocus);

		// the right-click menus, and the HD recorder's colours, which it sets as it is shown
		patternEditorControl->buildMenus();
		if (sectionHDRecorder->initialised)
			sectionHDRecorder->update(false);
	}

	// The instrument and sample lists: the same instrument and sample picked
	if (oldLists)
	{
		const bool shown = oldLists->isVisible();
		const pp_int32 instrument = listBoxInstruments->getSelectedIndex();
		const pp_int32 sample = listBoxSamples->getSelectedIndex();

		const pp_int32 at = screen->detachControl(oldLists);
		initListboxesSection(320, 0);
		PPControl* lists = screen->getControlByID(CONTAINER_INSTRUMENTLIST);
		takePlace(screen, lists, oldLists, at, parts, successors);
		lists->show(shown);

		listBoxInstruments->setSelectedIndex(instrument);
		updateInstrumentsListBox(false);
		updateSamplesListBox(false);
		listBoxSamples->setSelectedIndex(sample);
		listBoxInstruments->setShowFocus(showFocus);
		listBoxSamples->setShowFocus(showFocus);

		PPContainer* container = static_cast<PPContainer*>(lists);
		static_cast<PPButton*>(container->getControlByID(BUTTON_INSTRUMENT))->setPressed(patternEditorControl->isInstrumentEnabled());

		// the round robin range keeps its button and its highlight
		PatternEditorControl::RoundRobin* rr = patternEditorControl->getRoundRobin();
		if (rr->button)
		{
			rr->button = static_cast<PPButton*>(container->getControlByID(BUTTON_INSTRUMENTS_ROUNDROBIN));
			if (rr->userSelecting || rr->ins_size > 0)
			{
				rr->button->setPressed(true);
				rr->button->setTextColor(rr->ins_size > 0 ? TrackerConfig::colorSampleEditorWaveform :
										 PPUIConfig::getInstance()->getColor(PPUIConfig::ColorDefaultButtonText));
			}
			if (rr->ins_size > 0)
				listBoxInstruments->highlightRange(rr->ins_start-1, rr->ins_end-1);
		}
	}

	// The Config page: the same tab and pages, and the colour and font it was editing
	if (oldSettings)
	{
		SectionSettings& section = *sectionSettings;

		const pp_int32 tab = section.currentActiveTabNum;
		const pp_int32 pageStart = section.currentActivePageStart;
		pp_int32 subPages[NUMSETTINGSPAGES];
		for (pp_int32 i = 0; i < NUMSETTINGSPAGES; i++)
			subPages[i] = section.currentActiveSubPageNum[i];
		const pp_int32 color = section.listBoxColors ? section.listBoxColors->getSelectedIndex() : -1;
		const pp_int32 fontFamily = section.listBoxFontFamilies ? section.listBoxFontFamilies->getSelectedIndex() : -1;
		const pp_int32 fontEntry = section.listBoxFontEntries ? section.listBoxFontEntries->getSelectedIndex() : -1;

		const pp_int32 at = screen->detachControl(oldSettings);
		section.listBoxColors = section.listBoxFontFamilies = section.listBoxFontEntries = NULL;
		section.initialised = false;
		section.init();
		takePlace(screen, section.sectionContainer, oldSettings, at, parts, successors);

		section.showPage(tab, pageStart);
		for (pp_int32 i = 0; i < NUMSETTINGSPAGES; i++)
			section.currentActiveSubPageNum[i] = subPages[i];
		if (color >= 0 && section.listBoxColors)
			section.listBoxColors->setSelectedIndex(color);
		if (fontFamily >= 0 && section.listBoxFontFamilies)
			section.listBoxFontFamilies->setSelectedIndex(fontFamily);
		if (fontEntry >= 0 && section.listBoxFontEntries)
			section.listBoxFontEntries->setSelectedIndex(fontEntry);

		// showing it again would back up the palette and the mixer settings again, which Cancel goes back to
		if (section.visible)
		{
			showMainMenu(false, true);
			section.update(false);
			section.showSection(true);
		}
	}

	// The instrument editor: its envelope zoom and its piano's mode
	if (oldInstruments)
	{
		SectionInstruments& section = *sectionInstruments;

		const pp_int32 envelopeScale = section.envelopeEditorControl->getScale();
		const PianoControl::Modes pianoMode = section.pianoControl->getMode();

		const pp_int32 at = screen->detachControl(oldInstruments);
		section.initialised = false;
		section.storeEnvelope = false;
		section.init();
		takePlace(screen, section.containerEntire, oldInstruments, at, parts, successors);

		section.envelopeEditorControl->setScale(envelopeScale);
		section.envelopeEditorControl->setInvertMWheelZoom(settingsDatabase->restore("INVERTMWHEELZOOM")->getIntValue() != 0);
		section.pianoControl->setMode(pianoMode);

		if (section.visible)
		{
			section.realign();
			section.update(false);
			section.showSection(true);
		}
	}

	// The sample editor: the part of the sample in view, its selection and its tools' last values
	if (oldSamples)
	{
		SectionSamples& section = *sectionSamples;

		pp_int32 viewStart;
		float viewScale;
		section.sampleEditorControl->getView(viewStart, viewScale);
		const SampleEditorControlLastValues lastValues = section.sampleEditorControl->getLastValues();

		const pp_int32 at = screen->detachControl(oldSamples);
		section.initialised = false;
		section.init();
		takePlace(screen, section.containerEntire, oldSamples, at, parts, successors);

		section.sampleEditorControl->getLastValues() = lastValues;
		section.sampleEditorControl->setInvertMWheelZoom(settingsDatabase->restore("INVERTMWHEELZOOM")->getIntValue() != 0);
		section.setOffsetFormat(section.getOffsetFormat());

		// shown without attaching the sample again, which would clear its selection
		if (section.visible)
		{
			section.realign();
			section.sampleEditorControl->setView(viewStart, viewScale);
			section.refresh(false);
			section.showSection(true);
		}
		else
			section.sampleEditorControl->setView(viewStart, viewScale);
	}

	// A dialog that is open stays in the middle
	PPControl* modal = screen->getModalControl();
	if (!classic && modal && modal->isContainer())
	{
		const PPPoint p = modal->getLocation();
		const PPSize s = modal->getSize();
		static_cast<PPContainer*>(modal)->move(PPPoint((size.width - s.width) / 2 - p.x, (size.height - s.height) / 2 - p.y));
	}

	for (size_t j = 0; j < remembered.size(); j++)
	{
		const Remembered& r = remembered[j];
		PPControl* successor = r.part >= 0 && r.id ? static_cast<PPContainer*>(successors[r.part])->getControlByID(r.id) : NULL;
		if (successor)
			*r.control = successor;
	}

	// the keyboard goes where it was, and to the dialog first while one is open
	screen->setFocus(focus ? focus : patternEditorControl, false);
	if (modal)
		screen->setFocus(modal, false);
	screen->pauseUpdate(false);

	// cleared first for Classic UX, whose parts cover more of the screen than the others' do
	screen->paint(true, classic);
}

#else

void Tracker::setScreenSize(const PPSize& size)
{
}

void Tracker::layOut(const PPSize& size, bool classic)
{
}

#endif
