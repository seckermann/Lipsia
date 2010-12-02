#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define dvset gsl_vector_set
#define dvget gsl_vector_get
#define dmset gsl_matrix_set
#define dmget gsl_matrix_get


extern gsl_vector *dmat_x_vector( gsl_matrix *, gsl_vector *, gsl_vector * );


/*
**  y = Ax
*/
gsl_vector *
dmat_x_vector( gsl_matrix *A, gsl_vector *x, gsl_vector *y )
{
	int i, j, nrows, ncols;
	double *ptr1, *ptr2, *ptr3, sum;

	nrows = A->size1;
	ncols = A->size2;

	if ( y == NULL ) {
		y = gsl_vector_alloc ( nrows );
	}

	if ( x->size != ncols || y->size != nrows ) {
		fprintf( stderr, " dmat_x_vector: incongruent dimensions\n" );
		exit( 0 );
	}

	ptr1 = A->data;
	ptr3 = y->data;

	for ( i = 0; i < nrows; i++ ) {
		sum = 0;
		ptr2 = x->data;

		for ( j = 0; j < ncols; j++ ) {
			sum += ( *ptr1++ ) * ( *ptr2++ );
		}

		*ptr3++ = sum;
	}

	return y;
}


/* compute pseudoinverse: V * D^-1 * U^T */
void
PseudoInverse( gsl_matrix *U, gsl_matrix *V, gsl_vector *w, gsl_matrix *C )
{
	int k, l, j, n, m;
	double x, u, wmin, wmax, tiny = 1.0e-6;

	n = C->size1;
	m = C->size2;
	gsl_matrix_set_zero( C );

	wmax = 0;

	for ( j = 0; j < n; j++ ) {
		u = ABS( dvget( w, j ) );

		if ( u > wmax ) wmax = u;
	}

	wmin = wmax * tiny;

	if ( wmin < 1.0e-8 ) wmin = 1.0e-8;

	for ( k = 0; k < n; k++ ) {
		for ( l = 0; l < m; l++ ) {
			for ( j = 0; j < n; j++ ) {
				if ( dvget( w, j ) != 0 ) {
					x = dmget( C, k, l );
					u = dvget( w, j );

					if ( ABS( u ) > wmin )
						x += dmget( V, k, j ) * dmget( U, l, j ) / u;

					dmset( C, k, l, x );
				}
			}
		}
	}
}


double *Orthogonalize( double *regr, double *regr1, double *regr2, int m )
{
	int n = 2;
	int k, l;
	double u = 0;

	gsl_matrix *X = NULL, *X0 = NULL, *V = NULL, *XInv = NULL, *I = NULL;
	gsl_vector *w, *y, *z;


	X    = gsl_matrix_alloc ( m, n );
	X0   = gsl_matrix_alloc ( m, n );
	V    = gsl_matrix_alloc ( n, n );
	XInv = gsl_matrix_alloc ( n, m );

	w    = gsl_vector_alloc ( n );
	y    = gsl_vector_alloc ( m );
	z    = gsl_vector_alloc ( m );

	for ( k = 0; k < m; k++ ) {
		for ( l = 0; l < n; l++ ) {
			if ( l == 0 ) {
				dmset( X, k, l, regr[k] );
				dmset( X0, k, l, regr[k] );
			}

			if ( l == 1 ) {
				dmset( X, k, l, regr1[k] );
				dmset( X0, k, l, regr1[k] );
			}
		}

		dvset( y, k, regr2[k] );
	}

	/* Pseudoinverse */
	gsl_linalg_SV_decomp_jacobi( X, V, w );
	PseudoInverse( X, V, w, XInv );

	/* Multiplikation */
	dmat_x_vector( XInv, y, w );
	dmat_x_vector( X0, w, z );

	/* Orthogonalization */
	for ( k = 0; k < m; k++ )
		regr2[k] -= dvget( z, k );

	gsl_matrix_free( X );
	gsl_matrix_free( X0 );
	gsl_matrix_free( V );
	gsl_matrix_free( XInv );

	return regr2;
}
