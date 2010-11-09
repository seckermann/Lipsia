#ifndef		ARFIT_BASE_HEADER_INCLUDED
#define 	ARFIT_BASE_HEADER_INCLUDED


/* interface header for arfit functions */

#include	<gsl/gsl_matrix.h>
#include	<gsl/gsl_vector.h>
#include	<gsl/gsl_blas.h>
#include	<gsl/gsl_linalg.h>
#include	<gsl/gsl_randist.h>
#include 	<gsl/gsl_math.h>
#include 	<gsl/gsl_roots.h>

#include	<limits.h>
#include	<stdio.h>
#include	<stdlib.h>


/* types */

/* describes algorithm to be used */
typedef enum
{
	arfit_algorithm_schloegl	= 0,
	arfit_algorithm_schneider 	= 1,
}
arfit_algorithm;

/* describes selector criteria for arfit */
typedef enum
{
	arfit_selector_sbc 	= 0,
	arfit_selector_fpe	= 1,
}
arfit_selector;

/* describes kind of algorithm used in arfit_schloegl */
typedef enum
{
	arfit_mode_notset 		= 0,
	arfit_mode_ywbiased 	= 1,
	arfit_mode_vmunbiased	= 2,
	arfit_mode_nsbiased		= 3,
	arfit_mode_vmbiased		= 5,
	arfit_mode_ywunbiased	= 6,
	arfit_mode_nsunbiased	= 7
}
arfit_mode;


/* input of arfit function */
typedef struct
{
	gsl_matrix 		*v;			/* matrix containing the time series data, columns are variables, rows represent observations */
	int 			pmin;		/* minimum value of order p */
	int 			pmax;		/* maximum value of order p */
	arfit_selector 	selector;	/* criteria to optimize order p (Schwarz's Bayesian, Akaike's Final Prediction Error */
	int 			zero;		/* defines wether w is the zero-vector or not ( 0 - zero, > 1 - not zero */
	long			threshold;  /* timecourse with signalmeanvalue less then threshold are not computed. Defaultvalue is -99.999.999 */
}
arfit_input;

/* output of arfit function */
typedef struct
{
	gsl_vector		*w;			/* vector of intercept terms */
	gsl_matrix		*A;			/* matrix of ar coefficients */
	gsl_matrix		*C;			/* noise covariance matrix */
	gsl_vector		*sbc;		/* values of the selection criterion, Schwarz Baysian */
	gsl_vector		*fpe;		/* values of the selection criterion, Akaikes final prediction error */
	gsl_matrix 		*th;		/* matrix containing information needed to compute comfidence intervals, as used by Schneider */
	int				popt;		/* optimal order p */
	int				iprocessed;	/* indicates wether this sample has been skipped from processing */
}
arfit_output;

/* macros to clean and set up arfit structures */

#define arfit_input_alloc( in )		{ \
									  (in) 				= malloc( sizeof( arfit_input ) ); \
									  (in)->v 			= NULL; \
									  (in)->pmin		= 1; \
									  (in)->pmax		= 1; \
									  (in)->selector 	= arfit_selector_sbc; \
									  (in)->zero		= 0; \
									  (in)->threshold	= 2000; \
									}
											
#define arfit_output_alloc( out ) 	{ \
									  (out) = malloc( sizeof( arfit_output ) ); \
									  (out)->w 			= NULL; \
									  (out)->A 			= NULL; \
									  (out)->C 			= NULL; \
									  (out)->sbc 		= NULL; \
									  (out)->fpe 		= NULL; \
									  (out)->th 		= NULL; \
									  (out)->popt		= 0; \
									  (out)->iprocessed = 1; \
									}
											
#define arfit_input_free( in )		{ \
									  if( (in)->v ) \
										gsl_matrix_free( (in)->v ); \
									  free( (in) ); \
									  (in) = NULL; \
									}
											
#define arfit_output_free( out )	{ \
									  if( (out)->w ) \
										gsl_vector_free( (out)->w ); \
									  if( (out)->A ) \
									  	gsl_matrix_free( (out)->A ); \
									  if( (out)->C ) \
									  	gsl_matrix_free( (out)->C ); \
									  if( (out)->sbc ) \
									  	gsl_vector_free( (out)->sbc ); \
									  if( (out)->fpe ) \
									  	gsl_vector_free( (out)->fpe ); \
									  if( (out)->th ) \
									  	gsl_matrix_free( (out)->th ); \
									  free( (out) ); \
									  (out) = NULL; \
									}


/* arfit methods, input structure dependent */
arfit_output	*arfit_schneider( arfit_input *input, arfit_output **output );
arfit_output	*arfit_schloegl( arfit_input *input, arfit_mode mode, arfit_output **output );


#endif
