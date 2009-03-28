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
 *  PlayerIT.h
 *  MilyPlay core
 *
 *  Created by Peter Barth on Tue Oct 19 2004.
 *
 */
#ifndef __PLAYERIT_H__
#define __PLAYERIT_H__

#include "PlayerBase.h"
#include "XModule.h"

class PlayerIT : public PlayerBase
{
private:
	enum EXMMinPeriod
	{
		XM_MINPERIOD = 50
	};

	struct TPrEnv 
	{
		TEnvelope	*envstruc;
		bool		enabled;
		mp_sint32   a,b,step;
		mp_uint32	bpmCounter, bpmAdder;
		
	private:
		bool		reachedEnd(bool keyon)
		{
			bool reachedEnd = ((step >= envstruc->env[b][0]) && (b >= envstruc->num - 1));
			
			if (!reachedEnd)
				return false;
			
			bool ITBreakLoop = (envstruc->type & 16);
			if (ITBreakLoop && keyon)
				return false;
			
			bool AMSBreakLoop = (envstruc->type & (4+8)) == (4+8);
			bool XMBreakLoop = ((envstruc->type & (2+4)) == (2+4)) && (envstruc->sustain == envstruc->loope);
			
			bool brokeLoop = !keyon && (AMSBreakLoop || XMBreakLoop);
			
			//bool res = ((envstruc->type & 4) && brokeLoop) || (!(envstruc->type & 4));
			return ((envstruc->type & 4) && brokeLoop) || (!(envstruc->type & 4));
		}
	
	public:
		
		bool finished(bool keyon)
		{
			if (envstruc == NULL ||
				!enabled)
				return false;
			
			return reachedEnd(keyon);
		}
		
		bool cutted(bool keyon)
		{
			return (envstruc &&
					reachedEnd(keyon) &&
					envstruc->env[b][1] == 0);
		}
		
