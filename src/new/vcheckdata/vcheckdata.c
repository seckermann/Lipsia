/*
** check statistical parameters of of fMRI time series
**
** G.Lohmann, MPI-CBS, Oct 2010
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <fftw3.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define NSLICES 2000


extern void gsl_sort_vector( gsl_vector * );

VDictEntry TypeDict[] = {
	{ "mean", 0 },
	{ "std", 1 },
	{ "percent", 2 },
	{ "skewness", 3 },
	{ "kurtosis", 4 },
	{ "min", 5 },
	{ "max", 6 },
	{ "median", 7 },
	{ NULL }
};


void
HighPassFilter( double *z, int n, float tr, VFloat high )
{
	int i, nc;
	static double *in = NULL;
	static double *highp = NULL;
	static fftw_complex *out = NULL;
	double sharp = 0.8, x, alpha;
	fftw_plan p1, p2;

	/* repetition time */
	nc  = ( n / 2 ) + 1;

	if ( out == NULL ) {
		in  = ( double * ) VCalloc( n, sizeof( double ) );
		out = fftw_malloc ( sizeof ( fftw_complex ) * nc );
	}

	for ( i = 0; i < n; i++ ) in[i] = z[i];

	/* make plans */
	p1 = fftw_plan_dft_r2c_1d ( n, in, out, FFTW_ESTIMATE );
	p2 = fftw_plan_dft_c2r_1d ( n, out, in, FFTW_ESTIMATE );

	alpha = ( double )n * tr;

	if ( highp == NULL ) highp = ( double * ) VCalloc( nc, sizeof( double ) );

	sharp = 0.8;

	for ( i = 1; i < nc; i++ ) {
		highp[i] = 1.0 / ( 1.0 +  exp( ( alpha / high - ( double )i ) * sharp ) );
	}

	/* forward fft */
	fftw_execute( p1 );

	/* highpass */
	for ( i = 1; i < nc; i++ ) {
		x = highp[i];
		out[i][0] *= x;
		out[i][1] *= x;
	}

	/* inverse fft */
	fftw_execute( p2 );

	for ( i = 0; i < n; i++ ) z[i] = in[i] / ( double )n;
}



VAttrList
VCheckScanner( VAttrList list, int type, VFloat high )
{
	VAttrList out_list = NULL;
	VImage dest = NULL;
	VImage src[NSLICES];
	VString str;
	VAttrListPosn posn;
	int b, r, c, i, j;
	int nslices, nrows, ncols, ntimesteps, n = 0;
	double u = 0, v = 0;
	float tr = 0;
	gsl_vector *vec = NULL;

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

	dest = VCreateImage( nslices, nrows, ncols, VFloatRepn );
	VFillImage( dest, VAllBands, 0 );
	VCopyImageAttrs ( src[0], dest );
	VSetAttr( VImageAttrList( dest ), "modality", NULL, VStringRepn, "conimg" );
	VSetAttr( VImageAttrList( dest ), "patient", NULL, VStringRepn, "stability_check" );

	/* Get TR */
	if ( tr == 0 ) {
		if ( VGetAttr( VImageAttrList( src[0] ), "repetition_time", NULL, VFloatRepn, &tr ) == VAttrFound )
			tr /= 1000.0;
		else {
			if ( VGetAttr ( VImageAttrList ( src[0] ), "MPIL_vista_0", NULL, VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
				sscanf( str, " repetition_time=%f", &tr );
				tr /= 1000.0;
			} else
				VError( " TR unknown" );
		}
	}


	/* alloc vector */
	vec = gsl_vector_calloc( ntimesteps );
	n = ntimesteps;

	/* for each voxel... */
	for ( b = 0; b < nslices; b++ ) {
		if ( VImageNRows( src[b] ) < 2 ) continue;

		if ( b % 5 == 0 ) fprintf( stderr, " slice: %5d\r", b );

		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {

				for ( j = 0; j < ntimesteps; j++ ) {
					u = VPixel( src[b], j, r, c, VShort );
					gsl_vector_set( vec, j, u );
				}

				HighPassFilter( vec->data, ntimesteps, tr, high );

				switch ( type ) {

				case 0:
					u = gsl_stats_mean( vec->data, vec->stride, n );
					break;

				case 1:
					u = gsl_stats_sd( vec->data, vec->stride, n );
					break;

				case 2:
					u = gsl_stats_mean( vec->data, vec->stride, n );
					v = gsl_stats_sd( vec->data, vec->stride, n );

					if ( u != 0 )
						u = 100.0 * v / u;
					else
						u = 0;

					break;

				case 3:
					u = gsl_stats_skew( vec->data, vec->stride, n );
					break;

				case 4:
					u = gsl_stats_kurtosis( vec->data, vec->stride, n );
					break;

				case 5:
					u = gsl_stats_min( vec->data, vec->stride, n );
					break;

				case 6:
					u = gsl_stats_max( vec->data, vec->stride, n );
					break;

				case 7:
					gsl_sort_vector( vec );
					u = gsl_stats_median_from_sorted_data( vec->data, vec->stride, n );
					break;

				default:
					VError( " illegal type %d", type );
				}

				VPixel( dest, b, r, c, VFloat ) = u;
			}
		}
	}

	out_list = VCreateAttrList();
	VAppendAttr( out_list, ( VString )TypeDict[type].keyword, NULL, VImageRepn, dest );
	return out_list;
}


int
main( int argc, char *argv[] )
{
	static VShort type = 0;
	static VFloat high = 60;
	static VOptionDescRec  options[] = {
		{"type", VShortRepn, 1, ( VPointer ) &type, VOptionalOpt, TypeDict, "output type"},
		{"high", VFloatRepn, 1, ( VPointer ) &high, VOptionalOpt, NULL, "upper frequency theshold in seconds for detrending"}
	};
	FILE *in_file = NULL, *out_file = NULL;
	VAttrList list = NULL, out_list = NULL;
	char *prg = "vcheckdata";

	VParseFilterCmd( VNumber( options ), options, argc, argv, &in_file, &out_file );

	if ( type > 7 ) VError( " illegal type" );

	if ( ! ( list = VReadFile ( in_file, NULL ) ) ) exit ( 1 );

	fclose( in_file );


	fprintf( stderr, " computing %s...\n", ( VString )TypeDict[type].keyword );
	out_list = VCheckScanner( list, ( int )type, high );

	VHistory( VNumber( options ), options, prg, &list, &list );

	if ( ! VWriteFile ( out_file, out_list ) ) exit ( 1 );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	exit( 0 );
}
