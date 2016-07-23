/*
 *  tracker/PianoControl.cpp
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

#include "PianoControl.h"
#include "Screen.h"
#include "GraphicsAbstract.h"
#include "Font.h"
#include "ScrollBar.h"
#include "Piano.h"
#include "Tools.h"
#include "PatternTools.h"
#include "TrackerConfig.h"

#define SCROLLBARWIDTH SCROLLBUTTONSIZE

static const bool blackKeys[] =
{
	false,
	true,
	false,
	true,
	false,
	false,
	true,
	false,
	true,
	false,
	true,
	false
};

static const PPPoint positions[] = 
{
	PPPoint(1,17),
	PPPoint(5,6),
	PPPoint(9,17),
	PPPoint(13,6),
	PPPoint(17, 17),
	PPPoint(25, 17),
	PPPoint(29,6),
	PPPoint(33, 17),
	PPPoint(37,6),
	PPPoint(41, 17),
	PPPoint(45,6),
	PPPoint(49, 17)
};

static const PPColor colors[] = 
{
	PPColor(0,0,0),
	PPColor(255,255,255),
	PPColor(0,0,0),
	PPColor(255,255,255),
	PPColor(0,0,0),
	PPColor(0,0,0),
	PPColor(255,255,255),
	PPColor(0,0,0),
	PPColor(255,255,255),
	PPColor(0,0,0),
	PPColor(255,255,255),
	PPColor(0,0,0)
};

pp_int32 PianoControl::XMAX()
{
	return pianoBitmap->getBitmapWidth();
}

pp_int32 PianoControl::YMAX()
{
	return pianoBitmap->getBitmapHeight();
}

pp_int32 PianoControl::KEYWIDTH()
{
	return XMAX()/pianoBitmap->getBitmapLUTWidth();
}

PianoControl::PianoControl(pp_int32 id, 
						   PPScreen* parentScreen, 
						   EventListenerInterface* eventListener, 
						   const PPPoint& location, 
						   const PPSize& size, 
						   pp_uint8 numNotes,
						   bool border/*= true*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	border(border),
	NUMNOTES(numNotes),
	borderColor(&ourOwnBorderColor),
	mode(ModeEdit)
{
	// default color
	ourOwnBorderColor.set(192, 192, 192);

	hScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y + size.height - SCROLLBARWIDTH - 1), size.width - 1, true);

#ifndef __LOWRES__
	xscale = 1;
	yscale = 1;
	pianoBitmap = PianoBitmapLarge::getInstance();
#else
	xscale = 2;
	yscale = 1;
	pianoBitmap = PianoBitmapSmall::getInstance();
#endif

	xMax = XMAX()*xscale;
	yMax = YMAX()*yscale;

	visibleWidth = size.width - 2;
	visibleHeight = size.height - SCROLLBARWIDTH - 2;

	adjustScrollbars();
	
	startPos = 0;
	
	caughtControl = NULL;	
	controlCaughtByLMouseButton = controlCaughtByRMouseButton = false;

	nbu = new pp_uint8[NUMNOTES];
	memset(nbu, 0, NUMNOTES);

	keyState = new KeyState[NUMNOTES];

	sampleIndex = 0;
}

PianoControl::~PianoControl()
{
	delete hScrollbar;

	delete[] nbu;
	
	delete[] keyState;
}

void PianoControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	
	pp_int32 xOffset = 2;

	pp_int32 yOffset = 2;

	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height);

	if (border)
	{
		drawBorder(g, *borderColor);
	}

	g->setRect(location.x + 1, location.y + 1, location.x + size.width - 2, location.y + size.height - 2);

	pp_int32 width = visibleWidth;
	
	pp_int32 adder = 65536/xscale;
	
	pp_int32 oy = location.y + 1;
	
	// for black piano keys
	PPColor colCorrect(TrackerConfig::colorThemeMain);
	colCorrect.r<<=1; colCorrect.g<<=1; colCorrect.b<<=1;
	// for white piano keys
	PPColor colCorrect2(TrackerConfig::colorThemeMain);
	colCorrect2.scale(1.5f, 1.5f, 1.6f);
	pp_int32 avg = (colCorrect2.b+colCorrect2.g+colCorrect2.r) / 3;
	PPColor colCorrect4(avg, avg, avg);
	
	pp_int32 PIANO_LUT_WIDTH = pianoBitmap->getBitmapLUTWidth();
	const pp_uint8* PIANO_LUT = pianoBitmap->getBitmapLUT();
	const pp_uint8* PIANO = pianoBitmap->getBitmap();
	
	const pp_int32* DIVLUT = pianoBitmap->getDIVLUT();
	const pp_int32* MODLUT = pianoBitmap->getMODLUT(); 
	
	const pp_int32 XMAX = this->XMAX();
	
	for (pp_int32 y = 0; y < visibleHeight; y++)
	{
		pp_int32 ry = y/yscale;

		pp_int32 offset = (ry*XMAX+(startPos/xscale));
		pp_int32 ofs = 0;

		const pp_uint8* src = PIANO_LUT + (ry*PIANO_LUT_WIDTH*3);
		
		const pp_int32* divLutPtr = DIVLUT-ry*XMAX;
		const pp_int32* modLutPtr = MODLUT-ry*XMAX;
		
		pp_int32 ox = location.x + 1;
		for (pp_int32 x = 0; x < width; x++)
		{
			pp_int32 sx = offset+(ofs>>16);
			pp_int32 px = modLutPtr[sx];

			pp_int32 c = src[px];
			pp_int32 gr = src[px+1];
			pp_int32 b = src[px+2];
			
			if (c == 255 && gr == 0 && b == 0)
			{
				g->setColor(PIANO[sx], PIANO[sx], PIANO[sx]);
			}
			else
			{
				// color values equal/above 240
				if (c >= 240) c-=240;
			
				pp_int32 note = divLutPtr[sx] + c;
			
				if (keyState[note].pressed)
				{
					if (keyState[note].muted)
					{
						if (colors[c].r)
							g->setSafeColor((PIANO[sx]+(colCorrect4.r>>2)), (PIANO[sx]+(colCorrect4.g>>2)), (PIANO[sx]+(colCorrect4.b>>2)));
						else
							g->setSafeColor((PIANO[sx]*colCorrect4.r)>>8, (PIANO[sx]*colCorrect4.g)>>8, (PIANO[sx]*colCorrect4.b)>>8);
					}
					else
					{
						if (colors[c].r)
							g->setSafeColor((PIANO[sx]+(colCorrect.r>>1)), (PIANO[sx]+(colCorrect.g>>1)), (PIANO[sx]+(colCorrect.b>>1)));
						else
							g->setSafeColor((PIANO[sx]*colCorrect2.r)>>8, (PIANO[sx]*colCorrect2.g)>>8, (PIANO[sx]*colCorrect2.b)>>8);
					}
				}
				else
					g->setColor(PIANO[sx], PIANO[sx], PIANO[sx]);
			}
			g->setPixel(ox, oy);
			ofs+=adder;
			ox++;
		}
		oy++;
	}
	
	float newXScale = pianoBitmap->getBitmapWidth() / PianoBitmapSmall::getInstance()->getBitmapWidth();
	float newYScale = pianoBitmap->getBitmapHeight() / PianoBitmapSmall::getInstance()->getBitmapHeight();

	if (mode == ModeEdit)
	{
		PPFont* font;
		
		font = (xscale == 1 && pianoBitmap == PianoBitmapSmall::getInstance()) ? 
			PPFont::getFont(PPFont::FONT_TINY) : PPFont::getFont(PPFont::FONT_SYSTEM);
		
		g->setFont(font);
		
		xOffset = 0;
		yOffset = -1;
		if (xscale > 1)
		{
			xOffset = 2;
		}
		else if (pianoBitmap == PianoBitmapLarge::getInstance())
		{
			xOffset = 1;
		}
		
		if (yscale > 1)
		{
			yOffset = -2;
		}
		else if (pianoBitmap == PianoBitmapLarge::getInstance())
		{
			yOffset = 0;
		}
		
		for (pp_int32 i = 0; i < NUMNOTES; i++)
		{
			pp_int32 posx = location.x + 1 + (i/12)*(PIANO_LUT_WIDTH*xscale) - startPos;

			//if (blackKeys[i%12] && pianoBitmap == PianoBitmapLarge::getInstance())
			//	posx;

			g->setColor(colors[i%12]);
			
			char str[3] = "-";
			
			if (nbu[i] != 255)
				PPTools::convertToHex(str, nbu[i], 1);
			
			g->drawChar(str[0],
						(pp_int32)(posx + (positions[i%12].x*xscale + xOffset)*newXScale), 
						(pp_int32)(location.y + 1 + (positions[i%12].y*yscale + yOffset)*newYScale));
		}
		
	}
	else if (mode == ModePlay && (xscale >= 2 || pianoBitmap == PianoBitmapLarge::getInstance()))
	{
		PPFont* font = PPFont::getFont(PPFont::FONT_TINY);
		
		g->setFont(font);
		
		if (pianoBitmap == PianoBitmapLarge::getInstance())
		{
			xOffset = 1;
			yOffset = 0;
		}
		else
		{
			xOffset = 1;
			yOffset = -1;
		}
		
		for (pp_int32 i = 0; i < NUMNOTES; i++)
		{
			pp_int32 posx = location.x + 1 + (i/12)*(PIANO_LUT_WIDTH*xscale) - startPos;
			
			if (blackKeys[i%12] && pianoBitmap == PianoBitmapLarge::getInstance())
				posx--;
			
			g->setColor(colors[i%12]);
			
			char str[4]/* = "C#"*/;
			PatternTools::getNoteName(str, i+1);
			if (str[1] == '-')
			{
				str[1] = str[2];
				str[2] = '\0';
				xOffset = 0;
			}
			else
			{
				str[2] = '\0';
				xOffset = 1;
			}
		
			pp_int32 correctx = (KEYWIDTH()*xscale) / 2 - font->getStrWidth(str) / 2 - 3;
			if (correctx < 0)
				correctx = 0;
		
			xOffset += correctx;
					
			g->drawString(str,
						  (pp_int32)(posx + (positions[i%12].x*xscale + xOffset)*newXScale), 
						  (pp_int32)(location.y + 1 + (positions[i%12].y*yscale + yOffset)*newYScale));
		}
	}

	hScrollbar->paint(g);

}

