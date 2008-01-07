/*
 *  tracker/AnimatedFXControl.cpp
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
 *  PeakLevelControl.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *
 */

#include "AnimatedFXControl.h"
#include "Screen.h"
#include "GraphicsAbstract.h"
#include "Font.h"
#include "ScrollBar.h"
#include "TrackerConfig.h"
#include "LogoSmall.h"
#include "TrackerConfig.h"
#include "Tools.h"

#undef FXTOGGLE

#ifdef __LOWRES__
#define __SIMPLEFX__
#endif

#ifdef __SIMPLEFX__
#include "Starfield.h"
#undef FXTOGGLE
#else
#include "TwisterFX.h"
#include "Fire.h"
#endif

static const char* text = "Hello everybody and welcome to MilkyTracker the original portable tracker with REAL oldschool feeling guaranteed. "
						  "MilkyTracker offers almost identical FT2 behaviour in both user experience and playback. Why don't you play some tunes and decide for yourself! "
						  "Hihos fly out to everyone who has contributed: Kenet (for making these awesome logos), Kmuland, raina (the icons and everything), "
						  "Deltafire (fixing and enhancing the SDL port), Flasch (lots of bugfixing here), Gary P. Scavone for RtAudio and RtMidi, "
						  "Andrew Simper of Vellocet for the noise code, David Ross for the EQ code, Andreas@lucid-cake.net (providing a Zaurus SL-5500 and the OSX build environment), "
						  "Svenzzon^TiTAN (for his nice example tune) and all other TiTAN members, Strobe, Funky Fish, d0pefish (lavaburn.com, you know why =), Conna, Skyrunner^Brain Control, "
						  "505 on #atariscene, Dualtrax, grmbls (for starting this all off), StarFish, m0d and everyone who finds this program useful. "
						  "A very SPECIAL thanks goes out to everyone who made donations to support me, I really appreciate it. Now go ahead and make some nice tune or visit #milkytracker on Espernet... "
						  "----- wrapping ---- wrapping ---- wrapping ---- .....         ";

pp_int32 AnimatedFXControl::counter = 0;

void AnimatedFXControl::createFX()
{
	if (fx)
	{
		delete fx;
		fx = NULL;
	}
	if (!vscreen)
		vscreen = new pp_uint8[visibleWidth*visibleHeight*3];
#ifdef __SIMPLEFX__
	fx = new Starfield(visibleWidth, visibleHeight);
#else
#ifdef FXTOGGLE
	if (++counter & 1)
		fx = new Fire(visibleWidth, visibleHeight);
	else
		fx = new TwisterFX(visibleWidth, visibleHeight);
#else
	fx = new TwisterFX(visibleWidth, visibleHeight);
#endif
#endif
}

