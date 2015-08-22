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
 *  LoaderDBM.cpp
 *  MilkyPlay Module Loader: Digibooster Pro
 *
 *  Created by Peter Barth on 06.03.06.
 *
 */

#include "Loaders.h"

// to-do: improve detection
const char* LoaderDBM::identifyModule(const mp_ubyte* buffer)
{
	// check for .DTM module
	if (memcmp(buffer,"DBM0",4)) 
		return NULL;
		
	return "DBM";
}

#define CLEAN_DBM \
	{ \
		mp_uint32 i; \
		delete[] srcIns; \
		for (i = 0; i < header->smpnum; i++) \
			delete[] srcSmp[i].sample; \
		delete[] srcSmp; \
		delete[] orderListLengths; \
		for (i = 0; i < numSubSongs; i++) \
			delete[] orderLists[i]; \
		for (i = 0; i < header->patnum; i++) \
			delete[] patterns[i]; \
		delete[] orderLists; \
		delete[] patterns; \
	}


struct DBMInstrument
{
	mp_ubyte	name[30];
	mp_uword	sampnum;
	mp_uword	volume;
	mp_uint32	finetune;
	mp_uint32	repstart;
	mp_uint32	replen;
	mp_uword	panning;
	mp_uword	flags;
	// added by ME
	mp_sint32	venvnum;
	mp_sint32	penvnum;
};

struct DBMSample
{
	mp_uint32	flags;
	mp_uint32	samplen; // bit 0 set - 8 bit sample
                         // bit 1 set - 16 bit sample
                         // bit 2 set - 32 bit sample
	mp_ubyte*	sample;
};

static void convertDBMffects(mp_ubyte& effect, mp_ubyte& operand)
{
	//static const char eff[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

	switch (effect)
	{
		case 0x00:
			if (operand)
				effect = 0x20;
			break;
			
		case 0x01:
			if ((operand & 0xF0) == 0xF0)
			{
				// doc says: 1Fx == fine portamento up
				effect = 0x31;
				operand &= 0x0F;
			}
			break;
		case 0x02:
			if ((operand & 0xF0) == 0xF0)
			{
				// doc says: 2Fx == fine portamento down
				effect = 0x32;
				operand &= 0x0F;
			}
			break;

		case 0x03:
		case 0x04:
			break;

		case 0x05:
			if ((operand & 0x0F) == 0x0F)
			{
				// doc says: 5xF == Tone portamento+Fine Volume slide up
				// !! unsupported !!
				// Might be possible with MDL way of combining portamento + volume slide
				// but with S3M volslide instead of MDL volslide
				goto missing;
			}
			else if ((operand & 0xF0) == 0xF0)
			{
				// doc says: 5Fx == Tone portamento+Fine Volume slide down
				// !! unsupported !!
				// Might be possible with MDL way of combining portamento + volume slide
				// but with S3M volslide instead of MDL volslide
				goto missing;
			}
			break;

		case 0x06:
			if ((operand & 0x0F) == 0x0F)
			{
				// doc says: 6xF == Vibrato+Fine Volume slide up
				// !! unsupported !!
				// Might be possible with MDL way of combining vibrato + volume slide
				// but with S3M volslide instead of MDL volslide
				goto missing;
			}
			else if ((operand & 0xF0) == 0xF0)
			{
				// doc says: 6Fx == Vibrato+Fine Volume slide down
				// !! unsupported !!
				// Might be possible with MDL way of combining vibrato + volume slide
				// but with S3M volslide instead of MDL volslide
				goto missing;
			}
			break;

		// Tremolo doesn't exist, huh?
		case 0x07:
		// Panning
		case 0x08:
		// Sample offset
		case 0x09:
			break;

		// Volslide (S3M volslide fits best probably)
		case 0x0A:
			effect = 0x49;
			break;

		// Position jump
		case 0x0B:
			break;

		// set volume
		case 0x0C:
		// set global volume
		case 0x10:
			operand = XModule::vol64to255(operand);
			break;

		// Pattern break
		case 0x0D:
			break;

		// subcommands
		case 0x0E:
		{
			effect = 0x30 + (operand >> 4);
			operand &= 0xF;
			switch (effect)
			{
				// Set filter: unsupported
				case 0x30:
					goto missing;
					break;
				// Play backwards
				case 0x33:
					// AMS can do that
					effect = 0x4F;
					operand = 1;
					break;
				// Turn off sound in channel?
				// can probably be emulated with AMS set channel volume
				// right now unsupported
				case 0x34:
					goto missing;
					break;
				// Turn on/off sound in channel?
				// can probably be emulated with AMS set channel volume
				// right now unsupported
				case 0x35:
					goto missing;
					break;
				// set loop/loop
				case 0x36:
					goto missing;
					break;
				// set offset for loop or what?
				case 0x37:
					goto missing;
					break;
			}
			break;
		}

		// Pattern break
		case 0x0F:
			break;

		case 0x11:
			break;

		// key off
		case 0x14:
			if (operand == 0)
				// AMS key of at tick
				effect = 0x51;
			break;
		
		// Set envelope position
		case 0x15:
			break;

		// Sample offset slide (unsupported)
		case 0x18:
			goto missing;
			break;
		
		// set real BPM
		case 0x1C:
			effect = 0x52;
			break;
			
		// echo effects (unsupported)
		case 0x1f:
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
			goto missing;
			break;
			
		default:
#ifdef VERBOSE
			printf("Unknown DBM effect %x with operand %x\n", effect, operand);
#endif
missing:
#ifdef VERBOSE
			printf("Missing DBM effect %x with operand %x\n", effect, operand);
#endif
			effect = operand = 0;

	}
	
}

