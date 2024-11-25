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

	dialog(NULL),
	transposeHandlerResponder(NULL)
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
    patternMenuControl->addEntry("Render to sample", BUTTON_PATTERN_CAPTURE);
    patternMenuControl->addEntry("Transpose", MAINMENU_TRANSPOSE);
    patternMenuControl->addEntry("Advanced edit", MAINMENU_ADVEDIT);

    
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
	}

	assureCursorVisible();
}

void PatternEditorControl::setSize(const PPSize& size)
{
	PPControl::setSize(size);
	
	visibleWidth = size.width - (getRowCountWidth() + 4) - SCROLLBARWIDTH*2;	
	visibleHeight = size.height - (font->getCharHeight() + 4) - SCROLLBARWIDTH*2;

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
	visibleHeight = size.height - (font->getCharHeight() + 4) - SCROLLBARWIDTH*2;

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

static inline pp_int32 myMod(pp_int32 a, pp_int32 b)
{
	pp_int32 r = a % b;
	return r < 0 ? b + r : r;
}

void PatternEditorControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	// ;----------------- everything all right?
	validate();

	// ;----------------- colors
	PPColor lineColor;

	if (hasFocus || !properties.showFocus)
		lineColor = *cursorColor;
	else
		lineColor.r = lineColor.g = lineColor.b = 64;

	PPColor bColor = *borderColor, dColor = *borderColor, bCursor = lineColor, dCursor = lineColor;
	// adjust dark color
	dColor.scaleFixed(32768);
	// adjust bright color
	bColor.scaleFixed(87163);
	// adjust dark color
	dCursor.scaleFixed(32768);
	// adjust bright color
	bCursor.scaleFixed(87163);

	g->setRect(location.x+SCROLLBARWIDTH, location.y+SCROLLBARWIDTH, 
			   location.x + size.width - SCROLLBARWIDTH, location.y + size.height - SCROLLBARWIDTH);

	g->setColor(bgColor);

	g->fill();

	g->setFont(font);

	// ;----------------- not going any further with invalid pattern
	if (pattern == NULL)
		return;

	// ;----------------- make layout extents
	adjustExtents();

	char name[32];

	mp_sint32 i,j;

	// ;----------------- selection layout
	PatternEditorTools::Position selectionStart, selectionEnd;
	selectionStart = patternEditor->getSelection().start;
	selectionEnd = patternEditor->getSelection().end;

	PatternEditorTools::flattenSelection(selectionStart, selectionEnd);	

	// only entire instrument column is allowed
	if (selectionStart.inner >= 1 && selectionStart.inner<=2)
		selectionStart.inner = 1;
	if (selectionEnd.inner >= 1 && selectionEnd.inner<=2)
		selectionEnd.inner = 2;
	// only entire volume column is allowed
	if (selectionStart.inner >= 3 && selectionStart.inner<=4)
		selectionStart.inner = 3;
	if (selectionEnd.inner >= 3 && selectionEnd.inner<=4)
		selectionEnd.inner = 4;

	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	if (cursor.inner < 0 || cursor.inner >= 8)
		cursor.inner = 0;

	// ;----------------- some constants
	const pp_uint32 fontCharWidth3x = font->getCharWidth()*3 + 1;
	const pp_uint32 fontCharWidth2x = font->getCharWidth()*2 + 1;
	const pp_uint32 fontCharWidth1x = font->getCharWidth()*1 + 1;	
	
	PatternTools* patternTools = &this->patternTools;
	
	// ;----------------- Little adjustment for scrolling in center
	if (properties.scrollMode == ScrollModeToCenter)
	{
		if ((size.height - (SCROLLBARWIDTH + ((signed)font->getCharHeight()+4)))/(signed)font->getCharHeight() > (pattern->rows - startIndex + 1) && startIndex > 0)
			startIndex--;
	}

	// ;----------------- start painting rows
	pp_int32 startx = location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
	
	pp_int32 previousPatternIndex = currentOrderlistIndex;
	pp_int32 previousRowIndex = 0;

	pp_int32 nextPatternIndex = currentOrderlistIndex;
	pp_int32 nextRowIndex = this->pattern->rows-1;

	pp_int32 songPosOrderListIndex = currentOrderlistIndex;

	TXMPattern* pattern = this->pattern;

	// ----------------- colors ----------------- 
	PPColor noteColor = TrackerConfig::colorPatternEditorNote;
	PPColor insColor = TrackerConfig::colorPatternEditorInstrument;
	PPColor volColor = TrackerConfig::colorPatternEditorVolume;
	PPColor effColor = TrackerConfig::colorPatternEditorEffect;
	PPColor opColor = TrackerConfig::colorPatternEditorOperand;
	PPColor hiLightPrimary = TrackerConfig::colorHighLight_1;
	PPColor hiLightSecondary = TrackerConfig::colorHighLight_2;	
	PPColor hiLightPrimaryRow = TrackerConfig::colorRowHighLight_1;
	PPColor hiLightSecondaryRow = TrackerConfig::colorRowHighLight_2;

	PPColor textColor = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText);

	pp_int32 numVisibleChannels = patternEditor->getNumChannels();

	for (pp_int32 i2 = startIndex;; i2++)
	{
		i = i2 < 0 ? startIndex - i2 - 1: i2;

		pp_int32 px = location.x + SCROLLBARWIDTH;

		pp_int32 py = location.y + (i-startIndex) * font->getCharHeight() + SCROLLBARWIDTH + (font->getCharHeight() + 4);

		// rows are already in invisible area => abort
		if (py >= location.y + size.height)
			break;
			
		pp_int32 row = i;

		if (properties.prospective && properties.scrollMode == ScrollModeStayInCenter && currentOrderlistIndex != -1)
		{
			if (i < 0)
			{
				previousRowIndex--;
				if (previousRowIndex < 0)
				{
					previousPatternIndex--;
					if (previousPatternIndex >= 0)
					{
						pattern = &module->phead[module->header.ord[previousPatternIndex]];
						previousRowIndex = pattern->rows-1;
						
						noteColor.set(TrackerConfig::colorThemeMain.r, TrackerConfig::colorThemeMain.g, TrackerConfig::colorThemeMain.b);
						insColor = volColor = effColor = opColor = noteColor;
					}
					else
					{
						continue;
					}
				}
				
				songPosOrderListIndex = previousPatternIndex;
				row = previousRowIndex;
			}
			else if (i >= this->pattern->rows)
			{
				nextRowIndex++;
				if (nextRowIndex == pattern->rows && nextPatternIndex < module->header.ordnum)
				{
					nextPatternIndex++;
					if (nextPatternIndex < module->header.ordnum)
					{
						pattern = &module->phead[module->header.ord[nextPatternIndex]];
						nextRowIndex = 0;
						
						// Outside current range display colors of main theme
						noteColor.set(TrackerConfig::colorThemeMain.r, TrackerConfig::colorThemeMain.g, TrackerConfig::colorThemeMain.b);
						insColor = volColor = effColor = opColor = noteColor;
					}
					else 
					{
						continue;
					}
				}
				else if (nextPatternIndex >= module->header.ordnum)
				{
					continue;
				}
				
				songPosOrderListIndex = nextPatternIndex;
				row = nextRowIndex;
			}
			else
			{
				songPosOrderListIndex = currentOrderlistIndex;
				pattern = this->pattern;
				
				// inside current range display colors as usual
				noteColor = TrackerConfig::colorPatternEditorNote;
				insColor = TrackerConfig::colorPatternEditorInstrument;
				volColor = TrackerConfig::colorPatternEditorVolume;
				effColor = TrackerConfig::colorPatternEditorEffect;
				opColor = TrackerConfig::colorPatternEditorOperand;
			}
		}
		else
		{
			if (i2 < 0 || i2 >= pattern->rows)
				continue;

			row = i;
		}

		// draw rows
		if (!(i % properties.highlightSpacingPrimary) && properties.highLightRowPrimary)
		{
			g->setColor(hiLightPrimaryRow);			
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}
		else if (!(i % properties.highlightSpacingSecondary) && properties.highLightRowSecondary)
		{
			g->setColor(hiLightSecondaryRow);			
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}
		
		// draw position line
		if ((row == songPos.row && songPosOrderListIndex == songPos.orderListIndex) ||
			(i >= 0 && i <= pattern->rows - 1 && i == songPos.row && songPos.orderListIndex == -1))
		{
			PPColor lineColor(TrackerConfig::colorThemeMain.r>>1, TrackerConfig::colorThemeMain.g>>1, TrackerConfig::colorThemeMain.b>>1);
			g->setColor(lineColor);
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}

		// draw cursor line
		if (i == cursor.row)
		{
			g->setColor(bCursor);			
			g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py - 1);
			g->setColor(dCursor);			
			g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + (pp_int32)font->getCharHeight());

			g->setColor(lineColor);			
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}
		
		// draw rows
		if (!(i % properties.highlightSpacingPrimary))
			g->setColor(hiLightPrimary);
		else if (!(i % properties.highlightSpacingSecondary))
			g->setColor(hiLightSecondary);
		else
			g->setColor(textColor);

		if (properties.hexCount)
			PatternTools::convertToHex(name, myMod(row, pattern->rows), properties.prospective ? 2 : PatternTools::getHexNumDigits(pattern->rows-1));
		else
			PatternTools::convertToDec(name, myMod(row, pattern->rows), properties.prospective ? 3 : PatternTools::getDecNumDigits(pattern->rows-1));
		
		g->drawString(name, px, py);

		// draw channels
		for (j = startPos; j < numVisibleChannels; j++)
		{

			pp_int32 px = (location.x + (j-startPos) * slotSize + SCROLLBARWIDTH) + (getRowCountWidth() + 4);
			
			// columns are already in invisible area => abort
			if (px >= location.x + size.width)
				break;
			
			pp_int32 py = location.y + SCROLLBARWIDTH;

			if (menuInvokeChannel == j)
				g->setColor(255-dColor.r, 255-dColor.g, 255-dColor.b);
			else
				g->setColor(dColor);

			{
				PPColor nsdColor = g->getColor(), nsbColor = g->getColor();
				
				if (menuInvokeChannel != j)
				{
					// adjust not so dark color
					nsdColor.scaleFixed(50000);
					
					// adjust bright color
					nsbColor.scaleFixed(80000);
				}
				else
				{
					// adjust not so dark color
					nsdColor.scaleFixed(30000);
					
					// adjust bright color
					nsbColor.scaleFixed(60000);
				}
				
				PPRect rect(px, py, px+slotSize, py + font->getCharHeight()+1);
				g->fillVerticalShaded(rect, nsbColor, nsdColor, false);
				
			}
			
			if (muteChannels[j])
			{
				g->setColor(128, 128, 128);
			}
			else
			{
				if (!(j&1))
					g->setColor(hiLightPrimary);
				else
					g->setColor(textColor);
					
				if (j == menuInvokeChannel)
				{
					PPColor col = g->getColor();
					col.r = textColor.r - col.r;
					col.g = textColor.g - col.g;
					col.b = textColor.b - col.b;
					col.clamp();
					g->setColor(col);
				}
			}

			sprintf(name, "%i", j+1);

			if (muteChannels[j])
				strcat(name, " <Mute>");

			g->drawString(name, px + (slotSize>>1)-(((pp_int32)strlen(name)*font->getCharWidth())>>1), py+1);
		}

		for (j = startPos; j < numVisibleChannels; j++)
		{
			pp_int32 px = (j-startPos) * slotSize + startx;
			
			// columns are already in invisible area => abort
			if (px >= location.x + size.width)
				break;

			if (j >= selectionStart.channel && i >= selectionStart.row &&
				j <= selectionEnd.channel && i <= selectionEnd.row && i < this->pattern->rows)
			{
				g->setColor(*selectionColor);
				
				if ((row == songPos.row && songPosOrderListIndex == songPos.orderListIndex) ||
					(i >= 0 && i <= pattern->rows - 1 && i == songPos.row && songPos.orderListIndex == -1))
				{
					PPColor c = g->getColor();
					c.r = (TrackerConfig::colorThemeMain.r + c.r)>>1;
					c.g = (TrackerConfig::colorThemeMain.g + c.g)>>1;
					c.b = (TrackerConfig::colorThemeMain.b + c.b)>>1;
					c.clamp();
					g->setColor(c);
				}
				
				if (i == cursor.row)
				{
					PPColor c = g->getColor();
					c.r+=lineColor.r;
					c.g+=lineColor.g;
					c.b+=lineColor.b;
					c.clamp();
					g->setColor(c);
				}
				
				if (selectionStart.channel == selectionEnd.channel && j == selectionStart.channel)
				{
					pp_int32 startx = cursorPositions[selectionStart.inner];
					pp_int32 endx = cursorPositions[selectionEnd.inner] + cursorSizes[selectionEnd.inner];
					g->fill(PPRect(px + startx, py - (i == cursor.row ? 1 : 0), px + endx, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionStart.channel)
				{
					pp_int32 offset = cursorPositions[selectionStart.inner];
					g->fill(PPRect(px + offset, py - (i == cursor.row ? 1 : 0), px + slotSize, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionEnd.channel)
				{
					pp_int32 offset = cursorPositions[selectionEnd.inner] + cursorSizes[selectionEnd.inner];
					g->fill(PPRect(px, py - (i == cursor.row ? 1 : 0), px + offset, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
				else
				{
					g->fill(PPRect(px, py - (i == cursor.row ? 1 : 0), px + slotSize, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
			}

			// --------------------- draw cursor ---------------------
			if (j == cursor.channel &&
				i == cursor.row)
			{
				if (hasFocus || !properties.showFocus)
					g->setColor(TrackerConfig::colorPatternEditorCursor);
				else
					g->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorGrayedOutSelection));
					
				for (pp_int32 k = cursorPositions[cursor.inner]; k < cursorPositions[cursor.inner]+cursorSizes[cursor.inner]; k++)
					g->drawVLine(py, py + font->getCharHeight(), px + k);

				PPColor c = g->getColor();
				PPColor c2 = c;
				c.scaleFixed(32768);
				c2.scaleFixed(87163);
				g->setColor(c2);
				g->drawHLine(px + cursorPositions[cursor.inner], px + cursorPositions[cursor.inner]+cursorSizes[cursor.inner], py - 1);
				g->setColor(c);
				g->drawHLine(px + cursorPositions[cursor.inner], px + cursorPositions[cursor.inner]+cursorSizes[cursor.inner], py + font->getCharHeight());
			}

			patternTools->setPosition(pattern, j, row);

			PPColor noteCol = noteColor;

			// Show notes in red if outside PT 3 octaves
			if(properties.ptNoteLimit
			   && ((patternTools->getNote() >= 71 && patternTools->getNote() < patternTools->getNoteOffNote())
				   || patternTools->getNote() < 36))
			{
				noteCol.set(0xff,00,00);
			}

			if (muteChannels[j])
			{
				noteCol.scaleFixed(properties.muteFade);
			}

			g->setColor(noteCol);
			patternTools->getNoteName(name, patternTools->getNote());
			g->drawString(name,px, py);

			px += fontCharWidth3x + properties.spacing;
			
			if (muteChannels[j])
			{
				PPColor insCol = insColor;
				insCol.scaleFixed(properties.muteFade);
				g->setColor(insCol);
			}
			else
				g->setColor(insColor);

			pp_uint32 i = patternTools->getInstrument();

			if (i)
				patternTools->convertToHex(name, i, 2);
			else 
			{
				name[0] = name[1] = '\xf4';
				name[2] = 0;
			}
			
			if (name[0] == '0')
			name[0] = '\xf4';

			g->drawString(name,px, py);
			
			px += fontCharWidth2x + properties.spacing;

			if (muteChannels[j])
			{
				PPColor volCol = volColor;
				volCol.scaleFixed(properties.muteFade);
				g->setColor(volCol);
			}
			else
				g->setColor(volColor);

			pp_int32 eff, op;

			name[0] = name[1] = '\xf4';
			name[2] = 0;
			if (pattern->effnum >= 2)
			{
				patternTools->getFirstEffect(eff, op);
				
				patternTools->convertEffectsToFT2(eff, op);
				
				pp_int32 volume = patternTools->getVolumeFromEffect(eff, op);
			
				patternTools->getVolumeName(name, volume);
			}

			g->drawString(name,px, py);
			
			px += fontCharWidth2x + properties.spacing;

			if (muteChannels[j])
			{
				PPColor effCol = effColor;
				effCol.scaleFixed(properties.muteFade);
				g->setColor(effCol);
			}
			else
				g->setColor(effColor);
			
			if (pattern->effnum == 1)
			{
				patternTools->getFirstEffect(eff, op);				
				patternTools->convertEffectsToFT2(eff, op);
			}
			else
			{
				patternTools->getNextEffect(eff, op);				
				patternTools->convertEffectsToFT2(eff, op);
			}

			if (eff == 0 && op == 0)
			{
				name[0] = properties.zeroEffectCharacter;
				name[1] = 0;
			}
			else
			{
				patternTools->getEffectName(name, eff);
			}

			g->drawString(name,px, py);

			px += fontCharWidth1x;

			if (muteChannels[j])
			{
				PPColor opCol = opColor;
				opCol.scaleFixed(properties.muteFade);
				g->setColor(opCol);
			}
			else
				g->setColor(opColor);
			
			if (eff == 0 && op == 0)
			{
				name[0] = name[1] = properties.zeroEffectCharacter;
				name[2] = 0;
			}
			else
			{
				patternTools->convertToHex(name, op, 2);
			}			

			g->drawString(name,px, py);
		}
	}
	
	for (j = startPos; j < numVisibleChannels; j++)
	{

		pp_int32 px = (location.x + (j-startPos) * slotSize + SCROLLBARWIDTH) + (getRowCountWidth() + 4);
			
		// columns are already in invisible area => abort
		if (px >= location.x + size.width)
			break;

		px += fontCharWidth3x + properties.spacing;
		px += fontCharWidth2x*3-1 + properties.spacing*2;
		px += fontCharWidth1x;

		g->setColor(*borderColor);
		
		g->drawVLine(location.y, location.y + size.height, px+1);
		
		g->setColor(bColor);
		
		g->drawVLine(location.y, location.y + size.height, px);
		
		g->setColor(dColor);
		
		g->drawVLine(location.y, location.y + size.height, px+2);
	}

	// ;----------------- Margin lines
	// draw margin vertical line
	g->setColor(*borderColor);
		
	pp_int32 px = location.x + SCROLLBARWIDTH;
	px+=getRowCountWidth() + 1;
	g->drawVLine(location.y, location.y + size.height, px+1);
	
	g->setColor(bColor);	
	g->drawVLine(location.y, location.y + size.height, px);
	
	g->setColor(dColor);	
	g->drawVLine(location.y, location.y + size.height, px+2);
	
	// draw margin horizontal lines
	for (j = 0; j < visibleWidth / slotSize + 1; j++)
	{		
		pp_int32 px = (location.x + j * slotSize + SCROLLBARWIDTH) + (getRowCountWidth() + 4) - 1;
		
		// columns are already in invisible area => abort
		if (px >= location.x + size.width)
			break;
		
		pp_int32 py = location.y + SCROLLBARWIDTH;
		
		py+=font->getCharHeight() + 1;
		
		// Did we reach the maximum number of channels already?
		// no: just draw seperate horizontal line segments between the vertical margin lines
		if (startPos + j < numVisibleChannels)
		{
			g->setColor(*borderColor);	
			g->drawHLine(px, px + slotSize - 1, py+1);
			
			g->setColor(bColor);	
			g->drawHLine(px + 1, px + slotSize - 2, py);
			
			g->setColor(dColor);	
			g->drawHLine(px + 1, px + slotSize - 2, py+2);
		}
		// yes: draw the horizontal margin line completely to the right and abort loop
		else
		{
			g->setColor(*borderColor);	
			g->drawHLine(px, location.x + size.width, py+1);
			
			g->setColor(bColor);	
			g->drawHLine(px + 1, location.x + size.width, py);
			
			g->setColor(dColor);	
			g->drawHLine(px + 1, location.x + size.width, py+2);
			break;
		}
	}
	
	// --------------------- draw moved selection ---------------------
	
	if (properties.advancedDnd && hasValidSelection() && moveSelection)
	{
		pp_int32 moveSelectionRows = moveSelectionFinalPos.row - moveSelectionInitialPos.row;
		pp_int32 moveSelectionChannels = moveSelectionFinalPos.channel - moveSelectionInitialPos.channel;
		
		pp_int32 i1 = selectionStart.row + moveSelectionRows;
		pp_int32 j1 = selectionStart.channel + moveSelectionChannels;
		pp_int32 i2 = selectionEnd.row + moveSelectionRows;
		pp_int32 j2 = selectionEnd.channel + moveSelectionChannels;
		
		if (i2 >= 0 && j2 >= 0 && i1 < pattern->rows && j1 < numVisibleChannels)
		{
			i1 = PPTools::clamp(i1, 0, pattern->rows);
			i2 = PPTools::clamp(i2, 0, pattern->rows);
			j1 = PPTools::clamp(j1, 0, numVisibleChannels);
			j2 = PPTools::clamp(j2, 0, numVisibleChannels);
			
			pp_int32 x1 = (location.x + (j1 - startPos) * slotSize + SCROLLBARWIDTH) + cursorPositions[selectionStart.inner] + (getRowCountWidth() + 4);
			pp_int32 y1 = (location.y + (i1 - startIndex) * font->getCharHeight() + SCROLLBARWIDTH) + (font->getCharHeight() + 4);
			
			pp_int32 x2 = (location.x + (j2 - startPos) * slotSize + SCROLLBARWIDTH) + cursorPositions[selectionEnd.inner]+cursorSizes[selectionEnd.inner] + (getRowCountWidth() + 3);
			pp_int32 y2 = (location.y + (i2 - startIndex) * font->getCharHeight() + SCROLLBARWIDTH) + (font->getCharHeight() * 2 + 2);
			
			// use a different color for cloning the selection instead of moving it
			if (::getKeyModifier() & selectionKeyModifier)
				g->setColor(hiLightPrimary);
			else
				g->setColor(textColor);
			
			const pp_int32 dashLen = 6;
			
			// inner dashed lines
			g->drawHLineDashed(x1, x2, y1, dashLen, 3);
			g->drawHLineDashed(x1, x2, y2, dashLen, 3 + y2 - y1);
			g->drawVLineDashed(y1, y2, x1, dashLen, 3);
			g->drawVLineDashed(y1, y2+2, x2, dashLen, 3 + x2 - x1);
			
			// outer dashed lines
			g->drawHLineDashed(x1-1, x2+1, y1-1, dashLen, 1);
			g->drawHLineDashed(x1-1, x2, y2+1, dashLen, 3 + y2 - y1);
			g->drawVLineDashed(y1-1, y2+1, x1-1, dashLen, 1);
			g->drawVLineDashed(y1-1, y2+2, x2+1, dashLen, 3 + x2 - x1);
		}
		
	}
	
	// draw scrollbars
	hTopScrollbar->paint(g);
	hBottomScrollbar->paint(g);
	vLeftScrollbar->paint(g); 	
	vRightScrollbar->paint(g); 
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
	visibleWidth = size.width - (getRowCountWidth() + 4) - SCROLLBARWIDTH*2;	
	visibleHeight = size.height - (font->getCharHeight() + 4) - SCROLLBARWIDTH*2;
	
	slotSize = 10*font->getCharWidth() + 3*1 + 4 + 3*properties.spacing;

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
		pp_int32 visibleItems = (visibleHeight) / font->getCharHeight();
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
		s = (float)(visibleHeight) / (float)(pattern->rows*(font->getCharHeight()));
	}
	else
	{
		//s = (float)(visibleHeight>>1) / (float)((pattern->rows-1)*(font->getCharHeight()));
		
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
					pp_int32 visibleItems = (visibleHeight - font->getCharHeight()) / font->getCharHeight();
					
					
					if ((startIndex <= cursor.row) && 
						((cursor.row - startIndex) * font->getCharHeight()) <= (visibleHeight - font->getCharHeight()))
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
					pp_int32 mid = (visibleHeight/2) / font->getCharHeight();
					startIndex = cursor.row - mid;			
					if (startIndex < 0)
						startIndex = 0;						
					break;
				}
				
				case ScrollModeStayInCenter:
				{
					pp_int32 mid = (visibleHeight/2) / font->getCharHeight();
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
	
	pp_int32 visibleItems = (visibleHeight - font->getCharHeight()) / font->getCharHeight();
	
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
		case BUTTON_ABOUT_FOLLOWSONG:
		case BUTTON_OCTAVE_PLUS:
		case BUTTON_OCTAVE_MINUS:
		case BUTTON_ADD_PLUS:
		case BUTTON_ADD_MINUS:
		case BUTTON_PATTERN_CAPTURE:
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

