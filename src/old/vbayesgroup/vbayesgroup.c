/****************************************************************
 *
 * vbayesgroup: fixed effects 3nd level bayes
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Authors: Gabriele Lohmann,Jane Neumann  2004, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: vbayesgroup.c 3520 2009-03-26 15:04:41Z lohmann $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_integration.h>

#define N 200 /* max number of input images */

#define ABS(x) ((x) > 0 ? (x) : -(x))

extern double p2z( double );
extern VImage VBayesGroup( VImage, VImage, VImage, VImage, VBoolean );
extern char *getLipsiaVersion();


struct my_params {
	double mean;
	double sigma;
};



int main ( int argc, char *argv[] )
{
	static VString filename1 = NULL;
	static VString filename2 = NULL;
	static VBoolean zscore = FALSE;
	static VOptionDescRec options[] = {
		{
			"group1", VStringRepn, 1, & filename1, VRequiredOpt, NULL,
			"File containing first group vaverage"
		},
		{
			"group2", VStringRepn, 1, & filename2, VRequiredOpt, NULL,
			"File containing second group vaverage"
		}
		/*    { "zscore", VBooleanRepn, 1, & zscore, VOptionalOpt, NULL,
		  "Whether to produce z-scores as output"}*/
	};

	FILE *f, *out_file;
	VAttrList list1, list2, out_list;
	VAttrListPosn posn;
	VImage src = NULL, dest = NULL;
	VImage mean1 = NULL, sigma1 = NULL;
	VImage mean2 = NULL, sigma2 = NULL;
	VString str = NULL;
	char prg_name[50];
	sprintf( prg_name, "vbayesgroup V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );

	/* Parse command line arguments and identify files: */
	VParseFilterCmd ( VNumber ( options ), options, argc, argv, NULL, &out_file );


	/*
	** read first input file
	*/
	f = fopen ( ( char * )filename1, "r" );

	if ( ! f ) VError ( "Failed to open input file %s", filename1 );

	if ( ! ( list1 = VReadFile ( f, NULL ) ) ) exit ( EXIT_FAILURE );

	fclose ( f );


	for ( VFirstAttr ( list1, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) != VFloatRepn ) continue;

		if ( VGetAttr( VImageAttrList( src ), "modality", NULL, VStringRepn, &str ) != VAttrFound )
			VError( " attr 'modality' not found" );

		if ( strcmp( str, "mean" ) == 0 || strcmp( str, "conimg" ) == 0 ) {
			mean1  = VCopyImage( src, NULL, VAllBands );
		} else if ( strcmp( str, "sigma" ) == 0 || strcmp( str, "std_dev" ) == 0 || strcmp( str, "sd" ) == 0 ) {
			sigma1 = VCopyImage( src, NULL, VAllBands );
		}
	}

	if ( mean1 == NULL || sigma1 == NULL ) VError( "Incorrect input image" );


	/*
	** read second input file
	*/
	f = fopen ( ( char * )filename2, "r" );

	if ( ! f ) VError ( "Failed to open input file %s", filename2 );

	if ( ! ( list2 = VReadFile ( f, NULL ) ) ) exit ( EXIT_FAILURE );

	fclose ( f );

	for ( VFirstAttr ( list2, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) != VFloatRepn ) continue;

		if ( VGetAttr( VImageAttrList( src ), "modality", NULL, VStringRepn, &str ) != VAttrFound )
			VError( " attr 'modality' not found" );

		if ( strcmp( str, "mean" ) == 0 || strcmp( str, "conimg" ) == 0 ) {
			mean2  = VCopyImage( src, NULL, VAllBands );
		} else if ( strcmp( str, "sigma" ) == 0 || strcmp( str, "std_dev" ) == 0 || strcmp( str, "sd" ) == 0 ) {
			sigma2 = VCopyImage( src, NULL, VAllBands );
		}
	}

	if ( mean2 == NULL || sigma2 == NULL ) VError( "Incorrect input image" );


	/* calculate probabilities */
	dest = VBayesGroup( mean1, sigma1, mean2, sigma2, zscore );

	if ( dest == NULL ) VError( " empty dest image" );


	/* Write the results to the output file: */
	out_list = VCreateAttrList ();
	VHistory( VNumber( options ), options, prg_name, &list1, &out_list );
	VAppendAttr ( out_list, "zmap", NULL, VImageRepn, dest );

	if ( ! VWriteFile ( out_file, out_list ) ) VError( " can't write output file" );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return 0;
}