static void convertDBMEnvelope(TEnvelope& outEnv, const mp_ubyte* inEnv)
{
	memset(&outEnv, 0, sizeof(TEnvelope));
	outEnv.type = *inEnv & 7;
	
#ifdef VERBOSE
	if (*inEnv >> 3)
	{
		printf("Second sustain point used");

	}
#endif
	
	// one single point?
	// disable envelope
	if (!*(inEnv+1))
	{
		outEnv.type &= ~1;
		return;
	}
	
	// DBM stores numpoints-1
	outEnv.num = *(inEnv+1) + 1;
	outEnv.sustain = *(inEnv+2);
	outEnv.loops = *(inEnv+3);
	outEnv.loope = *(inEnv+4);
 
	inEnv+=6;

	for (mp_sint32 i = 0; i < outEnv.num; i++)
	{
		outEnv.env[i][0] = BigEndian::GET_WORD(inEnv);
		outEnv.env[i][1] = BigEndian::GET_WORD(inEnv+2) << 2;
		inEnv+=4;
	}
}

mp_sint32 LoaderDBM::load(XMFileBase& f, XModule* module)
{
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	

	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	mp_sint32 i,j;

	mp_uword numSubSongs		= 0;
	mp_uword** orderLists		= NULL;
	mp_uword* orderListLengths	= NULL;
	DBMInstrument* srcIns		= NULL;
	mp_ubyte** patterns			= NULL;
	DBMSample* srcSmp			= NULL;

	while (true)
	{		
		mp_ubyte ID[4], buffer[4];
		
		mp_uint32 bytesRead = f.read(ID, 4, 1);

		if (bytesRead != 4)
			break;	

		bytesRead = f.read(buffer, 4, 1);

		if (bytesRead != 4)
			break;	

		mp_uint32 chunkLen = BigEndian::GET_DWORD(buffer);
			
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x44424D30:	// 'DBM0'
				memcpy(header->sig, ID, 4);
				break;
			
			case 0x4E414D45:	// 'NAME'
			{
				mp_ubyte* name = new mp_ubyte[chunkLen];
				f.read(name, 1, chunkLen);
				memcpy(header->name, name, chunkLen > MP_MAXTEXT ? MP_MAXTEXT : chunkLen);
				delete[] name;
				break;
			}

			case 0x494E464F:	// 'INFO'
				f.read(buffer, 1, 2);
				header->insnum = BigEndian::GET_WORD(buffer);
				f.read(buffer, 1, 2);
				header->smpnum = BigEndian::GET_WORD(buffer);

				srcSmp = new DBMSample[header->smpnum];

				f.read(buffer, 1, 2);
				numSubSongs = BigEndian::GET_WORD(buffer);

				// Allocate order list table
				orderLists = new mp_uword*[numSubSongs];
				orderListLengths = new mp_uword[numSubSongs];

				f.read(buffer, 1, 2);
				header->patnum = BigEndian::GET_WORD(buffer);
				
				patterns = new mp_ubyte*[header->patnum];
				
				f.read(buffer, 1, 2);
				header->channum = BigEndian::GET_WORD(buffer);
				break;

			case 0x534F4E47:	// 'SONG'
			{
				// Skip name
				for (i = 0; i < numSubSongs; i++)
				{
					mp_ubyte name[44];
					f.read(name, 1, 44);

					f.read(buffer, 1, 2);
					orderListLengths[i] = BigEndian::GET_WORD(buffer);
					orderLists[i] = new mp_uword[BigEndian::GET_WORD(buffer)];

					for (j = 0; j < orderListLengths[i]; j++)
					{
						f.read(buffer, 1, 2);
						orderLists[i][j] = BigEndian::GET_WORD(buffer);
					}
				}
				break;
			}

			case 0x494E5354:	// 'INST'
			{
				mp_uword insNum = chunkLen / 50;
				
				srcIns = new DBMInstrument[insNum];

				for (i = 0; i < insNum; i++)
				{
					f.read(srcIns[i].name, 1, 30);
					f.read(buffer, 2, 1);
					srcIns[i].sampnum = BigEndian::GET_WORD(buffer);
					f.read(buffer, 2, 1);
					srcIns[i].volume = BigEndian::GET_WORD(buffer);
					f.read(buffer, 4, 1);
					srcIns[i].finetune = BigEndian::GET_DWORD(buffer);
					f.read(buffer, 4, 1);
					srcIns[i].repstart = BigEndian::GET_DWORD(buffer);
					f.read(buffer, 4, 1);
					srcIns[i].replen = BigEndian::GET_DWORD(buffer);
					f.read(buffer, 2, 1);
					srcIns[i].panning = BigEndian::GET_WORD(buffer);
					f.read(buffer, 2, 1);
					srcIns[i].flags = BigEndian::GET_WORD(buffer);
				
					srcIns[i].venvnum = srcIns[i].penvnum = -1;
				}
				break;
			}

			case 0x50415454:	// 'PATT'
			{
				for (i = 0; i < header->patnum; i++)
				{
					f.read(buffer, 2, 1);
					phead[i].rows = BigEndian::GET_WORD(buffer);
					f.read(buffer, 4, 1);
					phead[i].len = BigEndian::GET_DWORD(buffer);

					patterns[i] = new mp_ubyte[phead[i].len];
					f.read(patterns[i], 1, phead[i].len);
				}
				break;
			}

			case 0x534D504C:	// 'SMPL'
			{
				for (i = 0; i < header->smpnum; i++)
				{
					f.read(buffer, 4, 1);
					srcSmp[i].flags = BigEndian::GET_DWORD(buffer);
					f.read(buffer, 4, 1);
					srcSmp[i].samplen = BigEndian::GET_DWORD(buffer);
					
					if (srcSmp[i].flags == 1)
					{
						srcSmp[i].sample = new mp_ubyte[srcSmp[i].samplen];
						module->loadSample(f, srcSmp[i].sample, srcSmp[i].samplen, srcSmp[i].samplen);
					}
					else if (srcSmp[i].flags == 2)
					{
						srcSmp[i].sample = new mp_ubyte[srcSmp[i].samplen*2];
						module->loadSample(f, srcSmp[i].sample, srcSmp[i].samplen*2, srcSmp[i].samplen, XModule::ST_16BIT | XModule::ST_BIGENDIAN);
					}
					else
					{
#ifdef VERBOSE
						printf("Unsupported sample type");
#endif
					}
				}
				break;
			}

			case 0x56454E56:	// 'VENV'
			{
				f.read(buffer, 2, 1);
				mp_uword numEnvelopes = BigEndian::GET_WORD(buffer);
				for (i = 0; i < numEnvelopes; i++)
				{
					f.read(buffer, 2, 1);
					mp_uword index = BigEndian::GET_WORD(buffer);
					
					if (index)
						srcIns[index-1].venvnum = module->numVEnvs;

					mp_ubyte env[134];
					f.read(env, 1, 134);

					TEnvelope venv;
					convertDBMEnvelope(venv, env);
					
					if (!module->addVolumeEnvelope(venv)) 
					{
						CLEAN_DBM;
						return MP_OUT_OF_MEMORY;
					}
				}				
				break;
			}

			case 0x50454E56:	// 'PENV'
			{
				f.read(buffer, 2, 1);
				mp_uword numEnvelopes = BigEndian::GET_WORD(buffer);
				for (i = 0; i < numEnvelopes; i++)
				{
					f.read(buffer, 2, 1);
					mp_uword index = BigEndian::GET_WORD(buffer);
					
					if (index)
						srcIns[index-1].penvnum = module->numPEnvs;

					mp_ubyte env[134];
					f.read(env, 1, 134);

					TEnvelope penv;
					convertDBMEnvelope(penv, env);
					
					if (!module->addPanningEnvelope(penv)) 
					{
						CLEAN_DBM;
						return MP_OUT_OF_MEMORY;
					}
				}				
				break;
			}

			default:
				f.seekWithBaseOffset(f.posWithBaseOffset() + chunkLen);

		}
	}

	if (!(orderListLengths && srcIns && patterns && srcSmp))
		return MP_LOADER_FAILED;

	// Convert orderlist, subsongs are not supported yet
	j = 0;
	for (i = 0; i < orderListLengths[j]; i++)
	{
		if (i < 256)
			header->ord[i] = (mp_ubyte)orderLists[j][i];
	}

	header->ordnum = orderListLengths[j];

	header->mainvol = 255;
	header->speed = 125;
	header->tempo = 6;
	header->flags = XModule::MODULE_OLDPTINSTRUMENTCHANGE | XModule::MODULE_PTNEWINSTRUMENT;
	header->volenvnum = module->numVEnvs;
	header->panenvnum = module->numPEnvs;

	// Convert patterns
	for (i = 0; i < header->patnum; i++)
	{
		phead[i].effnum = 2;
		phead[i].channum = (mp_ubyte)header->channum;
		
		const mp_sint32 bps = phead[i].effnum*2+2;

		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*bps];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			CLEAN_DBM;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData, 0, phead[i].rows*header->channum*bps);
	
		mp_uint32 currentRow = 0;

		j = 0;
		
		mp_ubyte* src = patterns[i];
		
		while (j < (signed)phead[i].len && currentRow < phead[i].rows)
		{
			mp_ubyte pack = src[j];
			j++;
			if (!pack)
				currentRow++;
			else
			{
				mp_ubyte note = 0, ins = 0;
				mp_ubyte eff1 = 0, op1 = 0, eff2 = 0, op2 = 0;

				mp_ubyte channel = pack-1;

				pack = src[j];
				j++;
				
				// Note present?
				if (pack & 0x01)
				{
					if (src[j] == 0x1F) 
						note = XModule::NOTE_OFF;
					else
						note = (src[j] >> 4) * 12 + ((src[j] & 0xF)+1);
					j++;
				}
				// Instrument present?
				if (pack & 0x02)
				{
					ins = src[j];
					j++;
				}
				// Effect 1 present?
				if (pack & 0x04)
				{
					eff1 = src[j];
					j++;
				}
				// Operand 1 present?
				if (pack & 0x08)
				{
					op1 = src[j];
					j++;
				}
				// Effect 2 present?
				if (pack & 0x10)
				{
					eff2 = src[j];
					j++;
				}
				// Operand 1 present?
				if (pack & 0x20)
				{
					op2 = src[j];
					j++;
				}

				convertDBMffects(eff1, op1);
				convertDBMffects(eff2, op2);

				mp_ubyte* dstSlot = phead[i].patternData + 
									currentRow * header->channum*bps + channel*bps;	

				*dstSlot++ = note;
				*dstSlot++ = ins;
				*dstSlot++ = eff1;
				*dstSlot++ = op1;
				*dstSlot++ = eff2;
				*dstSlot++ = op2;
			}
		}
	
	}

	// convert instrument data
	for (i = 0; i < header->insnum; i++)
	{
		memcpy(instr[i].name, srcIns[i].name, MP_MAXTEXT);
		for (j = 0; j < 120; j++)
			instr[i].snum[j] = i;

		j = srcIns[i].sampnum;

		if (j && j <= header->smpnum)
		{
			j--;
			if (srcSmp[j].samplen)
			{
				instr[i].samp = 1;

				smp[i].flags = 1 | (srcIns[i].panning ? 2 : 0);
				smp[i].pan = (srcIns[i].panning ? srcIns[i].panning - 1 : 0x80);
				smp[i].vol = XModule::vol64to255(srcIns[i].volume);

				smp[i].samplen = srcSmp[j].samplen;

				if (srcSmp[j].flags == 1)
				{
					smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen);
					memcpy(smp[i].sample, srcSmp[j].sample, smp[i].samplen);
				}
				else if (srcSmp[j].flags == 2)
				{
					smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen*2);
					memcpy(smp[i].sample, srcSmp[j].sample, smp[i].samplen*2);
					smp[i].type |= 16;
				}

				XModule::convertc4spd(srcIns[i].finetune, &smp[i].finetune, &smp[i].relnote);

				smp[i].loopstart = srcIns[i].repstart;
				smp[i].looplen = srcIns[i].replen;

				if (((srcIns[i].flags & 3) == 1) && smp[i].looplen)
					smp[i].type |= 1;
				else if (((srcIns[i].flags & 2) == 2) && smp[i].looplen)
					smp[i].type |= 2;

				if (srcIns[i].venvnum >= 0)
					smp[i].venvnum = srcIns[i].venvnum+1;
				if (srcIns[i].penvnum >= 0)
					smp[i].penvnum = srcIns[i].penvnum+1;
			}
		}
	}

	CLEAN_DBM;

	header->smpnum = header->insnum;

	strcpy(header->tracker,"Digibooster Pro");

	module->setDefaultPanning();
	
	module->postProcessSamples();	

	return MP_OK;
}
