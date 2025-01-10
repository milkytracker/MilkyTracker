/*
 *  tracker/PatternEditorControl.cpp
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

#include "PatternEditorControl.h"
#include "GraphicsAbstract.h"
#include "Tools.h"
#include "Screen.h"
#include "Control.h"
#include "Font.h"
#include "XModule.h"
#include "Menu.h"
#include "Undo.h"
#include "ContextMenu.h"
#include "KeyBindings.h"
#include "DialogBase.h"
#include "PPUIConfig.h"

#include "TrackerConfig.h"
#include "ControlIDs.h"

#define SCROLLBARWIDTH  SCROLLBUTTONSIZE

static inline pp_int32 myMod(pp_int32 a, pp_int32 b)
{
	pp_int32 r = a % b;
	return r < 0 ? b + r : r;
}

#include "PatternEditorControlPaintPattern.cpp"
#include "PatternEditorControlPaintSteps.cpp"

PatternEditorControl::PatternEditorControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
										   const PPPoint& location, const PPSize& size, bool border/*= true*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	borderColor(&TrackerConfig::colorThemeMain),
	cursorColor(&TrackerConfig::colorPatternEditorCursorLine),
	selectionColor(&TrackerConfig::colorPatternEditorSelection),
	font(NULL),
	hTopScrollbar(NULL), hBottomScrollbar(NULL), vLeftScrollbar(NULL), vRightScrollbar(NULL),
	caughtControl(NULL),
	controlCaughtByLMouseButton(false), controlCaughtByRMouseButton(false),
	patternEditor(NULL), module(NULL), pattern(NULL),
	currentOrderlistIndex(0),
	songPos(),
	startIndex(0), startPos(0),
	visibleWidth(0), visibleHeight(0), slotSize(0),
	muteChannels(),
	recChannels(),
	cursorPositions(),
	cursorSizes(),
	cursorCopy(), preCursor(), ppreCursor(NULL),
	startSelection(false),
	keyboardStartSelection(false),
	assureUpdate(false), assureCursor(false),
	selectionTicker(0),
	hasDragged(false),
	moveSelection(false),
	moveSelectionInitialPos(),
	moveSelectionFinalPos(),
	menuPosX(0),
	menuPosY(0),
	menuInvokeChannel(-1), lastMenuInvokeChannel(-1),
	editMenuControl(NULL),
	eventKeyDownBindings(NULL),
	scanCodeBindings(NULL),
	eventKeyDownBindingsMilkyTracker(NULL), scanCodeBindingsMilkyTracker(NULL), eventKeyDownBindingsFastTracker(NULL), scanCodeBindingsFastTracker(NULL),
	editMode(),
	selectionKeyModifier(0),
	lastAction(RMouseDownActionInvalid), RMouseDownInChannelHeading(-1),
	rowHeight(1),
	dialog(NULL),
	transposeHandlerResponder(NULL),
	viewMode( ViewPattern )
{
	fprintf(stderr, "%d %d\n", undoInfo.startIndex, undoInfo.startPos);

	// default color
	bgColor.r = 0;
	bgColor.g = 0;
	bgColor.b = 0;

	vLeftScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y), size.height, false);
	vRightScrollbar = new PPScrollbar(1, parentScreen, this, PPPoint(location.x + size.width - SCROLLBARWIDTH, location.y), size.height, false);
	hTopScrollbar = new PPScrollbar(2, parentScreen, this, PPPoint(location.x + SCROLLBARWIDTH, location.y), size.width - SCROLLBARWIDTH*2, true);		
	hBottomScrollbar = new PPScrollbar(3, parentScreen, this, PPPoint(location.x + SCROLLBARWIDTH, location.y + size.height - SCROLLBARWIDTH), size.width - SCROLLBARWIDTH*2, true);

	
	songPos.orderListIndex = songPos.row = -1;

	status = "";

	// context menu
	editMenuControl = new PPContextMenu(4, parentScreen, this, PPPoint(0,0), TrackerConfig::colorPatternEditorCursorLine, false, PPFont::getFont(PPFont::FONT_SYSTEM));

  if( !parentScreen->getClassic() ){

    moduleMenuControl = new PPContextMenu(4, parentScreen, this, PPPoint(0,0), TrackerConfig::colorPatternEditorCursorLine);
    moduleMenuControl->setSubMenu(true);
    moduleMenuControl->addEntry("New", MAINMENU_ZAP);
    moduleMenuControl->addEntry("Load", MAINMENU_LOAD);
    moduleMenuControl->addEntry("Save", MAINMENU_SAVE);
    moduleMenuControl->addEntry("Save as", MAINMENU_SAVEAS);
    moduleMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    moduleMenuControl->addEntry("Optimize", MAINMENU_OPTIMIZE);
    moduleMenuControl->addEntry("Playback mode", MAINMENU_QUICKOPTIONS);

    patternMenuControl = new PPContextMenu(4, parentScreen, this, PPPoint(0,0), TrackerConfig::colorPatternEditorCursorLine);
    patternMenuControl->setSubMenu(true);
    patternMenuControl->addEntry("Transpose", MAINMENU_TRANSPOSE);
    patternMenuControl->addEntry("Advanced edit", MAINMENU_ADVEDIT);
    patternMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    patternMenuControl->addEntry("Render to sample", BUTTON_PATTERN_CAPTURE);
    patternMenuControl->addEntry("Render to sample [overdub]", BUTTON_PATTERN_CAPTURE_OVERDUB);
    patternMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    patternMenuControl->addEntry("pattern/grid [ctrl+tab]", BUTTON_PATTERN_TOGGLE_VIEW);

    
	keyboardMenuControl = new PPContextMenu(4, parentScreen, this, PPPoint(0,0), TrackerConfig::colorPatternEditorCursorLine);
    keyboardMenuControl->setSubMenu(true);
    keyboardMenuControl->addEntry("Octave +", BUTTON_OCTAVE_PLUS );
    keyboardMenuControl->addEntry("Octave -", BUTTON_OCTAVE_MINUS );
    keyboardMenuControl->addEntry("Step +", BUTTON_ADD_PLUS );
    keyboardMenuControl->addEntry("Step -", BUTTON_ADD_MINUS );

    editMenuControl->addEntry("Song        >", 0xFFFF, moduleMenuControl);
    editMenuControl->addEntry("Pattern     >", 0xFFFF, patternMenuControl);
    editMenuControl->addEntry("Keyboard    >", 0xFFFF, keyboardMenuControl);

    channelMenuControl = new PPContextMenu(4, parentScreen, this, PPPoint(0,0), TrackerConfig::colorPatternEditorCursorLine);
    channelMenuControl->setSubMenu(true);
    channelMenuControl->addEntry("Mute", MenuCommandIDMuteChannel);
    channelMenuControl->addEntry("Solo", MenuCommandIDSoloChannel);
    channelMenuControl->addEntry("Unmute all", MenuCommandIDUnmuteAll);
    channelMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    channelMenuControl->addEntry("Select", MenuCommandIDSelectChannel);
    channelMenuControl->addEntry("Select all", MenuCommandIDSelectAll);
    channelMenuControl->addEntry("Swap", MenuCommandIDSwapChannels);
    channelMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    channelMenuControl->addEntry("Add", MenuCommandIDChannelAdd);
    channelMenuControl->addEntry("Delete", MenuCommandIDChannelDelete);
    editMenuControl->addEntry("Channel     >", 0xFFFF, channelMenuControl);

	helpMenuControl = new PPContextMenu(4, parentScreen, this, PPPoint(0,0), TrackerConfig::colorPatternEditorCursorLine);
    helpMenuControl->setSubMenu(true);
    helpMenuControl->addEntry("Help", MAINMENU_HELP );
    helpMenuControl->addEntry("About", MAINMENU_ABOUT );
    editMenuControl->addEntry("Help        >", 0xFFFF, helpMenuControl);
    
    editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    editMenuControl->addEntry("Undo", MenuCommandIDUndo);
    editMenuControl->addEntry("Redo", MenuCommandIDRedo);
    editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    editMenuControl->addEntry("Cut", MenuCommandIDCut);
    editMenuControl->addEntry("Copy", MenuCommandIDCopy);
    editMenuControl->addEntry("Paste", MenuCommandIDPaste);
    editMenuControl->addEntry("Paste Porous", MenuCommandIDPorousPaste);
    editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    editMenuControl->addEntry("Toggle follow", BUTTON_ABOUT_FOLLOWSONG);

  }else{
    editMenuControl->addEntry("Mute channel", MenuCommandIDMuteChannel);
    editMenuControl->addEntry("Solo channel", MenuCommandIDSoloChannel);
    editMenuControl->addEntry("Unmute all", MenuCommandIDUnmuteAll);
    editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    editMenuControl->addEntry("Mark channel", MenuCommandIDSelectChannel);
    editMenuControl->addEntry("Mark all", MenuCommandIDSelectAll);
    editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    editMenuControl->addEntry("Undo", MenuCommandIDUndo);
    editMenuControl->addEntry("Redo", MenuCommandIDRedo);
    editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    editMenuControl->addEntry("Cut", MenuCommandIDCut);
    editMenuControl->addEntry("Copy", MenuCommandIDCopy);
    editMenuControl->addEntry("Paste", MenuCommandIDPaste);
    editMenuControl->addEntry("Porous Paste", MenuCommandIDPorousPaste);
    editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
    editMenuControl->addEntry("Swap channels", MenuCommandIDSwapChannels);
  }

	//editMenuControl->setNotifyParentOnHide(true);

	initKeyBindings();
	
#ifdef __LOWRES__
	setFont(PPFont::getFont(PPFont::FONT_TINY));
#else
	setFont(PPFont::getFont(PPFont::FONT_SYSTEM));
#endif
		
	setRecordMode(false);
	
	transposeHandlerResponder = new TransposeHandlerResponder(*this);
}

PatternEditorControl::~PatternEditorControl()
{
	if (patternEditor)
		patternEditor->removeNotificationListener(this);

	delete vLeftScrollbar;	
	delete vRightScrollbar;
	delete hTopScrollbar;
	delete hBottomScrollbar;

	delete editMenuControl;
	
	delete eventKeyDownBindingsMilkyTracker;
	delete scanCodeBindingsMilkyTracker;
	delete eventKeyDownBindingsFastTracker;
	delete scanCodeBindingsFastTracker;

	delete transposeHandlerResponder;
	delete dialog;
}

void PatternEditorControl::setFont(PPFont* font)
{
	this->font = font;
	adjustExtents();

	if( !parentScreen->getClassic() ){
		editMenuControl->setFont(font);
		moduleMenuControl->setFont(font);
		patternMenuControl->setFont(font);
		keyboardMenuControl->setFont(font);
		channelMenuControl->setFont(font);
		helpMenuControl->setFont(font);
	}

	assureCursorVisible();
}

void PatternEditorControl::setSize(const PPSize& size)
{
	PPControl::setSize(size);
	
	visibleWidth = size.width - (getRowCountWidth() + 4) - SCROLLBARWIDTH*2;	
	visibleHeight = size.height - (rowHeight + 4) - SCROLLBARWIDTH*2;


	delete vLeftScrollbar;
	delete vRightScrollbar;
	delete hTopScrollbar;
	delete hBottomScrollbar;
	
	vLeftScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y), size.height, false);
	vRightScrollbar = new PPScrollbar(1, parentScreen, this, PPPoint(location.x + size.width - SCROLLBARWIDTH, location.y), size.height, false);
	hTopScrollbar = new PPScrollbar(2, parentScreen, this, PPPoint(location.x + SCROLLBARWIDTH, location.y), size.width - SCROLLBARWIDTH*2, true);
	hBottomScrollbar = new PPScrollbar(3, parentScreen, this, PPPoint(location.x + SCROLLBARWIDTH, location.y + size.height - SCROLLBARWIDTH), size.width - SCROLLBARWIDTH*2, true);			

	adjustScrollBarSizes();
	assureCursorVisible();
}

void PatternEditorControl::setLocation(const PPPoint& location)
{
	PPControl::setLocation(location);

	visibleWidth = size.width - (getRowCountWidth() + 4) - SCROLLBARWIDTH*2;	
	visibleHeight = size.height - (rowHeight + 4) - SCROLLBARWIDTH*2;

	delete vLeftScrollbar;
	delete vRightScrollbar;
	delete hTopScrollbar;
	delete hBottomScrollbar;
	
	vLeftScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y), size.height, false);
	vRightScrollbar = new PPScrollbar(1, parentScreen, this, PPPoint(location.x + size.width - SCROLLBARWIDTH, location.y), size.height, false);
	hTopScrollbar = new PPScrollbar(2, parentScreen, this, PPPoint(location.x + SCROLLBARWIDTH, location.y), size.width - SCROLLBARWIDTH*2, true);
	hBottomScrollbar = new PPScrollbar(3, parentScreen, this, PPPoint(location.x + SCROLLBARWIDTH, location.y + size.height - SCROLLBARWIDTH), size.width - SCROLLBARWIDTH*2, true);			

	adjustScrollBarSizes();
	assureCursorVisible();
}

void PatternEditorControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	rowHeight = (viewMode == ViewSteps ? font->getCharHeight()*STEP_ROWHEIGHT : font->getCharHeight());
	if( viewMode == ViewPattern ) paintPattern(g);
	if( viewMode == ViewSteps   ) paintSteps(g);
}

void PatternEditorControl::attachPatternEditor(PatternEditor* patternEditor)
{
	if (this->patternEditor)
		this->patternEditor->removeNotificationListener(this);

	this->patternEditor = patternEditor;
	patternEditor->addNotificationListener(this);

	this->module = patternEditor->getModule();
	this->pattern = patternEditor->getPattern(); 

	adjustScrollBarPositionsAndSizes();
}

void PatternEditorControl::reset()
{
	patternEditor->reset();

	menuInvokeChannel = -1;
	menuPosX = menuPosY = 0;
	
	// clear selection
	startSelection = false;
}

void PatternEditorControl::setNumVisibleChannels(pp_int32 channels)
{
	patternEditor->setNumVisibleChannels(channels);

	adjustScrollBarPositionsAndSizes();

	validate();	
}

pp_int32 PatternEditorControl::getRowCountWidth()
{
	if (pattern == NULL)
		return 0;
	
	if (!properties.prospective)
	{
		if (properties.hexCount)
			return font->getCharWidth()*PatternTools::getHexNumDigits(pattern->rows-1);
		else
			return font->getCharWidth()*PatternTools::getDecNumDigits(pattern->rows-1);
	}
	else
	{
		if (properties.hexCount)
			return font->getCharWidth()*2;
		else
			return font->getCharWidth()*3;
	}
}

void PatternEditorControl::adjustExtents()
{
	rowHeight = (viewMode == ViewSteps ? font->getCharHeight()*STEP_ROWHEIGHT : font->getCharHeight());
	visibleWidth = size.width - (getRowCountWidth() + 4) - SCROLLBARWIDTH*2;	
	visibleHeight = size.height - (rowHeight + 4) - SCROLLBARWIDTH*2;
	
	slotSize = viewMode == ViewSteps 
			   ? 5*font->getCharWidth() 
			   : 10*font->getCharWidth() + 3*1 + 4 + 3*properties.spacing;

	cursorPositions[0] = 0;	
	cursorPositions[1] = 3 * font->getCharWidth() + 1 + properties.spacing;
	cursorPositions[2] = cursorPositions[1] + font->getCharWidth();
	cursorPositions[3] = cursorPositions[2] + font->getCharWidth() + 1 + properties.spacing;
	cursorPositions[4] = cursorPositions[3] + font->getCharWidth();
	cursorPositions[5] = cursorPositions[4] + font->getCharWidth() + 1 + properties.spacing;
	cursorPositions[6] = cursorPositions[5] + font->getCharWidth() + 1;
	cursorPositions[7] = cursorPositions[6] + font->getCharWidth();
	cursorPositions[8] = /*cursorPositions[7] + font->getCharWidth() + 1*/slotSize;	
	
	cursorSizes[0] = 3 * font->getCharWidth(); // note
	cursorSizes[1] = font->getCharWidth(); // instrument digit 1
	cursorSizes[2] = font->getCharWidth(); // instrument digit 2
	cursorSizes[3] = font->getCharWidth(); // volume digit 1
	cursorSizes[4] = font->getCharWidth(); // volume digit 2
	cursorSizes[5] = font->getCharWidth(); // effect "digit"
	cursorSizes[6] = font->getCharWidth(); // operand digit 1
	cursorSizes[7] = font->getCharWidth(); // operand digit 2
}

