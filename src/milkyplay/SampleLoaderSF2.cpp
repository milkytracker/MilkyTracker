/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
sf2towav - Extract WAV files from Soundfont 2 (SF2) Files
Copyright (C) 1998 - 2020 Jeremy Smith
-----
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
-----
INFORMATION
  www: https://github.com/jeremylsmith/sf2towav/
  e-mail: jeremy@decompiler.org
*/


/*
 *  SampleLoaderSF2.cpp
 *  MilkyPlay
 *
 *  Created by Coder of Salvation / Leon van Kammen on 02-03-2025
 *
 *  NOTE: https://github.com/jeremylsmith/sf2towav is a lightweight alternative 
 *        to TinySoundFont or other bloated SF2 parsers (we only need samples since 
 *        SF2-to-XI translation would be underwhelming given the lack of 
 *        velocity-maps and various other SF2-features).
 */

#include "SampleLoaderSF2.h"
#include "XMFile.h"
#include "XModule.h"
#include "LittleEndian.h"
#include <cstdint>

char SampleLoaderSF2::lastSF2File[255];
int  SampleLoaderSF2::lastSF2FileSamples;

SampleLoaderSF2::SampleLoaderSF2(const SYSCHAR* fileName, XModule& theModule) :
	SampleLoaderAbstract(fileName, theModule)
{
}

SampleLoaderSF2::~SampleLoaderSF2() { 
}

bool SampleLoaderSF2::identifySample()
{
  // tracker/BasicTypes.h has PPString with compareTo(..) but milkyplay has not :/
  //return (strstr(theFileName, ".SF2")!= NULL) || (strstr(theFileName,".sf2") != NULL);
  unsigned long longnumber;
  unsigned long RIFFlen, WAVElen, SMALLWAVElen, DATACHUNKlen, PATCHCHUNKlen;
  unsigned long WAVEfilepos;
  unsigned long allwavsn, allwavsc;
  unsigned long waveoutcount, fileoutcount, waveloopcount, patchcount;
  unsigned long tempfile;
  unsigned long wavstart, wavend, wavsloop, waveloop, wavsamprate;

  int onebyte;
  int nextbyte;

  inp = fopen (theFileName, "rb");

  if (!inp)
    return 1;

  /*Check for RIFF */
  getitem ();

  if (strcmp (item, "RIFF") != 0)
    return false;
  RIFFlen = bit32convl (inp);

  /*Check this is a Soundfont */
  getitem ();
  if (strcmp (item, "sfbk") != 0)
    return false;

  getitem ();
  if (strcmp (item, "LIST") != 0)
    return false;

  fclose (inp);

  return true;
}
	
mp_sint32 SampleLoaderSF2::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	return loadSampleIndex(index,channelIndex,0);
}

mp_sint32 SampleLoaderSF2::loadSampleIndex(mp_sint32 index, mp_sint32 channelIndex, int sampleIndex)
{

	TXMSample* smp = &theModule.smp[index];
	return loadSampleIndexToSample( smp, channelIndex, sampleIndex );
}

