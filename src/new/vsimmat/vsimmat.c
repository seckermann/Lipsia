/*
** get similarity matrix of time courses.
** serves as input into 'vspectralcluster'
**
** G.Lohmann, 2008
*/
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>

#define NSLICES 2000


double
Correlation( const float *arr1, const float *arr2, int n )
{
	int i;
	double sx, sy, sxx, syy, sxy, rx;
	double tiny = 1.0e-4;

	sxx = syy = sxy = sx = sy = 0;

	for ( i = 0; i < n; i++ ) {
		const double u = ( double )arr1[i];
		const double v = ( double )arr2[i];
		sx  += u;
		sy  += v;
		sxx += u * u;
		syy += v * v;
		sxy += u * v;
	}

	const double nx = n;

	const double u = nx * sxx - sx * sx;

	const double v = nx * syy - sy * sy;

	rx = 0;

	if ( u * v > tiny )
		rx = ( nx * sxy - sx * sy ) / sqrt( u * v );

	return rx;
}


VImage
SimilarityMatrix( VAttrList list, VImage mask, VShort minval, VShort fisher )
{
	VAttrListPosn posn;
	VImage src[NSLICES], dest = NULL, map = NULL;
	int b, r, c, i, j, n, ntimesteps, nrows, ncols, nslices;
	double u, x, y, smin, smax, tiny = 1.0e-6;
	gsl_matrix_float *mat = NULL;

	/*
	** get image dimensions
	*/
	i = ntimesteps = nrows = ncols = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src[i] );

		if ( VPixelRepn( src[i] ) != VShortRepn ) continue;

		if ( VImageNBands( src[i] ) > ntimesteps ) ntimesteps = VImageNBands( src[i] );

		if ( VImageNRows( src[i] )  > nrows )  nrows = VImageNRows( src[i] );

		if ( VImageNColumns( src[i] ) > ncols ) ncols = VImageNColumns( src[i] );

		i++;

		if ( i >= NSLICES ) VError( " too many slices" );
	}

	nslices = i;

	n = 0;

	for ( b = 0; b < nslices; b++ ) {
		if ( VImageNRows( src[b] ) < 2 ) continue;

		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				if ( VPixel( src[b], 0, r, c, VShort ) < minval ) continue;

				if ( VGetPixel( mask, b, r, c ) < 0.1 ) continue;

				n++;
			}
		}
	}

	fprintf( stderr, " n: %d\n", n );


	/*
	** voxel addresses
	*/
	map = VCreateImage( 1, 5, n, VFloatRepn );
	VFillImage( map, VAllBands, 0 );
	VPixel( map, 0, 3, 0, VFloat ) = nslices;
	VPixel( map, 0, 3, 1, VFloat ) = nrows;
	VPixel( map, 0, 3, 2, VFloat ) = ncols;

	i = 0;

	for ( b = 0; b < nslices; b++ ) {
		if ( VImageNRows( src[b] ) < 2 ) continue;

		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				if ( VPixel( src[b], 0, r, c, VShort ) < minval ) continue;

				if ( VGetPixel( mask, b, r, c ) < 0.1 ) continue;

				VPixel( map, 0, 0, i, VFloat ) = b;
				VPixel( map, 0, 1, i, VFloat ) = r;
				VPixel( map, 0, 2, i, VFloat ) = c;
				i++;
			}
		}
	}


	/* ini output image */
	if ( n < 8000 )
		dest = VCreateImage( 1, n, n, VFloatRepn );
	else if ( n >= 8000 && n < 10000 )
		dest = VCreateImage( 1, n, n, VShortRepn );
	else
		dest = VCreateImage( 1, n, n, VSByteRepn );

	VFillImage( dest, VAllBands, 0 );
	smin = VPixelMinValue( dest );
	smax = VPixelMaxValue( dest );


	/*
	** avoid casting to float, copy data to matrix
	*/
	mat = gsl_matrix_float_calloc( n, ntimesteps );

	for ( i = 0; i < n; i++ ) {

		b = VPixel( map, 0, 0, i, VFloat );
		r = VPixel( map, 0, 1, i, VFloat );
		c = VPixel( map, 0, 2, i, VFloat );

		float *ptr = gsl_matrix_float_ptr( mat, i, 0 );
		int k;
		j = 0;

		for ( k = 0; k < ntimesteps; k++ ) {
			if ( j >= mat->size2 ) VError( " j= %d %d", j, mat->size2 );

			if ( k >= VImageNBands( src[b] ) ) VError( " k= %d %d", k, VImageNBands( src[b] ) );

			*ptr++ = ( float ) VPixel( src[b], k, r, c, VShort );
			j++;
		}
	}


	/*
	** main process loop
	*/
	for ( i = 0; i < n; i++ ) {
		if ( i % 50 == 0 ) fprintf( stderr, " i: %4d\r", i );

		const float *arr1 = gsl_matrix_float_const_ptr( mat, i, 0 );

		for ( j = 0; j <= i; j++ ) {

			const float *arr2 = gsl_matrix_float_const_ptr( mat, j, 0 );
			u = Correlation( arr1, arr2, ntimesteps );

			/* Fisher r-to-z transform */
			if ( fisher ) {
				x = 1 + u;
				y = 1 - u;

				if ( x < tiny ) x = tiny;

				if ( y < tiny ) y = tiny;

				u = 0.5 * log( x / y );
			}

			if ( VPixelRepn( dest ) != VFloatRepn ) {
				u *= smax;

				if ( u < smin ) u = smin;

				if ( u > smax ) u = smax;
			}

			VSetPixel( dest, 0, i, j, u );
			VSetPixel( dest, 0, j, i, u );
		}
	}


	VCopyImageAttrs ( src[0], dest );
	VSetAttr( VImageAttrList( dest ), "similarity_metric", NULL, VStringRepn, "corr_pearson" );
	VSetAttr( VImageAttrList( dest ), "name", NULL, VStringRepn, "similarity_matrix" );
	VSetAttr( VImageAttrList( dest ), "map", NULL, VImageRepn, map );
	return dest;
}