void PatternEditorControl::adjustVerticalScrollBarPositions(mp_sint32 startIndex)
{
	// adjust scrollbar positions
	if (properties.scrollMode != ScrollModeStayInCenter)
	{
		pp_int32 visibleItems = (visibleHeight) / rowHeight;
		float v = (float)(pattern->rows - visibleItems);
		vLeftScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
		vRightScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
	}
	else
	{
		float v = (float)patternEditor->getCursor().row / (float)(pattern->rows-1);
		vLeftScrollbar->setBarPosition((pp_int32)(65536.0f*v));
		vRightScrollbar->setBarPosition((pp_int32)(65536.0f*v));
	}
}

void PatternEditorControl::adjustHorizontalScrollBarPositions(mp_sint32 startPos)
{
	pp_int32 visibleItems = (visibleWidth) / slotSize;

	float v = (float)(patternEditor->getNumChannels() - visibleItems);

	hTopScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	hBottomScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
}

void PatternEditorControl::adjustScrollBarSizes()
{
	float s;
	if (properties.scrollMode != ScrollModeStayInCenter)
	{
		s = (float)(visibleHeight) / (float)(pattern->rows*(rowHeight));
	}
	else
	{
		s = 1.0f / (float)pattern->rows;
		if (s > 1.0f)
			s = 1.0f;
	}
	
	vLeftScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
	vRightScrollbar->setBarSize((pp_int32)(s*65536.0f), false);

	s = (float)(visibleWidth) / (float)(patternEditor->getNumChannels()*slotSize);
	hTopScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
	hBottomScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
}