		void setEnabled(bool enabled) { this->enabled = enabled; }
		bool isEnabled() { return enabled && envstruc != NULL; }
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
		mp_ubyte chnvolslide;
		mp_ubyte panslide;
		mp_ubyte arpeg;
		mp_ubyte retrig;
		mp_ubyte tremor;
		mp_ubyte smpoffset;
		mp_ubyte temposlide;
	};
	
#define DEFINE_STATINTERFACE \
		mp_sint32	getNote() { return chnstat().note; } \
		mp_sint32	getIns() { return chnstat().ins; } \
		mp_uword	getInsflags() { return chnstat().insflags; } \
		mp_sint32	getSmp() { return chnstat().smp; } \
\
		void		setNote(mp_sint32 note) { chnstat().note = note; } \
		void		setIns(mp_sint32 ins) { chnstat().ins = ins; } \
		void		setInsflags(mp_uword insflags) { chnstat().insflags = insflags; } \
		void		setSmp(mp_sint32 smp) { chnstat().smp = smp; } \
\
		mp_sint32	getFlags() { return chnstat().flags; } \
		void		setFlags(mp_uint32 flags) { chnstat().flags = flags; } \
		void		setFlag(mp_uint32 flag) { chnstat().flags |= flag; } \
		void		resetFlag(mp_uint32 flag) { chnstat().flags &= ~flag; } \
		bool		isFlagSet(mp_uint32 flag) { return (chnstat().flags & flag); } \
\
		void		setVol(mp_sint32 vol) { chnstat().vol = vol; } \
		void		incVol(mp_sint32 offs) { chnstat().vol+=offs; if (chnstat().vol>255) chnstat().vol=255; } \
		void		decVol(mp_sint32 offs) { chnstat().vol-=offs; if (chnstat().vol<0) chnstat().vol=0; } \
		void		adjustTremoloTremorVol() { chnstat().tremorVol = chnstat().tremoloVol = chnstat().vol; chnstat().hasTremolo = false; chnstat().finalTremoloVol = 0; } \
		void		adjustTremoloVol() { chnstat().tremoloVol = chnstat().vol; chnstat().hasTremolo = false; chnstat().finalTremoloVol = 0; } \
		void		setFinalTremVol(mp_sint32 vol) { chnstat().finalTremoloVol = vol; chnstat().hasTremolo = true; } \
\
		void		setPan(mp_sint32 pan) { chnstat().pan = pan; } \
		void		incPan(mp_sint32 offs) { chnstat().pan+=offs; if (chnstat().pan>255) chnstat().pan=255; } \
		void		decPan(mp_sint32 offs) { chnstat().pan-=offs; if (chnstat().pan<0) chnstat().pan=0; } \
\
		void		setPer(mp_sint32 per) { chnstat().per = per; } \
		void		decPer(mp_sint32 offs) { chnstat().per-=offs; } \
		void		incPer(mp_sint32 offs) { chnstat().per+=offs; } \
		void		adjustVibratoPer() { chnstat().hasVibrato = false; chnstat().finalVibratoPer = 0; } \
		void		setFinalVibratoPer(mp_sint32 per) { chnstat().finalVibratoPer = per; chnstat().hasVibrato = true; } \
\
		void		clampPerMax(mp_sint32 max) { if (chnstat().per > max) chnstat().per = max; } \
		void		clampPerMin(mp_sint32 min) { if (chnstat().per < min) chnstat().per = min; } \
\
		void		setMasterVol(mp_sint32 vol) { chnstat().masterVol = vol; } \
		void		incMasterVol(mp_sint32 offs) { chnstat().masterVol+=offs; if (chnstat().masterVol>255) chnstat().masterVol=255; } \
		void		decMasterVol(mp_sint32 offs) { chnstat().masterVol-=offs; if (chnstat().masterVol<0) chnstat().masterVol=0; } \
		void		setInsMasterVol(mp_sint32 vol) { chnstat().insMasterVol = vol; } \
		void		setSmpMasterVol(mp_sint32 vol) { chnstat().smpMasterVol = vol; } \
		void		setFreqadjust(mp_sword freqadjust) { chnstat().freqadjust = freqadjust; } \
		void		setRelnote(mp_sint32 relnote) { chnstat().relnote = relnote; } \
		void		setFinetune(mp_sint32 finetune) { chnstat().finetune = finetune; } \
\
		mp_sint32	getVol() { return chnstat().vol; } \
		mp_sint32	getTremoloVol() { return chnstat().tremoloVol; } \
		mp_sint32	getTremorVol() { return chnstat().tremorVol; } \
		mp_sint32	getVolume() { return chnstat().getVolumeInternal(); } \
\
		mp_sint32	getPan() { return chnstat().pan; } \
		mp_sint32	getPer() { return chnstat().per; } \
		mp_sint32   getPeriod() { return chnstat().getPeriodInternal(); } \
\
		mp_sint32	getMasterVol() { return chnstat().masterVol; } \
		mp_sint32	getInsMasterVol() { return chnstat().insMasterVol; } \
		mp_sint32	getSmpMasterVol() { return chnstat().smpMasterVol; } \
		mp_sword	getFreqadjust() { return chnstat().freqadjust; } \
		mp_sint32	getRelnote() { return chnstat().relnote; } \
		mp_sint32	getFinetune() { return chnstat().finetune; } \
\
		bool		getKeyon() { return chnstat().keyon; } \
\
		TPrEnv&		getVenv() { return chnstat().venv; } \
		TPrEnv&		getPenv() { return chnstat().penv; } \
		TPrEnv&		getFenv() { return chnstat().fenv; } \
		TPrEnv&		getVibenv() { return chnstat().vibenv; } \
		TPrEnv&		getPitchenv() { return chnstat().pitchenv; } \
\
		mp_ubyte	getAvibused() { return chnstat().avibused; } \
		mp_ubyte	getAvibspd() { return chnstat().avibspd; } \
		mp_ubyte	getAvibdepth() { return chnstat().avibdepth; } \
		mp_ubyte	getAvibcnt() { return chnstat().avibcnt; } \
		mp_ubyte	getAvibsweep() { return chnstat().avibsweep; } \
		mp_sint32	getAvibswcnt() { return chnstat().avibswcnt; } \
\
		void		setKeyon(bool keyon) { chnstat().keyon = keyon; } \
		void		setAvibused(mp_ubyte avibused) { chnstat().avibused = avibused; } \
		void		setAvibspd(mp_ubyte avibspd) { chnstat().avibspd = avibspd; } \
		void		setAvibdepth(mp_ubyte avibdepth) { chnstat().avibdepth = avibdepth; } \
		void		setAvibcnt(mp_ubyte avibcnt) { chnstat().avibcnt = avibcnt; } \
		void		setAvibsweep(mp_ubyte avibsweep) { chnstat().avibsweep = avibsweep; } \
		void		setAvibswcnt(mp_sint32 avibswcnt) { chnstat().avibswcnt = avibswcnt; } \
\
		void		avibAdvance() \
		{ \
			chnstat().avibcnt+=chnstat().avibspd; \
			/* IT style auto vibrato */ \
			if (chnstat().avibused & 128) \
			{ \
				if (chnstat().avibswcnt < ((mp_sint32)chnstat().avibdepth << 8) && chnstat().avibsweep) \
					/* Our vibrato depth is two times finer than the one from IT, increment by sweep*2 */ \
					chnstat().avibswcnt+=(mp_sint32)chnstat().avibsweep<<1; \
			} \
			/* XM style auto vibrato */ \
			else \
			{ \
				if (chnstat().avibswcnt < chnstat().avibsweep) \
					chnstat().avibswcnt = (chnstat().avibswcnt+1) & 0xFF; \
			} \
		} \
\
		void		setFadevolstart(mp_sint32 fadevolstart) { chnstat().fadevolstart = fadevolstart; } \
		mp_sint32	getFadevolstart() { return chnstat().fadevolstart; } \
\
		void		setFadevolstep(mp_sint32 fadevolstep) { chnstat().fadevolstep = fadevolstep; } \
		mp_sint32	getFadevolstep() { return chnstat().fadevolstep; } \
\
		void		decFadevolstart() { chnstat().fadevolstart-=chnstat().fadevolstep; if (chnstat().fadevolstart<0) chnstat().fadevolstart=0; } \
\
		void		setFadeout(bool fadeout) { chnstat().fadeout = fadeout; } \
		bool		getFadeout() { return chnstat().fadeout; } \
\
		void		setCutoff(mp_ubyte cutoff) { chnstat().cutoff = cutoff; } \
		mp_ubyte	getCutoff() { return chnstat().cutoff; } \
\
		void		setResonance(mp_ubyte resonance) { chnstat().resonance = resonance; } \
		mp_ubyte	getResonance() { return chnstat().resonance; }

public:
	struct TChnState
	{
		TChnState&	chnstat() { return *this; }
	
