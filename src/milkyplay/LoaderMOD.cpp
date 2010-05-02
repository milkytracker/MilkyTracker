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
 *  LoaderMOD.cpp
 *  MilkyPlay Module Loader: Protracker compatible
 *  MilkyPlay Module Loader: Game Music Creator (very similiar to MOD, kept in here)
 *  MilkyPlay Module Loader: SoundFX (very similiar to MOD, kept in here)
 */
#include "Loaders.h"

// get number of channels of protracker compatible song
static mp_sint32 getPTnumchannels(char *id)
{
	struct TModID
	{
		const char*	ID;
		mp_sint32	numChannels;
	};
	
	TModID modIDs[] = 
	{
		{"M.K.",4},{"M!K!",4},{"FLT4",4},{"FLT8",8},{"OKTA",8},{"OCTA",8},{"FA08",8},{"CD81",8},
		{"1CHN",1},{"2CHN",2},{"3CHN",3},{"4CHN",4},{"5CHN",5},{"6CHN",6},{"7CHN",7},{"8CHN",8},{"9CHN",9},{"10CH",10},
		{"11CH",11},{"12CH",12},{"13CH",13},{"14CH",14},{"15CH",15},{"16CH",16},{"17CH",17},{"18CH",18},{"19CH",19},{"20CH",20},
		{"21CH",21},{"22CH",22},{"23CH",23},{"24CH",24},{"25CH",25},{"26CH",26},{"27CH",27},{"28CH",28},{"29CH",29},{"30CH",30},{"31CH",31},{"32CH",32}
	};
	
	mp_sint32 *id1 = (mp_sint32*)id;
	for (mp_uint32 x=0;x<sizeof(modIDs)/sizeof(TModID);x++) {
		mp_sint32 *id2 = (mp_sint32*)modIDs[x].ID;
		if (*id2==*id1) {
			return modIDs[x].numChannels;
		}
	}
	
	return 0;
}

static mp_sint32 mot2int(mp_sint32 x)
{
	return (x>>8)+((x&255)<<8);
}

const char* LoaderMOD::identifyModule(const mp_ubyte* buffer)
{
	// check for .MOD
	if (getPTnumchannels((char*)buffer+1080)) 
	{
		return "MOD";
	}	

	mp_sint32 i,j;
	mp_ubyte* uBuffer = (mp_ubyte*)buffer;
	
	// see if we're getting a song title
	for (i = 0; i < 20; i++)
		if (uBuffer[i] >= 126 || (uBuffer[i] < 32 && uBuffer[i]))
			return NULL;
			
	uBuffer+=20;

	mp_sint32 lastAsciiValues = -1;
	for (j = 0; j < 15; j++)
	{
		if (uBuffer[24])
			break;

		if (uBuffer[25] > 64)
			break;
	
		bool ascii = true;
		for (i = 0; i < 22; i++)
		{
			if (uBuffer[i] >= 126 || (uBuffer[i] < 14 && uBuffer[i]))
			{
				ascii = false;
				break;
			}
		}
		
		if (ascii)
			lastAsciiValues = j;
		else
			break;
			
		uBuffer+=30;
	}

	if (lastAsciiValues != 14)
		return NULL;

	if (!*uBuffer || *uBuffer > 128)
		return NULL;

	*uBuffer+=2;

	for (i = 0; i < 128; i++)
		if (uBuffer[i] > 128)
			return NULL;

	return "M15";
}