void PatternEditorControl::adjustScrollBarPositionsAndSizes()
{
	if (pattern == NULL)
		return;
	
	adjustScrollBarSizes();
	
	adjustVerticalScrollBarPositions(startIndex);
	adjustHorizontalScrollBarPositions(startPos);
}

void PatternEditorControl::setScrollbarPositions(mp_sint32 startIndex, mp_sint32 startPos)
{
	adjustHorizontalScrollBarPositions(startPos);
	adjustVerticalScrollBarPositions(startIndex);
}

void PatternEditorControl::scrollCursorUp()
{
	if (pattern == NULL)
		return;
		
	/*cursor.row--;
	if (cursor.row < 0)
		cursor.row = 0;*/
		
	eventKeyDownBinding_UP();
}

void PatternEditorControl::scrollCursorDown()
{
	if (pattern == NULL)
		return;
		
	/*cursor.row++;
	if (cursor.row > pattern->rows - 1)
		cursor.row = pattern->rows - 1;*/
		
	eventKeyDownBinding_DOWN();
}

void PatternEditorControl::assureCursorVisible(bool row/* = true*/, bool channel/* = true*/)
{
	if (pattern == NULL)
		return;
	
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
	
	if (row)
	{
		
		if (cursor.row >= 0)
		{
		
			switch (properties.scrollMode)
			{
				case ScrollModeToEnd:
				{
					pp_int32 visibleItems = (visibleHeight - rowHeight) / rowHeight;
					if ((startIndex <= cursor.row) && 
						((cursor.row - startIndex) * rowHeight) <= (visibleHeight - rowHeight))
					{
					}
					else if (cursor.row > startIndex &&
							 cursor.row + visibleItems < pattern->rows)
					{
						//startIndex = cursorPositionRow;
						startIndex+=(cursor.row-(startIndex+visibleItems));
					}
					else if (cursor.row < startIndex &&
							 cursor.row + visibleItems < pattern->rows)
					{
						//startIndex = cursorPositionRow;
						startIndex+=(cursor.row-startIndex);
					}
					else
					{
						startIndex = cursor.row - visibleItems;
						if (startIndex < 0)
							startIndex = 0;
					}
					break;
				}

				case ScrollModeToCenter:
				{
					pp_int32 mid = (visibleHeight/2) / rowHeight;
					startIndex = cursor.row - mid;			
					if (startIndex < 0)
						startIndex = 0;						
					break;
				}
				
				case ScrollModeStayInCenter:
				{
					pp_int32 mid = (visibleHeight/2) / rowHeight;
					startIndex = cursor.row - mid;			
					break;
				}
			}
		}

		adjustVerticalScrollBarPositions(startIndex);
	}

	if (channel)
	{
		
		if (cursor.channel >= 0)
		{
			pp_int32 visibleChannels = ((visibleWidth) / slotSize) - 1;
			
			pp_int32 cursorPos = (cursor.channel-startPos) * slotSize + 
								 cursorPositions[cursor.inner];
	
			pp_int32 cursorWidth = cursorPositions[cursor.inner+1] - 
								   cursorPositions[cursor.inner];

			if ((startPos <= cursor.channel) && 
				cursorPos <= visibleWidth - cursorWidth)
			{
			}
			else if (cursor.channel > startPos &&
				cursorPos < visibleWidth - cursorWidth)
			{
				startPos+=(cursor.channel-(startPos+visibleChannels));
			}
			else if (cursor.channel < startPos &&
				cursor.channel + visibleChannels < (signed)patternEditor->getNumChannels())
			{
				startPos+=(cursor.channel-startPos);
			}
			else
			{
				startPos = cursor.channel - visibleChannels;
				if (startPos < 0)
					startPos = 0;
			}
			
		}

		adjustHorizontalScrollBarPositions(startPos);
	}

}

