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
 *  PlayerSTD.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on Tue Oct 19 2004.
 *
 */
#ifndef __PLAYERSTD_H__
#define __PLAYERSTD_H__

#include "ChannelMixer.h"
#include "PlayerBase.h"
#include "XModule.h"

class PlayerSTD : public PlayerBase
{
public:
	struct StatusEventListener
	{
		virtual ~StatusEventListener() { }

		virtual void playerTickStarted(PlayerSTD& player, XModule& module) { }
		virtual void playerTickEnded(PlayerSTD& player, XModule& module) { }
		virtual void timerTickStarted(PlayerSTD& player, XModule& module) { }
		virtual void timerTickEnded(PlayerSTD& player, XModule& module) { }
		virtual void patternEndReached(PlayerSTD& player, XModule& module, mp_sint32& newOrderIndex) { }
	};

private:
	enum
	{
		XM_MINPERIOD = 50
	};

	struct TPrEnv 
	{
		TEnvelope*	envstruc;
		mp_sint32   a,b,step;
		mp_uint32	bpmCounter, bpmAdder;

		mp_uint32	timeTrackSize;
		
		struct TTimeRecord
		{
			mp_sword pos;
			const TEnvelope* envstruc;
		};
		
		TTimeRecord* timeRecord;
		
		TPrEnv() :
			timeTrackSize(0),
			timeRecord(NULL)
		{
		}
		
		~TPrEnv()
		{
			delete[] timeRecord;
		}
		
		void clear()
		{
			envstruc	= 0;
			a = b = step = 0;
			bpmCounter = bpmAdder = 0;
			
			if (timeTrackSize && timeRecord)
				memset(timeRecord, 0, sizeof(TTimeRecord)*timeTrackSize);
		}
		
		void reallocTimeRecord(mp_uint32 size)
		{
			timeTrackSize = size;
			delete[] timeRecord;
			timeRecord = new TTimeRecord[size];
		}
	};

	struct TLastOperands
	{
		mp_ubyte portaup;
		mp_ubyte portadown;
		mp_ubyte portanote;
		mp_ubyte fineportaup;
		mp_ubyte fineportadown;
		mp_ubyte xfineportaup;
		mp_ubyte xfineportadown;
		mp_ubyte volslide;
		mp_ubyte finevolslide;
		mp_ubyte gvolslide;
		mp_ubyte panslide;
		mp_ubyte arpeg;
		mp_ubyte retrig;
		mp_ubyte tremor;
		mp_ubyte smpoffset;
	};

	struct TModuleChannel 
	{
		mp_uint32		flags;
		mp_sint32		ins;
		mp_sint32		smp;
		bool			hasSetVolume;
		mp_sint32		vol, tremoloVol, finalTremoloVol, tremorVol;
		bool			hasTremolo;
		mp_sint32		masterVol;
		mp_sint32		pan;
		mp_sint32		per, finalVibratoPer, destper;
		bool			hasVibrato;
		//mp_sint32 c4spd;
		mp_sint32		currentnote, relnote;
		mp_sint32		finetune;
		mp_sword		freqadjust;
		mp_sint32		note, destnote, lastnoportanote;
		bool			validnote;
		mp_ubyte		eff[MP_NUMEFFECTS];
		mp_ubyte		eop[MP_NUMEFFECTS];
		TLastOperands	old[MP_NUMEFFECTS];

		mp_sint32		loopstart;
		mp_sint32		execloop;
		mp_sint32		loopcounter;
		bool			isLooping;
		mp_sint32		loopingValidPosition;

		mp_ubyte		vibdepth[MP_NUMEFFECTS];
		mp_ubyte		vibspeed[MP_NUMEFFECTS];
		mp_ubyte		vibpos[MP_NUMEFFECTS];
		mp_ubyte		trmdepth[MP_NUMEFFECTS];
		mp_ubyte		trmspeed[MP_NUMEFFECTS];
		mp_ubyte		trmpos[MP_NUMEFFECTS];
		mp_ubyte		tremorcnt[MP_NUMEFFECTS];
		mp_sint32		retrigcounterE9x[MP_NUMEFFECTS];
		mp_ubyte		retrigmaxE9x[MP_NUMEFFECTS];
		mp_sint32		retrigcounterRxx[MP_NUMEFFECTS];
		mp_ubyte		retrigmaxRxx[MP_NUMEFFECTS];