pp_int32 PianoControl::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	//if (!visible)
	//	return 0;

	switch (event->getID())
	{
		case eRMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			if (hScrollbar->hit(*p))
			{
				if (controlCaughtByLMouseButton)
					break;
				controlCaughtByRMouseButton = true;
				caughtControl = hScrollbar;
				caughtControl->dispatchEvent(event);
			}
			break;
		}		
	
		case eLMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			// Clicked on horizontal scrollbar -> route event to scrollbar and catch scrollbar control
			if (hScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				controlCaughtByLMouseButton = true;
				caughtControl = hScrollbar;
				caughtControl->dispatchEvent(event);
			}
			// Clicked in client area
			else
			{

				pp_int32 note = positionToNote(*p);

				if (note != -1)
				{

					switch (mode)
					{
						case ModeEdit:
						{
							nbu[note] = (pp_uint8)sampleIndex & 0xf;
							PPEvent e(eValueChanged, &nbu, sizeof(pp_uint8*));	
							eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
							break;
						}

						case ModePlay:
						{
							currentSelectedNote = note;
							PPEvent e(eSelection, &note, sizeof(note));
							eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
							break;
						}
					}

					parentScreen->paintControl(this);
				}
			}

			break;
		}

		case eRMouseUp:
		{
			controlCaughtByRMouseButton = false;
			if (caughtControl && !controlCaughtByLMouseButton && !controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				caughtControl = NULL;			
				break;
			}
			break;
		}

		case eLMouseUp:
			if (caughtControl == NULL)
			{
				pp_int32 note = currentSelectedNote;

				if (note != -1)
				{
					
					switch (mode)
					{
						case ModePlay:
						{
							pp_int32 v = (1 << 16) + currentSelectedNote;
							PPEvent e(eSelection, &v, sizeof(v));
							eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
							break;
						}
						case ModeEdit:
							break;
					}
					
					parentScreen->paintControl(this);
				}
				break;
			}
			
			controlCaughtByLMouseButton = false;
			if (!controlCaughtByLMouseButton && !controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				caughtControl = NULL;
			}
			break;

		case eLMouseDrag:
		{
			if (caughtControl && controlCaughtByLMouseButton)
			{
				caughtControl->dispatchEvent(event);
				break;
			}
			
			//positionToNote(*(PPPoint*)event->getDataPtr());

			//parentScreen->paintControl(this);

			pp_int32 note = positionToNote(*(PPPoint*)event->getDataPtr());

			if (note != -1)
			{
				
				switch (mode)
				{
					case ModeEdit:
					{
						nbu[note] = (pp_uint8)sampleIndex & 0xf;
						
						PPEvent e(eValueChanged, &nbu, sizeof(pp_uint8*));	
						
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
						break;
					}
					
					case ModePlay:
					{
						if (note == currentSelectedNote)
							break;
					
						pp_int32 v = (1 << 16) + currentSelectedNote;
						PPEvent e(eSelection, &v, sizeof(v));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
						
						currentSelectedNote = note;
						PPEvent e2(eSelection, &note, sizeof(note));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e2);
						break;
					}
				}
				
				parentScreen->paintControl(this);
			}

			break;
		}

		case eRMouseDrag:
		{
			if (caughtControl && controlCaughtByRMouseButton)
				caughtControl->dispatchEvent(event);
			break;
		}
		
		case eRMouseRepeat:
		{
			if (caughtControl && controlCaughtByRMouseButton)
				caughtControl->dispatchEvent(event);
			break;
		}
		
		case eMouseWheelMoved:
		{
			TMouseWheelEventParams* params = (TMouseWheelEventParams*)event->getDataPtr();
			
			if ((params->deltaX > 0 || params->deltaY < 0) && hScrollbar)
			{
				PPEvent e(eBarScrollDown);
				handleEvent(reinterpret_cast<PPObject*>(hScrollbar), &e);
			}
			else if ((params->deltaX < 0 || params->deltaY > 0) && hScrollbar)
			{
				PPEvent e(eBarScrollUp);
				handleEvent(reinterpret_cast<PPObject*>(hScrollbar), &e);
			}
			
			event->cancel();
			
			break;
		}
		
		default:
			if (caughtControl == NULL)
				break;

			caughtControl->dispatchEvent(event);
			break;

	}

	return 0;
}

