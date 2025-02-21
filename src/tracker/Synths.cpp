
/*
 *  tracker/Synths.cpp 
 *
 *  Copyright 2023 Leon van Kammen (coderofsalvation)
 * 
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


#include "Synth.h" 
#include "SynthFM.h"

#define PL_SYNTH_IMPLEMENTATION
#include "SynthPL.h"

#include "Tracker.h"
#include "ControlIDs.h"
#include "DialogSliders.h"
#include "FilterParameters.h"
#include "XModule.h"
#include "SampleEditor.h"
#include "ListBox.h"
#include "fx/Convolver.h"
#include <math.h>

/*
 * Lightweight synths from the milkyverse.
 *
 * Rules for new synths:
 *   1. don't include new classes/libraries/FFT libraries e.g., KISS.
 *   2. re-use SampleEditor operations (recursive sample-editing, Cycle is an example)  
 *   3. please use volume as first slider and ADSR after that (if any)
 *   4. use TINY demoscene-like 4K-like synths (not plugins) which don't TAX the codebase / lofi devices
 */


void Synth::init(){
	FM(true);
	Cycle(true); 
	PL(true); 
	UNZ(true); 
	ASCIISynthImport( preset[0] ); // initialize with first preset
}

void Synth::process( MSynth *s, PPString *preset ){

	if( s == NULL ){  //fetch current synth from param 0
		pp_uint32 ID = (int)getParam(0).value;
		s = &(synths[ID]);
	}

	// if slider 0 changed, change synth + relaunch dialog
	if( s->ID != synth->ID ){     
		if( !sampleEditor->isEmptySample() ){
			// FM synth does not fit into a single cycle waveform 
			sampleEditor->selectAll();
			sampleEditor->cut();
		}
		synth  = s;                 
		synth->param[0].value = (float)synth->ID;
		dialog();
		return;
	}

	switch( s->ID ){
		case SYNTH_CYCLE:  Cycle();   break;
		case SYNTH_FM:     FM();      break;
		case SYNTH_PL:     PL();      break;
		case SYNTH_UNZ:    UNZ();     break;
	}

}

void Synth::Cycle( bool init ){
	pp_int32 ID = SYNTH_CYCLE;

	if( init ){
		synths[ID].nparams = 8;   // < SYN_PARAMS_MAX
		synths[ID].ID      = ID;
		synths[ID].param[0].name  = PPString("\x11 cycle \x10");
		synths[ID].param[0].value = 0.0f;
		synths[ID].param[0].min   = 0;
		synths[ID].param[0].max   = SYNTH_LAST;


		synths[ID].param[1].name  = "volume";
		synths[ID].param[1].value = ((float)SYN_PARAM_MAX_VALUE)/2.0f;
		synths[ID].param[1].min   = 0;
		synths[ID].param[1].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[2].name  = "wave amp";
		synths[ID].param[2].value = 70.0f;
		synths[ID].param[2].min   = 0;
		synths[ID].param[2].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[3].name  = "wave type";
		synths[ID].param[3].value = 1.0f;
		synths[ID].param[3].min   = 1.0f;
		synths[ID].param[3].max   = 9.0f;

		synths[ID].param[4].name  = "multiply";
		synths[ID].param[4].value = 1.0f;
		synths[ID].param[4].min   = 0;
		synths[ID].param[4].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[5].name  = "feedback";
		synths[ID].param[5].value = 0;
		synths[ID].param[5].min   = 0;
		synths[ID].param[5].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[6].name  = "AM";
		synths[ID].param[6].value = 0;
		synths[ID].param[6].min   = 0;
		synths[ID].param[6].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[7].name  = "loop type";
		synths[ID].param[7].value = 1.0f;
		synths[ID].param[7].min   = 1.0f;
		synths[ID].param[7].max   = 2.0f;
		return;
	}

	// determine duration
	TXMSample *sample = sampleEditor->isEmptySample() || !this->additive ? prepareSample(100) : sampleEditor->getSample();

	// synthesize!
	FilterParameters parWave(2);
	parWave.setParameter(0, FilterParameters::Parameter( SYN_PARAM_NORMALIZE(synth->param[2].value) ));
	parWave.setParameter(1, FilterParameters::Parameter( 1.0f * (1.0f+synth->param[4].value) ) );
	switch( (int)synth->param[3].value ){
		case 1: sampleEditor->tool_generateSine(&parWave);         break;
		case 2: sampleEditor->tool_generateSquare(&parWave);       break;
		case 3: sampleEditor->tool_generateTriangle(&parWave);     break;
		case 4: sampleEditor->tool_generateSawtooth(&parWave);     break;
		case 5: sampleEditor->tool_generateHalfSine(&parWave);     break;
		case 6: sampleEditor->tool_generateAbsoluteSine(&parWave); break;
		case 7: 
		case 8:
		case 9: {
					parWave.setParameter(1, FilterParameters::Parameter( ((pp_int32)synth->param[3].value)-7 ) );
					sampleEditor->tool_generateNoise(&parWave);
					break;
				}
	}

	// scale volume
	float scale      = 2.0f * SYN_PARAM_NORMALIZE(synth->param[1].value);
	float foldback   = 1.0f + synth->param[5].value*2.0f; 
	int   AM         = synth->param[6].value;
	for( int i = 0; i < sample->samplen; i++ ){
		float v = sin( sampleEditor->getFloatSampleFromWaveform(i) * foldback );
		if( AM > 0 ) v = v * sin( (float)(i/2) * SYN_PARAM_NORMALIZE(synth->param[6].value) );
		sampleEditor->setFloatSampleInWaveform( i, v * scale );
	}

	// force loop 
	sampleEditor->setLoopType( synth->param[7].value );
	sampleEditor->setRepeatStart(0);
	sampleEditor->setRepeatEnd(sample->samplen);

}

