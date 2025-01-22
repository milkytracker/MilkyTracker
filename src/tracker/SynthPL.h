/*

Copyright (c) 2024, Dominic Szablewski - https://phoboslab.org
SPDX-License-Identifier: MIT

Based on Sonant, published under the Creative Commons Public License
(c) 2008-2009 Jake Taylor [ Ferris / Youth Uprising ] 


-- Synopsis

// Define `PL_SYNTH_IMPLEMENTATION` in *one* C/C++ file before including this
// library to create the implementation.

#define PL_SYNTH_IMPLEMENTATION
#include "pl_synth.h"

// Initialize the lookup table for the oscillators
void *synth_tab = malloc(PL_SYNTH_TAB_SIZE);
pl_synth_init(synth_tab);

// A sound is described by an instrument (synth), the row_len in samples and
// a note.
pl_synth_sound_t sound = {
	.synth = {7,0,0,0,192,0,7,0,0,0,192,0,0,200,2000,20000,192}, 
	.row_len = 5168, 
	.note = 135
};

// Determine the number of the samples for a sound effect and allocate the
// sample buffer for both (stereo) channels
int num_samples = pl_synth_sound_len(&sound);
uint16_t *sample_buffer = malloc(num_samples * 2 * sizeof(uint16_t));

// Generate the samples
pl_synth_sound(&sound, sample_buffer);

See below for a documentation of all functions exposed by this library.


*/

#ifndef PL_SYNTH_H
#define PL_SYNTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define PL_SYNTH_SAMPLERATE 44100
#define PL_SYNTH_TAB_LEN 4096
#define PL_SYNTH_TAB_SIZE (sizeof(float) * PL_SYNTH_TAB_LEN * 4)

typedef struct {
	uint8_t osc0_oct;
	uint8_t osc0_det;
	uint8_t osc0_detune;
	uint8_t osc0_xenv;
	uint8_t osc0_vol;
	uint8_t osc0_waveform;

	uint8_t osc1_oct;
	uint8_t osc1_det;
	uint8_t osc1_detune;
	uint8_t osc1_xenv;
	uint8_t osc1_vol;
	uint8_t osc1_waveform;

	uint8_t noise_fader;

	uint32_t env_attack;
	uint32_t env_sustain;
	uint32_t env_release;
	uint32_t env_master;

	uint8_t fx_filter;
	uint32_t fx_freq;
	uint8_t fx_resonance;
	uint8_t fx_delay_time;
	uint8_t fx_delay_amt;
	uint8_t fx_pan_freq;
	uint8_t fx_pan_amt;

	uint8_t lfo_osc_freq;
	uint8_t lfo_fx_freq;
	uint8_t lfo_freq;
	uint8_t lfo_amt;
	uint8_t lfo_waveform;
} pl_synth_t;

typedef struct {
	pl_synth_t synth;
	uint32_t row_len;
	uint8_t note;
} pl_synth_sound_t;

typedef struct {
	uint8_t notes[32];
} pl_synth_pattern_t;

typedef struct {
	pl_synth_t synth;
	uint32_t sequence_len;
	uint8_t *sequence;
	pl_synth_pattern_t *patterns;
} pl_synth_track_t;

typedef struct {
	uint32_t row_len;
	uint8_t num_tracks;
	pl_synth_track_t *tracks;
} pl_synth_song_t;

// Initialize the lookup table for all instruments. This needs to be done only
// once. The table will be written to the memory pointed to by tab_buffer, which
// must be PL_SYNTH_TAB_LEN elements long or PL_SYNTH_TAB_SIZE bytes in size.
void pl_synth_init(float *tab_buffer);

// Determine the number of samples needed for one channel of a particular sound
// effect.
int pl_synth_sound_len(pl_synth_sound_t *sound);

// Generate a stereo sound into the buffer pointed to by samples. The buffer 
// must be at least pl_synth_sound_len() * 2 elements long.
int pl_synth_sound(pl_synth_sound_t *sound, int16_t *samples);

// Determine the number of samples needed for one channel of a particular song.
int pl_synth_song_len(pl_synth_song_t *song);

// Generate a stereo song into the buffer pointed to by samples, with temporary
// storage provided to by temp_samples. The buffers samples and temp_samples 
// must each be at least pl_synth_song_len() * 2 elements long.
int pl_synth_song(pl_synth_song_t *song, int16_t *samples, int16_t *temp_samples);

#ifdef __cplusplus
}
#endif
#endif /* PL_SYNTH_H */



/* -----------------------------------------------------------------------------
	Implementation */

#ifdef PL_SYNTH_IMPLEMENTATION

