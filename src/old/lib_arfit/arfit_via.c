#include    "arfit_via.h"


/* get_file_info function
    get information about functional time series data in given via file
*/
arfit_file_info *get_file_info( FILE *in_file, arfit_file_info *info )
{
	VAttrListPosn   iterator;
	VImage          source;
	int             slices = 0, totalslices = 0;

	/* check parameters */
	if( !in_file )
		return NULL;

	/* check for existing info */
	if( !info )
		info = malloc( sizeof( arfit_file_info ) );

	if( !( info->vAttributes = VReadFile( in_file, NULL ) ) ) {
		free( info );
		return NULL;
	}

	info->vFunctionalImages         = NULL;
	info->nFunctionalImageIndices   = NULL;
	info->ntotalslices              = 0;
	info->nfunctionalslices         = 0;
	info->nsamples                  = 0;
	info->nrows                     = 0;
	info->ncolumns                  = 0;

	/* iterate through all file attributes */
	for( VFirstAttr( info->vAttributes, &iterator ); VAttrExists( &iterator ); VNextAttr( &iterator ) ) {
		/* check if current attribute is an image */
		if( VGetAttrRepn( &iterator ) == VImageRepn ) {
			/* query attribute value and check for short pixel format*/
			VGetAttrValue( &iterator, NULL, VImageRepn, &source );

			if( VPixelRepn( source ) == VShortRepn ) {
				int isNullImage = 0;

				++slices;

				/* initialize number of samples, rows and columns in current image if not set */
				if( info->nsamples == 0 && ( int )VImageNBands( source ) > 1 )
					info->nsamples = ( int )VImageNBands( source );

				if( info->nrows == 0 && ( int )VImageNRows( source ) > 1 )
					info->nrows = ( int )VImageNRows( source );

				if( info->ncolumns == 0 && ( int )VImageNColumns( source ) > 1 )
					info->ncolumns = ( int )VImageNColumns( source );

				/* check for a null image object */
				if( ( int )VImageNBands( source ) == 1 && ( int )VImageNRows( source ) == 1 && ( int )VImageNColumns( source ) == 1 )
					isNullImage = 1;

				/* check if this images dimensions differ from the first image found */
				if( info->nsamples != VImageNBands( source ) && !isNullImage )
					VWarning( "Found functional image but number of bands differs. FncImage %d will be ignored", slices );
				else {
					if( info->nrows != VImageNRows( source ) && !isNullImage )
						VWarning( "Found functional image but number of rows differs. FncImage %d will be ignored", slices );
					else {
						if( info->ncolumns != VImageNColumns( source ) && !isNullImage )
							VWarning( "Found functional image but number of columns differs. FncImage %d will be ignored", slices );
						else {
							++info->ntotalslices;

							if( !isNullImage )
								++info->nfunctionalslices;
						}
					}
				}
			}
		}
	}

	/* allocate memory for functional images */
	info->vFunctionalImages         = ( VImage * ) malloc( sizeof( VImage ) * info->nfunctionalslices );
	info->nFunctionalImageIndices   = ( int * ) malloc( sizeof( int ) * info->nfunctionalslices );

	slices = 0;

	/* iterate through all file attributes */
	for( VFirstAttr( info->vAttributes, &iterator ); VAttrExists( &iterator ); VNextAttr( &iterator ) ) {
		/* check if current attribute is an image */
		if( VGetAttrRepn( &iterator ) == VImageRepn ) {
			/* query attribute value and check for short pixel format*/
			VGetAttrValue( &iterator, NULL, VImageRepn, &source );

			if( VPixelRepn( source ) == VShortRepn ) {
				int isNullImage = 0;

				/* check for a null image object */
				if( ( int )VImageNBands( source ) == 1 && ( int )VImageNRows( source ) == 1 && ( int )VImageNColumns( source ) == 1 )
					isNullImage = 1;

				/* check if this images dimensions differ from the first image found */
				if( info->nsamples != VImageNBands( source ) && !isNullImage )
					VWarning( "Found functional image but number of bands differs. FncImage %d will be ignored", slices + 1 );
				else {
					if( info->nrows != VImageNRows( source ) && !isNullImage )
						VWarning( "Found functional image but number of rows differs. FncImage %d will be ignored", slices + 1 );
					else {
						if( info->ncolumns != VImageNColumns( source ) && !isNullImage )
							VWarning( "Found functional image but number of columns differs. FncImage %d will be ignored", slices + 1 );
						else {
							if( !isNullImage ) {
								info->vFunctionalImages[ slices ]   = source;
								info->nFunctionalImageIndices[ slices++ ] = totalslices;
							}

							++totalslices;
						}
					}
				}
			}
		}
	}

	return info;
}

/* free_arfit_file_info
    free memory allocated in arfit_file_info structure
*/
void free_file_info( arfit_file_info *info )
{
	/* check parameters */
	if( info ) {
		/* free functional image date */
		if( info->vFunctionalImages )
			free( info->vFunctionalImages );

		if( info->nFunctionalImageIndices )
			free( info->nFunctionalImageIndices );
	}
}

