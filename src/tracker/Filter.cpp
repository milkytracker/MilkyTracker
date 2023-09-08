#include "Filter.h"
#include <math.h>

void Filter::process(float in, filter_t *p) {
   // if( p->cutoff >= p->srate/2 ) p->cutoff = (p->srate/2) - 1000.0f; // protect
    float tcutoff = 2.0*sin(M_PI*p->cutoff/p->srate);
    float fb = p->q + p->q/(1.0 - tcutoff );
    float hp = in - p->s0;
    float bp = p->s0 - p->s1;
    p->s0 = p->s0 + tcutoff * (hp + fb * bp);
    p->s1 = p->s1 + tcutoff * (p->s0 - p->s1);
    p->out_lp = p->s1;
    p->out_hp = p->s1 - in;
}

void Filter::init( filter_t *p, int samplerate) {
  p->q = 0.0f;   // 0..0.999
  p->srate = (float)samplerate*2;
  p->cutoff = 500.0;
  p->s0 = 0.0f;
  p->s1 = 0.0f;
}

void Filter::multifilter_set(multifilter_t *filter, int sample_rate, multifilter_type_t type, float f0, float Q, float dBgain)
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

float Filter::multifilter(const multifilter_t *filter, multifilter_state_t *state, float sample)
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