mp_sint32 SampleLoaderSF2::loadSampleIndexToSample( TXMSample *smp, mp_sint32 channelIndex, int sampleIndex)
{
	unsigned long longnumber;
	unsigned long RIFFlen, WAVElen, SMALLWAVElen, DATACHUNKlen, PATCHCHUNKlen;
	unsigned long WAVEfilepos;
	unsigned long allwavsn, allwavsc;
	unsigned long waveoutcount, fileoutcount, waveloopcount, patchcount;
	unsigned long tempfile;
	unsigned long wavstart, wavend, wavsloop, waveloop, wavsamprate, originalPitch, pitchCorrection, sampleLink, sampleType;

	int onebyte;
	int nextbyte;

	inp = fopen (theFileName, "rb");

	if (!inp)
	{
		printf ("Cannot open input Soundfont file\n");
		return 1;
	}

	/*Check for RIFF*/
	getitem ();

	if (strcmp (item, "RIFF") != 0)
		return 1;
	RIFFlen = bit32convl (inp);

	/*Check this is a Soundfont*/
	getitem ();
	if (strcmp (item, "sfbk") != 0)
		return 1;

	/*Check this is a LIST*/
	getitem ();
	if (strcmp (item, "LIST") != 0)
		return 1;

	DATACHUNKlen = bit32convl (inp);
	fseek (inp, ftell (inp) + DATACHUNKlen, SEEK_SET);
	getitem ();
	if (strcmp (item, "LIST") != 0)
		return 1;
	WAVElen = bit32convl (inp);

	WAVEfilepos = ftell (inp);
	fseek (inp, ftell (inp) + WAVElen, SEEK_SET);
	getitem ();

	PATCHCHUNKlen = bit32convl (inp);

	getitem ();
	if (strcmp (item, "pdta") != 0) return 1;
	patchcount = 0;
	int sampleHeader = 0;
	while (patchcount < PATCHCHUNKlen)
	{
		getitem ();
		longnumber = bit32convl (inp);
		if (strcmp (item, "shdr") != 0)
		{
			fseek (inp, ftell (inp) + longnumber, SEEK_SET);
			patchcount += longnumber;
			continue;
		};
		allwavsn = longnumber;
		longnumber = ftell (inp);
		allwavsc = 0;
		while (allwavsc * 2 < allwavsn)
		{
			getstring ();

			if( string[0] == 'E' && string[1] == 'O' && string[2] == 'S' ) break; // EOS
			wavstart = bit32convl (inp);
			wavend = bit32convl (inp);
			wavsloop = bit32convl (inp);
			waveloop = bit32convl (inp);
			wavsamprate = bit32convl (inp);

			originalPitch = getc(inp);
			pitchCorrection = getc(inp);
			sampleLink = (getc(inp) << 8) | getc(inp);
			sampleType = (getc(inp) << 8) | getc(inp);
			tempfile = ftell(inp); // store current file-offset so the while-loop can continue properly (wavdata while-loop below will overwrite it)

			if( sampleIndex == sampleHeader ){
				/*
				printf("sampleIndex = %i\n",sampleIndex);
				printf ("File: %s\n", string);
				printf("wavstart %ld\n",wavstart);
				printf("wavend %ld\n",wavend);
				printf("wavsloop %ld\n",wavsloop);
				printf("waveloop %ld\n",waveloop);
				printf("wavsamprate %ld\n",wavsamprate);
				*/

				if (smp->sample)
				{
					theModule.freeSampleMem((mp_ubyte*)smp->sample);
					smp->sample = NULL;
				}					

				smp->vol = 255;
				smp->pan = 0x80;
				smp->flags = 3;
				smp->type |= 16;		
				smp->relnote = smp->relnote == 0 ? 12 : smp->relnote;
				smp->loopstart = wavsloop-wavstart;
				smp->looplen  = waveloop - wavsloop;

				// loop flag is defined per-instrument, so we do some poor man's guessing here for now
				// based on heuristics from the spec: 
				//   * dwStart must be less than dwStartloop-7, 
				//   * dwStartloop must be less than dwEndloop-31
				//   & dwEndloop must be less than dwEnd-7
				bool isLoop = wavstart < wavsloop-7 && wavsloop < waveloop-31 && waveloop < wavend;
				if( isLoop ){ // enable forward loop
					smp->type &= ~(3+32);
					smp->type |= 1;
				}

				memset(smp->name, 0, sizeof(smp->name));
				nameToSample(string,smp);

				smp->samplen = (wavend - wavstart);
				smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						

				int bitDepth = 16;
				if (sampleType & 0x08 ) bitDepth = 24;  // SF2.4 introduced 24-bit samples
				if (sampleType & 0x10 ) bitDepth = 32;  // Rare case

				fseek(inp, WAVEfilepos + (wavstart * 2) + 12L, SEEK_SET);
				waveoutcount = 0;

				void* dataSmpl = NULL;
				switch (bitDepth) {
					case 16:
						dataSmpl = new int16_t[smp->samplen];
						memset(dataSmpl, 0, smp->samplen*sizeof(int16_t)); // Use int32_t to avoid warnings
						break;
					case 24:
					case 32:
						dataSmpl = new int32_t[smp->samplen];
						memset(dataSmpl, 0, smp->samplen*sizeof(int32_t)); // Use int32_t to avoid warnings
						break;
					default:
						printf("Unsupported bit depth: %d\n", bitDepth);
						return MP_LOADER_FAILED;
				}


				while (waveoutcount < wavend - wavstart)
				{
					int32_t sample = 0;
					switch (bitDepth) {
						case 16:
							sample = (getc(inp) << 8) | getc(inp);
							break;
						case 24:
							sample = (getc(inp) << 16) | (getc(inp) << 8) | getc(inp);
							break;
						case 32:
							sample = (getc(inp) << 24) | (getc(inp) << 16) | (getc(inp) << 8) | getc(inp);
							break;
					}
					switch (bitDepth) {
						case 16:
							((int16_t*)dataSmpl)[waveoutcount] = sample;
							break;
						case 24:
						case 32:
							((int32_t*)dataSmpl)[waveoutcount] = sample;
							break;
					}
					waveoutcount++;
				};

				for (int i = 0; i < smp->samplen; i++) {
					switch (bitDepth) {
						case 16:
							((mp_sbyte*)smp->sample)[i*2] = ((int16_t*)dataSmpl)[i] >> 8;
							((mp_sbyte*)smp->sample)[i*2+1] = ((int16_t*)dataSmpl)[i] & 0xFF;
							break;
						case 24:
							((mp_sbyte*)smp->sample)[i*3] = ((int32_t*)dataSmpl)[i] >> 16;
							((mp_sbyte*)smp->sample)[i*3+1] = ((int32_t*)dataSmpl)[i] >> 8;
							((mp_sbyte*)smp->sample)[i*3+2] = ((int32_t*)dataSmpl)[i] & 0xFF;
							break;
						case 32:
							((mp_sbyte*)smp->sample)[i*4] = ((int32_t*)dataSmpl)[i] >> 24;
							((mp_sbyte*)smp->sample)[i*4+1] = ((int32_t*)dataSmpl)[i] >> 16; 
							((mp_sbyte*)smp->sample)[i*4+2] = ((int32_t*)dataSmpl)[i] >> 8;
							((mp_sbyte*)smp->sample)[i*4+3] = ((int32_t*)dataSmpl)[i] & 0xFF;
							break;
					}
				}

				delete[] dataSmpl; // Don't forget to free the memory

			}

			fseek (inp, tempfile, SEEK_SET);
			allwavsc += 26;
			sampleHeader++;
		};
	};
	if (inp) fclose (inp);

	lastSF2FileSamples = sampleHeader;

	if( strlen(theFileName) < 254 ){
		strncpy(lastSF2File, theFileName, strlen(theFileName) );
		lastSF2File[ strlen(theFileName)+1 ] = '\0';
	}

	return MP_OK;
}