mp_sint32 LoaderMOD::load(XMFileBase& f, XModule* module)
{	
	enum ModuleTypes
	{
		ModuleTypeUnknown,
		ModuleTypeIns31,
		ModuleTypeIns15
	};

	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;	
	
	char block[2048];
	f.read(block, 1, 2048);

	const char* id = identifyModule((mp_ubyte*)block);
	if (!id)
		return MP_LOADER_FAILED;

	ModuleTypes moduleType = ModuleTypeUnknown;
	if (strcmp(id, "M15") == 0)
		moduleType = ModuleTypeIns15;
	else if (strcmp(id, "MOD") == 0)
		moduleType = ModuleTypeIns31;

	if (moduleType == ModuleTypeUnknown)
		return MP_LOADER_FAILED;

	f.seekWithBaseOffset(0);
	
	f.read(&header->name,1,20);
	
	switch (moduleType)
	{
		case ModuleTypeIns15:
			header->insnum = 15;
			break;
		case ModuleTypeIns31:
			header->insnum = 31;
			break;
		default:
			return MP_LOADER_FAILED;
	}
	
#ifdef VERBOSE
	printf("Loading...\n");
#endif
	
	mp_sint32 i, s = 0;
	for (i = 0; i < header->insnum; i++) {
		mp_ubyte insname[22];
		mp_uword smplen=0;
		mp_ubyte finetune=0;
		mp_ubyte vol=0;
		mp_uword loopstart=0;
		mp_uword looplen=0;
		f.read(&insname,1,22);
		
		smplen = f.readWord();
		f.read(&finetune,1,1);
		f.read(&vol,1,1);
		loopstart = f.readWord();
		looplen = f.readWord();
		
#ifdef VERBOSE
		printf("Ins %i, smplen: %i, loopstart: %i, looplen: %i\n", i, mot2int(smplen), mot2int(loopstart), mot2int(looplen));
#endif
		
		memcpy(instr[i].name, insname, 22);

		// valid sample?
		if ((mot2int(smplen)<<1) > 2)
		{
			TXMSample* smp = &module->smp[s];
		
			memcpy(smp->name, insname, 22);
			
			instr[i].samp=1;

			for (mp_sint32 j=0;j<120;j++) 
				instr[i].snum[j] = s;
		
			smp->finetune = XModule::modfinetunes[finetune & 15];
			smp->relnote = 0;
			//module->convertc4spd(module->sfinetunes[finetune],&smp->finetune,&smp->relnote);
		
			smp->flags=1;
			smp->samplen=mot2int(smplen)<<1;
			smp->loopstart=mot2int(loopstart)<<1;
			smp->looplen=mot2int(looplen)<<1;
			smp->vol=XModule::vol64to255(vol);
		
			if (smp->samplen<=2) {
				smp->samplen=0;
				instr[s].samp=0;
			}
		
			if ((smp->loopstart+smp->looplen)>smp->samplen)
			{
				// first correct loop start
				mp_sint32 dx = (smp->loopstart+smp->looplen)-smp->samplen;
				smp->loopstart-=dx;
				// still incorrect? => correct loop length
				if ((smp->loopstart+smp->looplen)>smp->samplen)
				{
					dx = (smp->loopstart+smp->looplen)-smp->samplen;
					smp->looplen-=dx;
				}
			}
		
			if (smp->loopstart<2 && smp->looplen>2) 
			{
				if (smp->looplen < smp->samplen)
				//	smp->loopstart=0;
					smp->type |= 32;
#ifdef VERBOSE
				printf("Contains one shot samples %i...\n", s);
#endif
			}
			
			if (smp->looplen<=2) 
				smp->looplen=0;
			else 
			{
				/*if (smp->loopstart > 2)
				{
					smp->loopstart -=2;
					smp->looplen+=2;
				}*/
				smp->type|=1;
			}
				
			smp->pan=0x80;
		
			s++;
		}
		//ins[i].c4spd=sfinetunes[ins[i].finetune];
		
	}
	
	header->smpnum = s;	
	
	header->ordnum = f.readByte();
	
	f.read(&header->whythis1a,1,1);
	f.read(&header->ord,1,128);
	
	if (moduleType == ModuleTypeIns31)
		f.read(header->sig,1,4);
	
	if ((memcmp(header->sig+2,"CH",2) != 0 && 
		memcmp(header->sig+1,"CHN",3) != 0) ||
		moduleType == ModuleTypeIns15)
		header->flags = XModule::MODULE_PTNEWINSTRUMENT;
	
	header->patnum=0;
	for (i=0;i<128;i++)
		if (header->ord[i]>header->patnum) header->patnum=header->ord[i];
	
	header->patnum++;
	
	//patterns = new mp_ubyte*[modhead.numpatts];
	
	if (moduleType == ModuleTypeIns31)
		header->channum = getPTnumchannels((char*)&header->sig);
	else if (moduleType == ModuleTypeIns15)
		header->channum = 4;
	
	if (!header->channum) 
	{
		return MP_LOADER_FAILED;
	}
	
	//mp_sint32 patternsize = modhead.numchannels*modhead.numrows*5;
	mp_sint32 modpatternsize = header->channum*64*4;
	
	mp_ubyte *buffer = new mp_ubyte[modpatternsize];
	
	if (buffer == NULL) 
	{
		return MP_OUT_OF_MEMORY;
	}
	
	for (i=0;i<header->patnum;i++) {
		f.read(buffer,1,modpatternsize);
		
		phead[i].rows=64;
		phead[i].effnum=1;
		phead[i].channum=(mp_ubyte)header->channum;
		
		phead[i].patternData=new mp_ubyte[phead[i].rows*header->channum*4];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			delete[] buffer;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum*4);
		
		mp_sint32 r,c,cnt=0;
		for (r=0;r<64;r++) {
			for (c=0;c<header->channum;c++) {
				mp_ubyte b1 = buffer[cnt];
				mp_ubyte b2 = buffer[cnt+1];
				mp_ubyte b3 = buffer[cnt+2];
				mp_ubyte b4 = buffer[cnt+3];
				
				mp_sint32 note,ins,eff,notenum = 0;
				note = ((b1&0xf)<<8)+b2;
				ins = (b1&0xf0)+(b3>>4);
				eff = b3&0xf;
				
				if (eff==0xE) {
					eff=(b4>>4)+0x30;
					b4&=0xf;
				}
				
				if ((!eff)&&b4) 
					eff=0x20;
				
				// old style modules don't support last effect for:
				// - portamento up/down
				// - volume slide
				if (eff==0x1&&(!b4)) eff = 0;
				if (eff==0x2&&(!b4)) eff = 0;
				if (eff==0xA&&(!b4)) eff = 0;

				if (eff==0x5&&(!b4)) eff = 0x3;
				if (eff==0x6&&(!b4)) eff = 0x4;
				
				if (eff==0xC) {
					b4 = XModule::vol64to255(b4);
				}
				
				if (note) 
					notenum = XModule::amigaPeriodToNote(note);

				phead[i].patternData[cnt]=notenum;
				phead[i].patternData[cnt+1]=ins;
				phead[i].patternData[cnt+2]=eff;
				phead[i].patternData[cnt+3]=b4;
				
				cnt+=4;
			}
		}
		
	}
	delete[] buffer;
	
	for (i=0; i < header->smpnum; i++) 
	{
		// Take a peek of the sample and check if we have to do some nasty MODPLUG ADPCM decompression
		bool adpcm = false;
		
		if (f.posWithBaseOffset() + 5 <= f.sizeWithBaseOffset())
		{
			f.read(block, 1, 5);
			adpcm = memcmp(block, "ADPCM", 5) == 0;
			if (!adpcm)
				f.seekWithBaseOffset(f.posWithBaseOffset() - 5);
		}
					
		mp_sint32 result = module->loadModuleSample(f, i, adpcm ? XModule::ST_PACKING_ADPCM : XModule::ST_DEFAULT);
		if (result != MP_OK)
			return result;
	}

	header->speed=125;
	header->tempo=6;
	header->mainvol=255;
	//header->freqtab=1;

	if (moduleType == ModuleTypeIns15)
		strcpy(header->tracker,"Soundtracker");
	else
		strcpy(header->tracker,"Protracker");

	module->postLoadAnalyser();

	// Amiga panning LRRL
	for (i = 0; i < header->channum; i++)
	{
		switch (i & 3)
		{
			case 0:
			case 3:
				header->pan[i] = 0;
				break;
			case 1:
			case 2:
				header->pan[i] = 255;
				break;
		}
	}

	module->postProcessSamples();
	
