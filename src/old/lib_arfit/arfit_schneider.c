#include    "arfit_schneider.h"
#include    "gsl_matrix_wrapper.h"
#include    <limits.h>
#include    <math.h>

#define     EPS     0.00000000000000022204


/* the incomplete beta function fittet in gsl sheme for root estimation */
struct beta_params {
	double  n;
	double  p;
};

double beta( double x, void *params )
{
	return 1.0 - gsl_ran_beta_pdf( x, ( ( struct beta_params * ) params )->n * 0.5, 0.5 ) * 0.5 - ( ( struct beta_params * ) params )->p;
}

double betaInv( double x, void *params )
{
	return gsl_ran_beta_pdf( x, ( ( struct beta_params * ) params )->n * 0.5, 0.5 ) * 0.5 - ( ( struct beta_params * ) params )->p;;
}

double tquant( int n, double p )
{
	int                 status;
	int                 iter        = 0;
	int                 max_iter    = 100;
	double              r           = 0;
	/*double                r_expected  = 0.5;*/
	double              left        = 0.0;
	double              right       = 0.99;
	gsl_function        F;
	struct beta_params  params;
	gsl_root_fsolver    *solver;
	const gsl_root_fsolver_type *T;


	if( p < 0 || p > 1 )
		p = 0.5;

	if( n == 0 )
		return -9000;
	else if( n == 1 )
		return 9000;


	params.n    = n;
	params.p    = p;

	if( p >= 0.5 )
		F.function  = &beta;
	else
		F.function  = &betaInv;

	F.params    = &params;

	T           = gsl_root_fsolver_brent;
	solver      = gsl_root_fsolver_alloc ( T );
	gsl_root_fsolver_set( solver, &F, left, right );

	do {
		++iter;
		status  = gsl_root_fsolver_iterate( solver );
		r       = gsl_root_fsolver_root( solver );
		left    = gsl_root_fsolver_x_lower( solver );
		right   = gsl_root_fsolver_x_upper( solver );
		status  = gsl_root_test_interval( left, right, 0, 0.001 );
	} while( status == GSL_CONTINUE && iter < max_iter );

	gsl_root_fsolver_free( solver );

	if( p >= 0.5f )
		r = sqrt( ( double ) n / r - ( double ) n );
	else
		r = -sqrt( ( double ) n / r - ( double ) n );

	return r;
}

/* arconf function
    called by arfit to determine estimation errors of A and w
*/
int arconf( gsl_matrix *A, gsl_matrix *C, gsl_vector *w, gsl_matrix *th, int p, gsl_matrix **Aerr, gsl_vector **Werr )
{
	double      ccoeff  = 0.95, t/*, pquant*/;
	int         m       = C->size1;
	int         np, i, j, dof;
	gsl_matrix  *Aaug, *Uinv, *Aaug_err;

	*Aerr = gsl_matrix_calloc( A->size1, A->size2 );
	*Werr = gsl_vector_calloc( w->size );

	if( gsl_vector_isnull( w ) ) {
		Aaug = gsl_matrix_alloc( A->size1, A->size2 );
		gsl_matrix_memcpy( Aaug, A );
		np = m * p;
	} else {
		Aaug = gsl_matrix_alloc( A->size1, A->size2 + 1 );

		for( i = 0; i < w->size; ++i )
			gsl_matrix_set( Aaug, i, 0, gsl_vector_get( w, i ) );

		for( i = 0; i < A->size1; ++i ) for( j = 0; j < A->size2; ++j )
				gsl_matrix_set( Aaug, i, j + 1, gsl_matrix_get( A, i, j ) );

		np = m * p + 1;
	}

	dof     = ( int ) gsl_matrix_get( th, 0, 0 );
	t       = tquant( dof, 0.5 + 0.5 * ccoeff );

	Uinv = gsl_matrix_calloc( th->size1 - 1, th->size2 );
	gsl_matrix_part( th, &Uinv, 1, 0, th->size1 - 1, th->size2 - 1, 0, 0 );

	Aaug_err = gsl_matrix_calloc( m, np );

	for( i = 0; i < m; ++i ) for( j = 0; j < np; ++j )
			gsl_matrix_set( Aaug_err, i, j, t * sqrt( gsl_matrix_get( Uinv, j, j ) * gsl_matrix_get( C, i, i ) ) );

	if( gsl_vector_isnull( w ) )
		gsl_matrix_memcpy( *Aerr, Aaug_err );
	else {
		for( i = 0; i < ( *Werr )->size; ++i )
			gsl_vector_set( *Werr, i, gsl_matrix_get( Aaug_err, i, 0 ) );

		gsl_matrix_part( Aaug_err, Aerr, 0, 1, Aaug_err->size1 - 1, np - 1, 0, 0 );
	}

	gsl_matrix_free( Aaug );
	gsl_matrix_free( Aaug_err );
	gsl_matrix_free( Uinv );

	return 0;
}

