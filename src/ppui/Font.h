/*
 *  ppui/Font.h
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

// Font.h: interface for the PPFont class.
//
//////////////////////////////////////////////////////////////////////
#ifndef FONT__H
#define FONT__H

#include "BasicTypes.h"
#include "SimpleVector.h"

#define MAXFONTS 256

class PPGraphicsAbstract;

class Bitstream
{
public:
	pp_uint8* buffer;
	pp_uint32 bufferSize;

	void setSource(const void* buffer, pp_uint32 bufferSize) { this->buffer = (pp_uint8*)buffer;  this->bufferSize = bufferSize; }

	Bitstream(const void* buffer, pp_uint32 bufferSize) { setSource(buffer, bufferSize); }

	void clear() { for (pp_uint32 i = 0; i < bufferSize; i++) buffer[i] = 0; }

	bool read(pp_uint32 offset) const
	{		
		return (buffer[offset>>3]>>(offset&7))&1;
	}

	void write(pp_uint32 offset, bool bit)
	{
		pp_uint8 mask = ~(1<<(offset&7));;
		
		buffer[offset>>3] &= mask;
		buffer[offset>>3] |= ((pp_uint8)bit<<(offset&7));
	}

};

class PPFont  
{
private:
	// cache big font, is created from normal 8x8 font
	static pp_uint8 bigFont[];

public:
	enum FontID
	{
		FONT_TINY = 0,
		FONT_SYSTEM = 1,
		FONT_LARGE = 2,
		FONT_HUGE = 3,
		FONT_LAST
	};

private:
	static PPFont* fontInstances[MAXFONTS];
	static pp_uint32 numFontInstances;
	
	pp_uint32 fontId;

	PPFont(pp_uint8* bits, pp_uint32 charWidth, pp_uint32 charHeight, pp_uint32 fontId); 

	struct FontFamilyDescription
	{
		PPSize size;							// Size of font character (width, height)
		const char* name;						// Name of font familiy (Tiny, System, Large)
		const char* internalName;				// Internal name
		pp_uint32 fontEntryIndex;				// Index to font entry table (source for current selected font)
		const pp_uint32 defaultFontEntryIndex;	// Default index to font entry table
		pp_int32 enumerationIndex;				// Is currently enumerating entries

		FontFamilyDescription(PPSize theSize,
							  const char* theName,
							  const char* theInternalName,
							  pp_uint32 theFontEntryIndex,
							  const pp_uint32 theDefaultFontEntryIndex,
							  pp_int32 theEnumerationIndex) :
			size(theSize),							
			name(theName),						
			internalName(theInternalName),				
			fontEntryIndex(theFontEntryIndex),				
			defaultFontEntryIndex(theDefaultFontEntryIndex),
			enumerationIndex(theEnumerationIndex)		
		{
		}
	};

	static FontFamilyDescription fontFamilyDescriptions[FONT_LAST];
	static pp_int32 enumerationIndex;
	
	static void createLargeFromSystem(pp_uint32 index);

public:

	pp_uint8* fontBits;
	Bitstream* bitstream;

	const pp_uint32 charWidth, charHeight;
	const pp_uint32 charDim;

public:
	~PPFont();

	pp_uint32 getCharWidth() const { return charWidth; }
	pp_uint32 getCharHeight() const { return charHeight; }

	static PPFont* getFont(pp_uint32 fontId);

	bool getPixelBit(pp_uint8 chr, pp_uint32 x, pp_uint32 y) const { return bitstream->read(chr*charDim+y*charWidth+x); }

	pp_uint32 getStrWidth(const char* str) const;
	
	enum ShrinkTypes
	{
		ShrinkTypeMiddle,
		ShrinkTypeEnd
	};

	void shrinkString(const char* text, char* result, pp_uint32 maxWidth, ShrinkTypes shrinkType = ShrinkTypeMiddle)
	{
		pp_uint32 numchars = maxWidth / getCharWidth();
		
		pp_uint32 textLen = strlen(text);
		
		if (textLen <= numchars)
		{
			strcpy(result, text);
			return;
		}
		
		if (shrinkType == ShrinkTypeMiddle)
		{
			numchars-=(textLen & 1);

			char* temp = result;

			pp_uint32 i;
			for (i = 0; i < (numchars / 2) - ((textLen+1) & 1); i++)
				temp[i] = text[i];

			while (temp[i-1] == ' ' && i > 1)
				i--;

			temp[i] = '\xef';

			pp_uint32 j = i+1;
			pp_uint32 k = strlen(text) - (numchars / 2 + (textLen & 1));
			while (text[k] == ' ' && k < textLen)
				k++;

			for (i = k; i < textLen; i++, j++)
				temp[j] = text[i];

			temp[j] = '\0';
		}
		else if (shrinkType == ShrinkTypeEnd)
		{
			pp_int32 i = 0;
			for (i = 0; i < (signed)numchars-1; i++)
			{
				result[i] = text[i];
			}

			while (i > 0 && result[i-1] == '.')
				i--;
			
			result[i++] = '\xef';
			result[i] = '\0';
		}
	}
	
	PPString shrinkString(const PPString& text, pp_uint32 maxWidth, ShrinkTypes shrinkType = ShrinkTypeMiddle)
	{
		char* temp = new char[(text.length()+1)*2];
		shrinkString(text, temp, maxWidth, shrinkType);
		PPString result(temp);		
		delete[] temp;
		return result;
	}	
	
	static const char* getFamilyInternalName(FontID fontID);
	
	static const char* getFirstFontFamilyName();
	static const char* getNextFontFamilyName();

	static const char* getFirstFontEntryName(FontID fontID);
	static const char* getNextFontEntryName(FontID fontID);
	
	static void selectFontFace(FontID fontID, const char* faceName);
	static const char* getCurrentFontFace(FontID fontID);
};

#endif 