mp_sint32 PatternEditorControl::getNextRecordingChannel(mp_sint32 currentChannel)
{
	if (currentChannel < 0 || currentChannel >= TrackerConfig::MAXCHANNELS)
		return -1;

	for (pp_int32 i = currentChannel+1; i < currentChannel + 1 + module->header.channum; i++)
	{
		pp_int32 c = i % module->header.channum;
		if (recChannels[c])
			return c;
	}

	return currentChannel;
}

void PatternEditorControl::advanceRow(bool assureCursor/* = true*/, bool repaint/* = true*/)
{
	if (!properties.rowAdvance)
		return;

	/*if (cursor.row + rowInsertAdd < pattern->rows - 1)
	{
		cursor.row+=rowInsertAdd;
	}
	else
		cursor.row = pattern->rows - 1;*/
	
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
										
	cursor.row = (cursor.row + properties.rowInsertAdd)/* % pattern->rows*/;

	pp_int32 res = 0;
	if (cursor.row > pattern->rows-1)
	{
		res = notifyUpdate(PatternEditorControl::AdvanceCodeWrappedEnd);
	}

	if (!res && cursor.row > pattern->rows-1)
	{
		cursor.row %= pattern->rows;
	}
	
	if (assureCursor)
		assureCursorVisible();	
		
	if (repaint)
		parentScreen->paintControl(this);
}

