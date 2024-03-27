#include "Convolver.h"
#include "VRand.h"

/*
** convolve.c
**
** M. Farbood, August 5, 2011
**
** Function that convolves two signals.
** Factored discrete Fourier transform, or FFT, and its inverse iFFT.
**
** fft and ifft are taken from code for the book,
** Mathematics for Multimedia by Mladen Victor Wickerhauser
** The function convolve is based on Stephen G. McGovern's fconv.m
** Matlab implementation.
**
*/

/* Print a vector of complexes as ordered pairs. */
void print_vector(const char* title, complex* x, int n)
{
	int i;
	printf("%s (dim=%d):", title, n);
	for (i = 0; i < n; i++) printf(" %5.2f,%5.2f ", x[i].Re, x[i].Im);
	putchar('\n');
	return;
}

/* Multiply two complex numbers */
complex complex_mult(complex a, complex b)
{
	complex c;
	c.Re = (a.Re * b.Re) +(a.Im * b.Im * -1);
	c.Im = a.Re * b.Im +a.Im * b.Re;
	
	return c;
}

/*
   fft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute fft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute fft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = -sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */

void fft(complex* v, int n, complex* tmp)
{
	if (n > 1) {			/* otherwise, do nothing and return */
		int k, m;
		complex z, w, * vo, * ve;
		ve = tmp;
		vo = tmp + n / 2;
		for (k = 0; k < n / 2; k++) {
			ve[k] = v[2 * k];
			vo[k] = v[2 * k + 1];
		}
		fft(ve, n / 2, v);		/* FFT on even-indexed elements of v[] */
		fft(vo, n / 2, v);		/* FFT on odd-indexed elements of v[] */
		for (m = 0; m < n / 2; m++) {
			w.Re = cos(2 * PI * m / (double)n);
			w.Im = -sin(2 * PI * m / (double)n);
			z.Re = w.Re * vo[m].Re - w.Im * vo[m].Im;	/* Re(w*vo[m]) */
			z.Im = w.Re * vo[m].Im + w.Im * vo[m].Re;	/* Im(w*vo[m]) */
			v[m].Re = ve[m].Re + z.Re;
			v[m].Im = ve[m].Im + z.Im;
			v[m + n / 2].Re = ve[m].Re - z.Re;
			v[m + n / 2].Im = ve[m].Im - z.Im;
		}
	}
	return;
}

/*
   ifft(v,N):
   [0] If N == 1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute ifft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute ifft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void ifft(complex* v, int n, complex* tmp)
{
	if (n > 1) {			/* otherwise, do nothing and return */
		int k, m;
		complex z, w, * vo, * ve;
		ve = tmp; vo = tmp + n / 2;
		for (k = 0; k < n / 2; k++) {
			ve[k] = v[2 * k];
			vo[k] = v[2 * k + 1];
		}
		ifft(ve, n / 2, v);		/* FFT on even-indexed elements of v[] */
		ifft(vo, n / 2, v);		/* FFT on odd-indexed elements of v[] */
		for (m = 0; m < n / 2; m++) {
			w.Re = cos(2 * PI * m / (double)n);
			w.Im = sin(2 * PI * m / (double)n);
			z.Re = w.Re * vo[m].Re - w.Im * vo[m].Im;	/* Re(w*vo[m]) */
			z.Im = w.Re * vo[m].Im + w.Im * vo[m].Re;	/* Im(w*vo[m]) */
			v[m].Re = ve[m].Re + z.Re;
			v[m].Im = ve[m].Im + z.Im;
			v[m + n / 2].Re = ve[m].Re - z.Re;
			v[m + n / 2].Im = ve[m].Im - z.Im;
		}
	}
	return;
}

/* Convolve signal x with impulse response h.  The return value is
 * the length of the output signal */