pp_int32 PianoControl::handleEvent(PPObject* sender, PPEvent* event)
{	
	// Horizontal scrollbar, scroll up (=left)
	if (sender == reinterpret_cast<PPObject*>(hScrollbar) &&
			 event->getID() == eBarScrollUp)
	{
		startPos-=KEYWIDTH()*xscale;
		if (startPos < 0)
			startPos = 0;

		pp_int32 visibleItems = visibleWidth;

		float v = (float)(getMaxWidth() - visibleItems);

		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, scroll down (=right)
	else if (sender == reinterpret_cast<PPObject*>(hScrollbar) &&
			 event->getID() == eBarScrollDown)
	{
		pp_int32 visibleItems = visibleWidth;

		startPos+=KEYWIDTH()*xscale;
		if (startPos + visibleItems >= (signed)getMaxWidth())
			startPos = getMaxWidth() - visibleItems;

		float v = (float)(getMaxWidth() - visibleItems);

		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, position changed
	else if (sender == reinterpret_cast<PPObject*>(hScrollbar) &&
			 event->getID() == eBarPosChanged)
	{

		float pos = hScrollbar->getBarPosition()/65536.0f;

		pp_int32 visibleItems = visibleWidth;
		
		float v = (float)(getMaxWidth() - visibleItems);

		startPos = (pp_uint32)(v*pos);
	}

	parentScreen->paintControl(this);
	
	return 0;
}

pp_int32 PianoControl::getMaxWidth()
{
	return XMAX()*xscale;
}

void PianoControl::adjustScrollbars()
{
	float s = (float)visibleWidth / (float)getMaxWidth();

	float olds = hScrollbar->getBarSize() / 65536.0f;

	hScrollbar->setBarSize((pp_int32)(s*65536.0f), false);	

	s = hScrollbar->getBarSize() / 65536.0f;

	float scale = s / olds;

	float pos = hScrollbar->getBarPosition()/65536.0f;
	hScrollbar->setBarPosition((pp_int32)(pos*scale*65536.0f));

	pos = hScrollbar->getBarPosition()/65536.0f;

	pp_int32 visibleItems = visibleWidth;
		
	float v = (float)(getMaxWidth() - visibleItems);

	startPos = (pp_uint32)(v*pos);	

	if (startPos < 0)
	{
		startPos = 0;
	}

		//pp_int32 entireSize = (horizontal?this->size.width:this->size.height) - SCROLLBUTTONSIZE*2;
}

void PianoControl::setxScale(pp_int32 scale) 
{ 
	xscale = scale; 
	adjustScrollbars();
	assureNoteVisible(4*12);
}

void PianoControl::setyScale(pp_int32 scale) 
{ 
	yscale = scale; 
}

void PianoControl::setLocation(const PPPoint& location)
{
	PPControl::setLocation(location);

	pp_int32 p = hScrollbar->getBarPosition();
	pp_int32 s = hScrollbar->getBarSize();
	delete hScrollbar;
	hScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y + size.height - SCROLLBARWIDTH - 1), size.width - 1, true);
	hScrollbar->setBarSize(s);
	hScrollbar->setBarPosition(p);
}

