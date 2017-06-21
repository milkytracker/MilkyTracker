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
 *  XIInstrument.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 07.03.05.
 *
 */

#include "XIInstrument.h"

XIInstrument::XIInstrument() :
	owner(false)
{
	memset(&samples, 0, sizeof(samples));

	memset(sig, 0, sizeof(sig));
	memset(name, 0, sizeof(name));
	memset(tracker, 0, sizeof(tracker));
	memset(nbu, 0, sizeof(nbu));
	memset(&venv, 0, sizeof(venv));
	memset(&penv, 0, sizeof(penv));

	vibtype = vibsweep = vibdepth = vibrate = 0;
	volfade = 0;
	
	res = 0;
	
	memset(extra, 0, sizeof(extra));
	
	numsamples = 0;
}

XIInstrument::XIInstrument(const XIInstrument& source) :
	owner(true)
{
	memset(&samples, 0, sizeof(samples));

	memcpy(sig, source.sig, sizeof(sig));
	memcpy(name, source.name, sizeof(name));
	memcpy(tracker, source.tracker, sizeof(tracker));
	memcpy(nbu, source.nbu, sizeof(nbu));
	
	venv = source.venv;
	penv = source.penv;
	
	vibtype = source.vibtype;
	vibsweep = source.vibsweep;
	vibdepth = source.vibdepth;
	vibrate = source.vibrate;
	
	volfade = source.volfade;
	
	res = source.res;

	memcpy(extra, source.extra, sizeof(extra));	

	numsamples = source.numsamples;

	for (mp_sint32 i = 0; i < numsamples; i++)
	{
		const TXMSample* src = &source.samples[i];
		TXMSample* dst = &samples[i];
		
		dst->samplen = src->samplen;
		dst->loopstart = src->loopstart;
		dst->looplen = src->looplen;
		dst->flags = src->flags;
		dst->vol = src->vol;
		dst->finetune = src->finetune;
		dst->type = src->type;
		dst->pan = src->pan;
		dst->relnote = src->relnote;
		dst->venvnum = src->venvnum;
		dst->penvnum = src->penvnum;
		dst->fenvnum = src->fenvnum;
		dst->vibenvnum = src->vibenvnum;
		dst->vibtype = src->vibtype;
		dst->vibsweep = src->vibsweep;
		dst->vibdepth = src->vibdepth;
		dst->vibrate = src->vibrate;
		dst->volfade = src->volfade;
		dst->res = src->res;
		
		memcpy(dst->name, src->name, sizeof(src->name));
		
		dst->terminate = src->terminate;
		
		if (src->sample)
		{
			if (src->type & 16)
			{
				dst->sample = (mp_sbyte*)(TXMSample::allocPaddedMem(src->samplen*2));
				TXMSample::copyPaddedMem(dst->sample, src->sample, src->samplen*2);
			}
			else
			{
				dst->sample = (mp_sbyte*)(TXMSample::allocPaddedMem(src->samplen));
				TXMSample::copyPaddedMem(dst->sample, src->sample, src->samplen);
			}
		}
	}

}

XIInstrument::~XIInstrument()
{
	mp_sint32 numSamples = sizeof(samples)/sizeof(TXMSample);

	if (owner)
	{
		for (mp_sint32 i = 0; i < numSamples; i++)
			TXMSample::freePaddedMem((mp_ubyte*)samples[i].sample);
	}
}

void XIInstrument::clean()
{
	// find last used sample
	mp_sint32 numsmp = numsamples;
	mp_sint32 i;
	for (i = numsamples - 1; i >= 0; i--)
	{
		mp_ubyte buffer[MP_MAXTEXT+1];
		
		XModule::convertStr(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(samples[i].name), MP_MAXTEXT, false);
		
		if (samples[i].sample || strlen(reinterpret_cast<char*>(buffer)))
		{
			numsmp = i+1;
			break;
		}
	}
	if (i == -1)
		numsmp = 0;
		
	numsamples = numsmp;
}

