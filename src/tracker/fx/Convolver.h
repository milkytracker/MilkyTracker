#pragma once
/*
 *  tracker/Convolver.h
 *
 *  Copyright 2024 Leon van Kammen 
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
 *
 * Class-wrapper created by Leon van Kammen / coderofsalvation on 28th march 2024 
 *
 * based on convolve.c
 *
 * M. Farbood, August 5, 2011
 *
 * Function that convolves two signals.
 * Factored discrete Fourier transform, or FFT, and its inverse iFFT.
 *
 * fft and ifft are taken from code for the book,
 * Mathematics for Multimedia by Mladen Victor Wickerhauser
 * The function convolve is based on Stephen G. McGovern's fconv.m
 * Matlab implementation.
 *
 *
 */


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

class Convolver {

public:
	static void print_vector(const char* title, complex* x, int n);
	static complex complex_mult(complex a, complex b);
	static void fft(complex* v, int n, complex* tmp);
	static void ifft(complex* v, int n, complex* tmp);
	static int convolve(float* x, float* h, int lenX, int lenH, float** output);
	static int reverb( float *smpin, float **smpout, int frames, int verb_size );
	static void envelope_follow(float input, struct EnvelopeFollow* e);

};

#endif