AnimatedFXControl::AnimatedFXControl(pp_int32 id, 
								   PPScreen* parentScreen, 
								   EventListenerInterface* eventListener, 
								   PPPoint location, 
								   PPSize size, 
								   bool border/*= true*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	borderColor(&ourOwnBorderColor),
	fx(NULL), vscreen(NULL)
{
	this->border = border;

	// default color
	color.set(0, 0, 0);

	ourOwnBorderColor.set(192, 192, 192);

	visibleWidth = size.width - 2;
	visibleHeight = size.height - 2;
	
#ifndef __SIMPLEFX__
	createFX();
#endif
	
	xPos = visibleWidth << 8;
	
	font = PPFont::getFont(PPFont::FONT_SYSTEM);
	
	textBufferMaxChars = visibleWidth*2 / font->getCharWidth();
	textBuffer = new char[textBufferMaxChars + 1];

	currentCharIndex = 0;
	pp_int32 j = currentCharIndex % strlen(text);
	for (pp_int32 i = 0; i <  (signed)textBufferMaxChars; i++)
	{
		if (j >= (signed)strlen(text))
			j = 0;
		textBuffer[i] = text[j];
		j++;
	}
	
	lastTime = 0;
	
	milkyVersionString[0] = 'v';
	PPTools::convertToHex(milkyVersionString+1, (TrackerConfig::version>>16)&0xF, 1);
	milkyVersionString[2] = '.';
	PPTools::convertToHex(milkyVersionString+3, (TrackerConfig::version>>8)&0xFF, 2);
	milkyVersionString[5] = '.';
	PPTools::convertToHex(milkyVersionString+6, TrackerConfig::version&0xFF, 2);
	milkyVersionString[8] = '\0';
}

AnimatedFXControl::~AnimatedFXControl()
{
	delete[] textBuffer;
	delete fx;
	delete[] vscreen;
}

void AnimatedFXControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible() || !vscreen)
		return;

	g->setFont(font);

	pp_int32 xOffset = 2;
	pp_int32 yOffset = 2;

	g->setRect(location.x, location.y, location.x + size.width+1, location.y + size.height+1);

	if (border)
	{
		drawThickBorder(g, *borderColor);
	}

	g->setRect(location.x + 1, location.y + 1, location.x + size.width - 2, location.y + size.height - 2);
	
	fx->render(vscreen);
	
	pp_uint8* vptr = vscreen;
	const pp_uint8* iptr = LogoSmall::rawData;
	
	PPPoint p = location;
	p.x+=xOffset; p.y+=yOffset;
	PPSize s;
	s.width = visibleWidth-2;
	s.height = visibleHeight-2;
	
	for (pp_int32 i = 0; i < visibleHeight-2; i++)
		for (pp_int32 j = 0; j < visibleWidth-2; j++)
		{
			pp_int32 a = *(iptr+3);

			if (a == 255)
			{
				*vptr = *(iptr+2);
				*(vptr+1) = *(iptr+1);
				*(vptr+2) = *iptr;
			}
			else if (a)
			{
				*vptr = (a*(*(iptr+2))+(255-a)*(*vptr))>>8;
				*(vptr+1) = (a*(*(iptr+1))+(255-a)*(*(vptr+1)))>>8;
				*(vptr+2) = (a*(*iptr)+(255-a)*(*(vptr+2)))>>8;
			}
						
			vptr+=3;
			iptr+=4;
		}
		
	g->blit(vscreen, p, s, visibleWidth*3, 3);
	
	g->setRect(location.x+2, location.y+2, location.x + size.width-2, location.y + size.height-2);

	// Printf version string
	g->setColor(0, 0, 0);
	g->drawString(milkyVersionString, location.x + 4 + 1, location.y + 4 + 1);
	g->setColor(255, 255, 255);
	g->drawString(milkyVersionString, location.x + 4, location.y + 4);
	
	g->setColor(0, 0, 0);
	g->drawString(textBuffer, location.x + (xPos>>8) + 1, location.y + visibleHeight - 10 + 1);
	g->setColor(255, 255, 255);
	g->drawString(textBuffer, location.x + (xPos>>8), location.y + visibleHeight - 10);

	pp_uint32 currentTime = ::PPGetTickCount(); 
	pp_uint32 dTime;
	if (lastTime)
		dTime = currentTime - lastTime;
	else
		dTime = 40;
	
	xPos-=dTime*8;
	if (xPos < -(signed)(font->getCharWidth()<<8))
	{
		while (xPos < -(signed)(font->getCharWidth()<<8))
		{
			xPos+=font->getCharWidth()<<8;
			currentCharIndex++;
		}
		pp_int32 j = currentCharIndex % strlen(text);
		for (pp_int32 i = 0; i < (signed)textBufferMaxChars; i++)
		{
			if (j >= (signed)strlen(text))
				j = 0;
			textBuffer[i] = text[j];
			j++;
		}
			
	}

	fx->update(dTime<<11);
	
	lastTime = currentTime;
}

pp_int32 AnimatedFXControl::callEventListener(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	//if (!visible)
	//	return 0;

	switch (event->getID())
	{
		case eTimer:
			if (!(fxTicker & 1))
				parentScreen->paintControl(this);
			fxTicker++;
 			break;
	
		case eLMouseUp:
		case eRMouseUp:
		{
			PPEvent e(eCommand);
			pp_int32 res = eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e); 
			break;
		}
	}

	return 0;
}

void AnimatedFXControl::show(bool bShow)
{
	PPControl::show(bShow);
	if (!bShow)
	{
#if defined(FXTOGGLE) || defined(__LOWRES__) 
		delete fx;
		fx = NULL;
		delete[] vscreen;
		vscreen = NULL;
#endif
		lastTime = 0;
	}
	else
	{
#if defined(FXTOGGLE) || defined(__LOWRES__) 
		createFX();
#endif
	}
}