void SampleLoaderSF2::getstring ()
{
	int thisbyte, stringcount;
	stringcount = 0;
	thisbyte = -1;
	while((thisbyte !=0) && (stringcount<20))
	{
		thisbyte = getc (inp);
		string[stringcount] = thisbyte;
		stringcount++;
	};
	string[stringcount] = '\0';
	while (stringcount < 20)
	{
		getc (inp);
		stringcount++;
	};
};

unsigned long SampleLoaderSF2::bit32conv ()
{
	unsigned long temp;
	int inputchar;

	inputchar = getc (inp);
	temp = (unsigned long) inputchar *16777216;
	inputchar = getc (inp);
	temp += (unsigned long) inputchar *65536;
	inputchar = getc (inp);
	temp += (unsigned long) inputchar *256;
	inputchar = getc (inp);
	temp += (unsigned long) inputchar;
	return (temp);
}

void SampleLoaderSF2::put32 (unsigned long d)
{
	unsigned char *ptr = (unsigned char *) &d;

	putc (ptr[0], outp);
	putc (ptr[1], outp);
	putc (ptr[2], outp);
	putc (ptr[3], outp);
};

unsigned long SampleLoaderSF2::bit32convl (FILE * inp)
{
	unsigned long d;
	unsigned char *ptr = (unsigned char *) &d;
	ptr[0] = getc (inp);
	ptr[1] = getc (inp);
	ptr[2] = getc (inp);
	ptr[3] = getc (inp);
	return (d);
};

void SampleLoaderSF2::getitem ()
{
  item[0] = getc (inp);
  item[1] = getc (inp);
  item[2] = getc (inp);
  item[3] = getc (inp);
  item[4] = '\0';
};