		mp_uint32	flags;

		mp_sint32	note;
		mp_sint32	ins;
		mp_uword	insflags;
		mp_sint32	smp;

		mp_sint32	vol;
		mp_sint32	tremoloVol;
		mp_sint32	finalTremoloVol;
		mp_sint32	tremorVol;
		bool		hasTremolo;

		mp_sint32	pan;

		mp_sint32	per;
		mp_sint32	finalVibratoPer;
		bool		hasVibrato;

		mp_sint32	insMasterVol;
		mp_sint32	smpMasterVol;
		mp_sint32	masterVol;			

		mp_sint32	relnote;
		mp_sint32	finetune;
		mp_sword	freqadjust;			

		bool		keyon;	
		bool		fadeout;

		TPrEnv		venv;				
		TPrEnv		penv;
		TPrEnv		fenv;
		TPrEnv		vibenv;
		TPrEnv		pitchenv;
	
		mp_ubyte	avibused;
		mp_ubyte	avibspd;
		mp_ubyte	avibdepth;
		mp_ubyte	avibcnt;
		mp_ubyte	avibsweep;
		mp_sint32	avibswcnt;

		mp_sint32	fadevolstart;
		mp_sint32	fadevolstep;
		
		mp_sint32	getVolumeInternal() { return hasTremolo ? finalTremoloVol : vol; }
		mp_sint32   getPeriodInternal() { return hasVibrato ? finalVibratoPer : per; }
		