void PatternEditorControl::advance()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	// Multichannel editing means:
	// If we're in the note column and a note has been entered
	// locate cursor to next recording channel
	if (properties.multiChannelEdit && cursor.inner == 0)
		cursor.channel = getNextRecordingChannel(cursor.channel);

	advanceRow(false, false);
	
	assureCursorVisible();
}

void PatternEditorControl::setRow(pp_int32 row, bool bAssureCursorVisible/* = true*/)
{
	if (pattern == NULL)
		return;
		
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	if (row >= 0 && row < pattern->rows)
		cursor.row = row;

	if (bAssureCursorVisible)
		assureCursorVisible(true, false);
}

void PatternEditorControl::setChannel(pp_int32 chn, pp_int32 posInner)
{
	if (pattern == NULL)
		return;

	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	if (chn >= 0 && chn < pattern->channum)
		cursor.channel = chn;

	cursor.inner = posInner;

	assureCursorVisible(false, true);
}

void PatternEditorControl::validate()
{
	if (pattern == NULL)
		return;

	// validate scrollbar sizes
	adjustScrollBarSizes();
	/*float s = (float)(visibleHeight) / (float)(pattern->rows*(font->getCharHeight()));
	
	vLeftScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
	vRightScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
	
	s = (float)(visibleWidth) / (float)(numVisibleChannels*slotSize);
	
	hTopScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
	hBottomScrollbar->setBarSize((pp_int32)(s*65536.0f), false);*/
	
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
	
	// validate cursor position
	if (cursor.channel >= patternEditor->getNumChannels())
	{
		cursor.channel = patternEditor->getNumChannels()-1;
		assureCursorVisible(false, true);
	}

	if (cursor.row >= pattern->rows)
	{
		cursor.row = pattern->rows-1;
		assureCursorVisible(true, false);
	}
	
	pp_int32 visibleItems = (visibleHeight - rowHeight) / rowHeight;
	
	if (properties.scrollMode != ScrollModeStayInCenter)
	{
		if (startIndex + visibleItems > pattern->rows)
		{
			startIndex-=(startIndex + visibleItems)-pattern->rows;
		}
		
		if (startIndex < 0)
			startIndex = 0;
	}

	pp_int32 visibleChannels = (visibleWidth) / slotSize;
	if (startPos + visibleChannels > patternEditor->getNumChannels())
	{
		startPos-=(startPos + visibleChannels)-patternEditor->getNumChannels();
	}

	if (startPos < 0)
		startPos = 0;

	// validate selection
	/*flattenSelection();

	if ((selectionStart.channel >= 0 && selectionStart.channel > numVisibleChannels-1) || 
		(selectionStart.row >= 0 && selectionStart.row > pattern->rows-1))
	{
		resetSelection();
		return;
	}
	
	if (selectionEnd.channel >= 0 && selectionEnd.channel > numVisibleChannels-1)
		selectionEnd.channel = numVisibleChannels-1;
	
	if (selectionEnd.row >= 0 && selectionEnd.row > pattern->rows-1)
		selectionEnd.row = pattern->rows-1;*/
}