#include <math.h> // powf, sinf, logf, ceilf

#define PL_SYNTH_TAB_MASK (PL_SYNTH_TAB_LEN-1)
#define PL_SYNTH_TAB(WAVEFORM, K) pl_synth_tab[WAVEFORM][((int)((K) * PL_SYNTH_TAB_LEN)) & PL_SYNTH_TAB_MASK]

static float *pl_synth_tab[4];
static uint32_t pl_synth_rand = 0xd8f554a5;

void pl_synth_init(float *tab_buffer) {
	for (int i = 0; i < 4; i++) {
		pl_synth_tab[i] = tab_buffer + PL_SYNTH_TAB_LEN * i;
	}

	// sin
	for (int i = 0; i < PL_SYNTH_TAB_LEN; i++) { 
		pl_synth_tab[0][i] = sinf(i*(6.283184f/(float)PL_SYNTH_TAB_LEN));
	}
	// square
	for (int i = 0; i < PL_SYNTH_TAB_LEN; i++) {
		pl_synth_tab[1][i] = pl_synth_tab[0][i] < 0 ? -1 : 1;
	}
	// sawtooth
	for (int i = 0; i < PL_SYNTH_TAB_LEN; i++) {
		pl_synth_tab[2][i] = (float)i / PL_SYNTH_TAB_LEN - 0.5;
	}
	// triangle
	for (int i = 0; i < PL_SYNTH_TAB_LEN; i++) {
		pl_synth_tab[3][i] = i < PL_SYNTH_TAB_LEN/2 
			? (i/(PL_SYNTH_TAB_LEN/4.0)) - 1.0 
			: 3.0 - (i/(PL_SYNTH_TAB_LEN/4.0));
	} 
}

static inline float pl_synth_note_freq(int n, int oct, int semi, int detune) {
	return (0.00390625 * powf(1.059463094, n - 128 + (oct - 8) * 12 + semi)) * (1.0f + 0.0008f * detune);
}

static inline int pl_synth_clamp_s16(int v) {
	if ((unsigned int)(v + 32768) > 65535) {
		if (v < -32768) { return -32768; }
		if (v >  32767) { return  32767; }
	}
	return v;
}

static void pl_synth_gen(int16_t *samples, int write_pos, int row_len, int note, pl_synth_t *s) {
	float fx_pan_freq = powf(2, s->fx_pan_freq - 8) / row_len;
	float lfo_freq = powf(2, s->lfo_freq - 8) / row_len;
	
	// We need higher precision here, because the oscilator positions may be 
	// advanced by tiny values and error accumulates over time
	double osc0_pos = 0;
	double osc1_pos = 0;

	float fx_resonance = s->fx_resonance / 255.0f;
	float noise_vol = s->noise_fader * 4.6566129e-010f;
	float low = 0;
	float band = 0;
	float high = 0;

	float inv_attack = 1.0f / s->env_attack;
	float inv_release = 1.0f / s->env_release;
	float lfo_amt = s->lfo_amt / 512.0f;
	float pan_amt = s->fx_pan_amt / 512.0f;

	float osc0_freq = pl_synth_note_freq(note, s->osc0_oct, s->osc0_det, s->osc0_detune);
	float osc1_freq = pl_synth_note_freq(note, s->osc1_oct, s->osc1_det, s->osc1_detune);

	int num_samples = s->env_attack + s->env_sustain + s->env_release - 1;
	
	for (int j = num_samples; j >= 0; j--) {
		int k = j + write_pos;

		// LFO
		float lfor = PL_SYNTH_TAB(s->lfo_waveform, k * lfo_freq) * lfo_amt + 0.5f;

		float sample = 0;
		float filter_f = s->fx_freq;
		float temp_f;
		float envelope = 1;

		// Envelope
		if (j < s->env_attack) {
			envelope = (float)j * inv_attack;
		}
		else if (j >= s->env_attack + s->env_sustain) {
			envelope -= (float)(j - s->env_attack - s->env_sustain) * inv_release;
		}

		// Oscillator 1
		temp_f = osc0_freq;
		if (s->lfo_osc_freq) {
			temp_f *= lfor;
		}
		if (s->osc0_xenv) {
			temp_f *= envelope * envelope;
		}
		osc0_pos += temp_f;
		sample += PL_SYNTH_TAB(s->osc0_waveform, osc0_pos) * s->osc0_vol;

		// Oscillator 2
		temp_f = osc1_freq;
		if (s->osc1_xenv) {
			temp_f *= envelope * envelope;
		}
		osc1_pos += temp_f;
		sample += PL_SYNTH_TAB(s->osc1_waveform, osc1_pos) * s->osc1_vol;

		// Noise oscillator
		if (noise_vol) {
			int32_t r = (int32_t)pl_synth_rand;
			sample += (float)r * noise_vol * envelope;
			pl_synth_rand ^= pl_synth_rand << 13;
			pl_synth_rand ^= pl_synth_rand >> 17;
			pl_synth_rand ^= pl_synth_rand << 5;
		}

		sample *= envelope * (1.0f / 255.0f);

		// State variable filter
		if (s->fx_filter) {
			if (s->lfo_fx_freq) {
				filter_f *= lfor;
			}

			filter_f = PL_SYNTH_TAB(0, filter_f * (0.5f / PL_SYNTH_SAMPLERATE)) * 1.5f;
			low += filter_f * band;
			high = fx_resonance * (sample - band) - low;
			band += filter_f * high;
			sample = (float[5]){sample, high, low, band, low + high}[s->fx_filter];
		}

		// Panning & master volume
		temp_f = PL_SYNTH_TAB(0, k * fx_pan_freq) * pan_amt + 0.5f;
		sample *= 78 * s->env_master;


		samples[k * 2 + 0] += sample * (1-temp_f);
		samples[k * 2 + 1] += sample * temp_f;
	}
}