/* arqr function
    computes QR factorization for model of order pmax
*/
int arqr( gsl_matrix *v, int p, int zero, gsl_matrix **R, gsl_vector **scale )
{
	int m, n, ne, np, q, j, i;
	double delta, value;
	gsl_matrix *K, *diag, *expand;
	gsl_vector *Kr, *Kc, *vr, *vc;

	/* check paramters */
	if( p <= 0 || !v || !R || !scale ) {
		fprintf( stderr, "arqr failed due to invalid arguments" );
		return 1;
	}

	n   = v->size1;
	m   = v->size2;
	ne  = n - p;
	np  = m * p + zero;

	/* assemble the data matrix K ( of which a QR factorization will be computed ) */
	K   = gsl_matrix_calloc( ne, np + m );
	Kr  = gsl_vector_alloc( K->size2 );
	Kc  = gsl_vector_alloc( K->size1 );
	vr  = gsl_vector_alloc( v->size2 );
	vc  = gsl_vector_alloc( v->size1 );
	gsl_matrix_set_zero( K );

	/* if intercept vector w shall be computed first column of K is 1 */
	if( zero == 1 ) {
		gsl_vector_set_all( Kc, 1.0 );
		gsl_matrix_set_col( K, 0, Kc );
	}

	/* assemble 'predictors' u in K */
	for( j = 1; j <= p; ++j )
		gsl_matrix_part( v, &K, p - j, 0, n - j - 1, v->size2 - 1, 0, zero + m * ( j - 1 ) );

	/* Add observations' v (left hand side of regression model) to K  */
	gsl_matrix_part( v, &K, p, 0, n - 1, v->size2 - 1, 0, np );

	/* QR decomposition of K expanded by diagonal matrix of scale*/
	q       = np + m;
	delta   = sqrt( ( double )( q * q + q + 1 ) * EPS );

	*scale = gsl_vector_alloc( K->size2 );

	for( i = 0; i < K->size2; ++i ) {
		value = 0.0;

		for( j = 0; j < K->size1; ++j )
			value += gsl_matrix_get( K, j, i ) * gsl_matrix_get( K, j, i );

		gsl_vector_set( *scale, i, delta * sqrt( value ) );;
	}

	diag = gsl_matrix_calloc( ( *scale )->size, ( *scale )->size );

	for( i = 0; i < ( *scale )->size; ++i )
		gsl_matrix_set( diag, i, i, gsl_vector_get( *scale, i ) );

	expand = gsl_matrix_alloc( K->size1 + diag->size1, K->size2 );

	for( i = 0; i < K->size1; ++i ) {
		gsl_matrix_get_row( Kr, K, i );
		gsl_matrix_set_row( expand, i, Kr );
	}

	for( i = 0; i < diag->size1; ++i ) {
		gsl_matrix_get_row( Kr, diag, i );
		gsl_matrix_set_row( expand, K->size1 + i, Kr );
	}

	gsl_vector_free( vr );
	vr = gsl_vector_alloc( ( expand->size1 < expand->size2 ) ? expand->size1 : expand->size2 );
	gsl_linalg_QR_decomp( expand, vr );

	/* upper triangular matrix */
	*R = gsl_matrix_calloc( expand->size1, expand->size2 );

	for( i = 0; i < ( *R )->size1; ++i ) for( j = i; j < ( *R )->size2; ++j )
			gsl_matrix_set( *R, i, j, gsl_matrix_get( expand, i, j ) );

	gsl_vector_free( vr );
	gsl_vector_free( vc );
	gsl_vector_free( Kr );
	gsl_vector_free( Kc );
	gsl_matrix_free( K );
	gsl_matrix_free( expand );
	gsl_matrix_free( diag );

	return 0;
}