		mp_ubyte	cutoff;
		mp_ubyte	resonance;
		
		DEFINE_STATINTERFACE		
	};
	
private:
	struct TModuleChannel;
	
	struct TVirtualChannel
	{
	private:
		bool		active;
		
		mp_sint32	channelIndex;
		
		TChnState	state;
		
		TModuleChannel*	host;
		TModuleChannel*	oldHost;

	public:
		// if we're in background we work on our own state
		// if not, we're just going to work on the host state
		TChnState&		chnstat();
		
		TChnState&		getRealState() { return state; }
		
		void			updateState(const TChnState& src) 
		{ 
			state = src; 
			state.flags = 0;
		}
	
		void			setActive(bool active) { this->active = active; }
		bool			getActive() { return active; }
	
		void			setHost(TModuleChannel*	host) { this->host = host; }
		TModuleChannel* getHost() { return host; }
		bool			getBackground() { return host == NULL; }

		void			setOldHost(TModuleChannel*	host) { this->oldHost = host; }
		TModuleChannel* getOldHost() { return oldHost; }
	
		void			setChannelIndex(mp_sint32 channelIndex) { this->channelIndex = channelIndex; }
		mp_sint32		getChannelIndex() { return channelIndex; }
		
		DEFINE_STATINTERFACE

		mp_sint32		getResultingVolume()
		{
			mp_sint32 vol = (getVolume()*getFadevolstart())>>16;
			vol = (vol*getMasterVol()*getInsMasterVol())>>16;
			vol = (vol*getSmpMasterVol())>>8;
			return vol;
		}
		
		void			clear()
		{
			mp_sint32 cIndex = channelIndex;
			memset(this, 0, sizeof(TVirtualChannel));
			channelIndex = cIndex;
		}
	
		friend struct TModuleChannel;
	};

	struct TModuleChannel 
	{
	private:
		TChnState	state;
		
		TVirtualChannel* vchn;
		
	public:
		// if we're having a virtual channel, we need to work on the state of the virtual channel
		// if not, we're just going to work on our own state
		TChnState&	chnstat() { return vchn ? vchn->chnstat() : state; }
		
		TChnState&	getRealState() { return state; }
		
		bool		hasVchn() { return vchn != NULL; }
		TVirtualChannel* getVchn() { return vchn; }
		void		linkVchn(TVirtualChannel* vchn)
		{
			vchn->clear();
			vchn->setActive(true);
			this->vchn = vchn;
			vchn->setHost(this);
		}

		TVirtualChannel* unlinkVchn()
		{
			vchn->setHost(NULL);
			vchn->setOldHost(this);
			vchn->updateState(getRealState());
			TVirtualChannel* result = vchn;
			vchn = NULL;
			return result;
		}

		mp_sint32	getPlaybackChannelIndex() { return ((vchn == NULL) ? -1 : vchn->getChannelIndex()); }
	
		DEFINE_STATINTERFACE
		
		void		slideToPer(mp_sint32 op) 
		{ 
			if (destper>chnstat().per) { 
				chnstat().per+=op; 
				if (chnstat().per>destper) chnstat().per=destper; 
			} 
			else if (destper<chnstat().per) { 
				chnstat().per-=op; 
				if (chnstat().per<destper) chnstat().per=destper; 
			} 
		} 

		mp_sint32			channelIndex;
		
		bool				hasSetVolume;
		mp_sint32			destper;
	
		//mp_sint32 c4spd;
		mp_sint32			currentnote;
		mp_sint32			destnote;
		bool				validnote;
		mp_sint32			lastnoportanote;
		
		mp_uint32			smpoffs;	
		mp_uint32			smpoffshigh;
		
		mp_ubyte			eff[MP_NUMEFFECTS];
		mp_ubyte			eop[MP_NUMEFFECTS];
		TLastOperands		old[MP_NUMEFFECTS];

		mp_sint32			loopstart;
		mp_sint32			execloop;
		mp_sint32			loopcounter;
		bool				isLooping;
		mp_sint32			loopingValidPosition;