/* Gaussian function */
double gauss( double x, void *p )
{
	struct my_params *par = p;
	double y, z, a = 2.506628273;
	double mean = 0, sigma = 0;

	mean  = par->mean;
	sigma = par->sigma;

	z = ( x - mean ) / sigma;
	y = exp( ( double ) - z * z * 0.5 ) / ( sigma * a );
	return y;
}


/* Integrate over distribution */
double CumulativeNormal( double mean, double sigma )
{
	double a = 0, b = 0;
	double y = 0, error = 0;
	static gsl_integration_workspace *work = NULL;
	struct my_params params;
	gsl_function F;

	F.function = &gauss;
	F.params = &params;
	params.mean = mean;
	params.sigma = sigma;

	if ( work == NULL )
		work = gsl_integration_workspace_alloc ( 1000 );

	if ( mean >= 0 ) {
		a = 0;
		b = mean + 5.0 * sigma;
	} else {
		a = mean - 5.0 * sigma;
		b = 0;
	}

	gsl_integration_qag ( &F, a, b, 0, 1e-7, 1000, GSL_INTEG_GAUSS15, work, &y, &error );
	return y;
}




VImage
VBayesGroup( VImage mean1, VImage sigma1, VImage mean2, VImage sigma2, VBoolean zscore )
{
	VImage dest = NULL;
	int    i, npixels;
	VFloat *dest_pp, *mean1_pp, *mean2_pp;
	VFloat *sigma1_pp, *sigma2_pp;
	float  tiny = 1.0e-12;
	VFloat pmin, pmax;
	float  result, s1, s2, var, m1, m2;
	double mean = 0, sigma = 0;

	/*
	** create output image
	*/
	dest = VCopyImage( mean1, NULL, VAllBands );

	if ( !dest ) return NULL;

	VFillImage( dest, VAllBands, 0 );
	VSetAttr( VImageAttrList( dest ), "modality", NULL, VStringRepn, "zmap" );
	VSetAttr( VImageAttrList( dest ), "name", NULL, VStringRepn, "bayesgroup" );


	/*
	** for each voxel
	*/
	pmax = VRepnMinValue( VFloatRepn );
	pmin = VRepnMaxValue( VFloatRepn );

	npixels = VImageNPixels( mean1 );

	mean1_pp = ( VFloat * ) VImageData( mean1 );
	mean2_pp = ( VFloat * ) VImageData( mean2 );

	sigma1_pp = ( VFloat * ) VImageData( sigma1 );
	sigma2_pp = ( VFloat * ) VImageData( sigma2 );

	dest_pp = ( VFloat * ) VImageData( dest );

	for ( i = 0; i < npixels; i++ ) {
		result = mean = sigma = 0;

		m1 = *mean1_pp++;
		m2 = *mean2_pp++;
		mean = m2 - m1;

		s1  = *sigma1_pp++;
		s2  = *sigma2_pp++;
		var = ( s1 * s1 + s2 * s2 );
		sigma = sqrt( ( double )var );

		if ( ABS( m1 ) < tiny || ABS( m2 ) < tiny ) goto next;

		if ( ABS( mean ) < tiny ) goto next;

		if ( sigma < tiny ) goto next;

		result = CumulativeNormal( mean, sigma );


		/* result */
		if ( zscore ) {
			/* CHECK HERE IF "1.0-" IS CORRECT */
			result = ( float )p2z( ( double )( 1.0 - result ) );

			if ( mean < 0 ) result = -result;

			if ( result >  20 ) result =  20;

			if ( result < -20 ) result = -20;
		} else {
			result *= 100.0;

			if ( result >  100 ) result =  100;

			if ( mean < 0 ) result = -result;
		}


		if ( result < pmin ) pmin = result;

		if ( result > pmax ) pmax = result;

next:
		*dest_pp++ = result;
	}

	fprintf( stderr, " min: %.3f, max: %.3f\n", pmin, pmax );

	return dest;
}

