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
 *  PlayerFAR.cpp
 *
 *  Created by Peter Barth on Thu Jan 20 2005.
 *
 *	I'm not responsible for some very twisted algorithms used in here.
 *	It seems to work so i'm not going to rewrite any of this :)
 *
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  01/24/05: 669 modules are now handled by the FAR replayer
 *  01/20/05: Created PlayerFAR.cpp
 *
 */

// sin()
#include <math.h>
#include "PlayerFAR.h"
#include "XModule.h"

#define TRUE 1
#define FALSE 0

// GUS Frequencies for 16 channels
mp_uword PlayerFAR::Freqs[120] = {
	/*14,14,16,16,18,18,20,22,24,24,26,28,
	30,32,32,34,38,40,42,44,46,50,52,56,
	58,62,66,70,74,78,84,88,94,98,104,110,
	118,124,132,140,148,157,167,176,187,198,211,223,
	236,250,264,280,297,315,334,355,375,397,420,446,
	472,500,530,559,592,630,667,705,746,792,838,890,
	939,995,1057,1115,1179,1252,1318,1410,1493,1561,1664,1750,
	1880,2051,2071,2206,2360,2476,2603,2743,3076,3275,3661,3797,
	3905,4061,4415,4615,4836,5077,5344,5641,5973,6346,6769,7254,	
	7718,8181,8703,9090,9739,10226,11055,11687,12395,13195,13635,14609*/
	
	14,15,16,17,18,19,20,22,23,24,26,27,29,
	31,33,34,37,39,41,44,46,49,52,55,58,62,
	66,70,74,78,83,88,93,99,105,111,117,124,
	132,140,148,157,166,176,187,198,210,222,
	235,249,264,280,297,315,333,353,374,396,
	420,444,471,499,529,560,594,630,667,707,
	749,793,840,890,942,998,1058,1120,1189,
	1260,1335,1415,1499,1587,1680,1784,1884,
	1996,2122,2240,2386,2520,2670,2840,3009,
	3175,3360,3568,3769,3993,4245,4481,4801,
	5041,5377,5680,6019,6401,6721,7201,7609,
	8065,8580,8962,9602,10082,10899,11522,
	12221,13009,13443,14403
	
	// with finetune to 22
	/*14,15,16,17,18,19,20,22,23,24,26,28,29,
	31,33,35,37,39,41,44,47,49,52,56,59,62,
	66,70,74,79,83,88,94,99,105,112,118,125,
	133,141,149,158,167,177,188,199,211,224,
	237,251,266,282,298,317,335,355,377,399,
	423,448,475,503,533,564,598,635,672,712,
	755,800,847,898,951,1008,1066,1129,1196,
	1272,1344,1425,1510,1600,1694,1800,1902,
	2016,2133,2265,2400,2552,2688,2860,3032,
	3200,3389,3600,3804,4032,4290,4531,4801,
	5105,5377,5761,6110,6401,6835,7201,7609,
	8065,8580,9165,9602,10340,10899,11522,
	12221,13009,13906,14403	*/
};

mp_uword PlayerFAR::VolTab[64];
mp_sint32 PlayerFAR::SinTable[16][128];
mp_sint32 PlayerFAR::mTempo[16];

void PlayerFAR::SetFreq(mp_sint32 chn, mp_sint32 freq)
{
	setFreq(chn, /*freq*35*/(freq*72704)>>11);
}

///////////////////////////////////////
// FAR set volume, vol:[0-512]
///////////////////////////////////////
void PlayerFAR::SetVolume(mp_sint32 chn, mp_sint32 vol)
{
	setVol(chn, (vol*mainVolume)>>8);
}

///////////////////////////////////////
// FAR set panning, balance:[0-15]
///////////////////////////////////////
void PlayerFAR::SetBalance(mp_sint32 chn, mp_ubyte balance)
{
	setPan(chn, balance*0x11);
}

///////////////////////////////////////
// FAR player constructor
///////////////////////////////////////
PlayerFAR::PlayerFAR(mp_uint32 frequency) : 
	PlayerBase(frequency),
	OverFlow(0), OCount(0), PlayOrder(0), 
	VibAmp(4),TempoType(1), PlayTempo(4),
	TempoBend(0)