void PianoControl::setSize(const PPSize& size)
{
	PPControl::setSize(size);
	delete hScrollbar;

	hScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y + size.height - SCROLLBARWIDTH - 1), size.width - 1, true);

	visibleWidth = size.width - 2;
	visibleHeight = size.height - SCROLLBARWIDTH - 2;

	adjustScrollbars();
	assureNoteVisible(4*12);
}

void PianoControl::setSampleTable(const pp_uint8* nbu)
{
	if (nbu == NULL)
	{
		memset(this->nbu, 0, NUMNOTES);
		return;
	}

	memcpy(this->nbu, nbu, NUMNOTES);
}

void PianoControl::pressNote(pp_int32 note, bool pressed, bool muted/* = false*/)
{
	if (note >= 0 && note < NUMNOTES)
	{
		keyState[note].pressed = pressed;
		keyState[note].muted = muted;
	}
}

bool PianoControl::getNoteState(pp_int32 note) const
{
	if (note >= 0 && note < NUMNOTES)
		return keyState[note].pressed;
	return false;
}

void PianoControl::assureNoteVisible(pp_int32 note)
{
	pp_int32 PIANO_LUT_WIDTH = pianoBitmap->getBitmapLUTWidth();
	
	pp_int32 startPos = (PIANO_LUT_WIDTH*(note/12)+positions[note%12].x)*xscale;
	float v = (float)(getMaxWidth() - visibleWidth);
	hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	float pos = hScrollbar->getBarPosition()/65536.0f;
	startPos = (pp_uint32)(v*pos);
	this->startPos = startPos;
}

