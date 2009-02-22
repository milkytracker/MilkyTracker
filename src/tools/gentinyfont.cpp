/*
 *  tools/gentinyfont.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include "BasicTypes.h"
#include "PPPath_POSIX.h"
#include "XMFile.h"
#include "Font.h"

static pp_uint8 tinyFont[5*6*256/8];

const pp_uint32 tinyCharacters[] = 
{
// Capital letters from A
						  0x00100,
						  0x01010,
						  0x01010,
						  0x01110,
						  0x01010,
						  0x00000,

						  0x01100,
						  0x01010,
						  0x01100,
						  0x01010,
						  0x01100,
						  0x00000,
						  
						  0x00110,
						  0x01000,
						  0x01000,
						  0x01000,
						  0x00110,
						  0x00000,

						  0x01100,
						  0x01010,
						  0x01010,
						  0x01010,
						  0x01100,
						  0x00000,

						  0x01110,
						  0x01000,
						  0x01110,
						  0x01000,
						  0x01110,
						  0x00000,

						  0x01110,
						  0x01000,
						  0x01100,
						  0x01000,
						  0x01000,
						  0x00000,

						  0x00110,
						  0x01000,
						  0x01110,
						  0x01010,
						  0x00110,
						  0x00000,
						  
						  0x01010,
						  0x01010,
						  0x01110,
						  0x01010,
						  0x01010,
						  0x00000,
						  
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00000,
						  
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x01100,
						  0x00000,
						  
						  0x01010,
						  0x01010,
						  0x01100,
						  0x01010,
						  0x01010,
						  0x00000,
						  
						  0x01000,
						  0x01000,
						  0x01000,
						  0x01000,
						  0x01110,
						  0x00000,
						  
						  0x01001,
						  0x01111,
						  0x01001,
						  0x01001,
						  0x01001,
						  0x00000,
						  
						  0x01001,
						  0x01101,
						  0x01101,
						  0x01011,
						  0x01001,
						  0x00000,
						  
						  0x00100,
						  0x01010,
						  0x01010,
						  0x01010,
						  0x00100,
						  0x00000,
						  
						  0x01100,
						  0x01010,
						  0x01110,
						  0x01000,
						  0x01000,
						  0x00000,
						  
						  0x00100,
						  0x01010,
						  0x01010,
						  0x01010,
						  0x00111,
						  0x00000,
						  
						  0x01100,
						  0x01010,
						  0x01100,
						  0x01010,
						  0x01010,
						  0x00000,
						  
						  0x00110,
						  0x01000,
						  0x00100,
						  0x00010,
						  0x01100,
						  0x00000,
						  
						  0x01110,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00000,
						  
						  0x01010,
						  0x01010,
						  0x01010,
						  0x01010,
						  0x01110,
						  0x00000,
						  
						  0x01010,
						  0x01010,
						  0x01010,
						  0x01010,
						  0x00100,
						  0x00000,

						  0x01001,
						  0x01001,
						  0x01001,
						  0x01111,
						  0x01001,
						  0x00000,
						  
						  0x01010,
						  0x01010,
						  0x00100,
						  0x01010,
						  0x01010,
						  0x00000,
						  
						  0x01010,
						  0x01010,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00000,

						  0x01110,
						  0x00010,
						  0x00100,
						  0x01000,
						  0x01110,
						  0x00000,
						  
// small letters = capital letters (lazy guy)
						  0x00000,	// a
						  0x01110,
						  0x00011,
						  0x01101,
						  0x01111,
						  0x00000,

						  0x00000,	// b
						  0x01000,
						  0x01100,
						  0x01010,
						  0x00100,
						  0x00000,
						  
						  0x00000,	// c
						  0x00110,
						  0x01000,
						  0x01000,
						  0x00110,
						  0x00000,

						  0x00001,	// d
						  0x00001,
						  0x00111,
						  0x01001,
						  0x00111,
						  0x00000,

						  0x00000,	// e
						  0x00110,
						  0x01001,
						  0x01110,
						  0x00111,
						  0x00000,

						  0x00000,	// f
						  0x00011,
						  0x00100,
						  0x01110,
						  0x00100,
						  0x00000,

						  0x00000,	// g
						  0x00111,
						  0x01101,
						  0x00011,
						  0x01100,
						  0x00000,
						  
						  0x00000,	// h
						  0x01000,
						  0x01110,
						  0x01010,
						  0x01010,
						  0x00000,
						  
						  0x00000,	// i
						  0x00100,
						  0x00000,
						  0x00100,
						  0x00100,
						  0x00000,
						  
						  0x00000,	// j
						  0x00100,
						  0x00000,
						  0x00100,
						  0x01100,
						  0x00000,
						  
						  0x00000,	// k
						  0x01000,
						  0x01010,
						  0x01100,
						  0x01010,
						  0x00000,
						  
						  0x00000,	// l
						  0x01000,
						  0x01000,
						  0x01000,
						  0x00110,
						  0x00000,
						  
						  0x00000,	// m
						  0x01001,
						  0x01111,
						  0x01001,
						  0x01001,
						  0x00000,
						  
						  0x00000,	// n
						  0x01011,
						  0x01101,
						  0x01001,
						  0x01001,
						  0x00000,
						  
						  0x00000,	// o
						  0x00100,
						  0x01010,
						  0x01010,
						  0x00100,
						  0x00000,
						  
						  0x00000,
						  0x01100,	// p
						  0x01010,
						  0x01100,
						  0x01000,
						  0x00000,
						  
						  0x00000,	// q
						  0x01110,
						  0x01010,
						  0x01110,
						  0x00010,
						  0x00000,
						  
						  0x00000,	// r
						  0x01010,
						  0x01101,
						  0x01000,
						  0x01000,
						  0x00000,
						  
						  0x00000,	// s
						  0x00110,
						  0x01000,
						  0x00110,
						  0x01100,
						  0x00000,
						  
						  0x00000,	// t
						  0x00100,
						  0x01110,
						  0x00100,
						  0x00110,
						  0x00000,
						  
						  0x00000,	// u
						  0x01010,
						  0x01010,
						  0x01010,
						  0x00110,
						  0x00000,
						  
						  0x00000, 	// v
						  0x01010,
						  0x01010,
						  0x01010,
						  0x00100,
						  0x00000,

						  0x00000,	// w
						  0x01001,
						  0x01001,
						  0x01111,
						  0x01001,
						  0x00000,
						  
						  0x00000,	// x
						  0x01010,
						  0x00100,
						  0x01010,
						  0x01010,
						  0x00000,
						  
						  0x00000, // y
						  0x01010,
						  0x01010,
						  0x00100,
						  0x01100,
						  0x00000,

						  0x00000,	// z
						  0x01110,
						  0x00010,
						  0x01100,
						  0x01110,
						  0x00000,

// number from 0
						  0x00100,
						  0x01010,
						  0x01010,
						  0x01010,
						  0x00100,
						  0x00000,

						  0x00100,
						  0x01100,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00000,

						  0x01100,
						  0x00010,
						  0x01100,
						  0x01000,
						  0x01110,
						  0x00000,
						  
						  0x01100,
						  0x00010,
						  0x01100,
						  0x00010,
						  0x01100,
						  0x00000,

						  0x01000,
						  0x01010,
						  0x01110,
						  0x00010,
						  0x00010,
						  0x00000,
						  
						  0x01110,
						  0x01000,
						  0x01110,
						  0x00010,
						  0x01100,
						  0x00000,

						  0x00110,
						  0x01000,
						  0x01110,
						  0x01010,
						  0x01100,
						  0x00000,

						  0x01110,
						  0x00010,
						  0x00100,
						  0x00100,
						  0x00100,
						  0x00000,

						  0x00100,
						  0x01010,
						  0x00100,
						  0x01010,
						  0x00100,
						  0x00000,

						  0x00100,
						  0x01010,
						  0x00110,
						  0x00010,
						  0x01100,
						  0x00000,
						  
						  0x00000,	// "-"
						  0x00000,
						  0x01111,
						  0x00000,
						  0x00000,
						  0x00000,

						  0x00000,	// "+"
						  0x00100,
						  0x01110,
						  0x00100,
						  0x00000,
						  0x00000,
						  
						  0x00000,	// "="
						  0x01111,
						  0x00000,
						  0x01111,
						  0x00000,
						  0x00000,
						  
						  0x00000,	// "^"
						  0x00100,
						  0x01010,
						  0x00000,
						  0x00000,
						  0x00000,

						  0x01010,	// "#"
						  0x01111,
						  0x01010,
						  0x01111,
						  0x01010,
						  0x00000,

						  0x01000,	// ">"
						  0x00100,
						  0x00010,
						  0x00100,
						  0x01000,
						  0x00000,
						  
						  0x00010,	// "<"
						  0x00100,
						  0x01000,
						  0x00100,
						  0x00010,
						  0x00000,

						  0x00000,	// \xfa
						  0x00000,
						  0x00100,
						  0x00000,
						  0x00000,
						  0x00000,

						  0x00000,	// \xfd
						  0x00100,
						  0x00100,
						  0x01110,
						  0x00100,
						  0x00000,

						  0x00000,	// \xfe
						  0x00100,
						  0x01110,
						  0x00100,
						  0x00100,
						  0x00000,

						  0x00000,	// 239
						  0x00000,
						  0x00000,
						  0x10101,
						  0x10101,
						  0x00000,

						  0x00000,	// 240
						  0x00000,
						  0x00110,
						  0x01111,
						  0x00110,
						  0x00000,

						  0x00000,	// 241
						  0x01111,
						  0x01000,
						  0x01000,
						  0x01111,
						  0x00000,

						  0x00000,	// 242
						  0x11111,
						  0x00000,
						  0x00000,
						  0x11111,
						  0x00000,

						  0x00000,	// 243
						  0x11111,
						  0x00001,
						  0x00001,
						  0x11111,
						  0x00000,

						  0x00000,	// '.'
						  0x00000,
						  0x00000,
						  0x00000,
						  0x01100,
						  0x00000,

						  0x00001,	// '/'
						  0x00010,
						  0x00100,
						  0x01000,
						  0x10000,
						  0x00000,

						  0x10001,	// '%'
						  0x10010,
						  0x00100,
						  0x01001,
						  0x10001,
						  0x00000,

						  0x00000,	// ':'
						  0x01100,
						  0x00000,
						  0x01100,
						  0x00000,
						  0x00000
};

#define CONVERT_TINY_CHAR \
	for (y = 0; y < 6; y++) \
	{ \
		for (x = 0; x < 5; x++) \
		{ \
			if ((tinyCharacters[k]>>((4-x)<<2))&1) \
			{ \
				bitstream->write(i*(5*6)+y*5+x, true); \
			} \
		} \
		k++; \
	} 
	
void genTinyFont()
{
	pp_uint32 x,y,i,j,k;

	Bitstream* bitstream = new Bitstream(tinyFont, (6*5*256)/8);
	bitstream->clear();
	
	k = 0;
	for (i = 'A'; i <= 'Z'; i++)
	{
		CONVERT_TINY_CHAR;
	}
	for (i = 'a'; i <= 'z'; i++)
	{
		CONVERT_TINY_CHAR;
	}
	
	for (i = '0'; i <= '9'; i++)
	{
		CONVERT_TINY_CHAR;
	}
	
	
	i = '-';
	CONVERT_TINY_CHAR;
	
	i = '+';
	CONVERT_TINY_CHAR;
	
	i = '=';
	CONVERT_TINY_CHAR;
	
	i = '^';
	CONVERT_TINY_CHAR;
	
	i = '#';
	CONVERT_TINY_CHAR;
	
	i = '>';
	CONVERT_TINY_CHAR;
	
	i = '<';
	CONVERT_TINY_CHAR;
	
	i = 0xf4;
	CONVERT_TINY_CHAR;
	
	i = 0xfd;
	CONVERT_TINY_CHAR;
	
	i = 0xfe;
	CONVERT_TINY_CHAR;
	
	i = 239;
	CONVERT_TINY_CHAR;				
	i = 240;
	CONVERT_TINY_CHAR;				
	i = 241;
	CONVERT_TINY_CHAR;
	i = 242;
	CONVERT_TINY_CHAR;
	i = 243;
	CONVERT_TINY_CHAR;
	
	i = '.';
	CONVERT_TINY_CHAR;
	
	i = '/';
	CONVERT_TINY_CHAR;

	i = '%';
	CONVERT_TINY_CHAR;

	i = ':';
	CONVERT_TINY_CHAR;

	delete bitstream;
	
	XMFile f("TINY.6x5", true);
	
	f.write(tinyFont, 1, sizeof(tinyFont));
}

void genTinyFontUpperCase()
{
	pp_uint32 x,y,i,j,k;

	Bitstream* bitstream = new Bitstream(tinyFont, (6*5*256)/8);
	bitstream->clear();
	
	k = 0;
	for (i = 'A'; i <= 'Z'; i++)
	{
		CONVERT_TINY_CHAR;
	}
	pp_uint32 l = k;
	k = 0;
	for (i = 'a'; i <= 'z'; i++)
	{
		CONVERT_TINY_CHAR;
	}
	k+=l;
	for (i = '0'; i <= '9'; i++)
	{
		CONVERT_TINY_CHAR;
	}
	
	
	i = '-';
	CONVERT_TINY_CHAR;
	
	i = '+';
	CONVERT_TINY_CHAR;
	
	i = '=';
	CONVERT_TINY_CHAR;
	
	i = '^';
	CONVERT_TINY_CHAR;
	
	i = '#';
	CONVERT_TINY_CHAR;
	
	i = '>';
	CONVERT_TINY_CHAR;
	
	i = '<';
	CONVERT_TINY_CHAR;
	
	i = 0xf4;
	CONVERT_TINY_CHAR;
	
	i = 0xfd;
	CONVERT_TINY_CHAR;
	
	i = 0xfe;
	CONVERT_TINY_CHAR;
	
	i = 239;
	CONVERT_TINY_CHAR;				
	i = 240;
	CONVERT_TINY_CHAR;					
	i = 241;
	CONVERT_TINY_CHAR;
	i = 242;
	CONVERT_TINY_CHAR;
	i = 243;
	CONVERT_TINY_CHAR;
	
	i = '.';
	CONVERT_TINY_CHAR;
	
	i = '/';
	CONVERT_TINY_CHAR;

	i = '%';
	CONVERT_TINY_CHAR;

	i = ':';
	CONVERT_TINY_CHAR;

	delete bitstream;
	
	XMFile f("TINY_UPPERCASE.6x5", true);
	
	f.write(tinyFont, 1, sizeof(tinyFont));
}

int main()
{
	genTinyFont();
	genTinyFontUpperCase();
	return 0;
}
