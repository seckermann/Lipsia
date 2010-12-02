#ifndef     GSL_MATRIX_WRAPPER_HEADER_INCLUDED
#define     GSL_MATRIX_WRAPPER_HEADER_INCLUDED


#include    <gsl/gsl_matrix.h>
#include    <gsl/gsl_vector.h>
#include    <gsl/gsl_blas.h>
#include    <gsl/gsl_linalg.h>


/* function prototypes */

gsl_matrix *gsl_matrix_multiply( const gsl_matrix *factor1, const gsl_matrix *factor2 );
gsl_matrix *gsl_matrix_multiply_trans_notrans( const gsl_matrix *factor1, const gsl_matrix *factor2 );
gsl_matrix *gsl_matrix_multiply_notrans_trans( const gsl_matrix *factor1, const gsl_matrix *factor2 );
gsl_matrix *gsl_matrix_multiply_trans_trans( const gsl_matrix *factor1, const gsl_matrix *factor2 );

gsl_matrix *gsl_matrix_inverse( const gsl_matrix *src );
int         gsl_matrix_cov( const gsl_matrix *mat1, const gsl_matrix *mat2, gsl_matrix **cov, double *pn );
double      gsl_matrix_det( const gsl_matrix *mat );
gsl_matrix *gsl_matrix_part( const gsl_matrix *src, gsl_matrix **part, int src_from_row, int src_from_col, int src_to_row, int src_to_col, int dest_from_row, int dest_from_col );


#endif