/* arord function
    evaluates criteria for selecting the order of an AR model
*/
int arord( gsl_matrix *R, int m, int zero, int ne, int pmin, int pmax, gsl_vector **sbc, gsl_vector **fpe, gsl_vector **logdp, gsl_vector **np )
{
	int imax, i, j, p;
	double value;
	gsl_matrix *R22, *invR22, *invR22T, *Mp, *Rp, *I, *L, *RpT, *N, *NT;

	/* check parameters */
	if( !R || !sbc || !fpe ) {
		fprintf( stderr, "arord failed due to invalid arguments" );
		return 1;
	}

	imax = pmax - pmin + 1;
	*sbc = gsl_vector_calloc( imax );
	*fpe = gsl_vector_calloc( imax );
	*logdp = gsl_vector_calloc( imax );
	*np = gsl_vector_calloc( imax );
	gsl_vector_set( *np, imax - 1, m * pmax + zero );

	/* get lower right triangle R22 of R */
	R22     = gsl_matrix_part( R, NULL, gsl_vector_get( *np, imax - 1 ), gsl_vector_get( *np, imax - 1 ), gsl_vector_get( *np, imax - 1 ) + m - 1, gsl_vector_get( *np, imax - 1 ) + m - 1, 0, 0 );;
	invR22T = gsl_matrix_alloc( m, m );

	/* get inverse of residual cross-product matrix for model of order pmax */
	invR22 = gsl_matrix_inverse( R22 );
	gsl_matrix_transpose_memcpy( invR22T, invR22 );
	Mp = gsl_matrix_multiply( invR22, invR22T );
	gsl_matrix_free( invR22 );
	gsl_matrix_free( invR22T );

	/* get determinant of residual cross-product matrix */
	value = 1.0;

	for( i = 0; i < R22->size1; ++i )
		value *= gsl_matrix_get( R22, i, i );

	gsl_vector_set( *logdp, imax - 1, 2 * log( fabs( value ) ) );
	gsl_matrix_free( R22 );

	/* compute approximate order selection criteria for model of order pmin to pmax */
	i = imax;

	for( p = pmax; p >= pmin; --p ) {
		gsl_vector_set( *np, i - 1, m * p + zero );

		if( p < pmax ) {
			/* downdate determinant of residual cross-product matrix
			    Rp: Part of R to be added to Cholesky factor of covariance matrix
			    Rp       = R(np(i)+1:np(i)+m, np(imax)+1:np(imax)+m);*/
			Rp  = gsl_matrix_part( R, NULL, gsl_vector_get( *np, i - 1 ), gsl_vector_get( *np, imax - 1 ), gsl_vector_get( *np, i - 1 ) + m - 1, gsl_vector_get( *np, imax - 1 ) + m - 1, 0, 0 );
			RpT = gsl_matrix_alloc( m, m );
			gsl_matrix_transpose_memcpy( RpT, Rp );

			/* get Mp, the downdated inverse of the residual cross-product matrix, using the Woodbury formula */
			I       = gsl_matrix_alloc( m, m );
			L       = gsl_matrix_alloc( m, m );
			NT      = gsl_matrix_multiply( Rp, Mp );
			N       = gsl_matrix_multiply( NT, RpT );
			gsl_matrix_set_identity( I );
			gsl_matrix_add( I, N );
			gsl_linalg_cholesky_decomp( I );
			gsl_matrix_transpose_memcpy( L, I );
			gsl_matrix_free( I );
			gsl_matrix_free( RpT );
			I = gsl_matrix_multiply( Rp, Mp );
			RpT = gsl_matrix_inverse( L );
			gsl_matrix_free( N );
			N = gsl_matrix_multiply( RpT, I );
			gsl_matrix_transpose_memcpy( NT, N );
			gsl_matrix_free( RpT );
			RpT = gsl_matrix_multiply( NT, N );
			gsl_matrix_sub( Mp, RpT );
			value = 1.0;

			for( j = 0; j < L->size1; ++j )
				value *= gsl_matrix_get( L, j, j );

			gsl_vector_set( *logdp, i - 1, gsl_vector_get( *logdp, i ) + 2 * log( fabs( value ) ) );

			gsl_matrix_free( Rp );
			gsl_matrix_free( RpT );
			gsl_matrix_free( L );
			gsl_matrix_free( N );
			gsl_matrix_free( NT );
			gsl_matrix_free( I );
		}

		gsl_vector_set( *sbc, i - 1, gsl_vector_get( *logdp, i - 1 ) / m - log( ne ) * ( ne - gsl_vector_get( *np, i - 1 ) ) / ne );
		gsl_vector_set( *fpe, i - 1, gsl_vector_get( *logdp, i - 1 ) / m - log( ne * ( ne - gsl_vector_get( *np, i - 1 ) ) / ( ne + gsl_vector_get( *np, i - 1 ) ) ) );

		--i;
	}

	gsl_matrix_free( Mp );

	return 0;
}

