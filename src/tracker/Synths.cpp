
/*
 *  tracker/Synth.h 
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

#include "DialogSliders.h"
#include "FilterParameters.h"
#include "XModule.h"
#include "SampleEditor.h"
#include <math.h>

/*
 * Lightweight synths from the milkyverse.
 *
 * Rules for new synths:
 *   1. don't include new classes/libraries/FFT libraries e.g., KISS.
 *   2. re-use SampleEditor operations (recursive sample-editing, CyclePaint is an example)  
 *   3. please use volume as first slider and ADSR after that (if any)
 */


void Synth::init(){
	FMPaint(true);
	CyclePaint(true); 
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
		dialog(NULL,NULL,NULL);
		return;
	}

	switch( s->ID ){
		case SYNTH_CYCLE_PAINT:  CyclePaint();   break;
		case SYNTH_FM_PAINT:    FMPaint();     break;
	}
}

void Synth::CyclePaint( bool init ){
	pp_int32 ID = SYNTH_CYCLE_PAINT;

	if( init ){
		synths[ID].nparams = 7;   // < SYN_PARAMS_MAX
		synths[ID].ID      = ID;
		synths[ID].param[0].name  = PPString("\x11 paint wave \x10");
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

		synths[ID].param[4].name  = "harmonic";
		synths[ID].param[4].value = 0.0f;
		synths[ID].param[4].min   = 0;
		synths[ID].param[4].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[5].name  = "feedback";
		synths[ID].param[5].value = 0;
		synths[ID].param[5].min   = 0;
		synths[ID].param[5].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[6].name  = "loop type";
		synths[ID].param[6].value = 1.0f;
		synths[ID].param[6].min   = 1.0f;
		synths[ID].param[6].max   = 2.0f;
		return;
	}

	// determine duration
	TXMSample *sample = sampleEditor->isEmptySample() ? prepareSample(100) : sampleEditor->getSample();

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
	float scale    = 2.0f * SYN_PARAM_NORMALIZE(synth->param[1].value);
	float foldback = 1.0f + synth->param[5].value*2.0f; 
	for( int i = 0; i < sample->samplen; i++ )
		sampleEditor->setFloatSampleInWaveform( i, sin( sampleEditor->getFloatSampleFromWaveform(i) * foldback ) * scale );

	// force loop 
	sampleEditor->setLoopType( synth->param[6].value );
	sampleEditor->setRepeatStart(0);
	sampleEditor->setRepeatEnd(sample->samplen);

}

