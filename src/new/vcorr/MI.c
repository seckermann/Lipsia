/*
** Mutual information
**
** G.Lohmann, MPI-CBS, Aug 2007
*/
#include <viaio/Vlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#define N 3

/* normalize data */
void VNormArray( gsl_vector *array )
{
	int i;
	double *ptr, mean, sigma, u, sum1, sum2, nx;

	sum1 = sum2 = nx = 0;
	ptr = array->data;

	for ( i = 0; i < array->size; i++ ) {
		u = *ptr++;
		sum1 += u;
		sum2 += u * u;
		nx++;
	}

	mean = sum1 / nx;
	sigma = sqrt( ( sum2 - nx * mean * mean ) / ( nx - 1.0 ) );

	ptr = array->data;

	for ( i = 0; i < array->size; i++ ) {
		u = *ptr;
		u = ( u - mean ) / sigma;
		*ptr++ = u;
	}
}


int Binning( double u )
{
	int j;

	if ( u < -0.56 ) j = 0;
	else if ( u < 0.56 ) j = 1;
	else j = 2;

	return j;
}


double
VMutualInformation( gsl_vector *array1, gsl_vector *array2, int type )
{
	int    i, j, k, n = N;
	double nn[N][N];
	double hxy, hx, hy, h, p, px, sum, mi = 0;
	double uxy, uygx, uxgy, hygx, hxgy;
	double sumi[N], sumj[N];
	double tiny = 1.0e-20;
	double *ptr1, *ptr2, u, v;

	if ( array1->size != array2->size ) VError( " VMutualInformation: inconsistent dims" );

	VNormArray( array1 );
	VNormArray( array2 );

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < n; j++ ) {
			nn[i][j] = 0;
		}
	}

	ptr1 = array1->data;
	ptr2 = array2->data;

	for ( i = 0; i < array1->size; i++ ) {
		u = *ptr1++;
		j = Binning( u );

		v = *ptr2++;
		k = Binning( v );
		nn[j][k]++;
	}


	sum = 0;

	for ( i = 0; i < n; i++ ) {
		sumi[i] = 0;

		for ( j = 0; j < n; j++ ) {
			sumi[i] += nn[i][j];
			sum += nn[i][j];
		}
	}

	if ( sum < tiny ) return 0;

	for ( j = 0; j < n; j++ ) {
		sumj[j] = 0;

		for ( i = 0; i < n; i++ )
			sumj[j] += nn[i][j];
	}

	hx = hy = 0;

	for ( i = 0; i < n; i++ ) {
		if ( sumi[i] > 0 ) {
			px  = sumi[i] / sum;
			hx -= px * log( px );
		}

		if ( sumj[i] > 0 ) {
			px  = sumj[i] / sum;
			hy -= px * log( px );
		}
	}

	hxy = 0;

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < n; j++ ) {
			if ( nn[i][j] > 0 ) {
				px   = nn[i][j] / sum;
				hxy -= px * log( px );
			}
		}
	}

	h = 0;

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < n; j++ ) {
			if ( nn[i][j] > 0 ) {
				p = nn[i][j] / sum;
				h -= p * log( p );
			}
		}
	}

	hygx = h - hx;
	hxgy = h - hy;
	uygx = ( hy - hygx ) / ( hy + tiny );
	uxgy = ( hx - hxgy ) / ( hx + tiny );
	uxy  = 2.0 * ( hx + hy - h ) / ( hx + hy + tiny );
	mi   = hx + hy - hxy;

	if ( type == 0 ) return 100.0 * uxy;

	if ( type == 1 ) return 100.0 * uygx;

	if ( type == 2 ) return 100.0 * uxgy;

	return uxy;
}