mp_sint32 XIInstrument::load(const SYSCHAR* fileName)
{
	XMFile f(fileName);
	
	f.read(sig, 1, 21);

	if (memcmp(sig,"GF1PATCH",8) == 0)
	{
		f.seek(0);
		return loadPAT(f);
	}

	if (memcmp(sig,"Extended Instrument: ",21) != 0)
		return MP_LOADER_FAILED;
	
	f.read(name, 1, 22);
	
	if (f.readByte() != 0x1A)
		return MP_LOADER_FAILED;
	
	f.read(tracker, 1, 20);
	
	if (f.readWord() != 0x102)	// read version
		return MP_LOADER_FAILED;	
	
	// read note instrument table
	f.read(nbu, 1, 96);
	mp_sint32 k;
	for (k = 0; k < 96; k++)
		if (nbu[k]>15) nbu[k] = 15;
	
	// read envelopes
	memset(&venv,0,sizeof(venv));
	memset(&penv,0,sizeof(penv));
	
	for (k = 0; k < 12; k++)
	{
		venv.env[k][0] = f.readWord();
		venv.env[k][1] = f.readWord()<<2;
	}

	for (k = 0; k < 12; k++)
	{
		penv.env[k][0] = f.readWord();
		penv.env[k][1] = f.readWord()<<2;
	}
	
	// read envelope flags
	venv.num = f.readByte();
	penv.num = f.readByte();
	venv.sustain = f.readByte();
	venv.loops = f.readByte();
	venv.loope = f.readByte();
	penv.sustain = f.readByte();
	penv.loops = f.readByte();
	penv.loope = f.readByte();
	venv.type = f.readByte();
	penv.type = f.readByte();
	
	vibtype = f.readByte();
	vibsweep = f.readByte();
	vibdepth = f.readByte();
	vibrate = f.readByte();
	
	vibdepth<<=1;
	
	volfade = f.readWord();
	
	volfade<<=1;
	
	res = f.readWord();
		
	f.read(extra, 1, 20);
	
	numsamples = f.readWord();
	if (numsamples > 16)
		numsamples = 16;
	
	// read sample infos
	for (k = 0; k < numsamples; k++)
	{
		samples[k].samplen = f.readDword();
		samples[k].loopstart = f.readDword();
		samples[k].looplen = f.readDword();
		samples[k].vol = XModule::vol64to255(f.readByte());
		samples[k].finetune = f.readByte();
		samples[k].type = f.readByte();
		samples[k].pan = f.readByte();
		samples[k].relnote = f.readByte();
		samples[k].res = f.readByte();
		samples[k].volfade = volfade;
		samples[k].vibtype = vibtype;
		samples[k].vibsweep = vibsweep;
		samples[k].vibdepth = vibdepth;
		samples[k].vibrate = vibrate;
		f.read(samples[k].name, 1, 22);
	}
	
	// lazy guy
	TXMSample* smp = reinterpret_cast<TXMSample*>(&samples);
	
	// read samples
	for (k = 0; k < numsamples; k++)
	{
		if (!(smp[k].type&16) && smp[k].samplen) 
		{
			smp[k].sample = (mp_sbyte*)TXMSample::allocPaddedMem(smp[k].samplen);
			
			if (smp[k].sample == NULL)
			{
				return MP_OUT_OF_MEMORY;
			}
			
			owner = true;			
			
			if (!XModule::loadSample(f,smp[k].sample,smp[k].samplen,smp[k].samplen,XModule::ST_DELTA))
			{
				return MP_OUT_OF_MEMORY;
			}					
		}
		else if (smp[k].samplen)
		{
			smp[k].sample = (mp_sbyte*)TXMSample::allocPaddedMem(smp[k].samplen);
			
			if (smp[k].sample == NULL)
			{
				return MP_OUT_OF_MEMORY;
			}

			owner = true;			
			
			if (!XModule::loadSample(f,smp[k].sample,smp[k].samplen,smp[k].samplen>>1,XModule::ST_DELTA | XModule::ST_16BIT))
			{
				return MP_OUT_OF_MEMORY;
			}					
			
			smp[k].samplen>>=1;
			smp[k].loopstart>>=1;
			smp[k].looplen>>=1;
		}
	}	
	
	return MP_OK;
}