#ifdef VERBOSE
	printf("%i / %i\n", f.pos(), f.size());
#endif

	return MP_OK;
}

const char* LoaderGMC::identifyModule(const mp_ubyte* buffer)
{
	mp_sint32 i = 0;

	// check instrument volume for value from 0x00 to 0x40 
	
	const mp_ubyte* ptr = buffer + 7;
	bool ok = true;
	for (i = 0; i < 15 && ok; i++)
	{
		if (*ptr > 0x40)
			ok = false;
		ptr+=16;
	}
	
	if (!ok)
		return NULL;

	// Those 3 bytes should all be zero
	if (buffer[0xF0] || buffer[0xF1] || buffer[0xF2])
		return NULL;
	
	// this should not be zero
	if (!buffer[0xF3])
		return NULL;
				
	ptr = buffer + 0xF4;
	ok = true;
	// check orders to be divisible by 0x400
	for (i = 0; i < *(buffer + 0xF3) && ok; i++)
	{
		if (BigEndian::GET_WORD(ptr) & 0x3ff)
			ok = false;
		ptr+=2;
	}

	if (!ok)
		return NULL;

	return "GMC";
}

mp_sint32 LoaderGMC::load(XMFileBase& f, XModule* module)
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

	mp_sint32 i,j,k;

	// read GMC instruments, always 15
	header->insnum = 15;
	// channels always 4
	header->channum = 4;
	
	j = 0;
	for (i = 0; i < header->insnum; i++) 
	{
		// Ignore DWORD (probably some address)
		f.readDword();
		
		mp_uint32 samplen = mot2int(f.readWord()) << 1;
		
		mp_ubyte finetune = f.readByte();
		
		mp_ubyte vol = f.readByte();
		
		// Ignore DWORD (probably some address)
		f.readDword();
		
		mp_sint32 looplen = mot2int(f.readWord()) << 1;
		mp_sint32 loopstart = mot2int(f.readWord()) << 1;	
		
		mp_sint32 newloopstart = samplen - looplen;
		mp_sint32 newloopend = samplen - loopstart;
		
		if (looplen > 4)
		{
			loopstart = newloopstart;
			looplen = newloopend - loopstart;
		}
		
		// valid sample?
		if (samplen)
		{
			TXMSample* smp = &module->smp[j];
		
			instr[i].samp=1;

			for (k = 0; k < 120; k++) 
				instr[i].snum[k] = j;
		
			smp->finetune = XModule::modfinetunes[finetune & 15];
			smp->relnote = 0;
		
			smp->flags = 1;
			smp->samplen = samplen;
			smp->loopstart = loopstart;
			smp->looplen = looplen;
			smp->vol = XModule::vol64to255(vol);
		
			if (smp->samplen <= 4) {
				smp->samplen = 0;
				instr[i].samp = 0;
			}
		
			if (smp->looplen <= 4) 
				smp->looplen = 0;
			else 
			{
				smp->type|=1;
			}
				
			smp->pan = 0x80;
		
			j++;
		}
		
	}
	
	header->smpnum = j;		

	// skip something
	f.readByte();
	f.readByte();
	f.readByte();
	
	header->ordnum = f.readByte();
	mp_uword ord[100];
	
	f.readWords(ord, 100);
	
	mp_sint32 patnum = 0;
	for (i = 0; i < 100; i++)
	{
		header->ord[i] = mot2int(ord[i]) >> 10;
		if (header->ord[i] > patnum)
			patnum = header->ord[i];
	}
	
	header->patnum = patnum+1;

	mp_sint32 modpatternsize = header->channum*64*4;
	
	mp_ubyte *buffer = new mp_ubyte[modpatternsize];
	
	if (buffer == NULL) 
	{
		return MP_OUT_OF_MEMORY;
	}
	
	for ( i = 0; i < header->patnum; i++) {
		f.read(buffer, 1, modpatternsize);
		
		phead[i].rows = 64;
		phead[i].effnum = 1;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*4];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			delete[] buffer;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData, 0, phead[i].rows*header->channum*4);
		
		mp_sint32 r,c,cnt=0;
		for (r = 0; r < 64; r++) {
			for ( c = 0; c < header->channum; c++) 
			{
				mp_ubyte b1 = buffer[cnt];
				mp_ubyte b2 = buffer[cnt+1];
				mp_ubyte b3 = buffer[cnt+2];
				mp_ubyte b4 = buffer[cnt+3];
				
				mp_sint32 note,ins,eff,notenum = 0;
				note = ((b1&0xf)<<8)+b2;
				ins = (b1&0xf0)+(b3>>4);
				eff = b3&0xf;
				
				switch (eff)
				{
					case 0x01:
					case 0x02:
						break;
					case 0x03:
						eff = 0x0C;
						b4 = XModule::vol64to255(b4);
						break;
					case 0x04:
						eff = 0x0D;
						break;
					case 0x05:
						eff = 0x0B;
						break;
					case 0x08:
						eff = 0x0F;
						break;
						
					default:
						eff = b4 = 0;
					
				}
				
				if (note) 
					notenum = XModule::amigaPeriodToNote(note);

				phead[i].patternData[cnt] = notenum;
				phead[i].patternData[cnt+1] = ins;
				phead[i].patternData[cnt+2] = eff;
				phead[i].patternData[cnt+3] = b4;
				
				cnt+=4;
			}
		}
		
	}
	delete[] buffer;
	
	mp_sint32 result = module->loadModuleSamples(f);
	if (result != MP_OK)
		return result;

	header->speed = 125;
	header->tempo = 6;
	header->mainvol = 255;

	// Amiga panning LRRL
	for (i = 0; i < header->channum; i++)
	{
		switch (i & 3)
		{
			case 0:
			case 3:
				header->pan[i] = 0;
				break;
			case 1:
			case 2:
				header->pan[i] = 255;
				break;
		}
	}

	module->postProcessSamples();
	
	strcpy(header->tracker,"Game Music Creator");

	return MP_OK;
}