{
	bpm = 80;
	tickSpeed = 4;

	mp_sint32 i = 0;

	static bool done = false;
	
	// create sine table used for vibrato
	if (!done)
	{
		double t,f=1,y,amp;
		for (amp=0;amp<16;amp++)
			for (t=0;t<1;t+=(1.0/128)) {
				y=sin(2*3.1415*f*t)*amp;
				SinTable[(mp_sint32)amp][(mp_sint32)(t*128)]=(mp_sint32)y;
			}
		
		// create volume table for volumes 0-512
		for (i = 0; i < 64; i++)
			VolTab[i] = i<<3;
			
		done = true;
	}


	// debugging
	/*for (i = 0; i < 120; i++)
	{
		Freqs[i] = player->getlogfreq(player->getlogperiod(i+1,0,0));
	}*/

	memset(PitchWheel,0,sizeof(PitchWheel));
	memset(VibOn,0,sizeof(VibOn));
	memset(VibPtr,0,sizeof(VibPtr));
	memset(VibInc,0,sizeof(VibInc));
	memset(RetLeft,0,sizeof(RetLeft));

	// setup default values
	for (i = 0; i < 16; i++)
	{
		CurVols[i]		= 0xF;
		CurBalance[i]	= 0x8;

		DestPitch[i]	= 0;
		Increment[i]	= 0;
		PresPitch[i]	= 0;
		VolWheel[i]		= 0;
		DestVol[i]		= 0;
		VIncrement[i]	= 0;
		PresVol[i]		= 0;
		RetSpc[i]		= 0;
		CurSpc[i]		= 0;
		RetSmp[i]		= 0;
		RetVol[i]		= 0;
		OfftCnt[i]		= 0;
		RetCnt[i]		= 0;
		CurFreqs[i]		= 0;
		CurChSmp[i]		= 0;
		CurNote[i]		= 0;
		CurEff[i]		= 0;
	}
	
	CalcTempo();
}

///////////////////////////////////////
// FAR player destructor
///////////////////////////////////////
PlayerFAR::~PlayerFAR()
{
}

///////////////////////////////////////
// FAR restart (see PlayerBase)
///////////////////////////////////////
void PlayerFAR::restart(mp_uint32 startPosition/* = 0*/, mp_uint32 startRow/* = 0*/, bool resetMixer/* = true*/, const mp_ubyte* customPanningTable/* = NULL*/, bool playOneRowOnly/* = false*/)
{
	// base class restart
	PlayerBase::restart(startPosition, startRow, resetMixer, customPanningTable, playOneRowOnly);

	VibAmp=4;
	
	// 669 uses a different tempo scheme
	switch (module->getType())
	{
		case XModule::ModuleType_669:
			TempoType = 0;
			break;

		default:
			TempoType = 1;
	}

	//TempoType = 1;
	PlayTempo = tickSpeed = module->header.tempo;
	TempoBend = 0;

	memset(PitchWheel,0,sizeof(PitchWheel));
	memset(VibOn,0,sizeof(VibOn));
	memset(VibPtr,0,sizeof(VibPtr));
	memset(VibInc,0,sizeof(VibInc));
	memset(RetLeft,0,sizeof(RetLeft));
	
	for (mp_sint32 i = 0; i < 16; i++)
	{
		CurVols[i] = 0xF;
		CurBalance[i] = 0x8;
	}
	
	CalcTempo();	
}

///////////////////////////////////////
// Compute FAR tempos
///////////////////////////////////////
void PlayerFAR::CalcTempo()
{
	mp_uword q;

	mTempo[0]=256;
	for (q=1;q<16;q++) mTempo[q]=128/q;
			
	UpdateTempo(mTempo[4]);
}

///////////////////////////////////////
// Set new tempo
// My mixer class emulates a 
// 250Hz timer so we need to convert
// the PC-Int8 timing into our 250Hz
// model
///////////////////////////////////////
void PlayerFAR::UpdateTempo(mp_sint32 tps) 
{

	// original FAR replaying... 669 tempo not supported because of hi timer resolution
	mp_uint32 eax,di,cx;
	
	eax=1197255/tps;
	cx=0; di=0;
	while (eax>0xFFFF) {
        eax>>=1;
        di++; cx++;
	}
	if (cx>=2) di++;
	di+=3;
	OverFlow=di; OCount=di;
	
	// convert timer frequency into 250Hz base
	float t = (1197255.0f / (float)eax);
	
	t = 1.0f/(250.0f/t);
	
	// for tempo 0 we get a period that is slightly shorter than what we can
	// do with 250Hz but the difference is very small so just correct it by
	// clamping
	if (t > 1.0f) t = 1.0f;
	
	adder = (mp_uint32)((mp_int64)(t*65536.0*65536.0));
	
	// see above
	if (!adder) adder = 0xFFFFFFFF;
	
}