static mp_sint32 gusFreqToFT2Note(mp_dword freq)
{
	static const mp_dword scale_table[109] = 
	{ 
		/*C-0..B-*/
		/* Octave 0 */  16351, 17323, 18354, 19445, 20601, 21826, 23124, 24499, 25956, 27500, 29135, 30867,
		/* Octave 1 */  32703, 34647, 36708, 38890, 41203, 43653, 46249, 48999, 51913, 54999, 58270, 61735,
		/* Octave 2 */  65406, 69295, 73416, 77781, 82406, 87306, 92498, 97998, 103826, 109999, 116540, 123470,
		/* Octave 3 */  130812, 138591, 146832, 155563, 164813, 174614, 184997, 195997, 207652, 219999, 233081, 246941,
		/* Octave 4 */  261625, 277182, 293664, 311126, 329627, 349228, 369994, 391995, 415304, 440000, 466163, 493883,
		/* Octave 5 */  523251, 554365, 587329, 622254, 659255, 698456, 739989, 783991, 830609, 880000, 932328, 987767,
		/* Octave 6 */  1046503, 1108731, 1174660, 1244509, 1318511, 1396914, 1479979, 1567983, 1661220, 1760002, 1864657, 1975536,
		/* Octave 7 */  2093007, 2217464, 2349321, 2489019, 2637024, 2793830, 2959960, 3135968, 3322443, 3520006, 3729316, 3951073,
		/* Octave 8 */  4186073, 4434930, 4698645, 4978041, 5274051, 5587663, 5919922, 6271939, 6644889, 7040015, 7458636, 7902150, 0xFFFFFFFF
	};

	if (scale_table[0] > freq)
	{
		return 0-12;
	}
	for (mp_uint32 no = 0; no < sizeof(scale_table)/sizeof(mp_dword)-1; no++)
	{
		if (scale_table[no] <= freq && 
			scale_table[no+1] > freq)
			return (no-12);
	}
	
	return 4*12;
}

#define PAT_CLEAN \
	for (mp_uint32 j = 0; j < sizeof(nbu); j++) \
		if (nbu[j] == 0xFF)	nbu[j] = 0;