void Synth::FM( bool init ){
	pp_int32 ID = SYNTH_FM;

	if( init ){
		synths[ID].nparams = 23;  // < SYN_PARAMS_MAX
		synths[ID].ID      = ID;
		synths[ID].param[0].name  = PPString("\x11 FM \x10");
		synths[ID].param[0].value = 0.0f;
		synths[ID].param[0].min   = 0;
		synths[ID].param[0].max   = SYNTH_LAST;

		synths[ID].param[1].name  = "volume";
		synths[ID].param[1].value = 27.0f;
		synths[ID].param[1].min   = 0.0f;
		synths[ID].param[1].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[2].name  = "size";
		synths[ID].param[2].value = 4.0f;
		synths[ID].param[2].min   = 1.0f;
		synths[ID].param[2].max   = 12.0f;

		synths[ID].param[3].name  = "attack";
		synths[ID].param[3].value = 0.0f;
		synths[ID].param[3].min   = 0;
		synths[ID].param[3].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[4].name  = "decay";
		synths[ID].param[4].value = 7.0f;
		synths[ID].param[4].min   = 1.0f;
		synths[ID].param[4].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[5].name  = "release";
		synths[ID].param[5].value = 9.0f;
		synths[ID].param[5].min   = 0;
		synths[ID].param[5].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[6].name  = "algorithm";
		synths[ID].param[6].value = 0.0f;
		synths[ID].param[6].min   = 0;
		synths[ID].param[6].max   = 4;

		synths[ID].param[7].name  = "carrier freq";
		synths[ID].param[7].value = 24.0f;
		synths[ID].param[7].min   = 0;
		synths[ID].param[7].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[8].name  = "  SIN SQ SAW TRI NOIZ";
		synths[ID].param[8].value = 1.0f;
		synths[ID].param[8].min   = 1;
		synths[ID].param[8].max   = 5.0f;
		
		synths[ID].param[9].name  = "  AMP              ";
		synths[ID].param[9].value = 92.0f;
		synths[ID].param[9].min   = 0.0;
		synths[ID].param[9].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[10].name  = "mod freq";
		synths[ID].param[10].value = 49.0f;
		synths[ID].param[10].min   = 0;
		synths[ID].param[10].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[11].name  = "  SIN SQ SAW TRI NOIZ";
		synths[ID].param[11].value = 1.0f;
		synths[ID].param[11].min   = 1.0;
		synths[ID].param[11].max   = 5.0f; 

		synths[ID].param[12].name  = "  AMP             ";
		synths[ID].param[12].value = 79.0f; 
		synths[ID].param[12].min   = 0;
		synths[ID].param[12].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[13].name  = "NO AM FM RI TRE VIB KS";
		synths[ID].param[13].value = 3.0f;
		synths[ID].param[13].min   = 0.0f;
		synths[ID].param[13].max   = 6.0f;

		synths[ID].param[14].name  = "transient";
		synths[ID].param[14].value = 92.0f;
		synths[ID].param[14].min   = 0;
		synths[ID].param[14].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[15].name  = "  SIZE           ";
		synths[ID].param[15].value = 15.0f;
		synths[ID].param[15].min   = 1.0f;
		synths[ID].param[15].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[16].name  = "feedback";
		synths[ID].param[16].value = 11.0f;
		synths[ID].param[16].min   = 0.0f;
		synths[ID].param[16].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[17].name  = "delay";
		synths[ID].param[17].value = 0.0f;
		synths[ID].param[17].min   = 0.0f;
		synths[ID].param[17].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[18].name  = "  SIZE          ";
		synths[ID].param[18].value = 10.0f;
		synths[ID].param[18].min   = 0.0f;
		synths[ID].param[18].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[19].name  = "  FEEDBACK      ";
		synths[ID].param[19].value = 0.1f;
		synths[ID].param[19].min   = 0.1f;
		synths[ID].param[19].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[20].name  = "spacetime";
		synths[ID].param[20].value = 61.0f;
		synths[ID].param[20].min   = 0.0f;
		synths[ID].param[20].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[21].name  = "filter";
		synths[ID].param[21].value = 0.0f;
		synths[ID].param[21].min   = 0.0f;
		synths[ID].param[21].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[22].name  = "loop type";
		synths[ID].param[22].value = 3.0f;
		synths[ID].param[22].min   = 0.0f;
		synths[ID].param[22].max   = 10.0f;
		return;
	}

	// setup synth
	fm_control_t controls;
	fm_t instrument;
	pp_uint32 srate = 44100;

	switch( (int)synth->param[13].value ){
		case 0: controls.modulation = MODULATION_NONE;      break; 
		case 1: controls.modulation = MODULATION_AMPLITUDE; break;
		case 2: controls.modulation = MODULATION_FREQUENCY; break;
		case 3: controls.modulation = MODULATION_RING;      break;
		case 4: controls.modulation = MODULATION_TREMOLO;   break;
		case 5: controls.modulation = MODULATION_VIBRATO;   break;
		case 6: controls.modulation = MODULATION_AMPLITUDE; break;
	}

	// init carrier wave
	switch( (int)synth->param[8].value ){
		case 0: controls.carrier = OSCILLATOR_ZERO;     break;
		case 1: controls.carrier = OSCILLATOR_SINE;     break;
		case 2: controls.carrier = OSCILLATOR_SQUARE;   break;
		case 3: controls.carrier = OSCILLATOR_SAWTOOTH; break;
		case 4: controls.carrier = OSCILLATOR_TRIANGLE; break;
		case 5: controls.carrier = OSCILLATOR_NOISE;    break;
	}
	controls.carrier_amplitude = SYN_PARAM_NORMALIZE(synth->param[9].value);

	// init modulator wave
	switch( (int)synth->param[11].value ){
		case 0: controls.modulator = OSCILLATOR_ZERO;     break;
		case 1: controls.modulator = OSCILLATOR_SINE;     break;
		case 2: controls.modulator = OSCILLATOR_SQUARE;   break;
		case 3: controls.modulator = OSCILLATOR_SAWTOOTH; break;
		case 4: controls.modulator = OSCILLATOR_TRIANGLE; break;
		case 5: controls.modulator = OSCILLATOR_NOISE;    break;
	}

	controls.modulator_amplitude = SYN_PARAM_NORMALIZE( synth->param[12].value );

	instrument.modulator.phase = 0;
	instrument.carrier.phase   = 0;

	// since the slider-resolution is limited due to ASCIISYNTH spec 
	// applying different resolution via a slider offers more sonic abilities 
	float mf = 0.01f + SYN_PARAM_NORMALIZE(synth->param[10].value);
	controls.attack  = SYN_PARAM_NORMALIZE(synth->param[3].value);
	controls.decay   = SYN_PARAM_NORMALIZE(synth->param[4].value);
	controls.sustain = SYN_PARAM_NORMALIZE(synth->param[4].value);
	controls.release = SYN_PARAM_NORMALIZE(synth->param[5].value) * 0.5f;
  
	// init delay
	memset( &(instrument.echo.buffer), 0, sizeof(float)*ECHO_BUFFER_SIZE );
	instrument.echo.cursor = 0;
	float delay = SYN_PARAM_NORMALIZE(synth->param[18].value);
	controls.echo_delay_samples = (int)( (delay*delay*delay) * float(srate/2) );
	controls.echo_karplustrong = synth->param[13].value == 6;
    controls.echo_feedback = SYN_PARAM_NORMALIZE( synth->param[19].value);
	controls.echo_level = SYN_PARAM_NORMALIZE(synth->param[17].value);

	// init filter
	float filter     = SYN_PARAM_NORMALIZE(synth->param[21].value);
    controls.filter = filter > 0.05 ? FILTER_BANDPASS: FILTER_NONE;
    controls.filter_freq = float(srate/2)*(filter*filter*filter);
    controls.filter_resonance = 0.5;
    controls.filter_gain = 0.8;
	if( controls.echo_karplustrong ){ // https://en.wikipedia.org/wiki/Karplus%E2%80%93Strong_string_synthesis
		controls.filter = FILTER_LOWPASS;
		controls.filter_gain = 10.0;
		controls.filter_freq = controls.filter_freq < 150.0f ? 150.0f : controls.filter_freq;
		controls.echo_level = 1.0;
	}

	controls.spacetime = SYN_PARAM_NORMALIZE(synth->param[20].value);
	controls.feedback  = 1.0f + (100.0f * SYN_PARAM_NORMALIZE(synth->param[16].value));

	switch( (int)synth->param[6].value ){
		// syncs modulator & carrier to note frequenceis
		case 1: {
					instrument.carrier.freq   = NOTE2HZ( NOTE_START + (int)synth->param[7].value );
					controls.modulator_freq = NOTE2HZ(NOTE_START +  (int)synth->param[10].value );
					break;
				}

		// syncs modulator & carrier to note frequenceis + phasediff
		case 2: {
					instrument.carrier.freq = NOTE2HZ( NOTE_START + (int)synth->param[7].value );
					controls.modulator_freq = NOTE2HZ(NOTE_START +  (int)synth->param[10].value );
          instrument.modulator.phase = 0.3; 
					break;
				}

		// carrier freqs synced to notes + freeform modulator freqs 
		case 3: {
					instrument.carrier.freq   = NOTE2HZ( (int)synth->param[7].value );
					float mf = NOTE_START + SYN_PARAM_NORMALIZE(synth->param[10].value);
					controls.modulator_freq = SYN_PARAM_NORMALIZE( synth->param[10].value ) * float(srate)/2.0;
					break;
				}

		// like default but very slow LFO 
		case 4: {
					instrument.carrier.freq = NOTE2HZ( NOTE_START + (int)synth->param[7].value );
					controls.modulator_freq = mf * 0.01f;
					break;
				}		

		// basic focus on modulator freq as fast LFO 
		case 0:
		default:{
					instrument.carrier.freq = NOTE2HZ( NOTE_START + (int)synth->param[7].value );
					controls.modulator_freq = mf * (4.0 / float(srate)); 
					break;
				}

	}


	// determine duration
	pp_uint32 samples = (srate/6) * (int)synth->param[2].value; // 300ms * param
	// enable overflow rendering when loop is set to forward
	// to allow seamless loops without clicks
	pp_uint32 overflow = 3;
	pp_uint32 looptype = (pp_uint32)synth->param[22].value;
	pp_uint32 silence  = 0;
	float     last     = 0.0;
	if( looptype == 1 ){ // overflow until silence with forward loop
		overflow = controls.spacetime > 0.1 ? (int)(1.0 + controls.spacetime*10) : overflow;
	}
	TXMSample *sample = sampleEditor->isEmptySample() || !this->additive ? prepareSample(samples) : sampleEditor->getSample();

	// exponential positive drive into sin() function (produces foldback/freq multiply)
	// see curve @ https://graphtoy.com/?f1(x,t)=max(0,(x*10*x*x)%20)%20+x&v1=true 
	float scale    = SYN_PARAM_NORMALIZE(synth->param[1].value);
 	scale          = fmax(0,(scale*3*scale*scale))+scale; // exponential in positive side
	int frames     = overflow*(int)samples;
	float* smpin;
	smpin           = (float*)calloc(frames,  sizeof(float));
    float transFreq = instrument.carrier.freq; 
	float x;

	// synthesize! 
	for( pp_int32 i = 0; i < frames; i++ ){

		// apply transient to freq controllers (curve: tanh(x*x*x*1000)-1 )
		pp_uint32 transSamples = (pp_uint32)( float(srate)/2 * SYN_PARAM_NORMALIZE(synth->param[15].value ) ); 
		float offset   = (1.0f/(float)transSamples) * float(i);

		// add transient
		float transAmp = SYN_PARAM_NORMALIZE(synth->param[14].value);
		instrument.carrier.freq = transFreq + (transAmp * (transFreq * (tanh(-offset*offset*offset*1000)+1)*(10*transAmp)));

		SynthFM::instrument_control( &instrument, &controls, srate );

		// trigger note
		if( i == 0 ) SynthFM::adsr_trigger( &(instrument.adsr) );

		SynthFM::instrument_play( &instrument, srate, &x);
		smpin[i] = x;

		// cancel overflow rendering if lots of silence
		if( last == 0.0 && x == 0.0 ) silence++;
		if( i > samples && silence > 5) break;
		last = x;
	}
	// apply reverb  
	if( controls.spacetime > 0.04 ){    // avoid comb effect
		float* smpout;
		int size = (int)(controls.spacetime*50000.0);
		// apply reverb 
		int outlength = Convolver::reverb( smpin, &smpout, frames, size );

		for( pp_int32 i = 0; i < frames; i++ ){
			// after synthesized sample, overdub the existing sample from then on 
			float old = i < samples ? 0.0f : sampleEditor->getFloatSampleFromWaveform( i % (int)samples);
			//// one-slider reverb: amplify wet with curve 
			float   x = controls.spacetime;
			float wet = smpout[i] * (x*x*2);
			float dry = smpin[i]  * ((-x*x*x)+1);
			sampleEditor->setFloatSampleInWaveform( i % (int)samples, old + (( dry + wet ) * scale) );
		}
		free(smpout);
	}else{
		for( pp_int32 i = 0; i < frames; i++ ){
			// after synthesized sample, overdub the existing sample from then on 
			float old = i < samples ? 0.0f : sampleEditor->getFloatSampleFromWaveform( i % (int)samples);
			sampleEditor->setFloatSampleInWaveform( i % (int)frames, old + (smpin[i]*scale) );
		}
	}

	// force loop 
	if( looptype > 0 ){
		sampleEditor->setRepeatStart(0);
		sampleEditor->setRepeatEnd(sample->samplen);
	}
	sampleEditor->setLoopType( looptype > 2 ? 1 : looptype );
	if( looptype >= 3 ){
		FilterParameters parWave(2);
		for( pp_int32 i = 0; i < looptype-2; i++ ){
			sampleEditor->selectAll();
			sampleEditor->tool_foldSample(NULL);
			parWave.setParameter(0, FilterParameters::Parameter( 190.0f ) );
			parWave.setParameter(1, FilterParameters::Parameter( 190.0f ) );
		    sampleEditor->tool_scaleSample(&parWave); 
		}
	}

	sampleEditor->notifyListener(SampleEditor::NotificationChanges); // update UI
	free(smpin);
}