		mp_ubyte			vibdepth[MP_NUMEFFECTS];
		mp_ubyte			vibspeed[MP_NUMEFFECTS];
		mp_ubyte			vibpos[MP_NUMEFFECTS];
		mp_ubyte			trmdepth[MP_NUMEFFECTS];
		mp_ubyte			trmspeed[MP_NUMEFFECTS];
		mp_ubyte			trmpos[MP_NUMEFFECTS];
		mp_ubyte			panbrellodepth[MP_NUMEFFECTS];
		mp_ubyte			panbrellospeed[MP_NUMEFFECTS];
		mp_ubyte			panbrellopos[MP_NUMEFFECTS];
		mp_ubyte			tremorcnt[MP_NUMEFFECTS];
		mp_sint32			retrigcounterE9x[MP_NUMEFFECTS];
		mp_ubyte			retrigmaxE9x[MP_NUMEFFECTS];
		mp_sint32			retrigcounterRxx[MP_NUMEFFECTS];
		mp_ubyte			retrigmaxRxx[MP_NUMEFFECTS];
				
		friend struct TVirtualChannel;
	};

#undef DEFINE_STATINTERFACE
	
private:

	static const mp_sint32	vibtab[32];
	static const mp_sint32	finesintab[256];
	static const mp_uword	lintab[769];
	static const mp_uint32	logtab[];
	static const mp_uint32	powtab[];
	
	TModuleChannel	*chninfo;				// our channel information
	TVirtualChannel *vchninfo;				// our virtual channels
	
	mp_ubyte		*attick;
	
	mp_sint32		patternIndex;			// holds current pattern index
	mp_sint32		numModuleChannels;		// max number of "host" channels (from module header)
	mp_sint32		numMaxVirChannels;		// max number of virtual channels in total
	mp_sint32		numVirtualChannels;		// max number of virtual channels (dynamic)
	mp_sint32		numEffects;				// current number of effects
	mp_sint32		numChannels;			// current number of channels
	mp_sint32		curMaxVirChannels;
	
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

	TVirtualChannel*	allocateVirtualChannel();
	void				releaseVirtualChannel(TVirtualChannel* vchn)
	{
		vchn->setActive(false);
		if (vchn->getChannelIndex() == curMaxVirChannels-1)
			curMaxVirChannels--;
	}
		
	struct TNNATriggerInfo
	{
		mp_sint32 note;
		mp_sint32 ins;
		mp_sint32 smp;
	};
	
	void				handleNoteOFF(TChnState& state);
	void				handlePastNoteAction(TModuleChannel* chnInf, mp_ubyte pastNoteActionType);
	bool				handleDCT(TModuleChannel* chnInf, const TNNATriggerInfo& triggerInfo, mp_ubyte DCT, mp_ubyte DCA);
	bool				handleNNAs(TModuleChannel* chnInf, const TNNATriggerInfo& triggerInfo);
	void				adjustVirtualChannels();

	static void			prenvelope(TPrEnv* env, bool keyon, bool timingIT);		// process envelopes
	
	static mp_sint32	getenvval(TPrEnv* env, mp_sint32 n);					// get envelope value

	static mp_sint32	interpolate(mp_sint32 eax,mp_sint32 ebx,mp_sint32 ecx,mp_sint32 edi,mp_sint32 esi);

	// This takes the period *with* 8 bit fractional part
	static mp_sint32	getlinfreq(mp_sint32 per);	
	// This takes the period *with* 8 bit fractional part
	static mp_sint32	getlogfreq(mp_sint32 per);
	// this returns a period *without* the 8 bit fractional part
	mp_sint32			getlinperiod(mp_sint32 note, mp_sint32 relnote, mp_sint32 finetune);
	// this returns a period *without* the 8 bit fractional part
	mp_sint32			getlogperiod(mp_sint32 note, mp_sint32 relnote, mp_sint32 finetune);
	