/* add_to_input function
    add a single data sample to the given arfit_input structure
*/
arfit_input *add_sample_to_input( const arfit_file_info *info, arfit_input *input, int slice, int row, int column )
{
	gsl_matrix      *new_input;
	gsl_vector_view input_column;
	int             cc = 0, rr = 0, isNullImage = 1, nstoredindex = 0;

	/* check parameters */
	if( !info || !input ) {
		VWarning( "Invalid parameters" );
		return input;
	}

	if( slice >= info->ntotalslices || slice < 0 || row >= info->nrows || row < 0 || column >= info->ncolumns || column < 0 ) {
		fprintf( stderr, "totalsl: %d funcsl: %d sl: %d rows: %d cols: %d\n", info->ntotalslices, info->nfunctionalslices, slice, info->nrows, info->ncolumns );
		VWarning( "Incorrect indices of data sample" );
		return input;
	}

	/* allocate memory for new sample matrix */
	new_input = gsl_matrix_calloc( info->nsamples, ( input->v ) ? input->v->size2 + 1 : 1 );

	/* copy elements of v to new_input */
	for( cc = 0; cc < new_input->size2 - 1; ++cc ) {
		input_column = gsl_matrix_column( input->v, cc );
		gsl_matrix_set_col( new_input, cc, &input_column.vector );
	}

	/* check if old sample matrix needs to be freed */
	if( input->v )
		gsl_matrix_free( input->v );

	/* check for a not null object slice */
	for( rr = 0; rr < info->nfunctionalslices; ++rr ) if( info->nFunctionalImageIndices[ rr ] == slice ) {
			isNullImage     = 0;
			nstoredindex    = rr;
		}

	/* set new input data */
	for( rr = 0; rr < info->nsamples; ++rr ) {
		if( isNullImage )
			gsl_matrix_set( new_input, rr, new_input->size2 - 1, 0.0 );
		else
			gsl_matrix_set( new_input, rr, new_input->size2 - 1, ( double )VPixel( info->vFunctionalImages[ nstoredindex ], rr, row, column, VShort ) );
	}

	/* set new input matrix to input structure */
	input->v = new_input;

	return input;
}


/* add_to_input function
    add data samples to the given arfit_input structure
*/
arfit_input *add_samples_to_input( const arfit_file_info *info, arfit_input *input, int quantity, int *slices, int *rows, int *columns )
{
	int i;

	for( i = 0; i < quantity; ++i )
		add_sample_to_input( info, input, slices[ i ], rows[ i ], columns[ i ] );

	return input;
}

/* reset_input_data
    reset given channel by new input data
*/
arfit_input *reset_input_data( const arfit_file_info *info, arfit_input *input, int slice, int row, int column, int channel )
{
	int sample, isNullImage = 1, nstoredindex = 0;

	/* check parameters */
	if( !info || !input || info->nsamples != input->v->size1 || slice < 0 || row < 0 || column < 0 || slice >= info->ntotalslices || row >= info->nrows || column >= info->ncolumns || channel < 0 || channel >= input->v->size2 )
		return input;

	/* check for a not null object slice */
	for( sample = 0; sample < info->nfunctionalslices; ++sample ) if( info->nFunctionalImageIndices[ sample ] == slice ) {
			isNullImage     = 0;
			nstoredindex    = sample;
		}

	/* iterate through every timestep in given voxel */
	for( sample = 0; sample < input->v->size1; ++sample ) {
		/* replace value in input matrix */
		if( isNullImage )
			gsl_matrix_set( input->v, sample, channel, 0.0 );
		else
			gsl_matrix_set( input->v, sample, channel, ( double ) VPixel( info->vFunctionalImages[ nstoredindex ], sample, row, column, VShort ) );
	}

	return input;
}

/* detach_sample_from_input function
    removes a sample from the given arfit_input structure
*/
arfit_input *detach_sample_from_input( arfit_input *input, int index )
{
	gsl_matrix  *new_input;
	int         rr = 0, cc = 0;

	/* check parameters */
	if( !input ) {
		VWarning( "Invalid input argument" );
		return input;
	}

	if( !input->v || index < 0 || index >= input->v->size1 ) {
		VWarning( "Wrong index" );
		return input;
	}

	/* check if only one data channel left */
	if( input->v->size2 == 1 ) {
		gsl_matrix_free( input->v );
		input->v = NULL;
	} else {
		/* allocate memory for new data matrix */
		new_input = gsl_matrix_alloc( input->v->size1, input->v->size2 - 1 );

		/* copy data to new matrix */
		for( rr = 0; rr < input->v->size1; ++rr ) for( cc = 0; cc < index; ++cc )
				gsl_matrix_set( new_input, rr, cc, gsl_matrix_get( input->v, rr, cc ) );

		for( rr = 0; rr < input->v->size1; ++rr ) for( cc = index + 1; cc < input->v->size2; ++cc )
				gsl_matrix_set( new_input, rr - 1, cc, gsl_matrix_get( input->v, rr, cc ) );

		/* free old input matrix */
		gsl_matrix_free( input->v );

		/* set new data matrix to input */
		input->v = new_input;
	}

	return input;
}


/* clear_input function
    clears all time series data from given arfit_info structure
*/
arfit_input *clear_input( arfit_input *input )
{
	/* check for valid parameter */
	if( input ) {
		/* free data matrix and set it to NULL */
		gsl_matrix_free( input->v );
		input->v = NULL;
	}

	return input;
}
