/*
** gsl routines for float data type
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
** y = A*x
*/
/*
void
fmat_x_vector(gsl_matrix_float *A, gsl_vector_float *x, gsl_vector_float *y)
{
  int nrows,ncols;
  nrows = A->size1;
  ncols = A->size2;

  cblas_sgemv(CblasRowMajor,CblasNoTrans,nrows,ncols,1,A->data,ncols,
          x->data,1,0,y->data,1);
}
*/


/*
**  y = Ax
*/
gsl_vector_float *
fmat_x_vector( gsl_matrix_float *A, gsl_vector_float *x, gsl_vector_float *y )
{
	int i, j, nrows, ncols;
	float *ptr1, *ptr2, *ptr3, sum;

	nrows = A->size1;
	ncols = A->size2;

	if ( y == NULL ) {
		y = gsl_vector_float_alloc ( nrows );
	}

	if ( x->size != ncols || y->size != nrows ) {
		fprintf( stderr, " fmat_x_vect: incongruent dimensions\n" );
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
**  y = Ax,  A double, x float
*/
gsl_vector_float *
dmat_x_fvector( gsl_matrix *A, gsl_vector_float *x, gsl_vector_float *y )
{
	int i, j, nrows, ncols;
	float *ptr2, *ptr3, sum;
	double *ptr1;

	nrows = A->size1;
	ncols = A->size2;

	if ( y == NULL ) {
		y = gsl_vector_float_alloc ( nrows );
	}

	if ( x->size != ncols || y->size != nrows ) {
		fprintf( stderr, " fmat_x_vect: incongruent dimensions\n" );
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
float
fskalarproduct( gsl_vector_float *x, gsl_vector_float *y )
{
	int i, n;
	float *ptr1, *ptr2, sum;

	n = x->size;

	if ( y->size != n ) {
		fprintf( stderr, " fskalarproduct: incongruent vector sizes: %d %d", n, y->size );
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
**    C = A x B^T
*/
gsl_matrix_float *
fmat_x_matT( gsl_matrix_float *A, gsl_matrix_float *B, gsl_matrix_float *C )
{
	int i, j, k;
	int n, m, r;
	float *ptr1, *ptr2, *ptr3, sum;

	n = A->size1;
	r = A->size2;
	m = B->size1;


	if ( B->size2 != r ) {
		fprintf( stderr, "fmat_x_matT: incongruent matrix dimensions (A,B).\n" );
		exit( 0 );
	}

	if ( C == NULL ) {
		C = gsl_matrix_float_alloc ( n, m );
	} else {
		if ( C->size1 != n || C->size2 != m ) {
			fprintf( stderr, "fmat_x_matT: incongruent matrix dimensions(C).\n" );
			exit( 0 );
		}
	}

	ptr1 = C->data;

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < m; j++ ) {

			ptr2 = gsl_matrix_float_ptr ( A, i, 0 );
			ptr3 = gsl_matrix_float_ptr ( B, j, 0 );

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
**    C = A^T x B
*/
gsl_matrix_float *
fmatT_x_mat( gsl_matrix_float *A, gsl_matrix_float *B, gsl_matrix_float *C )
{
	int i, j, k;
	int n, m, r;
	float *ptr1, *ptr2, *ptr3, sum;

	n = A->size1;
	r = A->size2;
	m = B->size2;

	if ( B->size1 != n ) {
		fprintf( stderr, "fmatT_x_mat: incongruent matrix dimensions (A,B).\n" );
		exit( 0 );
	}

	if ( C == NULL ) {
		C = gsl_matrix_float_alloc ( r, m );
	} else {
		if ( C->size1 != r || C->size2 != m ) {
			fprintf( stderr, "fmatT_x_mat: incongruent matrix dimensions (C, %d %d, %d %d).\n",
					 C->size1, C->size2, r, m );
			exit( 0 );
		}
	}

	ptr1 = C->data;

	for ( i = 0; i < r; i++ ) {
		for ( j = 0; j < m; j++ ) {

			ptr2 = gsl_matrix_float_ptr ( A, 0, i );
			ptr3 = gsl_matrix_float_ptr ( B, 0, j );

			sum = 0;

			for ( k = 0; k < n; k++ ) {
				sum  += ( *ptr2 ) * ( *ptr3 );
				ptr2 += A->tda;
				ptr3 += B->tda;
			}

			*ptr1++ = sum;
		}
	}

	return C;
}


void
fmatprint( FILE *fp, gsl_matrix_float *A, const char *format )
{
	int i, j;

	for ( i = 0; i < A->size1; i++ ) {
		for ( j = 0; j < A->size2; j++ ) {
			fprintf( fp, format, fmget( A, i, j ) );
		}

		fprintf( fp, "\n" );
	}

	fprintf( fp, "\n" );

}


/*
**    C = A x B
*/
gsl_matrix_float *
fmat_x_mat( gsl_matrix_float *A, gsl_matrix_float *B, gsl_matrix_float *C )
{
	int i, j, k, m, n, r;
	float *ptr1, *ptr2, *ptr3, sum;


	n = A->size1;
	r = A->size2;
	m = B->size2;

	if ( B->size1 != r ) {
		fprintf( stderr, "fmat_x_mat: incongruent matrix dimensions (A,B).\n" );
		exit( 0 );
	}

	if ( C == NULL ) {
		C = gsl_matrix_float_alloc ( n, m );
	} else {
		if ( C->size1 != n || C->size2 != m ) {
			fprintf( stderr, "fmat_x_mat: incongruent matrix dimensions (C).\n" );
			exit( 0 );
		}
	}

	ptr1 = C->data;

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < m; j++ ) {

			ptr2 = gsl_matrix_float_ptr ( A, i, 0 );
			ptr3 = gsl_matrix_float_ptr ( B, 0, j );

			sum = 0;

			for ( k = 0; k < r; k++ ) {
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
  gsl_matrix_float *a=NULL,*b=NULL,*c=NULL;
  float x;

  n = 648;
  a = gsl_matrix_float_alloc(n,n);

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      x = i+j+1;
      fmset(a,i,j,x);
    }
  }

  b = gsl_matrix_float_alloc(n,n);
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      x = i-j+1;
      fmset(b,i,j,x);
    }
  }

  c = fmat_x_mat(a,b,NULL);
  exit(0);
}
*/

