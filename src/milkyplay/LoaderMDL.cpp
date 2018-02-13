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
 *  LoaderMDL.cpp
 *  MilkyPlay Module Loader: Digitracker 3
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  01/16/05: Added loader for version 0.0 (no joke)
 *  ??/??/04: Make it working with platform independent file loading routines
 *  ??/??/98: First version of this MDL loader
 */
#include "Loaders.h"

struct tmdlins {
	mp_ubyte sampnum;
	mp_ubyte playrangeend;
	mp_ubyte vol;
	mp_ubyte volenv;
	mp_ubyte pan;
	mp_ubyte panenv;
	mp_uint32 volfade;
	mp_ubyte vibspeed;
	mp_ubyte vibdepth;
	mp_ubyte vibsweep;
	mp_ubyte vibform;
	mp_ubyte reserved;
	mp_ubyte freqenv;
};

struct tmdlsamp {
	mp_ubyte sampnum;
	mp_ubyte smpname[32];
	mp_ubyte filename[8];
	mp_uint32 c4spd;
	mp_uint32 samplen;
	mp_uint32 loopstart;
	mp_uint32 looplen;
	mp_ubyte reserved;
	mp_ubyte infobyte;
	mp_sbyte *smp;
};

static inline mp_uint32 cvol(mp_ubyte vol)
{
	return (((mp_sint32)(vol-1))*65795)>>16; 
}

static inline mp_uint32 cpan(mp_ubyte pan)
{
	return (((mp_uint32)pan)*131589)>>16; 
}

const char* LoaderMDL::identifyModule(const mp_ubyte* buffer)
{
	// check for .MDL module
	if (!memcmp(buffer,"DMDL",4)) 
	{
		return "MDL";
	}

	return NULL;
}

