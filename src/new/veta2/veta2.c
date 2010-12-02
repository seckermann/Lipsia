/*
** Eta^2
** Cohen et al (2008) Neuroimage 41(1):45-57
**
** G.Lohmann, Sept 2010
*/
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NSLICES 2000
#define TINY 1.0e-6


#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


double
Correlation( const float *arr1, const float *arr2, int n )
{
	int i;
	double sx, sy, sxx, syy, sxy, rx;
	double tiny = TINY;

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


VAttrList
VEta2( VAttrList list, VImage mask, VShort minval, VShort first, VShort length )
{
	VAttrList out_list = NULL;
	VAttrListPosn posn;
	VImage src[NSLICES], map = NULL;
	VImage dest = NULL;
	size_t b, r, c, i, j, k, i1, n, nt, last, ntimesteps, nrows, ncols, nslices;
	gsl_matrix_float *mat = NULL;
	double *corr1 = NULL, *corr2 = NULL;
	double eta2, ax, bx, wx, mx, nx, sum1, sum2;
	double tiny = TINY;

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


	/* get time steps to include */
	if ( length < 1 ) length = ntimesteps - 2;

	last = first + length - 1;

	if ( last >= ntimesteps ) last = ntimesteps - 1;

	if ( first < 0 ) first = 1;

	nt = last - first + 1;
	i1 = first + 1;

	if ( nt < 2 ) VError( " not enough timesteps, nt= %d", nt );

	fprintf( stderr, "# ntimesteps: %d, first= %d, last= %d, nt= %d\n",
			 ( int )ntimesteps, ( int )first, ( int )last, ( int )nt );


	/* count number of voxels */
	n = 0;

	for ( b = 0; b < nslices; b++ ) {
		if ( VImageNRows( src[b] ) < 2 ) continue;

		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				if ( VPixel( src[b], i1, r, c, VShort ) < minval ) continue;

				if ( VGetPixel( mask, b, r, c ) < 0.5 ) continue;

				n++;
			}
		}
	}

	fprintf( stderr, " nvoxels: %ld\n", ( long )n );



	/*
	** voxel addresses
	*/
	map = VCreateImage( 1, 5, n, VFloatRepn );

	if ( map == NULL ) VError( " error allocating addr map" );

	VFillImage( map, VAllBands, 0 );
	VPixel( map, 0, 3, 0, VFloat ) = nslices;
	VPixel( map, 0, 3, 1, VFloat ) = nrows;
	VPixel( map, 0, 3, 2, VFloat ) = ncols;

	i = 0;

	for ( b = 0; b < nslices; b++ ) {
		if ( VImageNRows( src[b] ) < 2 ) continue;

		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				if ( VPixel( src[b], i1, r, c, VShort ) < minval ) continue;

				if ( VGetPixel( mask, b, r, c ) < 0.5 ) continue;

				VPixel( map, 0, 0, i, VFloat ) = b;
				VPixel( map, 0, 1, i, VFloat ) = r;
				VPixel( map, 0, 2, i, VFloat ) = c;
				i++;
			}
		}
	}


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

		for ( k = first; k <= last; k++ ) {
			if ( k >= VImageNBands( src[b] ) ) VError( " k= %d %d", k, VImageNBands( src[b] ) );

			*ptr++ = ( float ) VPixel( src[b], k, r, c, VShort );
			j++;
		}

		if ( j >= mat->size2 ) VError( " j= %d %d", j, mat->size2 );
	}

	/*
	** alloc dest image
	*/
	dest = VCreateImage( 1, n, n, VFloatRepn );

	if ( !dest ) VError( " error allocating dest image" );

	VFillImage( dest, VAllBands, 0 );
	VCopyImageAttrs ( src[0], dest );
	VSetAttr( VImageAttrList( dest ), "modality", NULL, VStringRepn, "conimg" );
	VSetAttr( VImageAttrList( dest ), "name", NULL, VStringRepn, "eta_squared" );
	nx = ( double )n;
	sum1 = ( nx * nx * 4 ) / ( 1024 * 1024 );
	fprintf( stderr, " output file size: %.2f MByte\n", sum1 );


	/*
	** compute eta^2 in each voxel
	*/
	corr1 = ( double * ) VCalloc( n, sizeof( double ) );
	corr2 = ( double * ) VCalloc( n, sizeof( double ) );


	for ( i = 0; i < n; i++ ) {
		fprintf( stderr, " %5d\r", ( int )i );

		/* correlation map in voxel i */
		const float *arr1 = gsl_matrix_float_const_ptr( mat, i, 0 );

		for ( k = 0; k < n; k++ ) {
			const float *arr2 = gsl_matrix_float_const_ptr( mat, k, 0 );
			const double u = Correlation( arr1, arr2, nt );
			corr1[k] = u;
		}

		/* correlation map in voxel j */
		for ( j = 0; j < n; j++ ) {
			const float *arr3 = gsl_matrix_float_const_ptr( mat, j, 0 );

			for ( k = 0; k < n; k++ ) {
				const float *arr4 = gsl_matrix_float_const_ptr( mat, k, 0 );
				const double v = Correlation( arr3, arr4, nt );
				corr2[k] = v;
			}

			/* grand mean over both maps */
			mx = 0;

			for ( k = 0; k < n; k++ ) mx += ( corr1[k] + corr2[k] );

			nx = ( double )( n * 2 );
			mx /= nx;

			/* eta^2 */
			sum1 = sum2 = 0;

			for ( k = 0; k < n; k++ ) {
				ax = corr1[k];
				bx = corr2[k];
				wx = 0.5 * ( ax + bx );
				sum1 += SQR( ax - wx ) + SQR( bx - wx );
				sum2 += SQR( ax - mx ) + SQR( bx - mx );
			}

			eta2 = 0;

			if ( sum2 > tiny ) eta2 = 1.0 - sum1 / sum2;

			if ( isnan( eta2 ) || isinf( eta2 ) ) eta2 = 0;

			VPixel( dest, 0, i, j, VFloat ) = ( VFloat )eta2;
		}
	}

	gsl_matrix_float_free( mat );


	/* output */
	VSetAttr( VImageAttrList( dest ), "map", NULL, VImageRepn, map );
	out_list = VCreateAttrList();
	VAppendAttr( out_list, "image", NULL, VImageRepn, dest );
	return out_list;
}