const char* LoaderSFX::identifyModule(const mp_ubyte* buffer)
{
	// check for .SFX module
	if (!memcmp(buffer+60,"SONG",4)) 
	{
		// Check if first 15 big endian DWORDS contain valid sample sizes
		for (mp_sint32 i = 0; i < 15; i++)
			if (BigEndian::GET_DWORD(buffer+i*4) > 65536*2)
				return NULL;
	
		return "SFX";
	}

	return NULL;
}

mp_sint32 LoaderSFX::load(XMFileBase& f, XModule* module)
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

	mp_sint32 i,j,k;

	// read SoundFX instruments, always 15
	header->insnum = header->smpnum = 15;
	// channels always 4
	header->channum = 4;
	
	mp_dword sampSizeTab[15];
	for (i = 0; i < header->smpnum; i++)
	{
		mp_ubyte temp[4];
		f.read(temp, 1, 4);
		sampSizeTab[i] = BigEndian::GET_DWORD(temp);
	}
	
	// read signature
	f.read(header->sig, 1, 4);
	
	mp_sint32 delayValue = mot2int(f.readWord());	
	
	// skip 14 bytes garbage
	f.readDword();
	f.readDword();
	f.readDword();
	f.readWord();
	
	header->speed = 122 * 14565 / delayValue;
	header->tempo = 6;
	header->mainvol = 255;
	
	j = 0;
	for (i = 0; i < header->insnum; i++) 
	{
		f.read(instr[i].name, 1, 22);
		
		mp_uint32 samplen = mot2int(f.readWord()) << 1;
		samplen = sampSizeTab[i];
		
		mp_ubyte finetune = f.readByte();
		
		mp_ubyte vol = f.readByte();
		
		mp_sint32 loopstart = mot2int(f.readWord());	
		mp_sint32 looplen = mot2int(f.readWord()) << 1;
		
		// valid sample?
		if (samplen > 4)
		{
			TXMSample* smp = &module->smp[j];
		
			instr[i].samp=1;

			for (k = 0; k < 120; k++) 
				instr[i].snum[k] = j;
		
			smp->finetune = XModule::modfinetunes[finetune & 15];
			smp->relnote = 0;
		
			smp->flags = 1;
			smp->samplen = samplen;
			smp->loopstart = loopstart;
			smp->looplen = looplen;
			smp->vol = XModule::vol64to255((mp_sint32)vol*64/63);
		
			if (smp->samplen <= 4) {
				smp->samplen = 0;
				instr[i].samp = 0;
			}
		
			if (smp->looplen <= 4) 
				smp->looplen = 0;
			else 
			{
				smp->type|=1;
			}
				
			smp->pan = 0x80;
		
			j++;
		}
		
	}
	
	header->smpnum = j;		
	header->ordnum = f.readByte();	
	header->restart = f.readByte(); 
	f.read(&header->ord, 1, 128);

	header->patnum = 0;
	for (i = 0; i < 128; i++)
		if (header->ord[i] > header->patnum) header->patnum = header->ord[i];
	
	header->patnum++;

	mp_sint32 modpatternsize = header->channum*64*4;
	
	mp_ubyte *buffer = new mp_ubyte[modpatternsize];
	
	if (buffer == NULL) 
	{
		return MP_OUT_OF_MEMORY;
	}
	
	for ( i = 0; i < header->patnum; i++) {
		f.read(buffer, 1, modpatternsize);
		
		phead[i].rows = 64;
		phead[i].effnum = 1;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*4];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			delete[] buffer;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData, 0, phead[i].rows*header->channum*4);
		
		mp_sint32 r,c,cnt=0;
		for (r = 0; r < 64; r++) {
			for ( c = 0; c < header->channum; c++) 
			{
				mp_ubyte b1 = buffer[cnt];
				mp_ubyte b2 = buffer[cnt+1];
				mp_ubyte b3 = buffer[cnt+2];
				mp_ubyte b4 = buffer[cnt+3];
				
				mp_sint32 note,ins,eff,notenum = 0;
				note = ((b1&0xf)<<8)+b2;
				ins = (b1&0xf0)+(b3>>4);
				eff = b3&0xf;
				
				if (b1 == 0xFF && b2 >= 0xFC)
				{
					if (b2 == 0xFE)
					{
						note = notenum = ins = 0;
						eff = 0x0C;
						b4 = 0;
					}
					else if (b2 == 0xFD)
					{
						ins = eff = b4 = 0;
					}
					else if (b2 == 0xFC)
					{
						note = notenum = 0;
					}
				}
				else
				{
					
					switch (eff)
					{
						// arpeggio?
						case 0x01:
							if (b4)
								eff = 0x20;
							break;
							// pitch bend?
						case 0x02:
							// portamento up
							if (b4 & 0xf)
								eff = 0x01;
							// porta down
							else if (b4 >> 4)
							{
								eff = 0x02;
								b4>>=4;
							}
								break;
							// add something to the volume?
						case 0x05:
							eff = 0x3A;
							break;
							// set volume
						case 0x06:
							eff = 0x0C;
							b4 = 255-XModule::vol64to255(b4);
							break;
							// portamento again?
						case 0x07:
							eff = 0x01;
							break;
							// portamento again?
						case 0x08:
							eff = 0x02;
							break;
							
						default:
							eff = b4 = 0;
							
					}
					
				}
				
				if (note) 
					notenum = XModule::amigaPeriodToNote(note);
				
				phead[i].patternData[cnt] = notenum;
				phead[i].patternData[cnt+1] = ins;
				phead[i].patternData[cnt+2] = eff;
				phead[i].patternData[cnt+3] = b4;
				
				cnt+=4;
			}
		}
		
	}
	
	delete[] buffer;
	
	mp_sint32 result = module->loadModuleSamples(f);
	if (result != MP_OK)
		return result;

	// Amiga panning LRRL
	for (i = 0; i < header->channum; i++)
	{
		switch (i & 3)
		{
			case 0:
			case 3:
				header->pan[i] = 0;
				break;
			case 1:
			case 2:
				header->pan[i] = 255;
				break;
		}
	}

	module->postProcessSamples();
	
	strcpy(header->tracker,"SoundFX");

	return MP_OK;
}