	mp_uint32		getbpmrate(mp_uint32 bpm)
	{
		// digibooster "real BPM" setting
		mp_uint32 realCiaTempo = (bpm * (baseBpm << 8) / 125) >> 8;

		if (!realCiaTempo) realCiaTempo++;
		
		mp_int64 t = ((mp_int64)realCiaTempo)<<(32+2);
		
		const mp_uint32 timerBase = (mp_uint32)(5.0f*500.0f*(MP_BEATLENGTH*MP_TIMERFREQ / (float)MP_BASEFREQ));
		
		return (mp_uint32)(t/timerBase);
	}

	mp_sint32		getperiod(mp_sint32 note, mp_sint32 relnote, mp_sint32 finetune)
	{
		/*mp_sint32 logper = getlogperiod(note,relnote,finetune);
		mp_sint32 linper = getlinperiod(note,relnote,finetune);
	
		mp_sint32 logfreq = getlogfreq(logper<<8);
		mp_sint32 linfreq = getlinfreq(linper<<8);
	
		return (module->header.freqtab&1) ? linper : logper;*/

		if (playModeFT2)
		{
			// FT2 doesn't support lower 3 bits
			if (finetune > 0)
				finetune &= ~3;
			else if (finetune < 0)
			{
				finetune = -((-finetune + 7) & ~3);
				if (finetune < -128)
					finetune = -128;
			}
		}
	
		return (module->header.freqtab&1) ? getlinperiod(note,relnote,finetune) : getlogperiod(note,relnote,finetune);
	}
	
	mp_sint32		getFinalVolume(TChnState& state, mp_sint32 nv, mp_sint32 mainVolume)
	{
		mp_sint32 vol = (nv*getenvval(&state.venv,256))>>7;
		vol = (vol*state.fadevolstart)>>16;
		vol = (vol*state.masterVol*state.insMasterVol)>>16;
		vol = (vol*state.smpMasterVol*mainVolume)>>16;
		return vol;
	}
	
	mp_sint32		getFinalCutoff(TChnState& state, mp_sint32 nc)
	{
		if (state.pitchenv.envstruc != NULL &&
			state.pitchenv.envstruc->type & 128)
			return (nc != MP_INVALID_VALUE) ? nc*getenvval(&state.pitchenv, 256) : 127*getenvval(&state.pitchenv, 256);
		else 
			return (nc != MP_INVALID_VALUE) ? (nc << 8) : nc;
	}
	
	mp_sint32		getFinalPanning(TChnState& state, mp_sint32 np)
	{
		mp_sint32 envpan = getenvval(&state.penv,128);
		//if (envpan!=256) cprintf("%i\r\n",envpan);
		mp_sint32 finalpan = np+(envpan-128)*(128-abs(np-128))/128;
		if (finalpan<0) finalpan=0;
		if (finalpan>255) finalpan=255;
		return finalpan;
	}
	
	mp_sint32		getFinalFreq(TChnState& state, mp_sint32 per)
	{
		if (per<1) return 0;
		
		// valid envelope and pitch envelope is not configured as filter envelope
		if (state.pitchenv.envstruc != NULL && !(state.pitchenv.envstruc->type & 128) && (state.pitchenv.envstruc->type & 1))
		{
			// scale the envelope point that 256 units match one semitone
			mp_sint32 pitch = (getenvval(&state.pitchenv, 128) - 128) * 32;		
			// add that semitone to the current note
			mp_sint32 note = state.getNote() + (pitch>>8);
			// the tone between two semitones
			mp_sint32 subnote = pitch & 255;
			// that would be the actual period we have
			mp_sint32 baseperiod = getperiod(state.getNote(), state.getRelnote(), state.getFinetune());
			
			mp_sint32 period1 = getperiod(note, state.getRelnote(), state.getFinetune());
			mp_sint32 period2 = getperiod(note+1, state.getRelnote(), state.getFinetune());
			mp_sint32 finalperiod = (period1 * (256-subnote) + period2 * subnote);
			
			mp_sint32 diff = finalperiod - (baseperiod << 8);
			per+=diff;
			if (per < XM_MINPERIOD)
				per = XM_MINPERIOD;
		}
		mp_sint32 eval = getenvval(&state.fenv,128)-128;
		mp_uint32 freq = (module->header.freqtab&1) ? getlinfreq(per) : getlogfreq(per);
		
		mp_sint32 finalFreq = (freq+(eval*63))+(mp_sint32)state.freqadjust;
		if (finalFreq < 0) finalFreq = 0;
		
		return finalFreq;
	}
	