mp_sint32 XIInstrument::loadPAT(XMFile& f)
{

	struct TGF1PatchHeader
	{
		mp_ubyte	sig[22];
		mp_ubyte	desc[60];
		mp_ubyte	insnum;
		mp_ubyte	voicenum;
		mp_ubyte	channum;
		mp_uword	waveforms;
		mp_uword	mastervol;
		mp_uint32	datasize;
		mp_ubyte	reserved1[36];
		mp_uword	insID;
		mp_ubyte	insname[16];
		mp_uint32	inssize;
		mp_ubyte	layers;
		mp_ubyte	reserved2[40];
		mp_ubyte	layerduplicate;
		mp_ubyte	layer;
		mp_uint32	layersize;
		mp_ubyte	smpnum;
		mp_ubyte	reserved3[40];
	};
	
	struct TGF1PatchSampleHeader
	{
		mp_ubyte	wavename[7];
		mp_ubyte	fractions;
		mp_uint32	samplesize;
		mp_sint32	loopstart;
		mp_sint32	loopend;
		mp_uword	samplerate;
		mp_uint32	lofreq;
		mp_uint32	hifreq;
		mp_uint32	rtfreq;
		mp_uword	tune;
		mp_ubyte	panning;
		
		mp_ubyte	envelopes[4*3];
		mp_ubyte	tremolo[3];
		mp_ubyte	vibrato[3];
		
		mp_ubyte	smpmode;
		mp_uword	scalefreq;
		mp_uword	scalefac;
		mp_ubyte	reserved[36];
	};
	
	TGF1PatchHeader header;
	
	f.read(header.sig, 1, 22);
	f.read(header.desc, 1, 60);
	header.insnum = f.readByte();
	header.voicenum = f.readByte();
	header.channum = f.readByte();
	header.waveforms = f.readWord();
	header.mastervol = f.readWord();
	header.datasize = f.readDword();
	f.read(header.reserved1, 1, 36);
	header.insID = f.readWord();
	f.read(header.insname, 1, 16);
	header.inssize = f.readDword();
	header.layers = f.readByte();
	f.read(header.reserved2, 1, 40);	
	header.layerduplicate = f.readByte();
	header.layer = f.readByte();
	header.layersize = f.readDword();
	header.smpnum = f.readByte();
	f.read(header.reserved3, 1, 40);	
	
	// apply data
	memcpy(name, header.insname, 16);
	numsamples = header.smpnum <= 16 ? header.smpnum : 16;

	// lazy guy
	TXMSample* smp = reinterpret_cast<TXMSample*>(&samples);
	
	memset(nbu, 0xFF, sizeof(nbu));
	
	for (mp_sint32 i = 0; i < numsamples; i++)
	{
		TGF1PatchSampleHeader smphdr;

		f.read(smphdr.wavename, 1, 7);
		smphdr.fractions = f.readByte();
		smphdr.samplesize = f.readDword();
		smphdr.loopstart = f.readDword();
		smphdr.loopend = f.readDword();
		smphdr.samplerate = f.readWord();
		smphdr.lofreq = f.readDword();
		smphdr.hifreq = f.readDword();
		smphdr.rtfreq = f.readDword();

		mp_sint32 lo = gusFreqToFT2Note(smphdr.lofreq);
		mp_sint32 hi = gusFreqToFT2Note(smphdr.hifreq);
		
		if (lo < 0)
			lo = 0;
		if (hi < 0)
			hi = 0;
		if (lo > 95)
			lo = 95;
		if (hi > 95)
			hi = 95;
		
		// sanity check
		if (lo > hi)
		{
			mp_sint32 s = lo;
			lo = hi;
			hi = s;
		}
		
		//if (i == numsamples - 1)
		//	hi = 96;
		
		for (mp_sint32 j = lo; j <= hi; j++)
			if (j >= 0 && j <= 95)
				if (nbu[j] == 0xFF) nbu[j] = i;
		
		//ld(x) = log(x)/log(2)
		
#ifdef VERBOSE
		printf("%i:\nlo:%i,hi:%i,rt:%i,%i:%i\n", i, smphdr.lofreq, smphdr.hifreq, smphdr.rtfreq,lo,hi);
#endif		
		smphdr.tune = f.readWord();
		smphdr.panning = f.readByte();
		f.read(smphdr.envelopes, 3, 4);
		f.read(smphdr.tremolo, 1, 3);
		f.read(smphdr.vibrato, 1, 3);
		smphdr.smpmode = f.readByte();
		smphdr.scalefreq = f.readWord();
		smphdr.scalefac = f.readWord();
		f.read(smphdr.reserved, 1, 36);

		samples[i].samplen = smphdr.samplesize;
		samples[i].loopstart = smphdr.loopstart;
		samples[i].looplen = smphdr.loopend - smphdr.loopstart; 
		// disable looping on invalid loop range
		if ((mp_sint32)samples[i].looplen < 0)
		{
			samples[i].looplen = 0;
			smphdr.smpmode &= ~(1<<2);
		}
		samples[i].vol = XModule::vol127to255(header.mastervol);
		samples[i].pan = (mp_ubyte)XModule::pan15to255(smphdr.panning);
		
#if 1
		mp_sint32 no = gusFreqToFT2Note(smphdr.rtfreq);		
		no-=48;
		
		//mp_sint32 rate = XModule::getc4spd(no, 0);
		XModule::convertc4spd(smphdr.samplerate/**rate/8363*/, &samples[i].finetune, &samples[i].relnote);
		
		mp_sint32 relnote = (mp_sint32)samples[i].relnote-no;
		// validate note range
		if (relnote < -96) relnote = -96;
		if (relnote > 95) relnote = 95;
		
		samples[i].relnote=relnote;
#else
		XModule::convertc4spd(smphdr.samplerate, &samples[i].finetune, &samples[i].relnote);
#endif

		// 16 bit sample
		if (smphdr.smpmode & 1) 
		{
			samples[i].type = 16;
		}
		// looping
		if (smphdr.smpmode & 4) samples[i].type |= 1;
		// bi-loop
		if (smphdr.smpmode & 8) 
		{
			samples[i].type &= ~3;
			samples[i].type |= 2;
		}
		
		memcpy(samples[i].name, smphdr.wavename, 7);

		if (!(smp[i].type&16) && smp[i].samplen) 
		{
			smp[i].sample = (mp_sbyte*)TXMSample::allocPaddedMem(smp[i].samplen);
			
			if (smp[i].sample == NULL)
			{
				PAT_CLEAN;
				return MP_OUT_OF_MEMORY;
			}
			
			owner = true;			
			
			if (smphdr.smpmode & 2)
			{
				if (!XModule::loadSample(f,smp[i].sample,smp[i].samplen,smp[i].samplen,XModule::ST_UNSIGNED))
				{
					PAT_CLEAN;
					return MP_OUT_OF_MEMORY;
				}					
			}
			else
			{
				if (!XModule::loadSample(f,smp[i].sample,smp[i].samplen,smp[i].samplen))
				{
					PAT_CLEAN;
					return MP_OUT_OF_MEMORY;
				}					
			}
		}
		else if (smp[i].samplen)
		{
			smp[i].sample = (mp_sbyte*)TXMSample::allocPaddedMem(smp[i].samplen);
			
			if (smp[i].sample == NULL)
			{
				PAT_CLEAN;
				return MP_OUT_OF_MEMORY;
			}

			owner = true;			
			
			if (smphdr.smpmode & 2)
			{
				if (!XModule::loadSample(f,smp[i].sample,smp[i].samplen,smp[i].samplen>>1,XModule::ST_16BIT | XModule::ST_UNSIGNED))
				{
					PAT_CLEAN;
					return MP_OUT_OF_MEMORY;
				}					
			}
			else
			{
				if (!XModule::loadSample(f,smp[i].sample,smp[i].samplen,smp[i].samplen>>1,XModule::ST_16BIT))
				{
					PAT_CLEAN;
					return MP_OUT_OF_MEMORY;
				}					
			}
			
			smp[i].samplen>>=1;
			smp[i].loopstart>>=1;
			smp[i].looplen>>=1;
		}

	}
	
	PAT_CLEAN;
	
	return MP_OK;
}