/* arfit_schneider function
    estimate ar model values
*/
arfit_output *arfit_schneider( arfit_input *input, arfit_output **output )
{
	int m, n, ne, npmax, nopt, popt, i, j, dof;
	double value, dofinv, sum1, sum2;
	gsl_matrix *R = NULL, *R11 = NULL, *R12 = NULL, *R22 = NULL, *Aaug = NULL, *tmp = NULL;
	gsl_vector  *scale, *logdp, *np;

	/* check parameters */
	if( !input ) {
		fprintf( stderr, "Invalid input argument" );
		return NULL;
	}

	if( input->pmax < input->pmin ) {
		fprintf( stderr, "pmax must be greater than or equal to pmin. pmax set to pmin" );
		input->pmax = input->pmin;
	}

	if( input->zero > 1 ) {
		fprintf( stderr, "zero is greater than 1. zero set to 1" );
		input->zero = 1;
	} else if( input->zero < 0 ) {
		fprintf( stderr, "zero is less than 0. zero set to 0" );
		input->zero = 0;
	}

	if( *output )
		arfit_output_free( *output );

	arfit_output_alloc( *output );
	( *output )->A = gsl_matrix_calloc( input->v->size2, input->pmax );
	( *output )->C = gsl_matrix_calloc( input->v->size2, input->v->size2 );

	/* thresholding and normalization */
	for( i = 0; i < input->v->size2; ++i ) {
		value   = 0.0;
		sum1    = 0.0;
		sum2    = 0.0;

		for( j = 0; j < input->v->size1; ++j ) {
			value    = gsl_matrix_get( input->v, j, i );
			sum1    += value;
			sum2    += value * value;
		}

		sum1    /= ( double ) input->v->size1;
		sum2     = sqrt( ( sum2 - ( double ) input->v->size1 * sum1 * sum1 ) / ( double )( input->v->size1 - 1 ) );


		if( sum1 < input->threshold ) {
			( *output )->sbc          = gsl_vector_calloc( 1 );
			( *output )->fpe          = gsl_vector_calloc( 1 );
			( *output )->w            = gsl_vector_calloc( input->v->size2 );
			( *output )->iprocessed   = 0;

			return *output;
		} else {
			for( j = 0; j < input->v->size1; ++j ) {
				if( sum2 > 1 )
					value = ( gsl_matrix_get( input->v, j, i ) - sum1 ) / sum2;
				else
					value = 0.0;

				gsl_matrix_set( input->v, j, i, value );
			}
		}
	}

	n       = input->v->size1;                      /* dimension of state vectors ( rows of v ) */
	m       = input->v->size2;                      /* number of observations (columns of v )*/
	ne      = n - input->pmax;                      /* number of block equations of size m */
	npmax   = m * input->pmax + input->zero;        /* maximum number of paramter vectors of length m*/

	if( ne <= npmax ) {
		fprintf( stderr, "Time series too short" );
		return NULL;
	}

	if( arqr( input->v, input->pmax, input->zero, &R, &scale ) )
		return NULL;

	if( arord( R, m, input->zero, ne, input->pmin, input->pmax, &( *output )->sbc, &( *output )->fpe, &logdp, &np ) )
		return NULL;

	/* get minimum of selected order selection criterion */
	if( input->selector == arfit_selector_sbc ) {
		nopt    = gsl_vector_min_index( ( *output )->sbc );
		value   = gsl_vector_get( ( *output )->sbc, nopt );
	} else {
		nopt    = gsl_vector_min_index( ( *output )->fpe );
		value   = gsl_vector_get( ( *output )->fpe, nopt );
	}

	( *output )->popt = popt = input->pmin + nopt;
	nopt            = m * popt + input->zero;

	/* decompose R */
	gsl_matrix_part( R, &R11, 0, 0, nopt - 1, nopt - 1, 0, 0 );
	gsl_matrix_part( R, &R12, 0, npmax, nopt - 1, npmax + m - 1, 0, 0 );
	gsl_matrix_part( R, &R22, nopt, npmax, npmax + m - 1, npmax + m - 1, 0, 0 );
	gsl_matrix_free( R );

	/* get augmented parameter matrix Aug = [w A] if zero = 1 and Aug = A if zero = 0 */
	if( nopt > 0 ) {
		if( input->zero == 1 ) {
			value = gsl_vector_get( scale, 1 );

			for( i = 2; i < scale->size; ++i ) if( value < gsl_vector_get( scale, i ) )
					value = gsl_vector_get( scale, i );

			value /= gsl_vector_get( scale, 0 );

			for( i = 0; i < R11->size1; ++i )
				gsl_matrix_set( R11, i, 0, gsl_matrix_get( R11, i, 0 ) * value );
		}

		Aaug = gsl_matrix_inverse( R11 );
		tmp = gsl_matrix_multiply( Aaug, R12 );
		gsl_matrix_free( Aaug );
		Aaug = gsl_matrix_alloc( tmp->size2, tmp->size1 );
		gsl_matrix_transpose_memcpy( Aaug, tmp );
		gsl_matrix_free( tmp );

		if( input->zero == 1 ) {
			( *output )->w = gsl_vector_alloc( Aaug->size1 );
			gsl_matrix_get_col( ( *output )->w, Aaug, 0 );
			gsl_vector_scale( ( *output )->w, value );

			for( i = 0; i < Aaug->size1; ++i ) for( j = 1; j < Aaug->size2; ++j ) {
					double aaug_value = gsl_matrix_get( Aaug, i, j );

					if( isnan( aaug_value ) )
						gsl_matrix_set( ( *output )->A, i, j - 1, 0.0 );
					else
						gsl_matrix_set( ( *output )->A, i, j - 1, gsl_matrix_get( Aaug, i, j ) );
				}
		} else {
			( *output )->w = gsl_vector_calloc( m );

			for( i = 0; i < Aaug->size1; ++i ) for( j = 0; j < Aaug->size2; ++j ) {
					double aaug_value = gsl_matrix_get( Aaug, i, j );

					if( isnan( aaug_value ) )
						gsl_matrix_set( ( *output )->A, i, j, 0.0 );
					else
						gsl_matrix_set( ( *output )->A, i, j, gsl_matrix_get( Aaug, i, j ) );
				}
		}

		gsl_matrix_free( Aaug );
	} else
		( *output )->w    = gsl_vector_calloc( m );

	/* estimated covariance matrix C */
	dof     = ne - nopt;
	dofinv  = 1.0 / ( double ) dof;
	gsl_matrix_cov( R22, R22, &( *output )->C, NULL );
	gsl_matrix_scale( ( *output )->C, dofinv );

	/* th matrix, helper matrix to determine estimation error by arconf*/
	if( nopt > 0 ) {
		tmp     = gsl_matrix_alloc( R11->size1, R11->size2 );
		Aaug    = gsl_matrix_inverse( R11 );

		if( input->zero == 1 ) {
			for( i = 0; i < Aaug->size2; ++i )
				gsl_matrix_set( Aaug, 0, i, gsl_matrix_get( Aaug, 0, i ) * value );
		}

		gsl_matrix_transpose_memcpy( tmp, Aaug );
		gsl_matrix_free( R11 );
		R11 = gsl_matrix_multiply( Aaug, tmp );
		( *output )->th = gsl_matrix_calloc( R11->size1 + 1, R11->size2 );
		gsl_matrix_set( ( *output )->th, 0, 0, dof );

		for( i = 1; i < R11->size1; ++i ) for( j = 0; j < R11->size2; ++j )
				gsl_matrix_set( ( *output )->th, 1 + i, j, gsl_matrix_get( R11, i, j ) );

		gsl_matrix_free( tmp );
	}

	/*  arconf( (*output)->A, (*output)->C, (*output)->w, (*output)->th, (*output)->popt, &(*output)->Aerr, &(*output)->Werr );
	*/
	if( R11 )
		gsl_matrix_free( R11 );

	if( R12 )
		gsl_matrix_free( R12 );

	if( R22 )
		gsl_matrix_free( R22 );

	if( Aaug )
		gsl_matrix_free( Aaug );

	gsl_vector_free( np );
	gsl_vector_free( logdp );
	gsl_vector_free( scale );

	return *output;
}
