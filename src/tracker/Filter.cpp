#include "Filter.h"
#include <math.h>

void Filter::process(float in, filter_t *p) {
    if( p->cutoff >= p->srate/2 ) p->cutoff = (p->srate/2) - 1000.0f; // protect
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
  p->srate = (float)samplerate;
  p->cutoff = (float)((p->srate/2)/2);
  p->s0 = 0.0f;
  p->s1 = 0.0f;
}
