#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x) * (x))

#define N 100       /* max num clusters */



/*
** get initial cluster centers
*/
void
Centers( int nvectors, int nclusters, int *list, gsl_rng *rx )
{
	double u, b;
	int i0, i, j;

	b = nvectors - 1;

	i = 0;

	while ( i < nclusters ) {
		u = gsl_rng_uniform( rx );
		i0 = ( int )( u * b );

		if ( i0 < 0 || i0 >= nvectors ) continue;

		for ( j = 0; j < nclusters; j++ )
			if ( list[j] == i0 ) goto next;

		list[i] = i0;
		i++;
next:
		;
	}
}




/* Euclidean distance */
double
dist( gsl_vector *v1, gsl_vector *v2 )
{
	int i;
	double sum, u, v;
	double *ptr1, *ptr2;

	sum = 0;
	ptr1 = v1->data;
	ptr2 = v2->data;

	for ( i = 0; i < v1->size; i++ ) {
		u = ( *ptr1++ );
		v = ( *ptr2++ );
		sum += ( u - v ) * ( u - v );
	}

	return sum;
}


int *
KMeans( gsl_matrix *mat, int nclusters, double *bic, double *aic )
{
	int *labels = NULL, *bestlabels = NULL, *list = NULL;
	int i, j, s, dim, iter, nvectors, maxiter = 1000;
	double xmax, dmin, d, nx, mx, best, xbic = 0;
	double rss = 0;
	gsl_vector *kmean[N], *kmean_sav[N];
	gsl_vector *vec = NULL, *tmp = NULL;
	unsigned long int seed;
	gsl_rng *rx = NULL;
	const gsl_rng_type *T = NULL;


	/* random */
	seed = 35521738;
	gsl_rng_env_setup();
	T  = gsl_rng_default;
	rx = gsl_rng_alloc( T );
	gsl_rng_set( rx, ( unsigned long int )seed );



	/* alloc */
	xmax = VRepnMaxValue( VDoubleRepn );
	dim = mat->size1;        /* vector length */
	nvectors = mat->size2;   /* num vectors (matrix columns) */
	vec = gsl_vector_calloc( dim );
	tmp = gsl_vector_calloc( dim );

	labels     = ( int * ) VCalloc( nvectors, sizeof( int ) );
	bestlabels = ( int * ) VCalloc( nvectors, sizeof( int ) );
	list       = ( int * ) VCalloc( nclusters, sizeof( int ) );


	for ( i = 0; i < nclusters; i++ ) {
		kmean[i] = gsl_vector_calloc( dim );
		kmean_sav[i] = gsl_vector_calloc( dim );
	}


	/* ini */
	best = VRepnMaxValue( VDoubleRepn );
	/* best = 1.0e+999 !!!!!!!!!!!!!! */

	for ( s = 0; s < 50; s++ ) { /* try several starting values */

		Centers( nvectors, nclusters, list, rx );

		for ( i = 0; i < nclusters; i++ )
			gsl_matrix_get_col( kmean[i], mat, list[i] );


		/* iterations */
		for ( iter = 0; iter < maxiter; iter++ ) {


			/* get nearest neighbour */
			for ( j = 0; j < nvectors; j++ ) {

				gsl_matrix_get_col( vec, mat, j );
				dmin = xmax;

				for ( i = 0; i < nclusters; i++ ) {
					d = dist( kmean[i], vec );

					if ( d < dmin ) {
						dmin = d;
						labels[j] = i;
					}
				}
			}

			/* update cluster means */
			for ( i = 0; i < nclusters; i++ ) {
				gsl_vector_memcpy( kmean_sav[i], kmean[i] );
				gsl_vector_set_zero( kmean[i] );

				nx = 0;

				for ( j = 0; j < nvectors; j++ ) {
					if ( labels[j] != i ) continue;

					gsl_matrix_get_col( vec, mat, j );
					gsl_vector_add( kmean[i], vec );
					nx++;
				}

				gsl_vector_scale( kmean[i], 1.0 / nx );
			}


			/* stop iterations if no significant changes occurr */
			d = 0;

			for ( i = 0; i < nclusters; i++ )
				d += dist( kmean[i], kmean_sav[i] );

			if ( d < 1.0e-10 ) break;
		}


		/* residual sum of squares, RSS */
		rss = 0;

		for ( i = 0; i < nclusters; i++ ) {
			for ( j = 0; j < nvectors; j++ ) {
				if ( labels[j] != i ) continue;

				gsl_matrix_get_col( vec, mat, j );
				rss += dist( kmean[i], vec );
			}
		}

		if ( rss < best ) {
			best = rss;

			for ( j = 0; j < nvectors; j++ ) bestlabels[j] = labels[j];
		}
	}


	/* Bayesian information criterion (not very useful) */
	nx = ( double )nvectors;
	mx = ( double )nclusters;

	xbic = nx * log( best ) + mx * log( nx );

	( *bic ) = log( best ) + log( nx ) * mx / nx;
	( *aic ) = log( best ) + 2.0 * mx / nx;
	( *bic ) = xbic;

	return bestlabels;
}