		bool			keyon;
		TPrEnv			venv;
		TPrEnv			penv;
		TPrEnv			fenv;
		TPrEnv			vibenv;
		mp_sint32		fadevolstart;
		mp_sint32		fadevolstep;
		mp_ubyte		avibused;
		mp_ubyte		avibspd;
		mp_ubyte		avibdepth;
		mp_ubyte		avibcnt;
		mp_ubyte		avibsweep;
		mp_ubyte		avibswcnt;
		
		void clear()
		{
			flags = 0;
			ins = 0;
			smp = 0;
			hasSetVolume = 0;
			vol = tremoloVol = finalTremoloVol = tremorVol = 0;
			hasTremolo = false;
			masterVol = 0;
			pan = 0;
			per = finalVibratoPer = destper = 0;
			hasVibrato = 0;
			currentnote = relnote = 0;
			finetune = 0;
			freqadjust = 0;
			note = destnote = lastnoportanote = 0;
			validnote = false;
			memset(&eff, 0, sizeof(eff));
			memset(&eop, 0, sizeof(eop));
			memset(&old, 0, sizeof(old));

			loopstart = 0;
			execloop = 0;
			loopcounter = 0;
			isLooping = false;
			loopingValidPosition = 0;

			memset(&vibdepth, 0, sizeof(vibdepth));
			memset(&vibpos, 0, sizeof(vibpos));
			memset(&trmdepth, 0, sizeof(trmdepth));
			memset(&trmspeed, 0, sizeof(trmspeed));
			memset(&trmpos, 0, sizeof(trmpos));
			memset(&tremorcnt, 0, sizeof(tremorcnt));
			memset(&retrigcounterE9x, 0, sizeof(retrigcounterE9x));
			
			memset(&retrigmaxE9x, 0, sizeof(retrigmaxE9x));
			memset(&retrigcounterRxx, 0, sizeof(retrigcounterRxx));
			memset(&retrigmaxRxx, 0, sizeof(retrigmaxRxx));
			
			keyon = false;
			venv.clear();
			penv.clear();
			fenv.clear();
			vibenv.clear();

			fadevolstart = 0;
			fadevolstep = 0;
			avibused = 0;
			avibspd = 0;
			avibdepth = 0;
			avibcnt = 0;
			avibsweep = 0;
			avibswcnt = 0;
		}
		
		void reallocTimeRecord(mp_uint32 size)
		{
			venv.reallocTimeRecord(size);
			penv.reallocTimeRecord(size);
			fenv.reallocTimeRecord(size);
			vibenv.reallocTimeRecord(size);			
		}
	};
	
private:

	static const mp_sint32	vibtab[32];
	static const mp_uword	lintab[769];
	static const mp_uint32	logtab[];
	
	StatusEventListener* statusEventListener;
	
	TModuleChannel*	chninfo;				// our channel information
	mp_sint32		lastNumAllocatedChannels;
	
	mp_uint32*		smpoffs;	
	mp_ubyte*		attick;
	
	mp_sint32		patternIndex;			// holds current pattern index
	mp_sint32		numEffects;				// current number of effects
	mp_sint32		numChannels;			// current number of channels
	
	mp_ubyte		pbreak;
	mp_ubyte		pbreakpos;
	mp_sint32		pbreakPriority;
	mp_ubyte		pjump;
	mp_ubyte		pjumppos,pjumprow;
	mp_sint32		pjumpPriority;
	bool			patDelay;
	bool			haltFlag;
	mp_sint32		startNextRow;
	
	mp_sint32		patDelayCount;

