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

void SynthFM::filter_set(filter_t *filter, int sample_rate, filter_type_t type, float f0, float Q, float dBgain)
{
    double cosw0 = cos(2*M_PI*f0/sample_rate);
    double sinw0 = sqrt(1.0 - cosw0 * cosw0); //sin(2*M_PI*f0/SAMPLE_RATE);
    double alpha = sinw0 / (2*Q);
    double A = pow(10.0, dBgain/40);
    double A2 = 2*sqrt(A);
    double a0, a1, a2, b0, b1, b2;

    switch (type)
    {
        case FILTER_LOWPASS:
            b0 = (1 - cosw0)/2;
            b1 = 1 - cosw0;
            b2 = (1 - cosw0)/2;
            a0 = 1 + alpha;
            a1 = -2.0 * cosw0;
            a2 = 1 - alpha;
            break;
        case FILTER_HIGHPASS:
            b0 = (1 + cosw0)/2;
            b1 = -(1 + cosw0);
            b2 = (1 + cosw0)/2;
            a0 = 1 + alpha;
            a1 = -2 * cosw0;
            a2 = 1 - alpha;
            break;
        case FILTER_BANDPASS:
            b0 = alpha;
            b1 = 0;
            b2 = -alpha;
            a0 = 1 + alpha;
            a1 = -2 * cosw0;
            a2 = 1 - alpha;
            break;
        case FILTER_NOTCH:
            b0 = 1;
            b1 = -2*cosw0;
            b2 = 1;
            a0 = 1 + alpha;
            a1 = -2*cosw0;
            a2 = 1-alpha;
            break;
        case FILTER_PEAKING_EQ:
            b0 = 1 + alpha*A;
            b1 = -2*cosw0;
            b2 = 1 - alpha*A;
            a0 = 1 + alpha/A;
            a1 = -2*cosw0;
            a2 = 1 - alpha/A;
            break;
        case FILTER_LOW_SHELF:
            b0 = A*((A+1) - (A-1)*cosw0 + A2 * alpha);
            b1 = 2*A*((A-1) - (A+1) * cosw0);
            b2 = A*((A+1) - (A-1) * cosw0 - A2 * alpha);
            a0 = (A+1) + (A-1) * cosw0 + A2 * alpha;
            a1 = -2*((A-1) + (A+1) * cosw0);
            a2 = (A+1) + (A-1) * cosw0 - A2 * alpha;
            break;
        case FILTER_HIGH_SHELF:
            b0 = A*((A+1) + (A-1) * cosw0 + A2 * alpha);
            b1 = -2*A*((A-1) + (A+1) * cosw0);
            b2  = A*((A+1) + (A-1) * cosw0 - A2 * alpha);
            a0 = (A+1) - (A-1) * cosw0 + A2 * alpha;
            a1 = 2*((A-1) - (A+1) * cosw0);
            a2 = (A+1) - (A-1) * cosw0 - A2 * alpha;
            break;
        case FILTER_NONE:
        default:
            b0 = a0 = 1.0;
            b1 = b2 = 0.0;
            a1 = a2 = 0.0;
            break;
    }

    filter->a0 = a0 * 64.0;
    filter->a1 = -a1 * 64.0;
    filter->a2 = -a2 * 64.0;
    filter->b0 = b0 * 64.0;
    filter->b1 = b1 * 64.0;
    filter->b2 = b2 * 64.0;
}

float SynthFM::filter(const filter_t *filter, filter_state_t *state, float sample)
{
    float out = (
        filter->b0 * sample +
        filter->b1 * state->x1 +
        filter->b2 * state->x2 +
        filter->a1 * state->y1 +
        filter->a2 * state->y2) /
        filter->a0;

    state->x2 = state->x1;
    state->x1 = sample;
    state->y2 = state->y1;
    state->y1 = out;

    return out;
}

float SynthFM::echo(echo_t *echo, float sample)
{
    int read_cursor =
        echo->cursor >= echo->delay_samples ?
        echo->cursor - echo->delay_samples :
        ECHO_BUFFER_SIZE - (echo->delay_samples - echo->cursor) % ECHO_BUFFER_SIZE;

    float delay_sample = echo->buffer[read_cursor];
    echo->buffer[echo->cursor] = sample + echo->feedback * delay_sample;
    echo->cursor = (echo->cursor + 1) % ECHO_BUFFER_SIZE;

    return sample + echo->level * delay_sample;
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

    filter_set(&instrument->filter,
        sample_rate,
        control->filter,
        control->filter_freq,
        control->filter_resonance,
        control->filter_gain);

    instrument->echo.delay_samples = control->echo_delay * sample_rate;
    instrument->echo.feedback = control->echo_feedback;
    instrument->echo.level = control->echo_level;
}

void SynthFM::instrument_play(fm_t *instrument, int sample_rate, float *out)
{
    float sample =
//        echo(&instrument->echo,
            filter(&instrument->filter, &instrument->filter1,
                filter(&instrument->filter, &instrument->filter0,
                    adsr_envelope(&instrument->adsr) *
                    modulate(instrument->modulation, sample_rate, &instrument->carrier, &instrument->modulator))
		);
	//);

    *out = sample;
}