void Synth::PL( bool init ){
	pp_int32 ID = SYNTH_PL;


	// slider: osc0_vol / osc1_vol

	if( init ){
		synths[ID].nparams = 23;  // < SYN_PARAMS_MAX
		synths[ID].ID      = ID;
		synths[ID].param[0].name  = PPString("\x11 SONANT \x10");
		synths[ID].param[0].value = 0.0f;
		synths[ID].param[0].min   = 0;
		synths[ID].param[0].max   = SYNTH_LAST;
		
		synths[ID].param[1].name  = "osc mix";
		synths[ID].param[1].value = (float)SYN_PARAM_MAX_VALUE/2;
		synths[ID].param[1].min   = 0.0f;
		synths[ID].param[1].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[2].name  = "attack";
		synths[ID].param[2].value = 0.0f;
		synths[ID].param[2].min   = 0.0f;
		synths[ID].param[2].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[3].name  = "decay";
		synths[ID].param[3].value = 67.0f;
		synths[ID].param[3].min   = 1.0f;
		synths[ID].param[3].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[4].name  = "noise";
		synths[ID].param[4].value = 0.0f;
		synths[ID].param[4].min   = 0.0f;
		synths[ID].param[4].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[5].name  = "osc waveforms";
		synths[ID].param[5].value = 10.0f;
		synths[ID].param[5].min   = 0.0f;
		synths[ID].param[5].max   = 16.0f; // 4x4 osc1_waveform && osc2_waveform 
		
		synths[ID].param[6].name  = "  OCTAVES         ";
		synths[ID].param[6].value = 5.0f;
		synths[ID].param[6].min   = 0.0f;
		synths[ID].param[6].max   = 16.0f;

		synths[ID].param[7].name  = "  ENVELOPE MODE   ";
		synths[ID].param[7].value = 0.0f;
		synths[ID].param[7].min   = 0.0f;
		synths[ID].param[7].max   = 4.0f;
		
		synths[ID].param[8].name  = "osc0";
		synths[ID].param[8].value = 0.0f;
		synths[ID].param[8].min   = 0.0f;
		synths[ID].param[8].max   = (float)SYN_PARAM_MAX_VALUE/2;

		synths[ID].param[9].name  =  "  DETUNE         ";
		synths[ID].param[9].value = 0.0f;
		synths[ID].param[9].min   = 0.0f;
		synths[ID].param[9].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[10].name  = "osc1";
		synths[ID].param[10].value = 0.0f;
		synths[ID].param[10].min   = 0.0f;
		synths[ID].param[10].max   = (float)SYN_PARAM_MAX_VALUE/2;

		synths[ID].param[11].name  =  "  DETUNE         ";
		synths[ID].param[11].value = 0.0f;
		synths[ID].param[11].min   = 0.0f;
		synths[ID].param[11].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[12].name  = "filter type";
		synths[ID].param[12].value = 2.0f;
		synths[ID].param[12].min   = 1.0f;
		synths[ID].param[12].max   = 4.0f;
		
		synths[ID].param[13].name  = "  FREQ         ";
		synths[ID].param[13].value = (float)SYN_PARAM_MAX_VALUE/2;
		synths[ID].param[13].min   = 1.0f;
		synths[ID].param[13].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[14].name  = "  RESONANCE    ";
		synths[ID].param[14].value = (float)SYN_PARAM_MAX_VALUE/2;
		synths[ID].param[14].min   = 5.0f;
		synths[ID].param[14].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[15].name  = "LFO amp/filter";
		synths[ID].param[15].value = 1.0f;
		synths[ID].param[15].min   = 0.0f;
		synths[ID].param[15].max   = 3.0f; // 0 = none 1 = amp 2 = filt 3 = amp+filt
										   
		synths[ID].param[16].name  = "  FREQ         ";
		synths[ID].param[16].value = (float)SYN_PARAM_MAX_VALUE/3;
		synths[ID].param[16].min   = 0.0f;
		synths[ID].param[16].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[17].name  = "  AMOUNT       ";
		synths[ID].param[17].value = 39.0f;
		synths[ID].param[17].min   = 0.0f;
		synths[ID].param[17].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[18].name  = "  WAVEFORM     ";
		synths[ID].param[18].value = 0.0f;
		synths[ID].param[18].min   = 0.0f;
		synths[ID].param[18].max   = 3.0f;
		
		synths[ID].param[19].name  = "delay";
		synths[ID].param[19].value = 0.0f;
		synths[ID].param[19].min   = 0.0f;
		synths[ID].param[19].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[20].name  = "trigger algo";
		synths[ID].param[20].value = 8.0f;
		synths[ID].param[20].min   = 1.0f;
		synths[ID].param[20].max   = 11.0f; // PL pattern 
		
		synths[ID].param[21].name  = "spacetime";
		synths[ID].param[21].value = 32.0f;
		synths[ID].param[21].min   = 0.0f;
		synths[ID].param[21].max   = (float)SYN_PARAM_MAX_VALUE;
										   
		synths[ID].param[22].name  = "loop type";
		synths[ID].param[22].value = 2.0f;
		synths[ID].param[22].min   = 0.0f;
		synths[ID].param[22].max   = 10.0f;

		return;
	}

	// setup synth
	pp_uint32 srate = 44100;
	// enable overflow rendering when loop is set to forward
	// to allow seamless loops without clicks
	pp_uint32 overflow = 1;
	pp_uint32 looptype = (pp_uint32)synth->param[22].value;
	pp_uint32 silence  = 0;
	float     osc0     = 1.0;
	float     osc1     = 1.0;
	float     space    = SYN_PARAM_NORMALIZE(synth->param[21].value);

	// setup oscillator waveform types + octaves
	uint8_t osc0_waveform;
	uint8_t osc1_waveform;
	uint8_t osc0_oct;
	uint8_t osc1_oct;
	uint8_t osc0_vol = (uint8_t)(255.0f * (1.0f - SYN_PARAM_NORMALIZE(synth->param[1].value)));
	uint8_t osc1_vol = (uint8_t)(255.0f * SYN_PARAM_NORMALIZE(synth->param[1].value));
	uint8_t osc0_xenv;
	uint8_t osc1_xenv;

	// below we do some demultiplexing of params [so it all fits into ASCIISYNTH format]
	for (int x = 0; x <= 3; x++) {
        for (int y = 0; y <= 3; y++) {
            if (x * 4 + y == (int)synth->param[5].value) {
				osc0_waveform = x;
				osc1_waveform = y;
            }
            if (x * 4 + y == (int)synth->param[6].value) {
				osc0_oct = 10 + (int8_t)(x-2) ;
				osc1_oct = 10 + (int8_t)(y-2);
            }
        }
    }
	switch( (int)synth->param[7].value ) {
		case 0: osc0_xenv = 0; osc1_xenv = 0; break;
		case 1: osc0_xenv = 1; osc1_xenv = 0; break;
		case 2: osc0_xenv = 0; osc1_xenv = 1; break;
		case 3: osc0_xenv = 1; osc1_xenv = 1; break;
	}

	// Initialize the lookup table for the oscillators
	void *synth_tab = (void *)malloc(PL_SYNTH_TAB_SIZE);
	pl_synth_init((float*)synth_tab);
	pl_synth_t s = {
		osc0_oct,// uint8_t osc0_oct;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[8].value)),// uint8_t osc0_det;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[9].value)),// uint8_t osc0_detune;
		osc0_xenv, // uint8_t osc1_xenv;
		osc0_vol,// uint8_t osc0_vol;
		osc0_waveform,// uint8_t osc0_waveform;

		osc1_oct,// uint8_t osc1_oct;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[10].value)),// uint8_t osc1_det;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[11].value)),// uint8_t osc1_detune;
		osc1_xenv, // uint8_t osc1_xenv;
		osc1_vol,// uint8_t osc1_vol;
		osc1_waveform,// uint8_t osc1_waveform;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[4].value)),// uint8_t noise_fader;

		(uint32_t)(20000.0f*SYN_PARAM_NORMALIZE(synth->param[2].value)),// uint32_t env_attack;
		(uint32_t)(20000.0f*SYN_PARAM_NORMALIZE(synth->param[3].value)),// uint32_t env_sustain;
		(uint32_t)(20000.0f*SYN_PARAM_NORMALIZE(synth->param[3].value)),// uint32_t env_release;
		((uint8_t)synths[ID].param[20].value) == 1 ? 512: 400,// uint32_t env_master;

		(uint8_t)synth->param[12].value, // uint8_t fx_filter;
		(uint32_t)((48000.0f/4.0f)*SYN_PARAM_NORMALIZE(synth->param[13].value)), // uint32_t fx_freq;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[14].value)), // uint32_t fx_res;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[19].value)),// uint8_t fx_delay_time;
		synth->param[19].value > 0.05f ? 128 : 0,// uint8_t fx_delay_amt;
		0,// uint8_t fx_pan_freq;
		0,// uint8_t fx_pan_amt;

		synth->param[15].value == 0.0f || synth->param[15].value == 3.0f, // uint8_t lfo_osc_freq;
		synth->param[15].value != 0.0f || synth->param[15].value == 3.0f, // uint8_t lfo_fx_freq;
		(uint8_t)(14.0f*SYN_PARAM_NORMALIZE(synth->param[16].value)), // uint8_t lfo_freq;
		(uint8_t)(255.0f*SYN_PARAM_NORMALIZE(synth->param[17].value)), // uint8_t lfo_amt;
		(uint8_t)synth->param[18].value // uint8_t lfo_waveform;
	};
	
	// A sound is described by an instrument (synth), the row_len in samples and a note.
	pl_synth_sound_t sound = {
		s,      // .synth
		5168*2, // .row_len
		135,    // .note
	};

	// the chords are unnamed & the comments might not be theoretically correct (please add new ones to not break user presets)
	pl_synth_song_t song = {
		(uint8_t)synths[ID].param[20].value < 9? 256 : 768, // row_len
		1,                                                   // num_tracks
		new pl_synth_track_t[1]{                             // tracks
			{
				s, // synth
				1, // sequence_len
				new uint8_t[1]{ (uint8_t)synths[ID].param[20].value }, // sequence
				new pl_synth_pattern_t[11]{                               
					{
						{135} // notes (single note)
					},
					{
						{135,135+12} // notes (perfect diad)
					},
					{
						{135,135+15} // notes (minor diad)
					},
					{
						{135,135+14} // notes (major diad)
					},
					{
						{135,135+7} // notes (perfect fift diad)
					},
					{
						{135,135+4,135+7} // notes (major triad)
					},
					{
						{135,135+3,135+7} // notes (minor triad)
					},
					{
						{135,135+4,135+7,135+11} // notes (major 7th)
					},
					{
						{135,135+3,135+7,135+11} // notes (minor 7th)
					},
					// arp minor 7th
					{
						{135,0,0,0,135+3,0,0,0,135+7,0,0,0,135+11,0,0,0,135+7,0,0,0,135+3,0,0,0}
					},
					// arp major 7th
					{
						{135,0,0,0,135+4,0,0,0,135+7,0,0,0,135+11,0,0,0,135+7,0,0,0,135+4,0,0,0}
					}
				}
			}
		}
	};

	// Determine the number of the samples for a sound effect and allocate the
	// sample buffer for both (stereo) channels
	//pp_uint32 samples = pl_synth_sound_len(&sound);
	pp_uint32 samples = pl_synth_song_len(&song);
	if( samples < 0 ) return; // something went wrong
							  
	int frames   = samples;
							  //
	TXMSample *sample = sampleEditor->isEmptySample() || !this->additive ? prepareSample(samples) : sampleEditor->getSample();
	int16_t *sample_buffer = (int16_t*)malloc(samples * 2 * sizeof(int16_t));
	int16_t *temp_samples = (int16_t*)malloc(samples * 2 * sizeof(int16_t));
	memset(sample_buffer,0,samples * 2 * sizeof(int16_t));
	
	// exponential positive drive into sin() function (produces foldback/freq multiply)
	// see curve @ https://graphtoy.com/?f1(x,t)=max(0,(x*10*x*x)%20)%20+x&v1=true 
	float* smpin;
	smpin           = (float*)calloc(frames,  sizeof(float));

	// Generate the samples
	//pl_synth_sound(&sound, sample_buffer);
	pl_synth_song(&song, sample_buffer, temp_samples);

	// synthesize! 
	if( looptype == 4){ // overdub half the sample
		for( pp_int32 i = 0; i < samples; i++ ){ 
			smpin[i % frames ] = (float) sample_buffer[i*2] / 32767.0f;
		}
	}else{
		for( pp_int32 i = 0; i < frames; i++ ){
			smpin[i] = i < samples ? (float) sample_buffer[i*2] / 32767.0f : 0.0f;
		}
	}

	// apply reverb  
	if( space > 0.04 ){    // avoid comb effect
		float* smpout;
		int size = (int)(space*50000.0);
		// apply reverb 
		int outlength = Convolver::reverb( smpin, &smpout, frames, size );

		for( pp_int32 i = 0; i < frames; i++ ){
			// after synthesized sample, overdub the existing sample from then on 
			float old = i < samples ? 0.0f : sampleEditor->getFloatSampleFromWaveform( i % (int)samples);
			//// one-slider reverb: amplify wet with curve 
			float   x = space;
			float wet = smpout[i] * (x*x*2);
			float dry = smpin[i]  * ((-x*x*x)+1);
			sampleEditor->setFloatSampleInWaveform( i % (int)samples, old + ( dry + wet ) );
		}
		free(smpout);
	}else{
		for( pp_int32 i = 0; i < frames; i++ ){
			// after synthesized sample, overdub the existing sample from then on 
			float old = i < samples ? 0.0f : sampleEditor->getFloatSampleFromWaveform( i % (int)samples);
			sampleEditor->setFloatSampleInWaveform( i % (int)frames, old + (smpin[i]) );
		}
	}

	// force loop 
	if( looptype > 0 ){
		sampleEditor->setRepeatStart(0);
		sampleEditor->setRepeatEnd(sample->samplen);
	}
	sampleEditor->setLoopType( looptype > 2 ? 1 : looptype );
	if( looptype >= 3 ){

		FilterParameters parWave(2);
		for( pp_int32 i = 0; i < looptype-2; i++ ){
			sampleEditor->selectAll();
			sampleEditor->tool_foldSample(NULL);
			parWave.setParameter(0, FilterParameters::Parameter( 190.0f ) );
			parWave.setParameter(1, FilterParameters::Parameter( 190.0f ) );
		    sampleEditor->tool_scaleSample(&parWave); 
		}
	}

	sampleEditor->notifyListener(SampleEditor::NotificationChanges); // update UI
																	 //
	free(synth_tab);
	free(sample_buffer);
	free(temp_samples);
	free(smpin);
	delete[] song.tracks[0].sequence;
	delete[] song.tracks[0].patterns;
	delete[] song.tracks;

}