///////////////////////////////////////
// FAR replay routine timer handler
// for 250Hz base timer
///////////////////////////////////////
void PlayerFAR::timerHandler(mp_sint32 currentBeatPacket)
{
	PlayerBase::timerHandler(currentBeatPacket);

	if (paused)
		return;

	if (module == NULL)
		return;
	
	// make sure this is a FAR tune
	if (module->getType() != XModule::ModuleType_FAR && 
		module->getType() != XModule::ModuleType_669)
		return;

	// make sure we're dealing with less or equal 16 channels
	if (module->header.channum > 16)
		return;

	setActiveChannels(/*numChannels*/module->header.channum);	

	mp_int64 dummy = (mp_int64)BPMCounter;
	dummy+=(mp_int64)adder;
	BPMCounter=(mp_sint32)dummy;
	
	// check overflow-carry 
	if ((dummy>>32)) 
	{
		
		mp_uword c,/*ov,*/m,q;
		mp_sint32 fp,sp,t,ch;
		mp_ubyte fekt;
		
		mp_ubyte* Pattern = module->phead[module->header.ord[poscnt]].patternData;
		
		mp_sint32 numChannels = module->phead[module->header.ord[poscnt]].channum;
		
		mp_sint32 BreakLoc = module->phead[module->header.ord[poscnt]].rows - 2;
		
		mp_sint32 CurSpot = rowcnt * module->header.channum * 6;
		
		for (ch=0;ch<numChannels;ch++) {		
			CurVoice = ch;
			if (VibOn[ch]) {
				VibPtr[ch]+=VibInc[ch];              // Update vibrato table cntr
				if (VibPtr[ch]>=128)                 // Reset counter
					VibPtr[ch]=0;
				if (!Increment[ch])                  // Note port doing it for us?
					SetFreq(ch,CurFreqs[ch]+PitchWheel[ch]+SinTable[VibAmp][VibPtr[ch]]);
			}
			if (Increment[ch]) {                   // Deal with note port
				t=(CurFreqs[ch]+PitchWheel[ch]);
				if (Increment[ch]<0) {
					if (t<=DestPitch[ch]) {
						Increment[ch]=0;
						PresPitch[ch]=0;
						CurFreqs[ch]=DestPitch[ch];
						DestPitch[ch]=0;
					}
					else
						PresPitch[ch]+=Increment[ch];
				}
				else {
					if (t>=DestPitch[ch]) {
						Increment[ch]=0;
						PresPitch[ch]=0;
						CurFreqs[ch]=DestPitch[ch];
						DestPitch[ch]=0;
					}
					else
						PresPitch[ch]+=Increment[ch];
				}
			}
			PitchWheel[ch]=PresPitch[ch]/256;
			SetFreq(ch,CurFreqs[ch]+PitchWheel[ch]+SinTable[VibAmp][VibPtr[ch]]);
			
			if (VIncrement[ch]) {                   // Deal with vol port
				t=(CurVols[ch]*4)+VolWheel[ch];
				if (VIncrement[ch]<0) {
					if (t<=DestVol[ch]) {
						VIncrement[ch]=0;
						PresVol[ch]=0;
						CurVols[ch]=DestVol[ch]/4;
						DestVol[ch]=0;
					}
					else
						PresVol[ch]+=VIncrement[ch];
				}
				else {
					if (t>=DestVol[ch]) {
						VIncrement[ch]=0;
						PresVol[ch]=0;
						CurVols[ch]=DestVol[ch]/4;
						DestPitch[ch]=0;
					}
					else
						PresVol[ch]+=VIncrement[ch];
				}
				VolWheel[ch]=PresVol[ch];
				if ( ((CurVols[ch]*4)+VolWheel[ch])< 0) {
					VIncrement[ch]=0;
					PresVol[ch]=0;
					CurVols[ch]=DestVol[ch]/4;
					DestVol[ch]=0;
					SetVolume(ch,VolTab[0]);
				}
				else
					SetVolume(ch,VolTab[(CurVols[ch]*4)+VolWheel[ch]]);
				
			}
			
			if (RetLeft[ch]) {               // Deal with retrigger
				CurSpc[ch]+=2;
				if (CurSpc[ch]>=RetSpc[ch]) {
					if (!OfftCnt[ch]) goto DoNote1;            // No note offset (retrig)
					if (OfftCnt[ch]==RetCnt[ch]) goto DoNote1; // Time for output of note
					goto NoNote1;
DoNote1:
						q=RetSmp[ch];
					m=module->smp[module->instr[q].snum[0]].type;
					CurVols[ch]=RetVol[ch]-1;
					CurChSmp[ch]=q;
					if ((m&3) && module->instr[q].snum[0] != -1)
					{
						playSample(CurVoice, 
										   module->smp[module->instr[q].snum[0]].sample, // sample buffer
										   module->smp[module->instr[q].snum[0]].samplen, // sample size
										   0, // sample offset 
										   0, // sample offset fraction
										   false, // wrap sample offset when exceeding sample length
										   module->smp[module->instr[q].snum[0]].loopstart, // loop start
										   module->smp[module->instr[q].snum[0]].loopstart+module->smp[module->instr[q].snum[0]].looplen, // loop end
										   m);
					}
					else
					{
						playSample(CurVoice, 
										   module->smp[module->instr[q].snum[0]].sample, // sample buffer
										   module->smp[module->instr[q].snum[0]].samplen, // sample size
										   0, // sample offset 
										   0, // sample offset fraction
										   false, // wrap sample offset when exceeding sample length
										   0, // loop start
										   module->smp[module->instr[q].snum[0]].samplen, // loop end
										   m);
					}
					SetVolume(ch,VolTab[(CurVols[ch]*4)+VolWheel[ch]]);
					RetLeft[ch]--;
					CurSpc[ch]=0;
NoNote1:
						RetCnt[ch]++;
				}
			}
		}
		
//oo:
			if (OCount--) 
				return;
		
		OCount=OverFlow;
		
		for (c=0;c<numChannels;c++) {
			CurVoice=c;
			
			CurNote[CurVoice] = Pattern[CurSpot];
			CurEff[CurVoice] = Pattern[CurSpot+5];
		
			OfftCnt[CurVoice]=0; RetCnt[CurVoice]=0; RetLeft[CurVoice]=0;
			//    if (Bars[CurVoice]<(PlayTempo*2)) Bars[CurVoice]=0;
			//    if (Bars[CurVoice]) Bars[CurVoice]-=PlayTempo*2;
			fekt=Pattern[CurSpot+5]&0xF0;
			if (Pattern[CurSpot] && fekt!=0x30) {
				q=Pattern[CurSpot+1]-1;
				m=module->smp[module->instr[q].snum[0]].type;
				//if (Sample[Pattern[CurSpot+1]].SType&1) m|=(1<<2);
				CurFreqs[CurVoice]=Freqs[Pattern[CurSpot]-1/*-3*12*/];
				SetFreq(CurVoice,CurFreqs[CurVoice]);
				CurChSmp[CurVoice]=q;
				
				/*if (m&(1<<2))
					PlaySample(module->smp[module->instr[q].snum[0]].Seg/2,
							   module->smp[module->instr[q].snum[0]].Off/2,
							   module->smp[module->instr[q].snum[0]].Rep/2,
							   module->smp[module->instr[q].snum[0]].RepEnd/2,
							   CurVoice,m);
				else
					PlaySample(module->smp[module->instr[q].snum[0]].Seg,
							   module->smp[module->instr[q].snum[0]].Off,
							   module->smp[module->instr[q].snum[0]].Rep,
							   module->smp[module->instr[q].snum[0]].RepEnd,
							   CurVoice,m);*/
				
				if ((m&3) && module->instr[q].snum[0] != -1)
				{
					playSample(CurVoice, 
									   module->smp[module->instr[q].snum[0]].sample, // sample buffer
									   module->smp[module->instr[q].snum[0]].samplen, // sample size
									   0, // sample offset 
									   0, // sample offset fraction
									   false, // wrap sample offset when exceeding sample length
									   module->smp[module->instr[q].snum[0]].loopstart, // loop start
									   module->smp[module->instr[q].snum[0]].loopstart+module->smp[module->instr[q].snum[0]].looplen, // loop end
									   m);
				}
				else
				{
					playSample(CurVoice, 
									   module->smp[module->instr[q].snum[0]].sample, // sample buffer
									   module->smp[module->instr[q].snum[0]].samplen, // sample size
									   0, // sample offset 
									   0, // sample offset fraction
									   false, // wrap sample offset when exceeding sample length
									   0, // loop start
									   module->smp[module->instr[q].snum[0]].samplen, // loop end
									   m);
				}
				
				
				PresPitch[CurVoice]=0;
				DestPitch[CurVoice]=0; Increment[CurVoice]=0;
				//      Bars[CurVoice]=(Pattern[CurSpot+2]*fs)/16;
			}
			SetBalance(CurVoice,CurBalance[CurVoice]);
			if ((Pattern[CurSpot+3]) && fekt!=0xa0) {
				PresVol[CurVoice]=0; VolWheel[CurVoice]=0;
				DestVol[CurVoice]=0; VIncrement[CurVoice]=0;
				
				CurVols[CurVoice]=((Pattern[CurSpot+3]>>4)-1);
				SetVolume(CurVoice,VolTab[(CurVols[CurVoice]*4)]);
				//      Bars[CurVoice]=(Pattern[CurSpot+2]*fs)/16;
			}
			if (Pattern[CurSpot+5]) {
				switch(Pattern[CurSpot+5]&0xF0) {
					case 0xf0:  // Modify tempo
						tickSpeed=Pattern[CurSpot+5]&0xF;
						PlayTempo=tickSpeed;
						if (TempoType)
							UpdateTempo(mTempo[PlayTempo]+TempoBend);
						else
							UpdateTempo(mTempo[PlayTempo]+(TempoBend*2));
						break;
					case 0xe0:  // Fine tempo up/cancel
						if (Pattern[CurSpot+5]&0xF) {
							TempoBend+=Pattern[CurSpot+5]&0xF;
							if ((TempoBend+mTempo[PlayTempo])>=100)
								TempoBend=100;
						}
						else
							TempoBend=0;
						if (TempoType)
							UpdateTempo(mTempo[PlayTempo]+TempoBend);
						else
							UpdateTempo(mTempo[PlayTempo]+(TempoBend*2));
						break;
					case 0xd0:  // Fine tempo down/cancel
						if (Pattern[CurSpot+5]&0xF) {
							TempoBend-=Pattern[CurSpot+5]&0xF;
							if ((TempoBend+mTempo[PlayTempo])<=0)
								TempoBend=0;
						}
						else
							TempoBend=0;
						if (TempoType)
							UpdateTempo(mTempo[PlayTempo]+TempoBend);
						else
							UpdateTempo(mTempo[PlayTempo]+(TempoBend*2));
						break;
					case 0xb0:  // Set Balance
						CurBalance[CurVoice]=Pattern[CurSpot+5]&0xF;
						SetBalance(CurVoice,CurBalance[CurVoice]);
						break;
					case 0x10:  // raise pitch
						PresPitch[CurVoice]+=((Pattern[CurSpot+5]&0xF)*4)*256;
						PitchWheel[CurVoice]=PresPitch[CurVoice]/256;
						DestPitch[CurVoice]=0; Increment[CurVoice]=0;
						SetFreq(ch,CurFreqs[ch]+PitchWheel[ch]+SinTable[VibAmp][VibPtr[ch]]);
						break;
					case 0x20:  // lower pitch
						PresPitch[CurVoice]-=((Pattern[CurSpot+5]&0xF)*4)*256;
						PitchWheel[CurVoice]=PresPitch[CurVoice]/256;
						DestPitch[CurVoice]=0; Increment[CurVoice]=0;
						SetFreq(ch,CurFreqs[ch]+PitchWheel[ch]+SinTable[VibAmp][VibPtr[ch]]);
						break;
					case 0x30:  // Port to note
						if (Pattern[CurSpot]) {
							t=0;
							fp=CurFreqs[CurVoice]+PitchWheel[CurVoice];
							sp=Freqs[Pattern[CurSpot]-1/*-3*12*/];
							DestPitch[CurVoice]=sp;
							if (fp>sp) {t=sp;sp=fp;fp=t;}
							
							if (module->getType() == XModule::ModuleType_669)
							{
								if (Pattern[CurSpot+5]&0xF)
									Increment[CurVoice]=((sp-fp)*256)/((Pattern[CurSpot+5]&0xF)*
																	   (mTempo[PlayTempo]+TempoBend)/4);
								else
									Increment[CurVoice]=((sp-fp)*256)/(1*
																	   (mTempo[PlayTempo]+TempoBend)/4);
							}
							else
							{
								if (Pattern[CurSpot+5]&0xF)
									Increment[CurVoice]=((sp-fp)*256)/((Pattern[CurSpot+5]&0xF)*
																	   (mTempo[PlayTempo]+TempoBend));
								else
									Increment[CurVoice]=((sp-fp)*256)/(1*
																	   (mTempo[PlayTempo]+TempoBend));
							}
							
							Increment[CurVoice]*=8;
							if (t) Increment[CurVoice]=-Increment[CurVoice];
						}
						break;
					case 0xC0:  // Note Offset
						OfftCnt[CurVoice]=Pattern[CurSpot+5]&0xF;
						if (Pattern[CurSpot]) {
							RetLeft[CurVoice]=0xE;
							if (TempoType)
								RetSpc[CurVoice]=((mTempo[PlayTempo]+TempoBend)/(RetLeft[CurVoice]+1))/4;
							else
								RetSpc[CurVoice]=((mTempo[PlayTempo]+TempoBend)/(RetLeft[CurVoice]+1))/2;
							RetSmp[CurVoice]=Pattern[CurSpot+1]-1;
							RetVol[CurVoice]=(Pattern[CurSpot+3]>>4);
							CurSpc[CurVoice]=0;
						}
							break;
					case 0x40:  // Retrigger
						if (Pattern[CurSpot]) {
							RetLeft[CurVoice]=(Pattern[CurSpot+5]&0xF)-1;
							if (TempoType)
								RetSpc[CurVoice]=((mTempo[PlayTempo]+TempoBend)/(RetLeft[CurVoice]+1))/4;
							else
								RetSpc[CurVoice]=((mTempo[PlayTempo]+TempoBend)/(RetLeft[CurVoice]+1))/2;
							RetSmp[CurVoice]=Pattern[CurSpot+1]-1;
							RetVol[CurVoice]=(Pattern[CurSpot+3]>>4);
							CurSpc[CurVoice]=0;
						}
						break;
					case 0x50:  // Set vibrato amplitude
						VibAmp=Pattern[CurSpot+5]&0xF;
						break;
					case 0x60:  // Vibrato Control
						if (!VibOn[CurVoice]) {
							VibOn[CurVoice]=1;
							VibInc[CurVoice]=(Pattern[CurSpot+5]&0xF)*6;
							VibPtr[CurVoice]=0;
						}
						else
							VibInc[CurVoice]=(Pattern[CurSpot+5]&0xF)*6;
						break;
					case 0x90:  // Sustained vibrato control
						if (Pattern[CurSpot+5]&0xF) {   // On
							if (!VibOn[CurVoice]) {
								VibOn[CurVoice]=2;
								VibInc[CurVoice]=(Pattern[CurSpot+5]&0xF)*6;
								VibPtr[CurVoice]=0;
							}
							else
								VibInc[CurVoice]=(Pattern[CurSpot+5]&0xF)*6;
						}
						else {                          // Off
							VibPtr[CurVoice]=0;
							VibInc[CurVoice]=0;
							VibOn[CurVoice]=FALSE;
						}
						break;
					case 0x70:  // VolSldUp
						q=Pattern[CurSpot+5]&0xF;
						CurVols[CurVoice]+=q;
						if (CurVols[CurVoice]>0xF) CurVols[CurVoice]=0xF;
							SetVolume(CurVoice,VolTab[(CurVols[CurVoice]*4)+VolWheel[CurVoice]]);
						//          Bars[CurVoice]=((CurVols[CurVoice]+1)*fs)/16;
						break;
					case 0x80:  // VolSldDn
						q=Pattern[CurSpot+5]&0xF;
						CurVols[CurVoice]-=q;
						if (CurVols[CurVoice]<0) CurVols[CurVoice]=0;
							SetVolume(CurVoice,VolTab[(CurVols[CurVoice]*4)+VolWheel[CurVoice]]);
						//          Bars[CurVoice]=((CurVols[CurVoice]+1)*fs)/16;
						break;
					case 0xA0:  // Port to Vol
						if ((Pattern[CurSpot+5])) {
							t=0;
							fp=(CurVols[CurVoice]*4)+VolWheel[CurVoice];
							sp=((Pattern[CurSpot+3]>>4)*4)-1;
							DestVol[CurVoice]=sp;
							if (fp>sp) {t=sp;sp=fp;fp=t;}
							if (Pattern[CurSpot+5]&0xF)
								VIncrement[CurVoice]=((sp-fp)*16)/((Pattern[CurSpot+5]&0xF)*
																   (mTempo[PlayTempo]+TempoBend));
							else
								VIncrement[CurVoice]=((sp-fp)*16)/(1*
																   (mTempo[PlayTempo]+TempoBend));
							if (t) VIncrement[CurVoice]=-VIncrement[CurVoice];
						}
						break;
					case 0x00: // Global funct
						switch(Pattern[CurSpot+5]&0xF) {
							case 1: // Ramp off
							case 2: // Ramp on
									//VolRamps=(Pattern[CurSpot+5]&0xF)-1;
								break;
							case 3:
								q=CurChSmp[CurVoice];
								//SetLoops(Sample[q].Seg,Sample[q].Off,Sample[q].Len,
								//		 Sample[q].LoopMode&0xf7,CurVoice);
								break;
							case 4: // 669 tempos
							case 5: // far tempos
								TempoType=(Pattern[CurSpot+5]&0xF)-4;
								if (TempoType)
									UpdateTempo(mTempo[PlayTempo]+TempoBend);
								else
									UpdateTempo(mTempo[PlayTempo]+(TempoBend*2));
								break;
						}
						break;
				}
			}
			if (VibOn[CurVoice]==1 && (Pattern[CurSpot+5]&0xF0)!=0x60) {
				VibPtr[CurVoice]=0;
				VibInc[CurVoice]=0;
				VibOn[CurVoice]=FALSE;
			}
			/*    if (Bars[CurVoice]>=fs) Bars[CurVoice]=fs-1;
			if (Bars[CurVoice]<1) Bars[CurVoice]=1; */
			
			CurSpot+=6;
			
		}
		//CurVoice=ov;
		if (rowcnt<=BreakLoc) {
			rowcnt++;
		}
		else {
			//PutAwayPat(CurPattern);
			if (poscnt + 1 == module->header.ordnum || module->header.ord[poscnt+1] >= 0xFF)
			{
				if (repeat)
					poscnt=module->header.restart;
				else
				{
					halted = true;
					BPMCounter = adder = 0;
					if (resetOnStopFlag)
						resetChannelsWithoutMuting();
					return;
				}
			}
			else
			{
				poscnt++;
			}
			CurPattern=module->header.ord[poscnt];
			//GetPat(CurPattern);
			rowcnt=0;
		}
		
	}
}

