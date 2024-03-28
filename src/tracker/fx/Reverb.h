/*
 *
 * reverb.c
 * Effect engine for a simple reverb plugin
 *
 * Copyright (c) 2015 Gordon JC Pearce <gordonjcp@gjcp.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 * */

/*
 * This is a simple schrodinger reverb 
 * which is cheap for the CPU and its artifacts 
 * can also be used for 'blooming' / smearing sounddesign-purposes.
 */

#ifndef __REVERB_H
#define __REVERB_H

/* note that buffers need to be a power of 2 */
/* if we scale the tap sizes for higher sample rates, this will need to be larger */
#define COMB_SIZE 4096
#define COMB_MASK (COMB_SIZE-1)
#define NUM_COMBS 4
#define NUM_APS 3

struct reverb_t{
  /* structure for reverb parameters */
  /* controls */
  float decay;
  float size;
  float colour;
  float lpo;

  /* internal plugin variables */
  float comb[NUM_COMBS][COMB_SIZE];   /* buffers for comb filters */
  unsigned long comb_pos;             /* position within comb filter */

  float ap[NUM_APS][COMB_SIZE];        /* lazy, reuse comb size */
  unsigned long ap_pos;               /* position within allpass filter */

};

class Reverb
{

  public:
    static void process(float *in, float *out, unsigned long sample_count, reverb_t *params);
    static void reset( reverb_t *params);
};

#endif
