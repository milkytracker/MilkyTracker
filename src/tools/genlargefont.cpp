/*
 *  tools/genlargefont.cpp
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

#define NUMCHARS	100
#define CHARWIDTH	12
#define CHARHEIGHT	12
#define CHARSIZE	(CHARWIDTH*CHARHEIGHT)	

static pp_uint8 dstFont[CHARSIZE*256/8];
static pp_uint8 src[CHARSIZE*NUMCHARS];

#define CONVERT_LARGE_CHAR \
	for (y = 0; y < CHARHEIGHT; y++) \
	{ \
		for (x = 0; x < CHARWIDTH; x++) \
		{ \
			if (src[k*CHARSIZE+y*CHARWIDTH+x]) \
			{ \
				bitstream->write(i*CHARSIZE+y*CHARWIDTH+x, true); \
			} \
		} \
	}

int main()
{
	pp_uint32 x,y,i,j,k;

	XMFile f("athena_ft2_style_12x12.raw");
	f.read(src, CHARSIZE, NUMCHARS);

	Bitstream* bitstream = new Bitstream(dstFont, (CHARSIZE*256)/8);
	bitstream->clear();
	
	for (i = ' '; i <= '}'; i++)
	{
		k = i - ' ';
		
		printf("%i\n",k);
		
		CONVERT_LARGE_CHAR
		
 	  	for (y = 0; y < CHARHEIGHT; y++)
 	  	{
 	    	for (x = 0; x < CHARWIDTH; x++) 
 	    	{
				if (bitstream->read(i*CHARSIZE+y*CHARWIDTH+x)) 
					printf("1");
				else
					printf("0");
			}
			
			printf("\n");
		}
		
	}
	
	k++;
	i = 254;
	CONVERT_LARGE_CHAR

	k++;
	i = 253;
	CONVERT_LARGE_CHAR

	k++;
	i = 0xf1;
	CONVERT_LARGE_CHAR

	k++;
	i = 0xf2;
	CONVERT_LARGE_CHAR

	k++;
	i = 0xf3;
	CONVERT_LARGE_CHAR

	k++;
	i = 0xc4;
	CONVERT_LARGE_CHAR

	XMFile f2("ATHENA.12X12", true);
	
	f2.write(dstFont, 1, sizeof(dstFont));
	return 0;
}