mp_sint32 XIInstrument::save(const SYSCHAR* fileName)
{
	clean();

	XMFile f(fileName, true);
	
	f.write("Extended Instrument: ", 1, 21);

	f.write(name, 1, 22);
	
	f.writeByte(0x1A);
	
	f.write("FastTracker v2.00   ", 1, 20);
	
	f.writeWord(0x102);
	
	f.write(nbu, 1, 96);
	
	mp_sint32 k;
	for (k = 0; k < 12; k++)
	{
		f.writeWord(venv.env[k][0]);
		f.writeWord(venv.env[k][1]>>2);
	}

	for (k = 0; k < 12; k++)
	{
		f.writeWord(penv.env[k][0]);
		f.writeWord(penv.env[k][1]>>2);
	}
	
	// read envelope flags
	f.writeByte(venv.num);
	f.writeByte(penv.num);
	f.writeByte(venv.sustain);
	f.writeByte(venv.loops);
	f.writeByte(venv.loope);
	f.writeByte(penv.sustain);
	f.writeByte(penv.loops);
	f.writeByte(penv.loope);
	f.writeByte(venv.type);
	f.writeByte(penv.type);
	
	f.writeByte(vibtype);
	f.writeByte(vibsweep);
	f.writeByte(vibdepth>>1);
	f.writeByte(vibrate);
	
	f.writeWord(volfade>>1);
	
	f.writeWord(0);
		
	memset(extra, 0, sizeof(extra));
	f.write(extra, 1, 20);
	
	f.writeWord(numsamples);
	
	// read sample infos
	for (k = 0; k < numsamples; k++)
	{
		if (samples[k].type&16)
		{
			f.writeDword(samples[k].samplen<<1);
			f.writeDword(samples[k].loopstart<<1);
			f.writeDword(samples[k].looplen<<1);
		}
		else
		{
			f.writeDword(samples[k].samplen);
			f.writeDword(samples[k].loopstart);
			f.writeDword(samples[k].looplen);
		}
		f.writeByte(samples[k].vol*64/255);
		f.writeByte(samples[k].finetune);
		f.writeByte(samples[k].type);
		f.writeByte(samples[k].pan);
		f.writeByte(samples[k].relnote);
		f.writeByte(samples[k].res);
		f.write(samples[k].name, 1, 22);
	}
	
	// lazy guy
	TXMSample* smp = reinterpret_cast<TXMSample*>(&samples);
	
	// write samples
	for (k = 0; k < numsamples; k++)
	{
		if ((smp[k].type&16) && smp[k].samplen && smp[k].sample) 
		{
			mp_sword* dst = new mp_sword[smp[k].samplen];
			
			mp_sword last = 0;
			for (mp_uint32 i = 0; i < smp[k].samplen; i++)
			{
				dst[i] = smp[k].getSampleValue(i)-last;
				last = smp[k].getSampleValue(i);
			}
		
			f.writeWords((mp_uword*)dst, smp[k].samplen);
			
			delete[] dst;
		}
		else if (smp[k].samplen && smp[k].sample)
		{
			mp_sbyte* dst = new mp_sbyte[smp[k].samplen];
			
			mp_sbyte last = 0;
			for (mp_uint32 i = 0; i < smp[k].samplen; i++)
			{
				dst[i] = smp[k].getSampleValue(i)-last;
				last = smp[k].getSampleValue(i);
			}

			f.write(dst, 1, smp[k].samplen);
			
			delete[] dst;
		}
	}	
	
	return MP_OK;
}