int convolve(float* x, float* h, int lenX, int lenH, float** output)
{
	complex* xComp = NULL;
	complex* hComp = NULL;
	complex* scratch = NULL;
	complex* yComp = NULL;
	complex c;

	int lenY = lenX + lenH - 1;
	int currPow = 0;
	int lenY2 = pow(2, currPow);
	float m = 0;
	int i;

	/* Get first first power of two larger than lenY */
	while (lenY2 < lenY) {
		currPow++;
		lenY2 = pow(2, currPow);
	}

	/* Allocate a lot of memory */
	scratch = (complex *)calloc(lenY2, sizeof(complex));
	if (scratch == NULL) {
		printf("Error: unable to allocate memory for convolution. Exiting.\n");
		exit(1);
	}
	xComp = (complex *)calloc(lenY2, sizeof(complex));
	if (xComp == NULL) {
		printf("Error: unable to allocate memory for convolution. Exiting.\n");
		exit(1);
	}
	hComp = (complex*)calloc(lenY2, sizeof(complex));
	if (hComp == NULL) {
		printf("Error: unable to allocate memory for convolution. Exiting.\n");
		exit(1);
	}
	yComp = (complex*)calloc(lenY2, sizeof(complex));
	if (yComp == NULL) {
		printf("Error: unable to allocate memory for convolution. Exiting.\n");
		exit(1);
	}

	/* Get max absolute value in X */
	for (i = 0; i < lenX; i++) {
		if (fabsf(x[i]) > m) {
			m = x[i];
		}
	}

	/* Copy over real values */
	for (i = 0; i < lenX; i++) {
		xComp[i].Re = x[i];
	}
	for (i = 0; i < lenH; i++) {
		hComp[i].Re = h[i];
	}

	/* FFT of x */
	//  print_vector("Orig", xComp, 40);
	fft(xComp, lenY2, scratch);
	//  print_vector(" FFT", xComp, lenY2);

	/* FFT of h */
	//  print_vector("Orig", hComp, 50);
	fft(hComp, lenY2, scratch);
	//  print_vector(" FFT", hComp, lenY2);

	/* Muliply ffts of x and h */
	for (i = 0; i < lenY2; i++) {
		c = complex_mult(xComp[i], hComp[i]);
		yComp[i].Re = c.Re;
		yComp[i].Im = c.Im;
	}
	//  print_vector("Y", yComp, lenY2);

	/* Take the inverse FFT of Y */
	ifft(yComp, lenY2, scratch);
	//  print_vector("iFFT", yComp, lenY2);    

	/* Take just the first N elements and find the largest value for scaling purposes */
	float maxY = 0;
	for (i = 0; i < lenY; i++) {
		if (fabsf(yComp[i].Re) > maxY) {
			maxY = fabsf(yComp[i].Re);
		}
	}

	/* Scale so that values are between 1 and -1 */
	m = m / maxY;

	free(scratch);
	free(xComp);
	free(hComp);

	*output = (float *)calloc(lenY, sizeof(float));
	if (output == NULL) {
		printf("Error: unable to allocate memory for convolution. Exiting.\n");
		exit(1);
	}

	for (i = 0; i < lenY; i++) {
		yComp[i].Re = yComp[i].Re * m;
		(*output)[i] = yComp[i].Re;
	}
	//  print_vector("Final", yComp, 400);

	free(yComp);
	return lenY;
}

void envelope_follow(float input, struct EnvelopeFollow* e) {
	float scalar = pow(0.5, 1.0 / ( (e->release) * (e->samplerate) ) );

	float inputAbs = fabs(input);

	if (input >= e->output)
	{
		/* When we hit a peak, ride the peak to the top. */
		e->output = input;
	}
	else
	{
		/* Exponential decay of output when signal is low. */
		e->output = e->output * scalar;//
		/*
		** When current gets close to 0.0, set current to 0.0 to prevent FP underflow
		** which can cause a severe performance degradation due to a flood
		** of interrupts.
		*/
		if (e->output < VERY_SMALL_FLOAT) e->output = 0.0;
	}
}

int reverb( float *smpin, float *smpout, int frames, int verb_size ){
	// create IR float array
	float* impulseResponse;
	impulseResponse = (float*)malloc(verb_size * sizeof(float));
	float f;
	VRand rand;
	rand.seed(1);
	for (pp_int32 i = 0; i < verb_size; i++) {
    f = rand.white() * (1.0f - ((1.0f / (float)verb_size) * (float)i));
    impulseResponse[i] = f;
	}
	int length = convolve(smpin, impulseResponse, frames, verb_size, &smpout);
  free(impulseResponse);
  return length;
}