int
main ( int argc, char *argv[] )
{
	static VString  filename = "";
	static VShort   first  = 2;
	static VShort   length = 0;
	static VShort   minval = 0;
	static VOptionDescRec  options[] = {
		{"mask", VStringRepn, 1, ( VPointer ) &filename, VRequiredOpt, NULL, "mask defining a region of interest"},
		{"minval", VShortRepn, 1, ( VPointer ) &minval, VOptionalOpt, NULL, "signal threshold"},
		{"first", VShortRepn, 1, ( VPointer ) &first, VOptionalOpt, NULL, "first timestep to use"},
		{"length", VShortRepn, 1, ( VPointer ) &length, VOptionalOpt, NULL, "length of time series to use, '0' to use full length"}
	};
	FILE *in_file, *out_file, *fp;
	VAttrList list = NULL, list1 = NULL, out_list = NULL;
	VAttrListPosn posn;
	VImage mask = NULL;
	char *prg = "veta2";

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

		if ( VPixelRepn( mask ) != VBitRepn && VPixelRepn( mask ) != VUByteRepn && VPixelRepn( mask ) != VShortRepn ) {
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
	out_list = VEta2( list, mask, minval, first, length );
	VHistory( VNumber( options ), options, prg, &list, &out_list );

	if ( ! VWriteFile ( out_file, out_list ) ) exit ( 1 );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return 0;
}
