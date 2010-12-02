
/*
** gsl routines for double data type
**
** G.Lohmann, July 2004
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

#define dvset gsl_vector_set
#define dvget gsl_vector_get
#define dmset gsl_matrix_set
#define dmget gsl_matrix_get

#define fvset gsl_vector_float_set
#define fvget gsl_vector_float_get
#define fmset gsl_matrix_float_set
#define fmget gsl_matrix_float_get


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



/*
**  y = x^T A x
*/
gsl_vector *
dvector_x_mat_x_vector( gsl_matrix *A, gsl_vector *x, gsl_vector *y )
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





/*
**  z = x^T y
*/
double
dskalarproduct( gsl_vector *x, gsl_vector *y )
{
	int i, n;
	double *ptr1, *ptr2, sum;

	n = x->size;

	if ( y->size != n ) {
		fprintf( stderr, " dfkalarproduct: incongruent vector sizes: %d %d", n, y->size );
		exit( 0 );
	}

	ptr1 = x->data;
	ptr2 = y->data;
	sum = 0;

	for ( i = 0; i < n; i++ ) {
		sum += ( *ptr1 ) * ( *ptr2 );
		ptr1++;
		ptr2++;
	}

	return sum;
}



/*
**    B = A^-1
*/
gsl_matrix *
dmat_PseudoInv( gsl_matrix *A, gsl_matrix *B )
{
	int i, j, k, n, m;
	double *ptr0, *ptr1, *ptr2, *ptr3, ux, vx, wx, x;
	static gsl_matrix *U = NULL, *V = NULL;
	static gsl_matrix *workmat = NULL;
	static gsl_vector *w = NULL;
	static gsl_vector *workvec = NULL;

	m = A->size1;
	n = A->size2;

	if ( B == NULL ) {
		B = gsl_matrix_alloc ( n, m );
	} else {
		if ( B->size1 != n || B->size2 != m )
			fprintf( stderr, "dmat_PseudoInv: incongruent matrix dimensions.\n" );

		return NULL;
	}

	if ( U == NULL ) {
		U = gsl_matrix_alloc ( m, n );
		V = gsl_matrix_alloc ( n, n );
		w = gsl_vector_alloc ( n );
	}

	/* singular value decomposition */
	if ( workvec == NULL ) workvec = gsl_vector_alloc( n );

	if ( workmat == NULL ) workmat = gsl_matrix_alloc( n, n );

	gsl_linalg_SV_decomp_mod( U, workmat, V, w, workvec );


	/*  compute pseudoinverse: V * D^-1 * U^T */
	gsl_matrix_set_zero( B );

	ptr0 = gsl_matrix_ptr ( B, 0, 0 );

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < m; j++ ) {

			ptr1 = w->data;
			ptr2 = gsl_matrix_ptr ( V, i, 0 );
			ptr3 = gsl_matrix_ptr ( U, j, 0 );

			for ( k = 0; k < n; k++ ) {
				wx = ( *ptr1++ );
				ux = ( *ptr2++ );
				vx = ( *ptr3++ );

				if ( wx != 0 ) {
					x = ( *ptr0 )  +  ux * vx / wx;
					( *ptr0 ) = x;
				}
			}

			( ptr0++ );
		}
	}

	return B;
}




/*
**    C = A x B^T
*/
gsl_matrix *
dmat_x_matT( gsl_matrix *A, gsl_matrix *B, gsl_matrix *C )
{
	int i, j, k;
	int n, r, m;
	double *ptr1, *ptr2, *ptr3, sum;


	n = A->size1;
	r = A->size2;
	m = B->size1;

	if ( B->size2 != r ) {
		fprintf( stderr, "fmat_x_matT: incongruent matrix dimensions (A,B).\n" );
		exit( 0 );
	}

	if ( C == NULL ) {
		C = gsl_matrix_alloc ( n, m );
	} else {
		if ( C->size1 != n || C->size2 != m ) {
			fprintf( stderr, "fmat_x_matT: incongruent matrix dimensions(C).\n" );
			exit( 0 );
		}
	}

	ptr1 = C->data;

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < m; j++ ) {

			ptr2 = gsl_matrix_ptr ( A, i, 0 );
			ptr3 = gsl_matrix_ptr ( B, j, 0 );

			sum = 0;

			for ( k = 0; k < r; k++ ) {
				sum  += ( *ptr2 ) * ( *ptr3 );
				ptr2++;
				ptr3++;
			}

			*ptr1++ = sum;
		}
	}

	return C;
}



/*
**    C = A x B
*/
gsl_matrix *
dmat_x_mat( gsl_matrix *A, gsl_matrix *B, gsl_matrix *C )
{
	int i, j, k, m, n, r;
	int nrowsA, ncolsA;
	int nrowsB, ncolsB;
	int nrowsC, ncolsC;
	double *ptr1, *ptr2, *ptr3, sum;


	nrowsA = A->size1;
	ncolsA = A->size2;

	nrowsB = B->size1;
	ncolsB = B->size2;

	if ( C == NULL )
		C = gsl_matrix_alloc ( nrowsA, ncolsB );


	nrowsC = C->size1;
	ncolsC = C->size2;

	if ( ncolsA != nrowsB || nrowsA != nrowsC || ncolsB != ncolsC ) {
		fprintf( stderr, "dmat_x_mat: incongruent matrix dimensions.\n" );
		exit( 0 );
	}

	m = A->size1;
	n = A->size2;
	r = B->size2;

	ptr1 = C->data;

	for ( i = 0; i < m; i++ ) {
		for ( j = 0; j < r; j++ ) {

			ptr2 = gsl_matrix_ptr ( A, i, 0 );
			ptr3 = gsl_matrix_ptr ( B, 0, j );

			sum = 0;

			for ( k = 0; k < n; k++ ) {
				sum += ( *ptr2++ ) * ( *ptr3 );
				ptr3 += B->tda;
			}

			*ptr1++ = sum;
		}
	}

	return C;
}




/*

int main (void)
{
  int i,j;
  int m,n,r;
  gsl_matrix *a=NULL,*b=NULL,*c=NULL;
  double x;

  n = 648;
  a = gsl_matrix_alloc(n,n);

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      x = i+j+1;
      dmset(a,i,j,x);
    }
  }

  b = gsl_matrix_alloc(n,n);
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      x = i-j+1;
      dmset(b,i,j,x);
    }
  }

  c = dmat_x_mat(a,b,NULL);
  exit(0);
}
*/
