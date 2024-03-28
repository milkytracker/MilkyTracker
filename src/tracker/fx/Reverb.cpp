#include "Reverb.h"
#include <math.h>

// see https://github.com/gordonjcp/reverb/blob/master/reverb.c

/* these values are based on the CCRMA article on Schroeder's original reverb */
/* they have been scaled to correspond to 44.1kHz Fs instead of 25kHz */
unsigned int tap[NUM_COMBS] = {2975, 2824, 3621, 3970};
float tap_gain[NUM_COMBS] = {0.964, 1.0, 0.939, 0.913};
unsigned int ap_tap[NUM_APS] = {612, 199, 113};

void Reverb::process(float *in, float *out, unsigned long sample_count, reverb_t *params) {
  // handle the actual processing
  unsigned long pos;
  unsigned long comb_pos = params->comb_pos;
  unsigned long ap_pos = params->ap_pos;

  int c;

  float * const input = in;
  float * const output = out;

  float in_s, in_s1, temp;

  float gl, gh;
  float tilt = params->colour;
  float decay=params->decay;

  if (tilt > 0) {
    gl = -5 * tilt;
    gh = tilt;
  } else {
    gl = -tilt;
    gh = 5 * tilt;
  }

  gl = exp(gl/8.66)-1;
  gh = exp(gh/8.66)-1;

  //printf("%f %f      ", gl, gh);

  float n = 1/(5340 + 132300.0);
  float a0 = 2 * 5340 * n;
  float b1 = (132300 - 5340) * n;


  tap[0] = (int)(2975 * params->size);
  tap[1] = (int)(2824 * (params->size/2));
  tap[2] = (int)(3621 * params->size);
  tap[3] = (int)(3970 * (params->size/1.5));

  ap_tap[2] = (int)(400 * params->size);

  /* loop around the buffer */
  for (pos = 0; pos < sample_count; pos++) {
    /* loop around the comb filters */
    temp = 0;
    in_s = input[pos]/3;

    params->lpo = a0 * in_s + b1 * params->lpo;
    in_s1 = in_s + gl * params->lpo + gh *(in_s - params->lpo);

    for (c = 0; c<NUM_COMBS; c++) {
      params->comb[c][(comb_pos + tap[c]) & COMB_MASK] =
        in_s1 + (decay * tap_gain[c]) * params->comb[c][comb_pos];
      temp += params->comb[c][comb_pos];
    }

    output[pos] = temp * 0.35;
    comb_pos++;
    comb_pos &= COMB_MASK;  /* increment and wrap buffer */
    ap_pos++;
    ap_pos &= COMB_MASK;  /* increment and wrap buffer */
    params->comb_pos = comb_pos;
    params->ap_pos = ap_pos;
  }
}

void Reverb::reset( reverb_t *params) {
  int i;
  int j;
  for( int i = 0; i < NUM_COMBS; i++ ){
    for( int j = 0; j < COMB_SIZE; j++ ){
      params->comb[i][j] = 0.0f;
    }
  }
  for( int i = 0; i < NUM_APS; i++ ){
    for( int j = 0; j < COMB_SIZE; j++ ){
      params->ap[i][j] = 0.0f;
    }
  }
  params->lpo      = 0;
  params->comb_pos = 0;
  params->ap_pos   = 0;
}