void Synth::UNZ( bool init ){
	pp_int32 ID = SYNTH_UNZ;

	if( init ){
		synths[ID].nparams = 21;   // < SYN_PARAMS_MAX
		synths[ID].ID      = ID;
		synths[ID].param[0].name  = PPString("\x11 UNZ! \x10");
		synths[ID].param[0].value = 0.0f;
		synths[ID].param[0].min   = 0;
		synths[ID].param[0].max   = SYNTH_LAST;


		synths[ID].param[1].name  = "volume";
		synths[ID].param[1].value = 57.0f;
		synths[ID].param[1].min   = 0;
		synths[ID].param[1].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[2].name  = "size";
		synths[ID].param[2].value = 51.0f;
		synths[ID].param[2].min   = 0;
		synths[ID].param[2].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[3].name  = "amp decay";
		synths[ID].param[3].value = 84.0f;
		synths[ID].param[3].min   = 0;
		synths[ID].param[3].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[4].name  = "frequency base";
		synths[ID].param[4].value = 79.0f;
		synths[ID].param[4].min   = 0;
		synths[ID].param[4].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[5].name  = "  BEGIN         ";
		synths[ID].param[5].value = 52.0f;
		synths[ID].param[5].min   = 1.0;
		synths[ID].param[5].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[6].name  = "  MIDDLE        ";
		synths[ID].param[6].value = 68.0f;
		synths[ID].param[6].min   = 0;
		synths[ID].param[6].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[7].name  = "  END           ";
		synths[ID].param[7].value = 12.0f;
		synths[ID].param[7].min   = 1.0;
		synths[ID].param[7].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[8].name  = "  SHAPE         ";
		synths[ID].param[8].value = 66.0f;
		synths[ID].param[8].min   = 0;
		synths[ID].param[8].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[9].name  = "noise";
		synths[ID].param[9].value = 0.0f;
		synths[ID].param[9].min   = 0;
		synths[ID].param[9].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[10].name  = "  TRANSIENT VOL  ";
		synths[ID].param[10].value = 61.0f;
		synths[ID].param[10].min   = 0;
		synths[ID].param[10].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[11].name  = "  TRANSIENT SIZE";
		synths[ID].param[11].value = 25.0f;
		synths[ID].param[11].min   = 0;
		synths[ID].param[11].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[12].name  = "  ATTACK       ";
		synths[ID].param[12].value = 30.0f;
		synths[ID].param[12].min   = 0;
		synths[ID].param[12].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[13].name  = "  SUSTAIN      ";
		synths[ID].param[13].value = 0;
		synths[ID].param[13].min   = 0;
		synths[ID].param[13].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[14].name  = "  RELEASE      ";
		synths[ID].param[14].value = 10;
		synths[ID].param[14].min   = 0;
		synths[ID].param[14].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[15].name  = "reverb";
		synths[ID].param[15].value = 1.0f;
		synths[ID].param[15].min   = 1.0f;
		synths[ID].param[15].max   = (float)SYN_PARAM_MAX_VALUE;
		
		synths[ID].param[16].name  = "  HP FREQ      ";
		synths[ID].param[16].value = 50.0f;
		synths[ID].param[16].min   = 0.0f;
		synths[ID].param[16].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[17].name  = "  HP Q         ";
		synths[ID].param[17].value = 10.0f;
		synths[ID].param[17].min   = 0.0f;
		synths[ID].param[17].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[18].name  = "  SIZE         ";
		synths[ID].param[18].value = 5.0f;
		synths[ID].param[18].min   = 1.0f;
		synths[ID].param[18].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[19].name  = "feedback";
		synths[ID].param[19].value = 1.0f;
		synths[ID].param[19].min   = 1.0f;
		synths[ID].param[19].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[20].name  = "attack";
		synths[ID].param[20].value = 0.0f;
		synths[ID].param[20].min   = 0.0f;
		synths[ID].param[20].max   = (float)SYN_PARAM_MAX_VALUE;

		return;
	}

	float attack = 0.05f * SYN_PARAM_NORMALIZE(synth->param[20].value);

	// Parameters for FSM kick 
	// Copyright (C) 2007 Krzysztof Foltman  <kfoltman@users.sourceforge.net>
    // this is an LLM resynthesis of FSM Kick [GPL2 or later] 
	float startFrq = ( SYN_PARAM_NORMALIZE(synth->param[5].value) *  280.0f) * 100.0;
	float midFrq = ( SYN_PARAM_NORMALIZE(synth->param[6].value) *  140.0f) * 2.0;
	float endFrq = ( SYN_PARAM_NORMALIZE(synth->param[7].value) *  140.0f) * 2.0;
	float tDecayRev = 1.0 - SYN_PARAM_NORMALIZE(synth->param[4].value);
	float tDecay = 0.1 * tDecayRev;
	float tShape = 0.1f * SYN_PARAM_NORMALIZE(synth->param[8].value);
	float aDecay = 10.0f - (SYN_PARAM_NORMALIZE(synth->param[3].value) * 10.0f);
	float curVolume = 1.0f;
	float xSin = 0.0f, xCos = 1.0f, dxSin, dxCos;
	float amp = curVolume, mulAmp = powf(1.0f / 256.0f, aDecay / 5000.0f);
	float freq = startFrq;
	float antiClick = 0.0f, lastValue = 0.0f;

	// We've extended FSM kick with a noise generator so it can produce snaredrums too
	// Parameters for noise generator
	float noiseTransientAmp = SYN_PARAM_NORMALIZE(synth->param[10].value);   // Attack duration (seconds)
	float noiseTransientSize = 0.005f * SYN_PARAM_NORMALIZE(synth->param[11].value);   // Attack duration (seconds)
	float noiseAttack = 0.05f  * SYN_PARAM_NORMALIZE(synth->param[12].value);   // Attack duration (seconds)
	float noiseSustain = 0.05f * SYN_PARAM_NORMALIZE(synth->param[13].value);  // Sustain duration (seconds)
	float noiseRelease = 0.1f * SYN_PARAM_NORMALIZE(synth->param[14].value);  // Release duration (seconds)
	float noiseVolume = 1.5f * SYN_PARAM_NORMALIZE(synth->param[9].value);   // Noise amplitude
	bool noiseTail   = (noiseAttack + noiseSustain + noiseRelease) > 0.0f;

	pp_int32 samplerate = 44100;

	// reverb
	filter_t hp;
	Filter::init( (filter_t *)&hp, samplerate );
	hp.cutoff       = SYN_PARAM_NORMALIZE(synth->param[16].value) * (float)samplerate/1.8; // allow grit / aliasing above nyquist 
	hp.q            = SYN_PARAM_NORMALIZE(synth->param[17].value);
	float reverbAmp = SYN_PARAM_NORMALIZE(synth->param[15].value) * 5.0f;
	int reverbSize  = (int)( 50000.0f * SYN_PARAM_NORMALIZE(synth->param[18].value));

	// Noise variables
	float noiseEnv = 0.0f;
	float noisePhase = 0.0f;

	// determine duration
	int samples = (int)(10000.0f * SYN_PARAM_NORMALIZE(synth->param[2].value));

	TXMSample *sample = sampleEditor->isEmptySample() || !this->additive ? prepareSample(samples) : sampleEditor->getSample();

	float feedback = 10.0f * SYN_PARAM_NORMALIZE(synth->param[19].value);   // Noise amplitude

	// Synthesis loop
	for (int i = 0; i < sample->samplen; i++) {
		// Calculate kick envelope and frequency
		if (amp < 16.0f && fabs(antiClick) < 256.0f) amp = 0.0f; // Stop condition
		float envPoint = (i * tDecay) / 400.0f;
		float shapedPoint = powf(envPoint, tShape * 2.0f);
	    //	freq = startFrq * powf(endFrq / startFrq, fmax(0.0f, shapedPoint));
		if (shapedPoint < 0.5f) {
			float localT = shapedPoint * 2.0f; // Scale to [0,1]
			freq = startFrq * powf(midFrq / startFrq, fmax(0.0f, localT));
		} else {
			float localT = (shapedPoint - 0.5f) * 2.0f; // Scale to [0,1]
			freq = midFrq * powf(endFrq / midFrq, fmax(0.0f, localT));
		}
		amp = curVolume * powf(1.0f / 256.0f, aDecay * i / 5000.0f);
		dxSin = sinf(2.0f * M_PI * freq / 44100.0f);
		dxCos = cosf(2.0f * M_PI * freq / 44100.0f);
		float kickSample = antiClick + amp * xSin;
		antiClick *= 0.98f;
		float xSin2 = xSin * dxCos + xCos * dxSin;
		float xCos2 = xCos * dxCos - xSin * dxSin;
		xSin = xSin2;
		xCos = xCos2;

		// Calculate noise envelope
		float time = i / (float)samplerate;
		if (time < noiseTransientSize) {
			noiseEnv = (1.0f - (time  /  noiseTransientSize)) * noiseTransientAmp;  // Transient decay 
		} else if (noiseTail && time < noiseTransientSize + noiseAttack) {
			noiseEnv = noiseVolume * (time / noiseAttack);  // Attack phase
		} else if (noiseTail && time < noiseAttack + noiseSustain + noiseTransientSize) {
			noiseEnv = noiseVolume * 1.0f;  // Sustain phase
		} else if (noiseTail && time < noiseAttack + noiseSustain + noiseRelease + noiseTransientSize) {
			noiseEnv = noiseVolume * (noiseAttack + noiseSustain + noiseRelease - time) / noiseRelease;  // Release phase
		} else {
			noiseEnv = 0.0f;  // Silence
		}

		// Generate high-frequency noise
		float noiseSample = noiseEnv  * ((rand() / (float)RAND_MAX) * 2.0f - 1.0f);

		// Mix kick and noise samples
		float finalSample = kickSample + noiseSample;
		finalSample *= (2.0f * SYN_PARAM_NORMALIZE(synth->param[1].value));

		// Write to waveform
		sampleEditor->setFloatSampleInWaveform(i, finalSample);
	}


	// instead of only distorting highfreqs (PTboost) we 
	// filter the highfreqs and smear them with a reverb 
	float* smpin;
	smpin = (float*)calloc(samples,  sizeof(float));
	float* smpout;

	for (int i = 0; i < sample->samplen; i++) { // copy source (and pad with zeros)
		Filter::process( sampleEditor->getFloatSampleFromWaveform(i), (filter_t *)&hp );  // apply HP
		smpin[i] = hp.out_hp;
	}
	// smear and smooth with roomverb
	int outlength = Convolver::reverb( smpin, &smpout, sample->samplen, reverbSize );

	for (int i = 0; i < sample->samplen; i++) // mix reverb 
	{
		float out = sampleEditor->getFloatSampleFromWaveform(i); // dry 
		out = sin( out * (1.0f + feedback) );                   // feedback final result
		out += smpout[i] * reverbAmp;
		// attack
		float time = i / (float)samplerate;
		out = time < attack ? (time / attack ) * out : out;
		sampleEditor->setFloatSampleInWaveform(i, out );
	}

	free(smpin);
	free(smpout);

	// disable loop 
	sampleEditor->setLoopType( 0 );
	sampleEditor->setRepeatStart(0);
	sampleEditor->setRepeatEnd(sample->samplen);

}