static void pl_synth_apply_delay(int16_t *samples, int len, int shift, float amount) {
	int len_2 = len * 2;
	int shift_2 = shift * 2;
	for (int i = 0, j = shift_2; j < len_2; i += 2, j += 2) {
		samples[j + 0] += samples[i + 1] * amount;
		samples[j + 1] += samples[i + 0] * amount;
	}
}

static int pl_synth_instrument_len(pl_synth_t *synth, int row_len) {
	int delay_shift = (synth->fx_delay_time * row_len) / 2;
	float delay_amount = synth->fx_delay_amt / 255.0;
	float delay_iter = ceilf(logf(0.1) / logf(delay_amount));
	return synth->env_attack + 
		synth->env_sustain + 
		synth->env_release + 
		delay_iter * delay_shift;
}

int pl_synth_sound_len(pl_synth_sound_t *sound) {
	return pl_synth_instrument_len(&sound->synth, sound->row_len);
}

int pl_synth_sound(pl_synth_sound_t *sound, int16_t *samples) {
	int len = pl_synth_sound_len(sound);
	pl_synth_gen(samples, 0, sound->row_len, sound->note, &sound->synth);

	if (sound->synth.fx_delay_amt) {
		int delay_shift = (sound->synth.fx_delay_time * sound->row_len) / 2;
		float delay_amount = sound->synth.fx_delay_amt / 256.0;
		pl_synth_apply_delay(samples, len, delay_shift, delay_amount);
	}

	return len;
}

int pl_synth_song_len(pl_synth_song_t *song) {
	int num_samples = 0;
	for (int t = 0; t < song->num_tracks; t++) {
		int track_samples = song->tracks[t].sequence_len * song->row_len * 32 +
			pl_synth_instrument_len(&song->tracks[t].synth, song->row_len);

		if (track_samples > num_samples) {
			num_samples = track_samples;
		}
	}
	
	return num_samples;
}

int pl_synth_song(pl_synth_song_t *song, int16_t *samples, int16_t *temp_samples) {
	int len = pl_synth_song_len(song);
	int len_2 = len * 2;
	memset(samples, 0, sizeof(int16_t) * len_2);

	for (int t = 0; t < song->num_tracks; t++) {
		pl_synth_track_t *track = &song->tracks[t];
		memset(temp_samples, 0, sizeof(int16_t) * len_2);

		for (int si = 0; si < track->sequence_len; si++) {
			int write_pos = song->row_len * si * 32;
			int pi = track->sequence[si];
			if (pi > 0) {
				unsigned char *pattern = track->patterns[pi-1].notes;
				for (int row = 0; row < 32; row++) {
					int note = pattern[row];
					if (note > 0) {
						pl_synth_gen(temp_samples, write_pos, song->row_len, note, &track->synth);
					}
					write_pos += song->row_len;
				}
			}
		}
		
		if (track->synth.fx_delay_amt) {
			int delay_shift = (track->synth.fx_delay_time * song->row_len) / 2;
			float delay_amount = track->synth.fx_delay_amt / 255.0;
			pl_synth_apply_delay(temp_samples, len, delay_shift, delay_amount);
		}

		for (int i = 0; i < len_2; i++) {
			samples[i] = pl_synth_clamp_s16(samples[i] + (int)temp_samples[i]);
		}
	}
	return len;
}


#endif /* PL_SYNTH_IMPLEMENTATION */
