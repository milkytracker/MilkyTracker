/*
 * JamToySynth by https://github.com/rikusalminen
 *
 * https://github.com/rikusalminen/jamtoysynth/blob/master/src/instrument.c 
 */

#ifndef SYNTHFM_H
#define SYNTHFM_H

#include <stdlib.h>
#include <math.h>
#include "Filter.h"
#include "Reverb.h"

#define ECHO_BUFFER_SIZE 96000 // 2 seconds @ 48 kHz
	
typedef enum oscillator_waveform_t
{
    OSCILLATOR_ZERO = 0,
    OSCILLATOR_SINE,
    OSCILLATOR_SQUARE,
    OSCILLATOR_SAWTOOTH,
    OSCILLATOR_TRIANGLE,
    OSCILLATOR_NOISE,
} oscillator_waveform_t;

typedef struct echo_t
{
    float buffer[ECHO_BUFFER_SIZE];
    int cursor;
    int delay_samples;
	bool karplustrong;
    float feedback, level;
} echo_t;

typedef struct oscillator_t
{
    oscillator_waveform_t waveform;
    float freq;
    float phase;
    float amplitude;
} oscillator_t;

typedef enum modulation_t
{
    MODULATION_NONE = 0,
    MODULATION_AMPLITUDE,
    MODULATION_FREQUENCY,
    MODULATION_RING,
    MODULATION_TREMOLO,
    MODULATION_VIBRATO,
} modulation_t;

typedef struct adsr_t
{
    float attackG, releaseG, decayG;
    float sustain, decay;
    float adsrG, adsrX;
    int decay_timer;
    float envelope;
} adsr_t;

typedef struct fm_control_t
{
    modulation_t modulation;
    oscillator_waveform_t carrier;
    float carrier_amplitude;
    oscillator_waveform_t modulator;
    float modulator_amplitude, modulator_freq;

    float attack, decay, sustain, release;
	multifilter_type_t filter;
	float filter_freq,filter_resonance,filter_gain;
	float feedback;
	float spacetime;	

	int echo_delay_samples;
	bool echo_karplustrong;
    float echo_feedback;
	float echo_level;
} fm_control_t;

typedef struct fm_t
{
    modulation_t modulation;
    oscillator_t carrier, modulator;
    adsr_t adsr;
	multifilter_t filter;
	multifilter_state_t filter0;
	reverb_t reverb;
	echo_t echo;
	float feedback;
} fm_t;

class SynthFM{

  public:
	  static float zero(float phase);
	  static float sine(float phase);
	  static float square(float phase);
	  static float sawtooth(float phase);
	  static float noise(float phase);
	  static float triangle(float phase);
	  static void instrument_control(fm_t *instrument, const fm_control_t *control, int sample_rate);
	  static float oscillator(oscillator_t *osc, int sample_rate, float freq_mod);
	  static float modulate(modulation_t modulation, int sample_rate, oscillator_t *carrier, oscillator_t *modulator);
	  static void adsr_set(adsr_t *adsr, int sample_rate, float attack, float decay, float sustain, float release);
	  static void adsr_trigger(adsr_t *adsr);
	  static float adsr_envelope(adsr_t *adsr);
	  static void instrument_play(fm_t *instrument, int sample_rate, float *out );
	  static float echo(fm_t *instrument, float sample);
};

#endif