bool PatternEditorControl::hasValidSelection() const
{
	return patternEditor->hasValidSelection();
}

void PatternEditorControl::markChannel(pp_int32 channel, bool invert/* = true*/)
{
	PatternEditor::Selection currentSelection = patternEditor->getSelection();

	if (currentSelection.start.channel == channel &&
		currentSelection.start.row == 0 &&
		currentSelection.end.channel == channel &&
		currentSelection.end.row == pattern->rows - 1 &&
		invert)
	{
		patternEditor->resetSelection();
	}
	else
	{
		patternEditor->selectChannel(channel);
	}	
}

void PatternEditorControl::selectAll()
{
	patternEditor->selectAll();
}

void PatternEditorControl::deselectAll()
{
	patternEditor->resetSelection();
}

void PatternEditorControl::executeMenuCommand(pp_int32 commandId)
{
	switch (commandId)
	{
		case MenuCommandIDMuteChannel:
		{
			muteChannels[menuInvokeChannel] = !muteChannels[menuInvokeChannel];
			PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels));						
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			break;
		}

		case MenuCommandIDSoloChannel:
		{
			for (pp_uint32 i = 0; i < sizeof(muteChannels)/sizeof(pp_uint8); i++)
				muteChannels[i] = true;
			muteChannels[menuInvokeChannel] = false;
			PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels));						
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			break;
		}

		case MenuCommandIDUnmuteAll:
		{
			for (pp_uint32 i = 0; i < sizeof(muteChannels)/sizeof(pp_uint8); i++)
				muteChannels[i] = false;
			PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels));						
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			break;
		}

		// cut
		case MenuCommandIDCut:
			eventKeyCharBinding_Cut();
			break;

		// copy
		case MenuCommandIDCopy:
			eventKeyCharBinding_Copy();
			break;
		
		// paste
		case MenuCommandIDPaste:
			eventKeyCharBinding_Paste();
			break;

		// transparent paste
		case MenuCommandIDPorousPaste:
			eventKeyCharBinding_TransparentPaste();
			break;

		// mark all
		case MenuCommandIDSelectAll:
			eventKeyCharBinding_SelectAll();
			break;
		
		// mark channel
		case MenuCommandIDSelectChannel:
			markChannel(menuInvokeChannel);
			break;
			
		case MenuCommandIDUndo:
			patternEditor->undo();
			break;
			
		case MenuCommandIDRedo:
			patternEditor->redo();
			break;
			
		case MenuCommandIDSwapChannels:
			patternEditor->swapChannels(patternEditor->getCursor().channel, menuInvokeChannel);
			break;

		case MenuCommandIDChannelAdd:{
			 patternEditor->triggerButton(BUTTON_MENU_ITEM_ADDCHANNELS, parentScreen, eventListener);
			 break;
		 }

		case MenuCommandIDChannelDelete:{
			 patternEditor->triggerButton(BUTTON_MENU_ITEM_SUBCHANNELS, parentScreen, eventListener);
			 break;
		}
		
		case MAINMENU_LOAD:
		case MAINMENU_ZAP:
		case MAINMENU_SAVE:
		case MAINMENU_SAVEAS:
		case MAINMENU_CONFIG:
		case MAINMENU_TRANSPOSE:
		case MAINMENU_ADVEDIT:
		case MAINMENU_QUICKOPTIONS:
		case MAINMENU_OPTIMIZE:
		case MAINMENU_HELP:
		case MAINMENU_ABOUT:
		case BUTTON_ABOUT_FOLLOWSONG:
		case BUTTON_OCTAVE_PLUS:
		case BUTTON_OCTAVE_MINUS:
		case BUTTON_ADD_PLUS:
		case BUTTON_ADD_MINUS:
		case BUTTON_PATTERN_CAPTURE:
		case BUTTON_PATTERN_CAPTURE_OVERDUB:
		case BUTTON_PATTERN_TOGGLE_VIEW:
		{
			 patternEditor->triggerButton(commandId, parentScreen, eventListener);
			 break;
		}
	}
	
	// Hack:
	if (parentScreen->getContextMenuControl() == editMenuControl)
	{
		menuInvokeChannel = -1;
		lastMenuInvokeChannel = -1;
	}
	
}