int
main ( int argc, char *argv[] )
{
	static VString filename = "";
	static VBoolean fisher = FALSE;
	static VShort  minval = 0;
	static VOptionDescRec  options[] = {
		{"mask", VStringRepn, 1, ( VPointer ) &filename, VRequiredOpt, NULL, "mask"},
		{"fisher", VBooleanRepn, 1, ( VPointer ) &fisher, VOptionalOpt, NULL, "Whether to do fisher transform"},
		{"minval", VShortRepn, 1, ( VPointer ) &minval, VOptionalOpt, NULL, "signal threshold"}
	};
	FILE *in_file, *out_file, *fp;
	VAttrList list = NULL, list1 = NULL, list2 = NULL;
	VAttrListPosn posn;
	VImage dest = NULL, mask = NULL;
	char *prg = "vsimmat";

	VParseFilterCmd ( VNumber ( options ), options, argc, argv, &in_file, &out_file );

	/*
	** read mask
	*/
	fp = VOpenInputFile ( filename, TRUE );
	list1 = VReadFile ( fp, NULL );

	if ( ! list1 ) VError( "Error reading mask file" );

	fclose( fp );

	for ( VFirstAttr ( list1, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & mask );

		if ( VPixelRepn( mask ) != VBitRepn ) {
			mask = NULL;
			continue;
		}
	}

	if ( mask == NULL ) VError( " no mask found" );


	/*
	** read functional data
	*/
	if ( ! ( list = VReadFile ( in_file, NULL ) ) ) exit ( 1 );

	fclose( in_file );


	/*
	** process
	*/
	dest = SimilarityMatrix( list, mask, minval, fisher );


	list2 = VCreateAttrList();
	VAppendAttr( list2, "matrix", NULL, VImageRepn, dest );
	VHistory( VNumber( options ), options, prg, &list, &list2 );

	if ( ! VWriteFile ( out_file, list2 ) ) exit ( 1 );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return 0;
}