	// keep track of what positions we already visited (bitmap)
	mp_ubyte		rowHits[256*256/8];
	bool			isLooping;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//					    this information is updated while the song plays
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool newInsPTFlag;			// New instrument PT like
	bool newInsST3Flag;			// New instrument ST3 like
	bool oldPTInsChangeFlag;	// sample without note flag (old PT style)
	bool playModePT;
	bool playModePTPitchLimit;
	bool playModeFT2;
	bool playModeChopSampleOffset;
	
	bool isRowVisited(mp_sint32 row)
	{		
		return (rowHits[row>>3]>>(row&7))&1;
	}

	void visitRow(mp_sint32 row)
	{
		rowHits[row>>3] |= (1<<(row&7));
	}	

	static void			prenvelope(mp_sint32 c, TPrEnv* env, mp_sint32 keyon);		// process envelopes
	
	static mp_sint32	getenvval(mp_sint32 c, TPrEnv* env, mp_sint32 n);			// get envelope value

	// This takes the period *with* 8 bit fractional part
	static mp_sint32	getlinfreq(mp_sint32 per);	
	// This takes the period *with* 8 bit fractional part
	static mp_sint32	getlogfreq(mp_sint32 per);
	// this returns a period *without* the 8 bit fractional part
	static mp_sint32	getlinperiod(mp_sint32 note,mp_sint32 relnote,mp_sint32 finetune);
	static mp_sint32	interpolate(mp_sint32 eax,mp_sint32 ebx,mp_sint32 ecx,mp_sint32 edi,mp_sint32 esi);
	// this returns a period *without* the 8 bit fractional part
	static mp_sint32	getlogperiod(mp_sint32 note,mp_sint32 relnote,mp_sint32 finetune);
	
	mp_uint32		getbpmrate(mp_uint32 bpm)
	{
		// digibooster "real BPM" setting
		mp_uint32 realCiaTempo = (bpm * (baseBpm << 8) / 125) >> 8;

		if (!realCiaTempo) realCiaTempo++;
		
		mp_int64 t = ((mp_int64)realCiaTempo)<<(32+2);
		
		const mp_uint32 timerBase = (mp_uint32)(5.0f*500.0f*(MP_BEATLENGTH*MP_TIMERFREQ / (float)MP_BASEFREQ));
		
		return (mp_uint32)(t/timerBase);
	}

	mp_sint32		getperiod(mp_sint32 note,mp_sint32 relnote,mp_sint32 finetune)
	{
		if (playModeFT2)
		{
			// FT2 doesn't support lower 3 bits
			if (finetune > 0)
				finetune &= 0xF8;
			else if (finetune < 0)
			{
				finetune = -((-finetune + 7) & 0xF8);
				if (finetune < -128)
					finetune = -128;
			}
		}
	
		return (module->header.freqtab&1) ? getlinperiod(note,relnote,finetune) : getlogperiod(note,relnote,finetune);
	}
	
	mp_sint32		getvolume(mp_sint32 c,mp_sint32 nv)
	{
		mp_sint32 vol = (nv*getenvval(c,&chninfo[c].venv,256))>>7;
		vol = (vol*chninfo[c].fadevolstart)>>16;
		vol = (vol*chninfo[c].masterVol)>>8;
		vol = (vol*mainVolume)>>8;
		return vol;
	}
	
	mp_sint32		getpanning(mp_sint32 c,mp_sint32 np)
	{
		mp_sint32 envpan = getenvval(c,&chninfo[c].penv,128);
		//if (envpan!=256) cprintf("%i\r\n",envpan);
		mp_sint32 finalpan = np+(envpan-128)*(128-abs(np-128))/128;
		if (finalpan<0) finalpan=0;
		if (finalpan>255) finalpan=255;
		return finalpan;
	}
	
	mp_sint32		getfreq(mp_sint32 c,mp_sint32 per,mp_sword freqadjust)
	{
		if (per<1) return 0;
		mp_sint32 eval = getenvval(c,&chninfo[c].fenv,128)-128;
		mp_uint32 freq;
		
		freq = (module->header.freqtab&1) ? getlinfreq(per) : getlogfreq(per);
		
		mp_sint32 finalFreq = (freq+(eval*63))+freqadjust;
		if (finalFreq < 0) finalFreq = 0;
		
		return finalFreq;
	}
	
