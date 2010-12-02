#include    "gsl_matrix_wrapper.h"
#include    <string.h>


/* gsl_matrix_multiply function
    multiplies two gsl_matrix'es and returns the product
*/
gsl_matrix *gsl_matrix_multiply( const gsl_matrix *factor1, const gsl_matrix *factor2 )
{
	gsl_matrix *prod = gsl_matrix_alloc( factor1->size1, factor2->size2 );

	gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1.0, factor1, factor2, 0.0, prod );

	return prod;
}

/* gsl_matrix_multiply function
    multiplies two gsl_matrix'es and returns the product
*/
gsl_matrix *gsl_matrix_multiply_trans_notrans( const gsl_matrix *factor1, const gsl_matrix *factor2 )
{
	gsl_matrix *prod = gsl_matrix_alloc( factor1->size2, factor2->size2 );

	gsl_blas_dgemm( CblasTrans, CblasNoTrans, 1.0, factor1, factor2, 0.0, prod );

	return prod;
}

/* gsl_matrix_multiply function
    multiplies two gsl_matrix'es and returns the product
*/
gsl_matrix *gsl_matrix_multiply_notrans_trans( const gsl_matrix *factor1, const gsl_matrix *factor2 )
{
	gsl_matrix *prod = gsl_matrix_alloc( factor1->size1, factor2->size1 );

	gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1.0, factor1, factor2, 0.0, prod );

	return prod;
}

/* gsl_matrix_multiply function
    multiplies two gsl_matrix'es and returns the product
*/
gsl_matrix *gsl_matrix_multiply_trans_trans( const gsl_matrix *factor1, const gsl_matrix *factor2 )
{
	gsl_matrix *prod = gsl_matrix_alloc( factor1->size2, factor2->size1 );

	gsl_blas_dgemm( CblasTrans, CblasTrans, 1.0, factor1, factor2, 0.0, prod );

	return prod;
}


/* gsl_matrix_inverse function
    inverts a gsl_matrix
*/
gsl_matrix *gsl_matrix_inverse( const gsl_matrix *src )
{
	int             sign;
	gsl_matrix      *LU     = gsl_matrix_alloc( src->size1, src->size2 );
	gsl_matrix      *inv    = gsl_matrix_calloc( src->size1, src->size2 );
	gsl_permutation *p      = gsl_permutation_alloc( src->size1 );

	gsl_matrix_memcpy( LU, src );
	gsl_linalg_LU_decomp( LU, p, &sign );
	gsl_linalg_LU_invert( LU, p, inv );

	gsl_permutation_free( p );
	gsl_matrix_free( LU );

	return inv;
}


/* gsl_matrix_cov function
    returns the covariance matrix of two gsl_matrix'es
*/
int gsl_matrix_cov( const gsl_matrix *mat1, const gsl_matrix *mat2, gsl_matrix **cov, double *pn )
{
	( *cov ) = gsl_matrix_multiply_trans_notrans( mat1, mat2 );

	if( pn )
		*pn = ( double ) mat1->size1;

	return 0;
}

/* gsl_matrix_det function
    returns the determinant of gsl_matrix mat
*/
double gsl_matrix_det( const gsl_matrix *mat )
{
	gsl_permutation *p      = gsl_permutation_alloc( mat->size1 );
	gsl_matrix      *LU = gsl_matrix_alloc( mat->size1, mat->size2 );
	int             sign    = 0;
	double          det = 0.0;

	gsl_matrix_memcpy( LU, mat );
	gsl_linalg_LU_decomp( LU, p, &sign );
	det = gsl_linalg_LU_det( LU, sign );

	gsl_matrix_free( LU );
	gsl_permutation_free( p );

	return det;
}

/* gsl_matrix_part function
    copies the specified elements of a gsl_matrix to the specified area of an other gsl_matrix
*/
gsl_matrix *gsl_matrix_part( const gsl_matrix *src, gsl_matrix **part, int src_from_row, int src_from_col, int src_to_row, int src_to_col, int dest_from_row, int dest_from_col )
{
	int row, col;
	int brow = src_to_row - src_from_row + 1;
	int bcol = src_to_col - src_from_col + 1;
	gsl_matrix *p;

	/* check if target matrix ist not initialized or no pointer has been given to copy data to */
	if( part == NULL || *part == NULL ) {
		/* allocate new matrix of correct size */
		p = gsl_matrix_alloc( dest_from_row + brow, dest_from_col + bcol );
	} else {
		/* check if given target matrix is smaller than needed*/
		if( ( *part )->size1 < dest_from_row + brow || ( *part )->size2 < dest_from_col + bcol ) {
			int psize1 = ( ( *part )->size1 >= dest_from_row + brow ) ? ( *part )->size1 : dest_from_row + brow;
			int psize2 = ( ( *part )->size2 >= dest_from_col + bcol ) ? ( *part )->size2 : dest_from_col + bcol;
			p = gsl_matrix_calloc( psize1, psize2 );

			/* copy constant part of target matrix to new matrix */
			for( row = 0; row < ( *part )->size1; ++row ) for( col = 0; col < ( *part )->size2; ++col )
					gsl_matrix_set( p, row, col, gsl_matrix_get( *part, row, col ) );

			/* free memory of old matrix */
			gsl_matrix_free( *part );
		} else {
			/* if size of target matrix fits than just copy the pointers */
			p = *part;
		}
	}

	/* copy new data to target matrix */
	for( row = 0; row < brow; ++row ) for( col = 0; col < bcol; ++col )
			gsl_matrix_set( p, dest_from_row + row, dest_from_col + col, gsl_matrix_get( src, src_from_row + row, src_from_col + col ) );

	/* check if to set pointer */
	if( part != NULL )
		*part = p;

	return p;
}
