/****************************************************************
 *
 * Program: vgetcontrast
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: vgetcontrast.c 3581 2009-06-04 07:33:10Z proeger $
 *
 *****************************************************************/


#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include "gsl_utils.h"


#define MBETA 64
#define ABS(x) ((x) > 0 ? (x) : -(x))

extern double t2z( double, double );
extern float t2z_approx( float, float );
extern char *getLipsiaVersion();


VAttrList
VGetContrast( VAttrList list, gsl_vector_float *con, VShort type )
{
	VAttrList out_list;
	int nbands = 0, nrows = 0, ncols = 0, band, row, col;
	VImage src = NULL, dest = NULL, std_image = NULL;
	VImage beta_images[MBETA], res_image = NULL, bcov_image = NULL;
	VString buf = NULL;
	VAttrListPosn posn;
	VString str;
	int    i, nbeta;
	float  t = 0, s = 0, tsigma = 0, z = 0, zmax = 0, zmin = 0;
	float  sigma, var, sum, df;
	float  *ptr1, *ptr2;
	char *constring = NULL;
	gsl_vector_float *beta = NULL, *tmp = NULL;
	gsl_matrix_float *bcov = NULL;


	i = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) != VFloatRepn ) continue;

		VGetAttr( VImageAttrList( src ), "modality", NULL, VStringRepn, &str );

		if ( strcmp( str, "BETA" ) == 0 ) {
			beta_images[i++] = VCopyImage( src, NULL, VAllBands );
		} else if ( strcmp( str, "RES/trRV" ) == 0 ) {
			res_image = VCopyImage( src, NULL, VAllBands );
		} else if ( strcmp( str, "BCOV" ) == 0 ) {
			bcov_image = VCopyImage( src, NULL, VAllBands );
		}
	}

	nbeta  = VImageNRows( bcov_image );
	nbands = VImageNBands( beta_images[0] );
	nrows  = VImageNRows( beta_images[0] );
	ncols  = VImageNColumns( beta_images[0] );

	if ( VGetAttr( VImageAttrList( beta_images[0] ), "df", NULL, VFloatRepn, &df ) != VAttrFound )
		VError( " attribute 'df' not found" );

	if ( nbeta > MBETA ) {
		fprintf( stderr, " number of betas: %d,  maximum number of betas: %d\n", nbeta, MBETA );
		VError( " maximum number of betas is exceeded" );
	}

	/*
	** read contrast vector
	*/
	if ( nbeta != con->size )
		VError( "contrast vector has bad length (%d), correct length is %d", con->size, nbeta );

	fprintf( stderr, " contrast vector:\n" );
	char str1[10];
	constring = ( char * )VMalloc( sizeof( char ) * 10 * nbeta );
	constring[0] = '\0';

	for ( i = 0; i < nbeta; i++ ) {
		fprintf( stderr, "  %.2f", fvget( con, i ) );
		sprintf( str1, "%1.2f ", fvget( con, i ) );
		strcat( ( char * )constring, ( const char * )str1 );
	}

	fprintf( stderr, "\n" );



	/* get variance estimation */
	bcov = gsl_matrix_float_alloc ( nbeta, nbeta );
	ptr1 = VImageData( bcov_image );
	ptr2 = bcov->data;

	for ( i = 0; i < nbeta * nbeta; i++ ) *ptr2++ = *ptr1++;

	gsl_matrix_float_transpose( bcov );
	tmp   = fmat_x_vector( bcov, con, tmp );
	var   = fskalarproduct( tmp, con );
	sigma = sqrt( var );


	/*
	** create output data structs
	*/
	out_list = VCreateAttrList();
	dest = VCreateImage( nbands, nrows, ncols, VFloatRepn );
	VFillImage( dest, VAllBands, 0 );
	VCopyImageAttrs( beta_images[0], dest );


	switch( type ) {
	case 0:    /* conimg  */
		buf = VNewString( "conimg" );
		break;
	case 1:    /* t-image */
		buf = VNewString( "tmap" );
		break;
	case 2:    /* zmap    */
		buf = VNewString( "zmap" );
		break;
	default:
		VError( " illegal type" );
	}

	fprintf( stderr, " output type: %s\n", buf );

	VSetAttr( VImageAttrList( dest ), "modality", NULL, VStringRepn, buf );
	VSetAttr( VImageAttrList( dest ), "name", NULL, VStringRepn, buf );
	VSetAttr( VImageAttrList( dest ), "contrast", NULL, VStringRepn, constring );
	VAppendAttr ( out_list, "image", NULL, VImageRepn, dest );

	if ( type == 0 ) {
		std_image = VCreateImage( nbands, nrows, ncols, VFloatRepn );
		VFillImage( std_image, VAllBands, 0 );
		VCopyImageAttrs( beta_images[0], std_image );
		VSetAttr( VImageAttrList( std_image ), "modality", NULL, VStringRepn, "std_dev" );
		VSetAttr( VImageAttrList( std_image ), "name", NULL, VStringRepn, "std_dev" );
		VAppendAttr ( out_list, "image", NULL, VImageRepn, std_image );
	}



	/*
	** loop thru image
	*/
	zmax = zmin = 0;
	beta = gsl_vector_float_alloc ( nbeta );

	for ( band = 0; band < nbands; band++ ) {
		for ( row = 0; row < nrows; row++ ) {
			for ( col = 0; col < ncols; col++ ) {
				t = z = sum = 0;

				ptr1 = beta->data;

				for ( i = 0; i < nbeta; i++ ) {
					*ptr1++ = VPixel( beta_images[i], band, row, col, VFloat );
				}

				sum  = fskalarproduct( beta, con );

				if ( ABS( sum ) < 1.0e-10 ) continue;

				s = VPixel( res_image, band, row, col, VFloat );
				tsigma = sqrt( s ) * sigma;

				if ( tsigma > 0.00001 ) t = sum / tsigma;
				else t = 0;

				if ( isnan( t ) || isinf( t ) ) t = 0;

				switch( type ) {
				case 0:    /* conimg  */
					z = sum;
					break;
				case 1:    /* t-image */
					z = t;
					break;
				case 2:    /* zmap    */
					z = t2z_approx( t, df );

					if ( z > 30 )  z = 30;

					if ( sum < 0 ) z = -z;

					break;
				default:
					;
				}

				if ( isnan( z ) || isinf( z ) ) z = 0;

				if ( z > zmax ) zmax = z;

				if ( z < zmin ) zmin = z;

				VPixel( dest, band, row, col, VFloat ) = z;

				if ( type == 0 ) VPixel( std_image, band, row, col, VFloat ) = tsigma;
			}
		}
	}

	fprintf( stderr, " min= %.3f,  max= %.3f\n", zmin, zmax );
	return out_list;
}