XIInstrument& XIInstrument::operator=(const XIInstrument& source)
{
	// no self-assignment
	if (this != &source)
	{
		// free up whatever is in there
		if (owner)
		{
			mp_sint32 numSamples = sizeof(samples)/sizeof(TXMSample);
			for (mp_sint32 i = 0; i < numSamples; i++)
				TXMSample::freePaddedMem((mp_ubyte*)samples[i].sample);
		}
		
		memset(&samples, 0, sizeof(samples));

		// owner is now true
		owner = true;		
		
		// copy
		memcpy(sig, source.sig, sizeof(sig));
		memcpy(name, source.name, sizeof(name));
		memcpy(tracker, source.tracker, sizeof(tracker));
		memcpy(nbu, source.nbu, sizeof(nbu));
		
		venv = source.venv;
		penv = source.penv;
		
		vibtype = source.vibtype;
		vibsweep = source.vibsweep;
		vibdepth = source.vibdepth;
		vibrate = source.vibrate;
		
		volfade = source.volfade;
		
		res = source.res;
		
		memcpy(extra, source.extra, sizeof(extra));	
		
		numsamples = source.numsamples;
		
		for (mp_sint32 i = 0; i < numsamples; i++)
		{
			const TXMSample* src = &source.samples[i];
			TXMSample* dst = &samples[i];
			
			dst->samplen = src->samplen;
			dst->loopstart = src->loopstart;
			dst->looplen = src->looplen;
			dst->flags = src->flags;
			dst->vol = src->vol;
			dst->finetune = src->finetune;
			dst->type = src->type;
			dst->pan = src->pan;
			dst->relnote = src->relnote;
			dst->venvnum = src->venvnum;
			dst->penvnum = src->penvnum;
			dst->fenvnum = src->fenvnum;
			dst->vibenvnum = src->vibenvnum;
			dst->vibtype = src->vibtype;
			dst->vibsweep = src->vibsweep;
			dst->vibdepth = src->vibdepth;
			dst->vibrate = src->vibrate;
			dst->volfade = src->volfade;
			dst->res = src->res;
			
			memcpy(dst->name, src->name, sizeof(src->name));
			
			dst->terminate = src->terminate;
			
			if (src->sample)
			{
				if (src->type & 16)
				{
					dst->sample = (mp_sbyte*)(TXMSample::allocPaddedMem(src->samplen*2));
					TXMSample::copyPaddedMem(dst->sample, src->sample, src->samplen*2);
				}
				else
				{
					dst->sample = (mp_sbyte*)(TXMSample::allocPaddedMem(src->samplen));
					TXMSample::copyPaddedMem(dst->sample, src->sample, src->samplen);
				}
			}
		}	
	}
	
	return *this;
}


