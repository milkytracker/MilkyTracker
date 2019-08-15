/*
 *  tracker/ScopesControl.cpp
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
 *  ScopesControl.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *
 */

#include "ScopesControl.h"
#include "Screen.h"
#include "GraphicsAbstract.h"
#include "Font.h"
#include "Button.h"
#include "PlayerController.h"
#include "Tools.h"
#include "PPUIConfig.h"
#include "TrackerConfig.h"

#include <math.h>

#undef PANNINGINDICATOR

pp_int32 ScopesControl::WRAPCHANNELS() const
{
#ifdef __LOWRES__
    return 16;
#else
    if (numChannels <= 32) {
        if (getSize().width < 800)
        {
            return 18;
        }
        if (getSize().width < 1024)
        {
            return 20;
        }
        if (getSize().width < 1280)
        {
            return 22;
        }
        
        return 32;
    }
    else
    {
        return 32;
    }
#endif
}

ScopesControl::ScopesControl(pp_int32 id,
							 PPScreen* parentScreen,
							 EventListenerInterface* eventListener,
							 const PPPoint& location, const PPSize& size,
							 bool border/*= true*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	border(border),
	borderColor(&ourOwnBorderColor),
	playerController(NULL),
	numChannels(0),
	enabled(true),
	appearance(AppearanceTypeNormal),
	currentClickType(ClickTypeMute)
{
	// default color
	color.set(0, 0, 0);

	ourOwnBorderColor.set(192, 192, 192);

	visibleWidth = size.width - 2;
	visibleHeight = size.height - 4;

	font = PPFont::getFont(PPFont::FONT_SYSTEM);
	smallFont = PPFont::getFont(PPFont::FONT_TINY);

	lastNumChannels = 0;
	memset(onOffState, 0, sizeof(onOffState));

	memset(muteChannels, 0 ,sizeof(muteChannels));
	memset(lastMuteChannels, 0 ,sizeof(lastMuteChannels));

	memset(recChannels, 0 ,sizeof(recChannels));
	memset(lastRecChannels, 0 ,sizeof(lastRecChannels));

	lMouseDownInChannel = rMouseDownInChannel = -1;

	backgroundButton = new PPButton(0, parentScreen, NULL, PPPoint(location.x, location.y), PPSize(size.width, size.height), false, false);
	backgroundButton->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorListBoxBackground));
	backgroundButton->setInvertShading(true);
}

ScopesControl::~ScopesControl()
{
	delete backgroundButton;
}

class ScopePainter : public PlayerController::SampleDataFetcher
{
private:
	PPGraphicsAbstract* g;
	const pp_uint32 count;
	const pp_uint32 channelHeight;
	const PPColor& scopebColor;
	const PPColor& scopedColor;
	pp_int32 locx, locy;
	const ScopesControl::AppearanceTypes appearance;
	bool flipped;
	pp_int32 counter, flipCounter, flipCounterStep;
	pp_int32 count2;
	pp_int32 sr, sg, sb;
	pp_int32 addr, addg, addb;
	pp_int32 lasty, lastx;

public:
	ScopePainter(PPGraphicsAbstract* g, pp_uint32 count, pp_uint32 channelHeight,
				 const PPColor& scopebColor, const PPColor& scopedColor,
				 pp_int32 locx, pp_int32 locy,
				 ScopesControl::AppearanceTypes appearance) :
		g(g),
		count(count),
		channelHeight(channelHeight),
		scopebColor(scopebColor),
		scopedColor(scopedColor),
		locx(locx), locy(locy),
		appearance(appearance),
		flipped(false),
		counter(0), flipCounter(0), flipCounterStep(1)
	{
		count2 = count - 3;
		if (count2 < 2)
			count2 = 2;
		flipCounter = 0;
		sr = scopedColor.r * 65536;
		sg = scopedColor.g * 65536;
		sb = scopedColor.b * 65536;
		addr = (scopebColor.r - scopedColor.r) * 65536 / (count2/2);
		addg = (scopebColor.g - scopedColor.g) * 65536 / (count2/2);
		addb = (scopebColor.b - scopedColor.b) * 65536 / (count2/2);

		lasty = locy;
		lastx = -1;
	}

	virtual void fetchSampleData(mp_sint32 sample)
	{
		const pp_int32 y = (((-sample >> 10)*(signed)channelHeight)>>6) + locy;

		g->setSafeColor(sr>>16, sg>>16, sb>>16);
		sr+=addr; sg+=addg; sb+=addb;
		if (flipCounter >= (count2>>1)-1 && !flipped)
		{
			flipped = true;
			addr=-addr;
			addg=-addg;
			addb=-addb;
		}
		flipCounter+=flipCounterStep;

		switch (appearance)
		{
			case ScopesControl::AppearanceTypeNormal:
				g->setPixel(locx, y);
				g->setColor(0,0,0);
				g->setPixel(locx, y+1);
				break;

			case ScopesControl::AppearanceTypeSolid:
				if (y < locy)
				{
					g->drawVLine(y, locy, locx);
					g->setColor(0,0,0);
					g->setPixel(locx, locy);
				}
				else if (y > locy)
				{
					g->drawVLine(locy, y, locx);
					g->setColor(0,0,0);
					g->setPixel(locx, y);
				}
				else
				{
					g->setPixel(locx, y);
					g->setColor(0,0,0);
					g->setPixel(locx, y+1);
				}
				break;

			case ScopesControl::AppearanceTypeLines:
				if (!(counter & 1) || (counter == (signed)count-1))
				{
					if (lastx == -1)
					{
						g->setPixel(locx, y);
						g->setColor(0,0,0);
						g->setPixel(locx, y+1);
					}
					else
					{
						g->drawAntialiasedLine(lastx,lasty,locx,y);
					}
					lasty = y;
					lastx = locx;
				}
				counter++;

		}

		locx++;
	}

	pp_int32 getLocx() const { return locx; }
};

void ScopesControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	PPColor bColor = *borderColor, dColor = *borderColor;
	// adjust dark color
	dColor.scaleFixed(32768);

	// adjust bright color
	bColor.scaleFixed(87163);

	pp_int32 xOffset = 2;
	pp_int32 yOffset = 2;

	backgroundButton->paint(g);

	g->setRect(location.x, location.y, location.x + size.width+1, location.y + size.height+1);

	if (border)
	{
		drawThickBorder(g, *borderColor);
	}

	g->setRect(location.x + 2, location.y + 2, location.x + size.width - 2, location.y + size.height - 2);

	//g->setColor(0,0,0);
	//g->fill();

	if (!playerController || numChannels < 2)
		return;

	PPColor foregroundColor = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText);

	pp_int32 channelWidth = visibleWidth / numChannels;
	pp_int32 channelHeight = visibleHeight;

    const pp_uint32 numLines = ceil((float)numChannels / (float)WRAPCHANNELS());
    const pp_uint32 channelsPerLine = ceil((float)numChannels/(float)numLines);
    if (isWrapped())
	{
        
		channelWidth = visibleWidth / channelsPerLine;
		channelHeight/=numLines;

		pp_int32 i;
		pp_int32 r = visibleWidth % channelsPerLine;

		for (i = 0; i < channelsPerLine; i++)
			channelWidthTable[i] = channelWidth;

		for (i = 0; i < r; i++)
		{
			pp_int32 j = (i*channelsPerLine)/r;
			channelWidthTable[j]++;
		}
	}
	else
	{
		pp_int32 i;
		pp_int32 r = visibleWidth % numChannels;

		for (i = 0; i < numChannels; i++)
			channelWidthTable[i] = channelWidth;

		for (i = 0; i < r; i++)
		{
			pp_int32 j = (i*numChannels)/r;
			channelWidthTable[j]++;
		}
	}

	pp_int32 cn = 0;
	pp_int32 y = 0;
	pp_int32 x = 0;

	// calculate shading for scopes
	PPColor scopebColor = TrackerConfig::colorScopes, scopedColor = TrackerConfig::colorScopes;
	// adjust dark color
	scopedColor.scaleFixed(32768);
	PPColor tempCol = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorListBoxBackground);
	tempCol.scaleFixed(32768);
	scopedColor+=tempCol;
	scopedColor.scaleFixed(49192);

    const bool wrapped = isWrapped();
    
	for (pp_int32 c = 0; c < numChannels; c++)
	{
		char buffer[8];
		PPTools::convertToDec(buffer, c + 1, PPTools::getDecNumDigits(c+1));

        const mp_sint32 starty = location.y + yOffset + y*channelHeight;
		mp_sint32 locy = starty + channelHeight / 2;
		mp_sint32 locx = location.x + xOffset + x;
		mp_sint32 cWidth = channelWidthTable[cn];

        channelRects[c].x1 = locx;
        channelRects[c].y1 = starty;
        channelRects[c].x2 = locx + cWidth;
        channelRects[c].y2 = starty + channelHeight;
        
		pp_int32 count = cWidth;

		// correct last channel
		cn++;
		if (wrapped && cn >= channelsPerLine)
		{
			cn = 0;
			y++;
			count = visibleWidth - (xOffset + x);
		}
		else if (c == numChannels-1 && numChannels == (numLines * channelsPerLine))
		{
			count = visibleWidth - (xOffset + x);
		}

		g->setColor(TrackerConfig::colorScopes);

		pp_int32 sx = locx + 3;
		pp_int32 sy = locy - channelHeight / 2 + 3;

		pp_int32 sy2 = locy + channelHeight / 2 - smallFont->getCharHeight() - 1;

		if (!muteChannels[c])
		{
			ScopePainter scopePainter(g, count, channelHeight, scopebColor, scopedColor, locx, locy, appearance);

			if (enabled)
				playerController->grabSampleData(c, count, 160, scopePainter);
			else
			{
				for (pp_int32 i = 0; i < count; i++)
					scopePainter.fetchSampleData(0);
			}

#ifdef PANNINGINDICATOR
			pp_int32 panx = locx;
			pp_int32 they = sy;

			g->drawHLine(panx + 3, panx + count2, they + channelHeight - 8);
			g->drawHLine(panx + 3, panx + count2, they + channelHeight - 5);
			g->drawVLine(they + channelHeight - 8, they + channelHeight - 5 + 1, panx + 2);
			g->drawVLine(they + channelHeight - 8, they + channelHeight - 5 + 1, panx + count2);

			PPColor col = g->getColor();
			g->setColor(TrackerConfig::colorHighLight);
			pp_int32 thex = panx + 3 + (channel.pan*(count2 - 5)) / 255;
			g->drawVLine(they + channelHeight - 7, they + channelHeight - 6 + 1, thex);
			g->drawVLine(they + channelHeight - 7, they + channelHeight - 6 + 1, thex+1);
			g->setColor(col);
#endif
			locx = scopePainter.getLocx();
		}
		else
		{
			PPFont* font = this->font;
			PPColor col = g->getColor();
			const char* muteStr = "MUTE";
			if ((signed)font->getStrWidth(muteStr) >= cWidth-2)
				font = smallFont;
			if ((signed)font->getStrWidth(muteStr) >= cWidth-2)
				muteStr = "<M>";
			pp_int32 x = locx + (cWidth / 2) - font->getStrWidth(muteStr) / 2;
			pp_int32 y = locy - font->getCharHeight() / 2;
			g->setColor(TrackerConfig::colorHighLight_1);
			g->setFont(font);
			g->drawString(muteStr, x, y);
			g->setColor(col);
		}

		if (recChannels[c])
		{
			g->setFont(smallFont);
			g->setColor(0, 0, 0);
			g->drawString("\xf0", sx, sy2+1);
			g->setColor(TrackerConfig::colorScopesRecordIndicator);
			g->drawString("\xf0", sx-1, sy2);
		}

		g->setFont((channelWidth < 40 || wrapped) ? smallFont : font);
		g->setColor(0, 0, 0);
		g->drawString(buffer, sx+1, sy+1);
		g->setColor(foregroundColor);
		g->drawString(buffer, sx, sy);

		if (cn == 0)
			x = -cWidth;

		x+=cWidth;

	}

	g->setRect(location.x + 1, location.y + 1, location.x + size.width - 1, location.y + size.height - 1);

    if (isWrapped())
    {
        pp_int32 xPos = location.x + xOffset;
        pp_int32 yPos = location.y + yOffset + channelHeight;
        for (pp_int32 y = 1; y < numLines; y++)
        {
            g->setColor(bColor);
            g->drawHLine(xPos, xPos + visibleWidth - 1, yPos - 1);
            
            g->setColor(*borderColor);
            g->drawHLine(xPos, xPos + visibleWidth - 1, yPos);
            
            g->setColor(dColor);
            g->drawHLine(xPos, xPos + visibleWidth - 1, yPos + 1);
            
            yPos += channelHeight;
        }
    }
    
    
    x = location.x + xOffset + channelWidthTable[0];
	for (cn = 0; cn < (visibleWidth / channelWidth) - 1; cn++)
	{
		g->setColor(bColor);
		g->drawVLine(location.y + yOffset-1, location.y + yOffset + visibleHeight, x-1);
		g->setColor(*borderColor);
		g->drawVLine(location.y + yOffset-1, location.y + yOffset + visibleHeight, x);
		g->setColor(dColor);
		g->drawVLine(location.y + yOffset, location.y + yOffset + visibleHeight, x+1);
		x+=channelWidthTable[cn+1];
	}

}

void ScopesControl::attachSource(PlayerController* playerController)
{
	this->playerController = playerController;
	// should force redraw
	lastNumChannels = 0;
}

bool ScopesControl::needsUpdate()
{
	if (playerController == NULL ||
		(numChannels == 0 && lastNumChannels == 0))
		return false;

	if (numChannels != lastNumChannels)
	{
		lastNumChannels = numChannels;
		return true;
	}

	bool res = false;
	for (pp_int32 c = 0; c < numChannels; c++)
	{
		if (muteChannels[c] != lastMuteChannels[c])
		{
			lastMuteChannels[c] = muteChannels[c];
			res = true;
		}

		if (recChannels[c] != lastRecChannels[c])
		{
			lastRecChannels[c] = recChannels[c];
			res = true;
		}

		bool bPlay = enabled ? playerController->hasSampleData(c) : false;

		if (bPlay != onOffState[c] || bPlay)
			res = true;

		onOffState[c] = bPlay;
	}

	return res;
}

void ScopesControl::handleMute(pp_int32 channel)
{
	muteChannels[channel] = !muteChannels[channel];
	PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels), ChangeValueMuting);
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
}

void ScopesControl::handleSolo(pp_int32 channel)
{
	// unmute?
	if (isSoloChannel(channel))
	{
		for (pp_uint32 i = 0; i < sizeof(muteChannels)/sizeof(pp_uint8); i++)
			muteChannels[i] = false;
	}
	else
	{
		for (pp_uint32 i = 0; i < sizeof(muteChannels)/sizeof(pp_uint8); i++)
			muteChannels[i] = true;
		muteChannels[channel] = false;
	}
	PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels), ChangeValueMuting);
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
}

void ScopesControl::handleRec(pp_int32 channel)
{
	recChannels[channel] = !recChannels[channel];
	PPEvent e(eValueChanged, &recChannels, sizeof(recChannels), ChangeValueRecording);
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
}

void ScopesControl::handleSingleRec(pp_int32 channel)
{
	// unmute?
	if (isSingleRecChannel(channel))
	{
		for (pp_uint32 i = 0; i < sizeof(recChannels)/sizeof(pp_uint8); i++)
			recChannels[i] = true;
	}
	else
	{
		for (pp_uint32 i = 0; i < sizeof(recChannels)/sizeof(pp_uint8); i++)
			recChannels[i] = false;
		recChannels[channel] = true;
	}
	PPEvent e(eValueChanged, &recChannels, sizeof(recChannels), ChangeValueRecording);
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
}

void ScopesControl::handleUnmuteAll()
{
	for (pp_uint32 i = 0; i < sizeof(muteChannels)/sizeof(pp_uint8); i++)
		muteChannels[i] = false;
	PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels), ChangeValueMuting);
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
}

pp_int32 ScopesControl::dispatchEvent(PPEvent* event)
{
	if (eventListener == NULL)
		return -1;

	switch (event->getID())
	{
		case eLMouseDown:
			didSoloChannel = false;
			lMouseDownInChannel = pointToChannel(*(PPPoint*)event->getDataPtr());
			break;

		case eRMouseDown:
			didSoloChannel = false;
			rMouseDownInChannel = pointToChannel(*(PPPoint*)event->getDataPtr());
			break;

		case eMMouseUp:
		{
			pp_int32 channel = pointToChannel(*(PPPoint*)event->getDataPtr());
			handleSolo(channel);
			break;
		}

		case eLMouseUp:
		{
			pp_int32 channel = pointToChannel(*(PPPoint*)event->getDataPtr());

			// left-right combined mouse click
			if (channel != -1 && lMouseDownInChannel == channel && rMouseDownInChannel == channel)
			{
				handleSolo(channel);
				didSoloChannel = true;
			}
			// normal click
			else if (channel != -1 && lMouseDownInChannel == channel && rMouseDownInChannel == -1)
			{
				switch (currentClickType)
				{
					case ClickTypeMute:
						handleMute(channel);
						break;
					case ClickTypeSolo:
						handleSolo(channel);
						break;
					case ClickTypeRec:
						handleRec(channel);
						break;
					case ClickTypeSingleRec:
						handleSingleRec(channel);
						break;
					default:
						ASSERT(false);
				}
			}

			lMouseDownInChannel = -1;
			break;
		}

		case eRMouseUp:
		{
			pp_int32 channel = pointToChannel(*(PPPoint*)event->getDataPtr());

			if (channel != -1 && channel == rMouseDownInChannel && lMouseDownInChannel == -1 && !didSoloChannel)
			{
				if ((::getKeyModifier() & KeyModifierSHIFT))
				{
					handleSingleRec(channel);
				}
				else handleRec(channel);
			}

			rMouseDownInChannel = -1;
			break;
		}
		default:
			break;
	}

	return 0;
}

bool ScopesControl::isSoloChannel(pp_int32 c) const
{
	pp_int32 i = 0;
	for (pp_int32 j = 0; j < numChannels; j++)
		if (muteChannels[j])
			i++;

	if (!muteChannels[c] && i == numChannels-1)
		return true;

	return false;
}

bool ScopesControl::isSingleRecChannel(pp_int32 c) const
{
	pp_int32 i = 0;
	for (pp_int32 j = 0; j < numChannels; j++)
		if (!recChannels[j])
			i++;

	if (recChannels[c] && i == numChannels-1)
		return true;

	return false;
}

pp_int32 ScopesControl::pointToChannel(const PPPoint& pt)
{
	if (numChannels < 2)
		return -1;

    for (pp_int32 c = 0; c < numChannels; c++)
    {
        if (pt.x >= channelRects[c].x1 && pt.x < channelRects[c].x2 &&
            pt.y >= channelRects[c].y1 && pt.y < channelRects[c].y2) {
            return c;
        }
    }

	return -1;
}
