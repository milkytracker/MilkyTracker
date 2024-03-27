#pragma once

#ifndef CONCOLVER_H
#define CONCOLVER_H

//#include "BasicTypes.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define FFT_SPECTRUM_WINDOW 64
typedef float real;
typedef struct { real Re; real Im; } complex;

#ifndef PI
#define PI 3.14159265358979323846264338327950288
#endif

#ifndef VERY_SMALL_FLOAT
#define VERY_SMALL_FLOAT 1.0e-30F
#endif

struct EnvelopeFollow {
	float output;
	float samplerate;
	float release; // time in seconds for output to decay to half value after an impulse
};

extern void print_vector(const char* title, complex* x, int n);
extern complex complex_mult(complex a, complex b);
extern void fft(complex* v, int n, complex* tmp);
extern void ifft(complex* v, int n, complex* tmp);
extern int convolve(float* x, float* h, int lenX, int lenH, float** output);
extern void reverb( float *smpin, float *smpout, int frames, int verb_size );
extern void envelope_follow(float input, struct EnvelopeFollow* e);

#endif