mp_sint32 LoaderMDL::load(XMFileBase& f, XModule* module) 
{ 
	mp_uint32 i,e;
	
	mp_ubyte dummy[256];

	module->cleanUp();

	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	

	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;	
	
	mp_ubyte sig[5];
	f.read(&sig,1,5);
	
	if (memcmp(sig,"DMDL",4)) {
		return MP_LOADER_FAILED;
	}
	
	mp_ubyte hVer = sig[4]>>4;
	
	if (hVer > 1) {
		return MP_LOADER_FAILED;
	}
	
	memcpy(header->sig,sig,4);
	strcpy(header->tracker,"Digitracker 3");
	
	mp_uint32 numtracks=0;
	mp_uint32 numins=0;
	mp_uint32 numsamples=0;
	mp_ubyte *tracks = NULL;
	
	// waste some memory
	tmdlins *mdlins = new tmdlins[16*256];	
	tmdlsamp *mdlsamp = new tmdlsamp[256];
	mp_uint32 *trackseq = new mp_uint32[256*32];
	
	if (mdlins == NULL || mdlsamp == NULL || trackseq == NULL)
	{
		if (mdlins) delete[] mdlins;
		if (mdlsamp) delete[] mdlsamp;
		if (trackseq) delete[] trackseq;
		
		return MP_OUT_OF_MEMORY;
	}
	
	memset(mdlins,0,sizeof(tmdlins)*16*256);
	memset(mdlsamp,0,sizeof(tmdlsamp)*256);
	memset(trackseq,0,256*32*4);
	
	header->ver=sig[4];
	
	mp_ubyte blockhead[6];
	
	while (f.read(&blockhead,1,6)) {
		
		mp_ubyte id[3];
		memcpy(id,blockhead,2);
		id[2]=0;
		mp_uint32 blocklen = LittleEndian::GET_DWORD(blockhead+2);
		
		if (!memcmp(&blockhead,"IN",2)) {
			f.read(&header->name,1,32);
			f.read(&dummy,1,20);
			mp_uword ordnum = 0;
			f.readWords(&ordnum,1);
			if(ordnum > MP_MAXORDERS) {
				return MP_LOADER_FAILED;
			}
			header->ordnum = ordnum;
			f.readWords(&header->restart,1);
			header->mainvol = f.readByte();
			header->tempo = f.readByte();
			header->speed = f.readByte();
			f.read(&dummy,1,32);
			mp_uint32 c=32;
			while ((dummy[c-1]&128)) c--;
			header->channum=c;
			
			for (i=0;i<c;i++) header->pan[i]=cpan(dummy[i]&127);
			
			f.read(&header->ord,1,header->ordnum);
			for (i=0;i<c;i++) 
				f.read(&dummy,1,8);
		}
		else if (!memcmp(&blockhead,"PA",2)) {
			header->patnum = f.readByte();
			for (mp_sint32 p=0;p<header->patnum;p++) {
				mp_ubyte numch = 0;
				mp_sint32 numrows=0;
				
				if (hVer == 1)
				{
					numch = f.readByte();
					numrows = f.readByte();
					f.read(&dummy,1,16);
				}
				else if (hVer == 0)
				{
					numch = 32;
					numrows = 63;
				}
				
				for (mp_sint32 c=0;c<numch;c++) {
					mp_uint32 tr=0;
					tr = f.readWord();
					trackseq[p*32+c]=tr;
				}
				
				//if ((mp_uword)numch>header->channum) numch=(char)header->channum;
				numch=(mp_ubyte)header->channum;
				
				numrows++;
				
				phead[p].rows=numrows;
				phead[p].effnum=3;
				phead[p].channum=numch < (mp_ubyte)header->channum ? numch : (mp_ubyte)header->channum;
				phead[p].patternData=new mp_ubyte[phead[p].rows*phead[p].channum*8];
				
				// out of memory?
				if (phead[p].patternData == NULL)
				{
					if (mdlins) delete[] mdlins;
					if (mdlsamp) delete[] mdlsamp;
					if (trackseq) delete[] trackseq;
					
					return MP_OUT_OF_MEMORY;
				}
				
				memset(phead[p].patternData,0,phead[p].rows*phead[p].channum*8);
				
			}
		}
		else if (!memcmp(&blockhead,"TR",2)) {
			mp_ubyte *buffer = new mp_ubyte[2048];
			numtracks = f.readWord();
			tracks = new mp_ubyte[(mp_uint32)numtracks*256*6];
			
			if (buffer == NULL || tracks == NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (buffer) delete[] buffer;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			
			for (mp_uint32 t=0;t<numtracks;t++) {
				mp_uint32 tracksize = f.readWord();
				memset(buffer,0,2048);
				f.read(buffer,1,tracksize);
				//tracks[t]=new mp_ubyte[256*6];
				
				memset(tracks+t*256*6,0,256*6);
				
				mp_ubyte *track = tracks+t*256*6;
				//mp_ubyte *track = tracks[t];
				
				mp_uint32 d=0,p=0;
				while (d<tracksize) {
					mp_ubyte b=buffer[d++];
					mp_uint32 pb=b&3;
					mp_uint32 op=b>>2;
					
					switch (pb) {
						case 0:	{
							for (i=0;i<(op+1);i++) {
								memset(track+(p*6),0,6);
								p++;
							}
							break;
						}; 
						case 1: {
							mp_sint32 lp=p-1;
							for (i=0;i<(op+1);i++) {
								memcpy(track+(p*6),track+(lp*6),6);
								p++;
							}
							break;
						}; 
						case 2: {
							memcpy(track+(p*6),track+(op*6),6);
							p++;
							break;
						}; 
						case 3: {
							if ((op&1)) track[p*6]=buffer[d++];
							if ((op&2)) track[p*6+1]=buffer[d++];
							if ((op&4)) track[p*6+2]=buffer[d++];
							if ((op&8)) track[p*6+3]=buffer[d++];
							if ((op&16)) track[p*6+4]=buffer[d++];
							if ((op&32)) track[p*6+5]=buffer[d++];
							p++;
							break;
						}; 
					}
				}
				
				// something went wrong here
				if (d!=tracksize) {
					if (mdlins) delete[] mdlins;
					if (mdlsamp) delete[] mdlsamp;
					if (trackseq) delete[] trackseq;
					if (buffer) delete[] buffer;
					if (tracks) delete[] tracks;
					
					return MP_LOADER_FAILED;
				}
				
			}
			delete[] buffer;
		}
		else if (!memcmp(&blockhead,"II",2)) {
			mp_uint32 insnum = 0,numsamples = 0;
			numins = f.readByte();
			for (i=0;i<numins;i++) {
				insnum = f.readByte(); 
				numsamples = f.readByte();
				
				if (!insnum) {
					if (mdlins) delete[] mdlins;
					if (mdlsamp) delete[] mdlsamp;
					if (trackseq) delete[] trackseq;

					return MP_LOADER_FAILED;
				}
				
				f.read(&instr[insnum-1].name,1,32);
				
				instr[insnum-1].samp=numsamples;
				
				for (mp_uint32 s=0;s<numsamples;s++) {
					mdlins[(insnum-1)*16+s].sampnum = f.readByte();
					mdlins[(insnum-1)*16+s].playrangeend = f.readByte();
					mdlins[(insnum-1)*16+s].vol = f.readByte();
					mdlins[(insnum-1)*16+s].volenv = f.readByte();
					mdlins[(insnum-1)*16+s].pan = f.readByte();
					mdlins[(insnum-1)*16+s].panenv = f.readByte();
					mdlins[(insnum-1)*16+s].volfade = f.readWord();
					mdlins[(insnum-1)*16+s].vibspeed = f.readByte();
					mdlins[(insnum-1)*16+s].vibspeed = f.readByte();
					mdlins[(insnum-1)*16+s].vibsweep = f.readByte();
					mdlins[(insnum-1)*16+s].vibform = f.readByte();
					mdlins[(insnum-1)*16+s].reserved = f.readByte();
					mdlins[(insnum-1)*16+s].freqenv = f.readByte();
				}
			}
			header->insnum=insnum;
		}
		else if (!memcmp(&blockhead,"IS",2)) {
			numsamples = f.readByte();
			for (mp_uint32 s=0;s<numsamples;s++) {
				mdlsamp[s].sampnum = f.readByte();
				f.read(&mdlsamp[s].smpname,1,32);
				
				f.read(&mdlsamp[s].filename,1,8);

 				if (hVer == 1)
					mdlsamp[s].c4spd = f.readDword();
				else if (hVer == 0)
					mdlsamp[s].c4spd = f.readWord();
					
				mdlsamp[s].samplen = f.readDword();
				mdlsamp[s].loopstart = f.readDword();
				mdlsamp[s].looplen = f.readDword();
				mdlsamp[s].reserved = f.readByte();
				mdlsamp[s].infobyte = f.readByte();
			}
		}
		else if (!memcmp(&blockhead,"SA",2)) {
			for (mp_uint32 s=0;s<numsamples;s++) {
				
				mp_ubyte pb = (mdlsamp[s].infobyte>>2)&3;
				
				switch (pb) {
					case 0:	{
						if (!(mdlsamp[s].infobyte&1)) {
							mdlsamp[s].smp = (mp_sbyte*)module->allocSampleMem(mdlsamp[s].samplen);
							
							// out of memory
							if (mdlsamp[s].smp==NULL)
							{
								if (mdlins) delete[] mdlins;
								if (mdlsamp) delete[] mdlsamp;
								if (trackseq) delete[] trackseq;
								if (tracks) delete[] tracks;
								
								return MP_OUT_OF_MEMORY;
							}
							
							if (!module->loadSample(f,mdlsamp[s].smp,mdlsamp[s].samplen,mdlsamp[s].samplen))
							{
								if (mdlins) delete[] mdlins;
								if (mdlsamp) delete[] mdlsamp;
								if (trackseq) delete[] trackseq;
								if (tracks) delete[] tracks;
								
								return MP_OUT_OF_MEMORY;
							}
							
						}
						else {
							mdlsamp[s].smp = (mp_sbyte*)module->allocSampleMem(mdlsamp[s].samplen);
							
							// out of memory
							if (mdlsamp[s].smp==NULL)
							{
								if (mdlins) delete[] mdlins;
								if (mdlsamp) delete[] mdlsamp;
								if (trackseq) delete[] trackseq;
								if (tracks) delete[] tracks;
								
								return MP_OUT_OF_MEMORY;
							}
							
							if (!module->loadSample(f,mdlsamp[s].smp,mdlsamp[s].samplen,mdlsamp[s].samplen>>1,XModule::ST_16BIT))
							{
								if (mdlins) delete[] mdlins;
								if (mdlsamp) delete[] mdlsamp;
								if (trackseq) delete[] trackseq;
								if (tracks) delete[] tracks;
								
								return MP_OUT_OF_MEMORY;
							}
							
							//mp_uint32 samplen = mdlsamp[s].samplen>>1;
							//mp_uint32 loopstart = mdlsamp[s].loopstart>>1;
							//mp_uint32 looplen = mdlsamp[s].looplen>>1;
						}
					}; break;
					case 1: {
						mp_sint32 size = (mp_sint32)f.readDword();
						
						mdlsamp[s].smp = (mp_sbyte*)module->allocSampleMem(mdlsamp[s].samplen);
						
						// out of memory
						if (mdlsamp[s].smp==NULL)
						{
							if (mdlins) delete[] mdlins;
							if (mdlsamp) delete[] mdlsamp;
							if (trackseq) delete[] trackseq;
							if (tracks) delete[] tracks;
							
							return MP_OUT_OF_MEMORY;
						}
						
						if (!module->loadSample(f,mdlsamp[s].smp,size,mdlsamp[s].samplen,XModule::ST_PACKING_MDL))
						{
							if (mdlins) delete[] mdlins;
							if (mdlsamp) delete[] mdlsamp;
							if (trackseq) delete[] trackseq;
							if (tracks) delete[] tracks;
							
							return MP_OUT_OF_MEMORY;								
						}
						
					}; break;
					case 2: {
						mp_sint32 size = (mp_sint32)f.readDword();
						
						mdlsamp[s].smp = (mp_sbyte*)module->allocSampleMem(mdlsamp[s].samplen);					
						
						// out of memory
						if (mdlsamp[s].smp==NULL)
						{
							if (mdlins) delete[] mdlins;
							if (mdlsamp) delete[] mdlsamp;
							if (trackseq) delete[] trackseq;
							if (tracks) delete[] tracks;
							
							return MP_OUT_OF_MEMORY;
						}
						
						mp_uint32 samplen = mdlsamp[s].samplen>>1;
						//mp_uint32 loopstart = mdlsamp[s].loopstart>>1;
						//mp_uint32 looplen = mdlsamp[s].looplen>>1;
						
						if (!module->loadSample(f,mdlsamp[s].smp,size,samplen,XModule::ST_PACKING_MDL | XModule::ST_16BIT))
						{
							if (mdlins) delete[] mdlins;
							if (mdlsamp) delete[] mdlsamp;
							if (trackseq) delete[] trackseq;
							if (tracks) delete[] tracks;
							
							return MP_OUT_OF_MEMORY;								
						}
																								
					}; break;
				}
			}
		}
		else if (!memcmp(&blockhead,"VE",2)) {
			mp_uint32 numenvs = f.readByte();
			
			mp_ubyte *envelopes = new mp_ubyte[numenvs*33];
			
			// out of memory
			if (envelopes==NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			
			f.read(envelopes,33,numenvs);
			
			mp_ubyte *env=envelopes;

			mp_ubyte num;
			mp_uint32 envnum = 0;

			mp_uint32 lastEnv = 0;
			for (e=0;e<numenvs;e++) 
			{
				envnum=env[e*33];
				if (envnum>lastEnv)
					lastEnv = envnum;
			}					
					
			module->venvs = new TEnvelope[lastEnv+1];
			if (module->venvs == NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			memset(module->venvs, 0, sizeof(TEnvelope)*(lastEnv+1));
			module->numVEnvsAlloc = lastEnv+1;
			module->numVEnvs = lastEnv+1;
				
			for (e=0;e<numenvs;e++) {
				envnum=env[0];
				
				ASSERT(envnum<lastEnv+1);
				
				num=0;
				mp_uint32 x=0;
				mp_uint32 y;
				while (env[((mp_uint32)num*2)+1]&&(num<15)) {
					x+=env[((mp_uint32)num*2)+1];
					y=(((mp_uint32)env[((mp_uint32)num*2)+2])*266306)>>16;
					module->venvs[envnum].env[num][0]=x-1;
					module->venvs[envnum].env[num][1]=y;
					num++;
				}
				module->venvs[envnum].num=num;
				module->venvs[envnum].sustain=env[31]&0xf;
				module->venvs[envnum].loops=env[32]&0xf;
				module->venvs[envnum].loope=env[32]>>4;
				
				module->venvs[envnum].type=1;
				if ((env[31]&16)) module->venvs[envnum].type|=2;
				if ((env[31]&32)) module->venvs[envnum].type|=4;
				
				env+=33;
			}
			
			header->volenvnum=envnum+1;
			
			delete[] envelopes;
			
		}
		else if (!memcmp(&blockhead,"PE",2)) {
			mp_uint32 numenvs = f.readByte();
			
			mp_ubyte *envelopes = new mp_ubyte[numenvs*33];
			
			// out of memory
			if (envelopes==NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			
			f.read(envelopes,33,numenvs);
			
			mp_ubyte *env=envelopes;
			
			mp_ubyte num;
			mp_uint32 envnum = 0;

			mp_uint32 lastEnv = 0;
			for (e=0;e<numenvs;e++) 
			{
				envnum=env[e*33];
				if (envnum>lastEnv)
					lastEnv = envnum;
			}					
			
			module->penvs = new TEnvelope[lastEnv+1];
			if (module->penvs == NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			memset(module->penvs, 0, sizeof(TEnvelope)*(lastEnv+1));
			module->numPEnvsAlloc = lastEnv+1;
			module->numPEnvs = lastEnv+1;			
			
			for (e=0;e<numenvs;e++) {
				envnum=env[0];
				
				ASSERT(envnum<lastEnv+1);
				
				num=0;
				mp_uint32 x=0;
				mp_uint32 y;
				while (env[((mp_uint32)num*2)+1]&&(num<15)) {
					x+=env[((mp_uint32)num*2)+1];
					y=(((mp_uint32)env[((mp_uint32)num*2)+2])*266306)>>16;
					module->penvs[envnum].env[num][0]=x-1;
					module->penvs[envnum].env[num][1]=y;
					num++;
				}
				module->penvs[envnum].num=num;
				module->penvs[envnum].sustain=env[31]&0xf;
				module->penvs[envnum].loops=env[32]&0xf;
				module->penvs[envnum].loope=env[32]>>4;
				
				module->penvs[envnum].type=1;
				if ((env[31]&16)) module->penvs[envnum].type|=2;
				if ((env[31]&32)) module->penvs[envnum].type|=4;
				
				env+=33;
			}
			
			header->panenvnum=envnum+1;
			
			delete[] envelopes;
			
		}
		else if (!memcmp(&blockhead,"FE",2)) {
			mp_uint32 numenvs = f.readByte();			
			
			mp_ubyte *envelopes = new mp_ubyte[numenvs*33];
			
			// out of memory
			if (envelopes==NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			
			f.read(envelopes,33,numenvs);
			
			mp_ubyte *env=envelopes;
			
			mp_ubyte num;
			mp_uint32 envnum = 0;

			mp_uint32 lastEnv = 0;
			for (e=0;e<numenvs;e++) 
			{
				envnum=env[e*33];
				if (envnum>lastEnv)
					lastEnv = envnum;
			}
			
			module->fenvs = new TEnvelope[lastEnv+1];
			if (module->fenvs == NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			memset(module->fenvs, 0, sizeof(TEnvelope)*(lastEnv+1));
			module->numFEnvsAlloc = lastEnv+1;
			module->numFEnvs = lastEnv+1;						
			
			for (e=0;e<numenvs;e++) {
				envnum=env[0];
				
				ASSERT(envnum<lastEnv+1);
				
				num=0;
				mp_uint32 x=0;
				mp_uint32 y;
				while (env[((mp_uint32)num*2)+1]&&(num<15)) {
					x+=env[((mp_uint32)num*2)+1];
					y=(((mp_uint32)env[((mp_uint32)num*2)+2])*266306)>>16;
					module->fenvs[envnum].env[num][0]=x-1;
					module->fenvs[envnum].env[num][1]=y;
					num++;
				}
				module->fenvs[envnum].num=num;
				module->fenvs[envnum].sustain=env[31]&0xf;
				module->fenvs[envnum].loops=env[32]&0xf;
				module->fenvs[envnum].loope=env[32]>>4;
				
				module->fenvs[envnum].type=1;
				if ((env[31]&16)) module->fenvs[envnum].type|=2;
				if ((env[31]&32)) module->fenvs[envnum].type|=4;
				
				env+=33;
			}
			
			header->frqenvnum=envnum+1;
			
			delete[] envelopes;
			
		}
		else if (!memcmp(&blockhead,"ME",2)) {
			
			// MDL doc says song message is always 0 terminated,
			// well allocate one more byte to make sure it's really, really 0-terminated
			module->allocateSongMessage(blocklen+1);
			
			if (module->message == NULL)
			{
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				return MP_OUT_OF_MEMORY;
			}

			// read song message
			f.read(module->message, 1, blocklen);			
		}
		else {
			/*mp_ubyte *buffer = new mp_ubyte[blocklen];
			
			if (buffer == NULL)
			{
				// out of memory
				if (mdlins) delete[] mdlins;
				if (mdlsamp) delete[] mdlsamp;
				if (trackseq) delete[] trackseq;
				if (tracks) delete[] tracks;
				
				return MP_OUT_OF_MEMORY;
			}
			
			f.read(buffer,1,blocklen);
			
			delete[] buffer;*/
			
			f.seekWithBaseOffset(f.posWithBaseOffset() + blocklen);
		}
		
	}
	
	// ---------------------------------------------------------------
	//                   build new song structure
	// ---------------------------------------------------------------

	if (hVer == 1)
	{
	
		// create static envelope for samples samples without volume envelope
		TEnvelope venv;
		
		//venvs[header->volenvnum].type=5;
		//venvs[header->volenvnum].num=2;
		//venvs[header->volenvnum].loops=0;
		//venvs[header->volenvnum].loope=1;
		//venvs[header->volenvnum].env[0][0]=0;
		//venvs[header->volenvnum].env[0][1]=256;
		//venvs[header->volenvnum].env[1][0]=128;
		//venvs[header->volenvnum].env[1][1]=256;
		
		venv.type=5;
		venv.num=2;
		venv.loops=0;
		venv.loope=1;
		venv.env[0][0]=0;
		venv.env[0][1]=256;
		venv.env[1][0]=128;
		venv.env[1][1]=256;
		
		if (!module->addVolumeEnvelope(venv))
		{
			if (mdlins) delete[] mdlins;
			if (mdlsamp) delete[] mdlsamp;
			if (trackseq) delete[] trackseq;
			if (tracks) delete[] tracks;
			
			return MP_OUT_OF_MEMORY;
		}
		
		header->volenvnum++;
		
		mp_uint32 sampcnt=0;
		for (i=0;i<header->insnum;i++) {
			//cprintf("%i\r\n",instr[i].samp);
			if (instr[i].samp) {
				mp_sint32 bs=0;
				for (mp_uint32 s=0;s<instr[i].samp;s++) {
					//cprintf("%i, %i\r\n",bs,mdlins[i*16+s].playrangeend);
					//getch();
					mp_uint32 l;
					for (l=bs;l<=mdlins[i*16+s].playrangeend;l++) instr[i].snum[l]=sampcnt;
					bs=l;
					for (l=0;l<numsamples;l++) {
						if (mdlins[i*16+s].sampnum==mdlsamp[l].sampnum) {
							smp[sampcnt].samplen=mdlsamp[l].samplen;
							smp[sampcnt].loopstart=mdlsamp[l].loopstart;
							smp[sampcnt].looplen=mdlsamp[l].looplen;
							
							if ((mdlins[i*16+s].volenv&64)) smp[sampcnt].flags|=1;
							if ((mdlins[i*16+s].panenv&64)) smp[sampcnt].flags|=2;
							
							if ((mdlins[i*16+s].volenv&128)) smp[sampcnt].venvnum=(mdlins[i*16+s].volenv&63)+1;
							else smp[sampcnt].venvnum=header->volenvnum;
							if ((mdlins[i*16+s].panenv&128)) smp[sampcnt].penvnum=(mdlins[i*16+s].panenv&63)+1;
							if ((mdlins[i*16+s].freqenv&128)) smp[sampcnt].fenvnum=(mdlins[i*16+s].freqenv&63)+1;
							
							smp[sampcnt].vol=cvol(mdlins[i*16+s].vol);
							XModule::convertc4spd(mdlsamp[l].c4spd,&smp[sampcnt].finetune,&smp[sampcnt].relnote);
							if (mdlsamp[l].looplen&&(mdlsamp[l].infobyte&2)) smp[sampcnt].type=2;
							else if (mdlsamp[l].looplen&&(!(mdlsamp[l].infobyte&2))) smp[sampcnt].type=1;
							
							if ((mdlsamp[l].infobyte&1)) {
								smp[sampcnt].samplen>>=1;
								smp[sampcnt].loopstart>>=1;
								smp[sampcnt].looplen>>=1;
								smp[sampcnt].type|=16;
							}
							
							smp[sampcnt].pan=cpan(mdlins[i*16+s].pan);
							smp[sampcnt].vibtype=mdlins[i*16+s].vibform;
							smp[sampcnt].vibsweep=mdlins[i*16+s].vibsweep;
							smp[sampcnt].vibdepth=mdlins[i*16+s].vibdepth;
							smp[sampcnt].vibrate=mdlins[i*16+s].vibspeed;
							smp[sampcnt].volfade=mdlins[i*16+s].volfade;
							smp[sampcnt].sample=mdlsamp[l].smp;
							memcpy(&smp[sampcnt].name,&mdlsamp[l].smpname,32);
							
							sampcnt++;		  
							continue;
						}
					}
				}
			}
		}
		
		header->smpnum=sampcnt;
		
	}
	else if (hVer == 0)
	{

		header->insnum = 0;

		for (mp_uint32 s = 0; s < numsamples; s++)
		{
			if(mdlsamp[s].sampnum == 0)
				continue;
			i = mdlsamp[s].sampnum - 1;
		
			if ((i+1) > header->insnum)
				header->insnum = i + 1;
					
			instr[i].samp = 1;
			
			for (mp_sint32 j = 0; j < 120; j++)
				instr[i].snum[j] = s;
			
			memcpy(instr[i].name, mdlsamp[s].smpname, 32);
			memcpy(smp[s].name, mdlsamp[s].filename, 8);
			
			XModule::convertc4spd(mdlsamp[s].c4spd,&smp[s].finetune,&smp[s].relnote);

			smp[s].vol=cvol(mdlsamp[s].reserved);

			smp[s].flags = 1;

			smp[s].samplen = mdlsamp[s].samplen;
			smp[s].loopstart = mdlsamp[s].loopstart;
			smp[s].looplen= mdlsamp[s].looplen;

			if (mdlsamp[s].looplen&&(mdlsamp[s].infobyte&2)) smp[s].type=2;
			else if (mdlsamp[s].looplen&&(!(mdlsamp[s].infobyte&2))) smp[s].type=1;
							
			if ((mdlsamp[s].infobyte&1)) {
				smp[s].samplen>>=1;
				smp[s].loopstart>>=1;
				smp[s].looplen>>=1;
				smp[s].type|=16;
			}
			
			smp[s].sample = mdlsamp[s].smp;
							
				/*mdlsamp[s].samplen = f.readDword();
				mdlsamp[s].loopstart = f.readDword();
				mdlsamp[s].looplen = f.readDword();
				mdlsamp[s].reserved = f.readByte();
				mdlsamp[s].infobyte = f.readByte();*/
		}

		header->smpnum = numsamples;

	}
	
	for (i=0;i<header->patnum;i++) {
		
		mp_ubyte* pattern = phead[i].patternData;
		mp_uint32 numrows = phead[i].rows;
		mp_uint32 numch = phead[i].channum;
		
		for (mp_uint32 r=0;r<numrows;r++) {
			
			for (mp_uint32 c=0;c<numch;c++) {
				
				mp_uint32 tnum = trackseq[i*32+c];
				if (tnum) {
					
					mp_ubyte *track=tracks+((tnum-1)*256*6)+r*6;
					//mp_ubyte *track=tracks[tnum-1]+r*6;
					
					mp_uint32 pos=r*(numch*8)+(c*8);
					
					if (track[0]<=120) pattern[pos]=track[0];
					else pattern[pos]=XModule::NOTE_OFF;
					
					pattern[pos+1]=track[1];
					if (track[2]) {
						if (pattern[pos] != XModule::NOTE_OFF)
						{
							pattern[pos+2]=0xC;
							pattern[pos+3]=cvol(track[2]);
						}
					}
					
					mp_ubyte eff1 = track[3]&0xf;
					mp_ubyte eff2 = track[3]>>4;
					
					switch (eff1) {
						case 0x1 : {
							pattern[pos+4]=0x43;
							pattern[pos+5]=track[4];
						}; break;
						case 0x2 : {
							pattern[pos+4]=0x44;
							pattern[pos+5]=track[4];
						}; break;
						case 0x3 : {
							pattern[pos+4]=0x03;
							pattern[pos+5]=track[4];
						}; break;
						case 0x4 : {
							pattern[pos+4]=0x04;
							pattern[pos+5]=track[4];
						}; break;
						case 0x5 : {
							pattern[pos+4]=0x20;
							pattern[pos+5]=track[4];
						}; break;
						case 0x7 : {
							pattern[pos+4]=0x16;
							pattern[pos+5]=track[4];
						}; break;
						case 0x8 : {
							pattern[pos+4]=0x08;
							pattern[pos+5]=cpan(track[4]);
						}; break;
						case 0xB : {
							pattern[pos+4]=0x0B;
							pattern[pos+5]=track[4];
						}; break;
						case 0xC : {
							pattern[pos+4]=0x10;
							pattern[pos+5]=cvol(track[4]);
						}; break;
						case 0xD : {
							pattern[pos+4]=0x0D;
							pattern[pos+5]=track[4];
						}; break;
						case 0xE : {
							switch (track[4]>>4) {
								case 0x1 : {
									pattern[pos+4]=0x1E;
									pattern[pos+5]=track[4];
								}; break;
								case 0x2 : {
									pattern[pos+4]=0x1E;
									pattern[pos+5]=track[4];
								}; break;
								case 0x6 : {
									pattern[pos+4]=0x36;
									pattern[pos+5]=track[4]&0xf;
								}; break;
								case 0x7 : {
									pattern[pos+4]=0x37;
									pattern[pos+5]=track[4]&0xf;
								}; break;
								case 0x9 : {
									pattern[pos+4]=0x39;
									pattern[pos+5]=track[4]&0xf;
								}; break;
								case 0xA : {
									pattern[pos+4]=0x1E;
									pattern[pos+5]=track[4];
								}; break;
								case 0xB : {
									pattern[pos+4]=0x1E;
									pattern[pos+5]=track[4];
								}; break;
								case 0xC : {
									pattern[pos+4]=0x3C;
									pattern[pos+5]=track[4]&0xf;
								}; break;
								case 0xD : {
									pattern[pos+4]=0x3D;
									pattern[pos+5]=track[4]&0xf;
								}; break;
								case 0xE : {
									pattern[pos+4]=0x3E;
									pattern[pos+5]=track[4]&0xf;
								}; break;
								case 0xF : {
									pattern[pos+4]=0x1F;
									pattern[pos+5]=track[5];
									pattern[pos+6]=0;
									pattern[pos+7]=track[4]&0xf;
								}; break;
							}
						}; break;
						case 0xF : {
							pattern[pos+4]=0x1C;
							pattern[pos+5]=track[4];
						}; break;
						default  : {
							if (eff1) {
								//cprintf("Eff1: %i, %i\r\n",eff1,track[4]);
								//getch();
							}
						}; break;
					}
					
					switch (eff2) {
						case 0x1 : {
							pattern[pos+6]=0x45;
							pattern[pos+7]=track[5];
						}; break;
						case 0x2 : {
							pattern[pos+6]=0x46;
							pattern[pos+7]=track[5];
						}; break;
						case 0x3 : {
							pattern[pos+6]=0x1B;
							pattern[pos+7]=track[5];
						}; break;
						case 0x4 : {
							pattern[pos+6]=0x07;
							pattern[pos+7]=track[5];
						}; break;
						case 0x7 : {
							pattern[pos+6]=0x16;
							pattern[pos+7]=track[5];
						}; break;
						case 0x8 : {
							pattern[pos+6]=0x08;
							pattern[pos+7]=cpan(track[5]);
						}; break;
						case 0xB : {
							pattern[pos+6]=0x0B;
							pattern[pos+7]=track[5];
						}; break;
						case 0xC : {
							pattern[pos+6]=0x10;
							pattern[pos+7]=cvol(track[5]);
						}; break;
						case 0xD : {
							pattern[pos+6]=0x0D;
							pattern[pos+7]=track[5];
						}; break;
						case 0xE : {
							switch (track[5]>>4) {
								case 0x1 : {
									pattern[pos+6]=0x1E;
									pattern[pos+7]=track[5];
								}; break;
								case 0x2 : {
									pattern[pos+6]=0x1E;
									pattern[pos+7]=track[5];
								}; break;
								case 0x6 : {
									pattern[pos+6]=0x36;
									pattern[pos+7]=track[5]&0xf;
								}; break;
								case 0x7 : {
									pattern[pos+6]=0x37;
									pattern[pos+7]=track[5]&0xf;
								}; break;
								case 0x9 : {
									pattern[pos+6]=0x39;
									pattern[pos+7]=track[5]&0xf;
								}; break;
								case 0xA : {
									pattern[pos+6]=0x1E;
									pattern[pos+7]=track[5];
								}; break;
								case 0xB : {
									pattern[pos+6]=0x1E;
									pattern[pos+7]=track[5];
								}; break;
								case 0xC : {
									pattern[pos+6]=0x3C;
									pattern[pos+7]=track[5]&0xf;
								}; break;
								case 0xD : {
									pattern[pos+6]=0x3D;
									pattern[pos+7]=track[5]&0xf;
								}; break;
								case 0xE : {
									pattern[pos+6]=0x3E;
									pattern[pos+7]=track[5]&0xf;
								}; break;
								case 0xF : {
									pattern[pos+4]=0x1F;
									pattern[pos+5]=track[4];
									pattern[pos+6]=0;
									pattern[pos+7]=track[5]&0xf;
								}; break;
							}
						}; break;
						case 0xF : {
							pattern[pos+6]=0x1C;
							pattern[pos+7]=track[5];
						}; break;
						default  : {
							if (eff2) {
								//cprintf("Eff2: %i, %i\r\n",eff2,track[5]);
								//getch();
							}
						}; break;
							
					}
					
				}
				
			}
			
		}
		
	}
	
	//cprintf("%i\r\n",phead[1].channum);
	
	//for (mp_sint32 r=0;r<64;r++) {
	//mp_sint32 o=(24*8)*r-2;
	//for (i=15;i<16;i++) {
	//	cprintf("n: %x, i: %x\r\n",phead[1].pattern[i*8+6+o],phead[1].pattern[i*8+7+o]);
	//   getch();
	//}
	//}

	delete[] tracks;
	delete[] trackseq;
	delete[] mdlins;
	delete[] mdlsamp;

	module->postProcessSamples();

	return MP_OK;
}