VDictEntry TYPDict[] = {
	{ "conimg", 0 },
	{ "tmap", 1 },
	{ "zmap", 2 },
	{ NULL }
};

int
main ( int argc, char *argv[] )
{
	static VShort type = 0;
	static VArgVector contrast;
	static VOptionDescRec  options[] = {
		{"type", VShortRepn, 1, ( VPointer ) &type, VOptionalOpt, TYPDict, "type of output"},
		{"contrast", VFloatRepn, 0, ( VPointer ) &contrast, VRequiredOpt, NULL, "contrast vector"}
	};
	FILE *in_file, *out_file;
	VAttrList list = NULL, out_list = NULL;
	gsl_vector_float *cont;
	float u;
	int i;
	char prg[50];
	sprintf( prg, "vgetcontrast V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg );

	VParseFilterCmd( VNumber( options ), options, argc, argv, &in_file, &out_file );
	fprintf( stderr, " %s\n", prg );


	cont = gsl_vector_float_alloc( contrast.number );

	for ( i = 0; i < contrast.number; i++ ) {
		u = ( ( VFloat * )contrast.vector )[i];
		fvset( cont, i, u );
	}

	if ( ! ( list = VReadFile ( in_file, NULL ) ) ) exit ( 1 );

	fclose( in_file );

	out_list = VGetContrast( list, cont, type );


	/* Output: */
	VHistory( VNumber( options ), options, prg, &list, &out_list );

	if ( ! VWriteFile ( out_file, out_list ) ) exit ( 1 );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return 0;
}