///////////////////////////////////////
// clear effect memory for song seeking
// see PlayerBase
///////////////////////////////////////
void PlayerFAR::clearEffectMemory()
{
	mp_sint32 i;

	OCount=OverFlow;
	
	memset(PitchWheel,0,sizeof(PitchWheel));
	memset(VibOn,0,sizeof(VibOn));
	memset(VibPtr,0,sizeof(VibPtr));
	memset(VibInc,0,sizeof(VibInc));
	memset(RetLeft,0,sizeof(RetLeft));

	for (i = 0; i < 16; i++)
	{
		DestPitch[i] = 0;
		Increment[i] = 0;
		PresPitch[i] = 0;
		VolWheel[i] = 0;
		DestVol[i] = 0;
		VIncrement[i] = 0;
		PresVol[i] = 0;
		RetSpc[i] = 0;
		CurSpc[i] = 0;
		RetSmp[i] = 0;
		RetVol[i] = 0;
		OfftCnt[i] = 0;
		RetCnt[i] = 0;
		CurFreqs[i] = 0;
		CurChSmp[i] = 0;
		CurNote[i] = 0;
		CurEff[i] = 0;
	}
}

bool PlayerFAR::grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const
{
	channelInfo.note = CurNote[chn];
	channelInfo.instrument = CurChSmp[chn]+1;
	channelInfo.volume = (CurVols[chn] == 0xF) ? 0xFF : CurVols[chn]*0x10;
	channelInfo.panning = CurBalance[chn]*0x11;
	channelInfo.numeffects = 2;
	memset(channelInfo.effects, 0, sizeof(channelInfo.effects));
	memset(channelInfo.operands, 0, sizeof(channelInfo.operands));
	
	channelInfo.effects[0] = 0x70;
	channelInfo.operands[0] = CurEff[chn];
	
	//memcpy(channelInfo.effects, chninfo[chn].eff, sizeof(chninfo[chn].eff));
	//memcpy(channelInfo.operands, chninfo[chn].eop, sizeof(chninfo[chn].eop));
	
	return true;
}

