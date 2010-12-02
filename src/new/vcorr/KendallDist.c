/*
** Kendall tau distance
**
** G.Lohmann, MPI-CBS, Dec 2009
*/
#include <viaio/Vlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

/*
** http://en.wikipedia.org/wiki/Kendall_tau_distance
**
*/
double
VKendallDist( double *array1, double *array2, int n )
{
	int    i, j;
	double x, y, u, v, sum, nx;

	sum = nx = 0;

	for ( i = 0; i < n; i++ ) {
		x = array1[i];
		u = array2[i];

		for ( j = 0; j < i; j++ ) {
			y = array1[j];
			v = array2[j];

			if ( ( x < y && u > v ) || ( ( x > y ) && ( u < v ) ) ) sum++;

			nx++;
		}
	}

	return 1.0 - sum / nx;
}
