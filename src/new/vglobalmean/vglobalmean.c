/*
** compute global mean
**
**
** G.Lohmann, Aug 2007
*/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/headerinfo.h>

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



#define NSLICES 2500   /* max number of image slices */
#define MBETA    120   /* max number of covariates */

#define SQR(x) ((x)*(x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


gsl_vector_float *
VGlobalMean( VAttrList list, VImage mask, VShort minval )
{
	VAttrListPosn posn;
	int nslices = 0, nbands = 0, nrows = 0, ncols = 0, slice, row, col;
	VImage xsrc, src[NSLICES];
	int   i, k, m = 0;
	float x, sum = 0, nx = 0, mean = 0;
	float sum1, sum2, sigma, *ptr;
	gsl_vector_float *z;

	gsl_set_error_handler_off();


	/* get image dimensions */
	m = k = nbands = nrows = ncols = nslices = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & xsrc );

		if ( VPixelRepn( xsrc ) != VShortRepn ) continue;

		if ( VImageNBands( xsrc ) > nbands ) nbands = VImageNBands( xsrc );

		if ( VImageNRows( xsrc ) > nrows ) nrows = VImageNRows( xsrc );

		if ( VImageNColumns( xsrc ) > ncols ) ncols = VImageNColumns( xsrc );

		src[k] = xsrc;

		if ( k >= NSLICES ) VError( " too many slices, max is %d", NSLICES );

		k++;
	}

	nslices = k;
	m = nbands;

	z = gsl_vector_float_calloc ( m );

	for ( i = 0; i < m; i++ ) {

		sum = nx = 0;

		for ( slice = 0; slice < nslices; slice++ ) {
			if ( VImageNRows( src[slice] ) < 2 ) continue;

			for ( row = 0; row < nrows; row++ ) {
				for ( col = 0; col < ncols; col++ ) {
					if ( VPixel( src[slice], 0, row, col, VShort ) < minval ) continue;

					if ( mask && VPixel( mask, slice, row, col, VBit ) < 1 ) continue;

					x = VPixel( src[slice], i, row, col, VShort );
					sum  += x;
					nx++;
				}
			}
		}

		mean = sum / nx;
		gsl_vector_float_set( z, i, mean );
	}


	/* normalize */
	sum1 = sum2 = nx = 0;
	ptr = z->data;

	for ( i = 0; i < m; i++ ) {
		x = *ptr++;
		sum1 += x;
		sum2 += x * x;
		nx++;
	}

	mean = sum1 / nx;
	sigma = sqrt( ( double )( ( sum2 - nx * mean * mean ) / ( nx - 1.0 ) ) );

	ptr = z->data;

	for ( i = 0; i < m; i++ ) {
		x = *ptr;
		*ptr++ = ( x - mean ) / sigma;
	}

	return z;
}





int
main ( int argc, char *argv[] )
{
	static VString filename = "";
	static VShort  minval = 2000;
	static VOptionDescRec  options[] = {
		{"mask", VStringRepn, 1, ( VPointer ) &filename, VOptionalOpt, NULL, "mask"},
		{"minval", VShortRepn, 1, ( VPointer ) &minval, VOptionalOpt, NULL, "Signal threshold"}
	};
	FILE *in_file = NULL, *fp = NULL;
	VAttrList list = NULL, list1 = NULL;
	VAttrListPosn posn;
	VImage mask = NULL;
	int  i;
	gsl_vector_float *gmean = NULL;

	VParseFilterCmd( VNumber( options ), options, argc, argv, &in_file, NULL );


	/* mask image */
	if ( strlen( filename ) > 2 ) {
		fp = VOpenInputFile ( filename, TRUE );
		list1 = VReadFile ( fp, NULL );

		if ( ! list1 )  VError( "Error reading image" );

		fclose( fp );

		for ( VFirstAttr ( list1, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
			if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

			VGetAttrValue ( & posn, NULL, VImageRepn, & mask );

			if ( VPixelRepn( mask ) != VBitRepn ) continue;

			break;
		}
	}


	/* read data   */
	if ( ! ( list = VReadFile ( in_file, NULL ) ) ) exit ( 1 );

	fclose( in_file );

	gmean = VGlobalMean( list, mask, minval );

	for ( i = 0; i < gmean->size; i++ )
		fprintf( stderr, " %10.6f\n", gsl_vector_float_get( gmean, i ) );

	exit( 0 );
}


