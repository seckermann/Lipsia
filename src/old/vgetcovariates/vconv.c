#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <fftw3.h>


/* standard parameter values for gamma function,Glover 99 */
double a1 = 6;
double b1 = 0.9;
double a2 = 12;
double b2 = 0.9;
double cc = 0.35;


typedef struct ComplexStruct {
	double re;
	double im;
} Complex;

Complex
complex_mult( Complex a, Complex b )
{
	Complex w;
	w.re = a.re * b.re  -  a.im * b.im;
	w.im = a.re * b.im  +  a.im * b.re;
	return w;
}

double *
Convolve( double *dest, fftw_plan iplan, int TR, int ntimesteps, double delta,
		  fftw_complex *fkernel, double *ibuf, fftw_complex *nbuf, fftw_complex *obuf,
		  int n )
{
	Complex a, b, c;
	int i, j, nc;

	nc  = ( n / 2 ) + 1;


	/* convolution   */
	for ( j = 0; j < nc; j++ ) {
		a.re = obuf[j][0];
		a.im = obuf[j][1];
		b.re = fkernel[j][0];
		b.im = fkernel[j][1];
		c = complex_mult( a, b );
		nbuf[j][0] = c.re;
		nbuf[j][1] = c.im;
	}

	/* inverse fft */
	fftw_execute( iplan );


	/* scaling */
	for ( j = 0; j < n; j++ ) ibuf[j] /= ( double )n;


	/* sampling */
	for ( i = 0; i < ntimesteps; i++ ) {
		j = ( int )( i * TR / delta + 0.5 );

		if ( j < 0 || j >= n ) continue;

		dest[i] = ibuf[j];
	}

	/* for (i=0; i<ntimesteps; i++) fprintf(stderr," %f",dest[i]); */

	return dest;
}

/*
** Glover kernel, gamma function
*/
double
xgamma( double xx, double t0 )
{
	double x, y, scale = 20;
	double y1, y2;
	double d1, d2;

	x = xx - t0;

	if ( x < 0 || x > 50 ) return 0;

	d1 = a1 * b1;
	d2 = a2 * b2;

	y1 = pow( x / d1, a1 ) * exp( -( x - d1 ) / b1 );
	y2 = pow( x / d2, a2 ) * exp( -( x - d2 ) / b2 );

	y = y1 - cc * y2;
	y /= scale;
	return y;
}

double *
VConvolveRegr( double *regr1, int *ptrvi )
{
	fftw_complex *fkernel0 = NULL;
	fftw_complex *obuf = NULL, *nbuf = NULL;
	fftw_plan p1, pinv, pk0;
	double t, t1, dt;
	double *kernel0 = NULL;
	double *ibuf = NULL, *ibuf1 = NULL;
	float delta, total_duration;
	int i, n, nc;

	delta = 20.0;
	total_duration = ptrvi[5] * ptrvi[17] / 1000.0;
	n = ( int )( total_duration * 1000.0 / delta );
	nc  = ( n / 2 ) + 1;

	/* allocate memory KERNEL */
	kernel0  = ( double * )fftw_malloc( sizeof( double ) * n );
	fkernel0 = ( fftw_complex * )fftw_malloc ( sizeof ( fftw_complex ) * nc );
	memset( kernel0, 0, sizeof( double ) * n );
	pk0 = fftw_plan_dft_r2c_1d ( n, kernel0, fkernel0, FFTW_ESTIMATE );

	/* fill the KERNEL and fftw */
	i = 0;
	t1 = 30.0;
	dt = delta / 1000.0;

	for ( t = 0; t < t1; t += dt ) {
		if ( i >= n ) break;

		kernel0[i] = xgamma( t, 0 );
		i++;
	}

	fftw_execute( pk0 );

	/* allocate memory DATA */
	ibuf  = ( double * )fftw_malloc( sizeof( double ) * n );
	ibuf1 = ( double * )fftw_malloc( sizeof( double ) * n );
	obuf  =  ( fftw_complex * ) fftw_malloc ( sizeof ( fftw_complex ) * nc );
	nbuf  =  ( fftw_complex * ) fftw_malloc ( sizeof ( fftw_complex ) * nc );
	memset( ibuf, 0, sizeof( double ) * n );
	memset( ibuf1, 0, sizeof( double ) * n );
	p1    = fftw_plan_dft_r2c_1d ( n, ibuf, obuf, FFTW_ESTIMATE );
	pinv  = fftw_plan_dft_c2r_1d ( n, nbuf, ibuf1, FFTW_ESTIMATE );

	/* fill the input DATA and fftw */
	for ( i = 0; i < n; i++ ) ibuf[i] = regr1[( int )( i * delta / ptrvi[17] )];

	fftw_execute( p1 );

	/* Convolve and return */
	regr1 = Convolve( regr1, pinv, ptrvi[17], ptrvi[5], delta, fkernel0, ibuf1, nbuf, obuf, n );
	return regr1;
}