pp_int32 PianoControl::positionToNote(PPPoint cp)
{

	if (sampleIndex < 0)
		return -1;

	cp.x -= location.x + 1;
	cp.y -= location.y + 1;
				
	if (cp.x < 0 || cp.x >= visibleWidth || cp.y < 0 || cp.y >= visibleHeight)
		return -1;

	pp_int32 PIANO_LUT_WIDTH = pianoBitmap->getBitmapLUTWidth();
	const pp_uint8* PIANO_LUT = pianoBitmap->getBitmapLUT();
	
	cp.x/=xscale;
	cp.y/=yscale;
	
	cp.x+=startPos/xscale;
				
	pp_int32 octave = cp.x / PIANO_LUT_WIDTH;
	pp_int32 ox = cp.x % PIANO_LUT_WIDTH;
				
	pp_int32 c = PIANO_LUT[(cp.y * PIANO_LUT_WIDTH + ox)*3];
	pp_int32 g = PIANO_LUT[(cp.y * PIANO_LUT_WIDTH + ox)*3+1];
	pp_int32 b = PIANO_LUT[(cp.y * PIANO_LUT_WIDTH + ox)*3+2];
	
	if (c == 255 && g == 0 && b == 0)
		return -1;
	
	// color values equal/above 240
	if (c >= 240) c-=240;
				
	pp_int32 note = octave*12 + c;
				
	if (note < 0 || note >= NUMNOTES)
		return -1;

	return note;
}
