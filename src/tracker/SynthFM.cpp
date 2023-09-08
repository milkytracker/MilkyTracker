/*
 * JamToySynth by https://github.com/rikusalminen
 *
 * https://github.com/rikusalminen/jamtoysynth/blob/master/src/instrument.c 
 */

#include "SynthFM.h"

float SynthFM::zero(float phase) { (void)phase; return 0; }
float SynthFM::sine(float phase) { return sinf(phase); }
float SynthFM::square(float phase) { return phase <= M_PI ? 1.0 : -1.0; }
float SynthFM::sawtooth(float phase) { return -1.0 + 2.0 * fmod(phase / M_PI, 1.0); }
float SynthFM::noise(float phase) { (void)phase; return rand() / (float)RAND_MAX; }
float SynthFM::triangle(float phase)
{
    if(phase < M_PI/2.0) return phase / (M_PI/2.0);
    if(phase <= 3.0*M_PI/2.0) return 1.0 - 2.0 * (phase-M_PI/2.0)/M_PI;
    return -1.0 + (phase - 3.0*M_PI/2.0) / (M_PI/2.0);
}

float SynthFM::oscillator(oscillator_t *osc, int sample_rate, float freq_mod)
{
    typedef float (*wave_fun)(float);
    wave_fun waves[] = { zero, sine, square, sawtooth, triangle, noise };
    float sample = osc->amplitude * waves[osc->waveform](osc->phase);
    osc->phase = fmod(osc->phase + freq_mod * osc->freq * 2.0 * M_PI / sample_rate, 2.0 * M_PI);
    return sample;
}

float SynthFM::modulate(modulation_t modulation, int sample_rate, oscillator_t *carrier, oscillator_t *modulator)
{
    switch(modulation)
    {
        case MODULATION_NONE:
            oscillator(modulator, sample_rate, 1.0); // update phase, ignore result
            return oscillator(carrier, sample_rate, 1.0);
        case MODULATION_RING:
            return oscillator(carrier, sample_rate, 1.0) * oscillator(modulator, sample_rate, 1.0);
        case MODULATION_AMPLITUDE:
            return oscillator(carrier, sample_rate, 1.0) * (1.0 + oscillator(modulator, sample_rate, carrier->freq));
        case MODULATION_FREQUENCY:
            return oscillator(carrier, sample_rate, 1.0 + oscillator(modulator, sample_rate, carrier->freq));
        case MODULATION_TREMOLO:
            return oscillator(carrier, sample_rate, 1.0) * (1.0 + oscillator(modulator, sample_rate, 1.0));
        case MODULATION_VIBRATO:
            return oscillator(carrier, sample_rate, 1.0 + oscillator(modulator, sample_rate, 1.0));
        default:
            break;
    }

    return 0;
}

void SynthFM::adsr_set(adsr_t *adsr, int sample_rate, float attack, float decay, float sustain, float release)
{
    adsr->attackG = expf(-1.0 / (attack * sample_rate));
    adsr->decayG = expf(-1.0 / (decay * sample_rate));
    adsr->releaseG = expf(-1.0 / (release * sample_rate));
    adsr->sustain = sustain;
    adsr->decay = decay * sample_rate;
}

void SynthFM::adsr_trigger(adsr_t *adsr)
{
    adsr->adsrG = adsr->attackG;
    adsr->adsrX = 1.5; // TODO: play with this magic constant (velocity?)
    adsr->decay_timer = 0;
}

float SynthFM::adsr_envelope(adsr_t *adsr)
{
    adsr->envelope = adsr->adsrG * (adsr->envelope - adsr->adsrX) + adsr->adsrX;

    if(adsr->envelope >= 1.0)
    {
        adsr->adsrG = adsr->decayG;
        adsr->adsrX = adsr->sustain;
        adsr->decay_timer = adsr->decay;
        adsr->envelope = 1.0;
    }

    adsr->decay_timer -= 1;

    if(adsr->decay_timer == 0)
    {
        adsr->adsrG = adsr->releaseG;
        adsr->adsrX = 0.0;
    }

    return adsr->envelope;
}

void SynthFM::instrument_control(fm_t *instrument, const fm_control_t *control, int sample_rate)
{
    instrument->modulation = control->modulation;

    instrument->carrier.waveform = control->carrier;
    instrument->carrier.amplitude = control->carrier_amplitude;

    instrument->modulator.waveform = control->modulator;
    instrument->modulator.amplitude = control->modulator_amplitude;
    instrument->modulator.freq = control->modulator_freq;

    adsr_set(&instrument->adsr,
        sample_rate,
        control->attack,
        control->decay,
        control->sustain,
        control->release);

	instrument->feedback = control->feedback;

	Filter::multifilter_set(&instrument->filter,
		sample_rate,
		control->filter,
		control->filter_freq,
		control->filter_resonance,
		control->filter_gain);

	if( instrument->reverb.size != control->spacetime ){
		instrument->reverb.size   = control->spacetime;
		instrument->reverb.decay  = 0.97;
		instrument->reverb.colour = -6.0;
		Reverb::reset( (reverb_t *)&instrument->reverb );
	}
    //size=0.040000 decay=0.970000 colour=-6.000000	
    //size=1.000000 decay=0.970000 colour=-6.000000

}

void SynthFM::instrument_play(fm_t *instrument, int sample_rate, float *out)
{
    float sample = sin(
			adsr_envelope(&instrument->adsr) * modulate(instrument->modulation, sample_rate, &instrument->carrier, &instrument->modulator)
		    * instrument->feedback
	);

    if( instrument->reverb.size > 0.04 ){    // avoid comb effect
		float size = instrument->reverb.size;
		float wet;
		Reverb::process( &sample, &wet, 1, (reverb_t *)&instrument->reverb);
		// amplify wet with curve 
		*out = wet * (size*size*2);
		// add dry back in using curve
		*out += sample * ((-size*size*size)+1);
	}else *out = sample;
	*out = Filter::multifilter(&instrument->filter, &instrument->filter0, *out );
}