void Synth::FMPaint( bool init ){
	pp_int32 ID = SYNTH_FM_PAINT;

	if( init ){
		synths[ID].nparams = 23;  // < SYN_PARAMS_MAX
		synths[ID].ID      = ID;
		synths[ID].param[0].name  = PPString("\x11 paint FM \x10");
		synths[ID].param[0].value = 0.0f;
		synths[ID].param[0].min   = 0;
		synths[ID].param[0].max   = SYNTH_LAST;

		synths[ID].param[1].name  = "volume";
		synths[ID].param[1].value = 38.0f;
		synths[ID].param[1].min   = 0.0f;
		synths[ID].param[1].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[2].name  = "size";
		synths[ID].param[2].value = 3.0f;
		synths[ID].param[2].min   = 1.0f;
		synths[ID].param[2].max   = 12.0f;

		synths[ID].param[3].name  = "attack";
		synths[ID].param[3].value = 0.0f;
		synths[ID].param[3].min   = 0;
		synths[ID].param[3].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[4].name  = "decay";
		synths[ID].param[4].value = 4.0f;
		synths[ID].param[4].min   = 1.0f;
		synths[ID].param[4].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[5].name  = "sustain";
		synths[ID].param[5].value = 4.0f;
		synths[ID].param[5].min   = 0;
		synths[ID].param[5].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[6].name  = "release";
		synths[ID].param[6].value = 2.0f;
		synths[ID].param[6].min   = 0;
		synths[ID].param[6].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[7].name  = "carrier freq";
		synths[ID].param[7].value = 0.0f;
		synths[ID].param[7].min   = 0;
		synths[ID].param[7].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[8].name  = "  SIN SQ SAW TRI NOIZ";
		synths[ID].param[8].value = 1.0f;
		synths[ID].param[8].min   = 1;
		synths[ID].param[8].max   = 5.0f;
		
		synths[ID].param[9].name  = "  AMP              ";
		synths[ID].param[9].value = (float)SYN_PARAM_MAX_VALUE;
		synths[ID].param[9].min   = 0.0;
		synths[ID].param[9].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[10].name  = "mod freq";
		synths[ID].param[10].value = 40.0f;
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
		synths[ID].param[15].value = 8.0f;
		synths[ID].param[15].min   = 1.0f;
		synths[ID].param[15].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[16].name  = "feedback";
		synths[ID].param[16].value = 9.0f;
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
		synths[ID].param[20].value = 33.0f;
		synths[ID].param[20].min   = 0.0f;
		synths[ID].param[20].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[21].name  = "filter";
		synths[ID].param[21].value = 0.0f;
		synths[ID].param[21].min   = 0.0f;
		synths[ID].param[21].max   = (float)SYN_PARAM_MAX_VALUE;

		synths[ID].param[22].name  = "loop type";
		synths[ID].param[22].value = 0.0f;
		synths[ID].param[22].min   = 0.0f;
		synths[ID].param[22].max   = 3.0f;
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
    float mf = 0.01f + SYN_PARAM_NORMALIZE(synth->param[10].value);
    controls.modulator_freq = pow(mf,5) * float(srate/2); // lazy sloop to finetune lowfreqs

	controls.attack  = SYN_PARAM_NORMALIZE(synth->param[3].value);
	controls.decay   = SYN_PARAM_NORMALIZE(synth->param[4].value);
	controls.sustain = SYN_PARAM_NORMALIZE(synth->param[5].value);
	controls.release = SYN_PARAM_NORMALIZE(synth->param[6].value) * 0.5f;
  
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

	instrument.modulator.phase = 0;
	instrument.carrier.phase   = 0;

	// determine duration
	pp_uint32 samples = (srate/6) * (int)synth->param[2].value; // 300ms * param
	// enable overflow rendering when loop is set to forward
	// to allow seamless loops without clicks
	pp_uint32 overflow = 1;
	pp_uint32 looptype = (pp_uint32)synth->param[22].value;
	pp_uint32 silence  = 0;
	float     last     = 0.0;
	if( looptype == 1 ){ // overflow until silence with forward loop
		overflow = controls.spacetime > 0.1 ? (int)(1.0 + controls.spacetime*10) : 3;
	}
	TXMSample *sample = sampleEditor->isEmptySample() ? prepareSample(samples) : sampleEditor->getSample();

	// exponential positive drive into sin() function (produces foldback/freq multiply)
	// see curve @ https://graphtoy.com/?f1(x,t)=max(0,(x*10*x*x)%20)%20+x&v1=true 
	float scale    = SYN_PARAM_NORMALIZE(synth->param[1].value);
 	scale          = fmax(0,(scale*3*scale*scale))+scale; // exponential in positive side
	int frames     = overflow*(int)samples;
	float* smpin;
	smpin          = (float*)calloc(frames,  sizeof(float));
	float x;

	// synthesize! 
	for( pp_int32 i = 0; i < frames; i++ ){

		// apply transient to freq controllers (see trans @ https://graphtoy.com/?f1(x,t)=-0.5*tanh((x*92)-3)+0.5&v1=true)
		pp_uint32 transSamples = (pp_uint32)( (float(srate)/100) * SYN_PARAM_NORMALIZE(synth->param[15].value ) ); 
		float offset   = (1.0f/(float)transSamples) * float(i);
		float transAmp = SYN_PARAM_NORMALIZE(synth->param[14].value) * float(srate/4); 
		float c_trans  = fmax( 0.0f, transAmp * (1.0f+(-offset*offset) ) );
		instrument.carrier.freq   = NOTE2HZ( NOTE_START + (int)synth->param[7].value );
		instrument.carrier.freq   += c_trans;
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
    if( instrument.reverb.size > 0.04 ){    // avoid comb effect
		float size = (int)instrument.reverb.size*100.0;
		float* smpout;
		// apply reverb 
		int outlength = Convolver::reverb( smpin, &smpout, frames, 100 * (int)instrument.reverb.size*100.0 );
		for( pp_int32 i = 0; i < frames; i++ ){
			float old = sampleEditor->getFloatSampleFromWaveform( i % (int)samples);
			sampleEditor->setFloatSampleInWaveform( i % (int)samples, old + ((smpin[i] + smpout[i])*scale) );
			//// one-slider reverb: amplify wet with curve 
			//*out = wet * (size*size*2);
			//// and add dry back in using curve
			//*out += sample * ((-size*size*size)+1);
		}
		free(smpin);
		free(smpout);
	}else{
		for( pp_int32 i = 0; i < frames; i++ ){
			float old = sampleEditor->getFloatSampleFromWaveform( i % (int)samples);
			sampleEditor->setFloatSampleInWaveform( i % (int)samples, old + (smpin[i]*scale) );
		}
	}



	// force loop 
	if( looptype > 0 ){
		sampleEditor->setRepeatStart(0);
		sampleEditor->setRepeatEnd(sample->samplen);
	}
	sampleEditor->setLoopType( looptype < 4 ? looptype : 0 );

	sampleEditor->notifyListener(SampleEditor::NotificationChanges); // update UI
}
