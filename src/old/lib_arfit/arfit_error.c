#include "arfit_error.h"


#define 	MSE_ERROR				-7000.0					/* error on wrong input data for arfit_mse function */
#define 	MSY_ERROR				-700.0					/* error on wrong input data for arfit_rev function */
#define		REV_ERROR				-70.0					/* error in input or output data for rev function */



/* the error variance
*/
double arfit_mse( const arfit_output *output )
{
	if( !output )
		return MSE_ERROR;
	
	if( output->iprocessed == 0 )
		return MSE_ERROR;
	
	gsl_vector_view diag = gsl_matrix_diagonal( output->C );
	
	/* check if this vector is empty */
	if( diag.vector.size == 0 )
		return MSE_ERROR;
		
	double 			root = 0.0;
	int 			i;
	
	/* accumulate squared error value */
	for( i = 0; i < diag.vector.size; ++i )
		root += gsl_vector_get( &diag.vector, i ) * gsl_vector_get( &diag.vector, i );
		
	/* divide accumulated squared error by number of its elements to gain mean value */
	root /= diag.vector.size;

	return root;
}

/* the signal variance
*/
double arfit_msy( const arfit_input *input, int sample )
{
	if( !input )
		return 0.0;

	if( input->v->size2 <= sample || sample < 0 )
		return 0.0;
	
	double 	sig = 0.0;
	int		i;
	gsl_vector_view	col = gsl_matrix_column( input->v, (size_t) sample );
	
	/* check if this vector is empty */
	if( col.vector.size == 0 )
		return 0.0;

	/* accumulate the signal */	
	for( i = 0; i < col.vector.size; ++i )
		sig += gsl_vector_get( &col.vector, i ) * gsl_vector_get( &col.vector, i );
		
	/* divide accumulated signal by number of samples to gain mean value*/
	sig /= col.vector.size;
	
	return sig;
}

/* relative error variance
	normalized error variance
*/
double arfit_rev( const arfit_input *input, int sample, const arfit_output *output )
{
	double msy = arfit_msy( input, sample );
	double mse = arfit_mse( output );

	if( mse == MSE_ERROR )
		return MSE_ERROR;
	if( msy == 0.0 )
		return MSY_ERROR;
	if( !input || !output )
		return REV_ERROR;

	return mse / msy;
}

/* Goodness of Fit
	a measure for how much of the data has been fitted with the ar process
*/
double arfit_gof( const arfit_input *input, int sample, const arfit_output *output )
{
	if( !input || !output )
		return 0.0;

	return 1.0 - arfit_rev( input, sample, output );
}