	mp_sint32		getfinalperiod(mp_sint32 c, mp_sint32 p);
	
	void			playInstrument(mp_sint32 chn, TModuleChannel* chnInf, bool bNoRestart = false);
	
	void			triggerEnvelope(TPrEnv& dstEnv, TEnvelope& srcEnv);
	void			triggerEnvelopes(TModuleChannel* chnInf);	
	void			triggerAutovibrato(TModuleChannel* chnInf);	
	void			triggerInstrumentFX(TModuleChannel* chnInf);
	
	void			updatePlayModeFlags();
	
	void			handlePeriodOverflow(mp_sint32 channel)
	{
		// PTK/FT1 playmode
		if (playModePTPitchLimit && options[PlayModeOptionForcePTPitchLimit])
		{
			if (chninfo[channel].per > 856*4)
				chninfo[channel].per = 856*4;
		}
		// FT2 playmode (does nothing right now)
		else
		{
			//if (chninfo[channel].per > 14150)
			//	chninfo[channel].per %= 14150;
		}
	}
	
	void			handlePeriodUnderflow(mp_sint32 channel)
	{
		// PTK/FT1 playmode
		if (playModePTPitchLimit && options[PlayModeOptionForcePTPitchLimit])
		{
			if (chninfo[channel].per < 113*4)
				chninfo[channel].per = 113*4;
		}
		// FT2 playmode (clamp on low value, not what FT2 does btw.)
		else
		{
			if (chninfo[channel].per < XM_MINPERIOD) 
				chninfo[channel].per = XM_MINPERIOD;
			//chninfo[channel].per &= 0x3FFF;
		}
	}
	
	mp_sint32		calcVibrato(TModuleChannel* chnInf, mp_sint32 effcnt);
	void			doTickEffect(mp_sint32 chn, TModuleChannel* chnInf, mp_sint32 effcnt);
	void			doEffect(mp_sint32 chn, TModuleChannel* chnInf, mp_sint32 effcnt);
	
	void			doTickeffects();	
	void			progressRow();	
	void			update();	
	void			updateBPMIndependent();

	//void			handleQueuedPositions(mp_sint32& poscnt);
	void			setNewPosition(mp_sint32 poscnt);

	void			tickhandler();
	
	mp_sint32		allocateStructures();
	void			freeMemory();
	
	// stop song by setting flag and setting speed to zero
	void			halt();

protected:
	virtual void	clearEffectMemory();
	
public:
					PlayerSTD(mp_uint32 frequency,
							  StatusEventListener* statusEventListener = NULL);
					
	virtual			~PlayerSTD();
	
	virtual PlayerTypes getType() const { return PlayerType_Generic; }

	virtual mp_sint32 adjustFrequency(mp_uint32 frequency);
	virtual mp_sint32 setBufferSize(mp_uint32 bufferSize);	
	
	// virtual from mixer class, perform playing here
	virtual void	timerHandler(mp_sint32 currentBeatPacket);
	
	virtual void	restart(mp_uint32 startPosition = 0, mp_uint32 startRow = 0, 
							bool resetMixer = true, 
							const mp_ubyte* customPanningTable = NULL, 
							bool playOneRowOnly = false); 
	
	virtual void	reset();

	virtual void	resetAllSpeed();

	virtual bool	grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const;

	// milkytracker
	virtual void	playNote(mp_ubyte chn, mp_sint32 note, mp_sint32 ins, mp_sint32 vol = -1);
							 
	virtual void	setPanning(mp_ubyte chn, mp_ubyte pan) { chninfo[chn].pan = pan; }

#ifdef MILKYTRACKER
	friend class PlayerController;
	friend class PlayerGeneric;
	friend class PlayerStatusTracker;
#endif
};

#endif