	mp_sint32		getFinalPeriod(TChnState& state, mp_sint32 p);
	
	void			playInstrument(TModuleChannel* chnInf, bool bNoRestart = false);
	
	void			triggerEnvelope(TPrEnv& dstEnv, TEnvelope& srcEnv);
	void			triggerEnvelopes(TModuleChannel* chnInf);	
	void			triggerAutovibrato(TModuleChannel* chnInf);	
	void			triggerInstrumentFX(TModuleChannel* chnInf, bool triggerEnv = true);
	
	void			updatePlayModeFlags();
	
	void			handlePeriodOverflow(TModuleChannel* chnInf)
	{
		// PTK/FT1 playmode
		if (playModePTPitchLimit && options[PlayModeOptionForcePTPitchLimit])
		{
			chnInf->clampPerMax(856*4);
		}
		// FT2 playmode (does nothing right now)
		else
		{
			//if (chnInf->per > 14150)
			//	chnInf->per %= 14150;
		}
	}
	
	void			handlePeriodUnderflow(TModuleChannel* chnInf)
	{
		// PTK/FT1 playmode
		if (playModePTPitchLimit && options[PlayModeOptionForcePTPitchLimit])
		{
			chnInf->clampPerMin(113*4);
		}
		// FT2 playmode (clamp on low value, not what FT2 does btw.)
		else
		{
			chnInf->clampPerMin(XM_MINPERIOD);
			//chninfo[channel].per &= 0x3FFF;
		}
	}
	
	mp_sint32		calcVibrato(TModuleChannel* chnInf, mp_sint32 effcnt, mp_sint32 depthShift = 5);
	void			doTickVolslidePT(TModuleChannel* chnInf, mp_sint32 effcnt);
	void			doTickVolslideST(TModuleChannel* chnInf, mp_sint32 effcnt);
	void			doTickEffect(TModuleChannel* chnInf, mp_sint32 effcnt);

	void			doVolslidePT(TModuleChannel* chnInf, mp_sint32 effcnt, mp_ubyte eop);
	void			doVolslideST(TModuleChannel* chnInf, mp_sint32 effcnt, mp_ubyte eop);
	void			doEffect(TModuleChannel* chnInf, mp_sint32 effcnt);
	
	void			doTickeffects();	
	void			progressRow();	
	void			update();	
	void			updateBPMIndependent();

	void			setNewPosition(mp_sint32 poscnt);

	void			tickhandler();
	
	mp_sint32		allocateStructures();
	void			freeMemory();
	
	// stop song by setting flag and setting speed to zero
	void			halt();

protected:
	virtual void	clearEffectMemory();
	
public:
					PlayerIT(mp_uint32 frequency);
					
	virtual			~PlayerIT();
	
	virtual PlayerTypes getType() const { return PlayerType_IT; }
	
	void			setNumMaxVirChannels(mp_sint32 max) { numMaxVirChannels = max; }
	mp_sint32		getNumMaxVirChannels() const { return numMaxVirChannels; }
	
	// virtual from mixer class, perform playing here
	virtual void	timerHandler(mp_sint32 currentBeatPacket);
	
	// override base class method
	virtual mp_sint32   startPlaying(XModule* module, 
								 bool repeat = false, 
								 mp_uint32 startPosition = 0, 
								 mp_uint32 startRow = 0,
								 mp_sint32 numChannels = -1, 
								 const mp_ubyte* customPanningTable = NULL,
								 bool idle = false,
								 mp_sint32 patternIndex = -1,
								 bool playOneRowOnly = false);

	virtual void	restart(mp_uint32 startPosition = 0, mp_uint32 startRow = 0, bool resetMixer = true, const mp_ubyte* customPanningTable = NULL, bool playOneRowOnly = false); 

	virtual void	reset();

	virtual void	resetAllSpeed();

	virtual bool	grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const;

	mp_sint32		getCurMaxVirChannels() const { return curMaxVirChannels; }	
};

#endif