void PatternEditorControl::switchEditMode(EditModes mode)
{
	switch (mode)
	{
		case EditModeMilkyTracker:
		{
			// Assign keyboard bindings
			eventKeyDownBindings = eventKeyDownBindingsMilkyTracker;
			scanCodeBindings = scanCodeBindingsMilkyTracker;

			selectionKeyModifier = KeyModifierSHIFT;
			break;
		}

		case EditModeFastTracker:
		{
			// Assign keyboard bindings
			eventKeyDownBindings = eventKeyDownBindingsFastTracker;
			scanCodeBindings = scanCodeBindingsFastTracker;

			selectionKeyModifier = KeyModifierALT;
			break;
		}
	}

	editMode = mode;
}

void PatternEditorControl::unmuteAll()
{
	memset(muteChannels, 0, sizeof(muteChannels));	

	PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels));						
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
}

void PatternEditorControl::setRecordMode(bool b)
{
	cursorColor = b ? &TrackerConfig::colorPatternEditorCursorLineHighLight : &TrackerConfig::colorPatternEditorCursorLine;
}

void PatternEditorControl::editorNotification(EditorBase* sender, EditorBase::EditorNotifications notification)
{
	switch (notification)
	{
		case PatternEditor::EditorDestruction:
		{
			patternEditor = NULL;
			break;
		}
	
		case PatternEditor::NotificationReload:
		{
			pattern = patternEditor->getPattern();
			module = patternEditor->getModule();
			adjustScrollBarPositionsAndSizes();
			validate();
			break;
		}
		
		case PatternEditor::NotificationChanges:
		{
			bool lazyUpdateNotifications = patternEditor->getLazyUpdateNotifications();
			bool numRowsChanged = patternEditor->getLastOperationDidChangeRows();
			bool cursorChanged = patternEditor->getLastOperationDidChangeCursor();
			
			// if this is a lazy notification it means we're only
			// notifying that something has changed, instead of refreshing the screen
			// which means that the screen is probably refreshed somewhere else
			// and the notification itself comes actually from this control
			if (lazyUpdateNotifications)
			{
				notifyChanges();
				return;
			}
			
			notifyChanges();
			
			if (numRowsChanged)
				adjustScrollBarPositionsAndSizes();
			
			if (cursorChanged)
				assureCursorVisible();
			
			validate();
			
			parentScreen->paintControl(this, false);
			break;	
		}
		
		case PatternEditor::NotificationFeedUndoData:
		{
			undoInfo = UndoInfo(startIndex, startPos);
			patternEditor->setUndoUserData(&undoInfo, sizeof(undoInfo));
			break;
		}

		case PatternEditor::NotificationFetchUndoData:
			if (sizeof(undoInfo) == patternEditor->getUndoUserDataLen())
			{
				memcpy(&undoInfo, patternEditor->getUndoUserData(), sizeof(undoInfo));
				
				assureCursorVisible();
				setScrollbarPositions(undoInfo.startIndex, undoInfo.startPos);
				validate();		
				parentScreen->paintControl(this, false);		
				notifyUpdate();
			}	
			break;
		default:
			break;
	}
}


