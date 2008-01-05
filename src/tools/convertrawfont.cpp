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

#define NUMCHARS	99
#define CHARWIDTH	8
#define CHARHEIGHT	8
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
				bitstream->write(i*CHARSIZE+y*CHARWIDTH+(CHARWIDTH-1-x), true); \
			} \
		} \
	}

void convert(const char* infile, const char* outfile)
{
	pp_uint32 x,y,i,j,k;

	XMFile f(infile);
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

	// replace space and put in horizontal line
	for (x = 0; x < CHARWIDTH; x++)
	{
		src[(CHARHEIGHT/2-1)*CHARWIDTH+x] = 0xFF;
		src[(CHARHEIGHT/2)*CHARWIDTH+x] = 0xFF;
	}
	k = ' ';
	i = 0xc4;
	CONVERT_LARGE_CHAR

	XMFile f2(outfile, true);
	
	f2.write(dstFont, 1, sizeof(dstFont));
}

int main()
{
	convert("IDC-Harmonica.raw", "IDC-Harmonica.8X8");
	convert("IDC-Hoodlum.raw", "IDC-Hoodlum.8X8");
	convert("IDC-MicroKnight.raw", "IDC-MicroKnight.8X8");
	convert("REZ-ASCII.raw", "REZ-ASCII.8X8");

	return 0;
}