void PatternEditorControl::updateUnderCursor( mp_sint32 incr ){
	PatternTools patternTools;
	pp_int32 eff = 0;
	pp_int32 op  = 0;
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
	patternTools.setPosition( patternEditor->getPattern(), cursor.channel, cursor.row);

	if( viewMode == ViewSteps ) {

		pp_int32 ins = patternEditor->getCurrentActiveInstrument();	
		patternEditor->writeInstrument(PatternEditor::NibbleTypeBoth, ins, true, NULL);

	}
	// *FUTURE* possibly cycle through values of volume/effect values/types by 
	// ctrl up/down in stepmode by calling following functions:
	//
	//bool writeFT2Volume(NibbleTypes nibleType, pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);
	//bool writeEffectNumber(pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);
	//bool writeEffectOperand(NibbleTypes nibleType, pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);
	//bool writeEffect(pp_int32 effNum, pp_uint8 eff, pp_uint8 op, 
	//				 bool withUndo = false, 
	//				 PatternAdvanceInterface* advanceImpl = NULL);
	//bool writeEffectNumber(pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);
	//bool writeEffectOperand(NibbleTypes nibleType, pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);

}
	
void PatternEditorControl::drawStatus( 
		PPString s, 
		PPColor c,
		PPGraphicsAbstract* g, 
		PatternEditorTools::Position cursor,
		PPFont *font,
		pp_uint32 px)
{
	g->setColor(c);
	switch( cursor.inner ){
		case 0: s = PPString("note"); break;
		case 1:
		case 2: s = PPString("instrument"); break;
		case 3: s = PPString("FX1 type"); break;
		case 4: s = PPString("FX1 param"); break;
		case 5: s = PPString("FX2 type"); break;
		case 6: s = PPString("FX2 param1"); break;
		case 7: s = PPString("FX2 param2"); break;
		default: s = PPString(""); break;
	}
	if( status.length() > 0 ){
		s.append(": ");
		s.append(status);
	}
	if( viewMode == ViewSteps && cursor.inner <= 2 ){ 
		s.append(" // use space tab ctrl+up/down/left/right to navigate grid");
	}
	if( cursor.inner == 3 ) s.append(" // use 01234+-lrpdmsuv");
	if( cursor.inner == 4 ) s.append(" // use 0123456789");
	if( cursor.inner == 5 ) s.append(" // use 1-9 A-F");
	if( cursor.inner == 6 || cursor.inner == 7 ) s.append(" // use 0-9 A-F");
	g->drawString( s, px+3+font->getCharWidth()*2, location.y + SCROLLBARWIDTH + font->getCharHeight() + 6 );
}
