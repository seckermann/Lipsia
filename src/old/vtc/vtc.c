/****************************************************************
 *
 * Program: vtc
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2007, <lipsia@cbs.mpg.de>
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
 * $Id: vtc.c 3191 2008-04-01 16:11:38Z karstenm $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include "gsl_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <via.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x)*(x))

#define NSLICES 256 /* max number of slices */

extern char *getLipsiaVersion();

typedef struct SpointStruct {
	VShort x;
	VShort y;
	VShort z;
} SPoint;


void
Normalize( gsl_matrix *mat2 )
{
	int i, j;
	double u, sum1, sum2, nx, ave, sigma;

	for ( i = 0; i < mat2->size1; i++ ) {
		sum1 = sum2 = nx = 0;

		for ( j = 0; j < mat2->size2; j++ ) {
			u = gsl_matrix_get( mat2, i, j );
			sum1 += u;
			sum2 += u * u;
			nx++;
			gsl_matrix_set( mat2, i, j, u );
		}

		ave  = sum1 / nx;
		sigma = sqrt( ( sum2 - nx * ave * ave ) / ( nx - 1.0 ) );

		if ( sigma < 0.0001 ) sigma = 1;

		for ( j = 0; j < mat2->size2; j++ ) {
			u = gsl_matrix_get( mat2, i, j );
			u = ( u - ave ) / sigma;
			gsl_matrix_set( mat2, i, j, u );
		}
	}
}



void
GetMean( gsl_matrix *mat1, gsl_matrix *mat2, int id )
{
	int i, j;
	double sum, mx;

	for ( j = 0; j < mat1->size2; j++ ) {
		sum = mx = 0;

		for ( i = 0; i < mat1->size1; i++ ) {
			sum += gsl_matrix_get( mat1, i, j );
			mx++;
		}

		if ( mx < 1 ) continue;

		gsl_matrix_set( mat2, id, j, sum / mx );
	}
}



void
GetPCA( gsl_matrix *mat1, gsl_matrix *mat2, int id )
{
	int i, j;
	double sum, mx, u, v, d1, d2, sign, *ptr1, *ptr2;
	gsl_vector *ev = NULL, *mean = NULL;
	gsl_vector *p1 = NULL, *p2 = NULL;
	gsl_matrix *E = NULL, *A = NULL, *B = NULL;
	extern gsl_vector *GComputeMean( gsl_matrix *, gsl_vector * );
	extern gsl_matrix *GSubMean( gsl_matrix *, gsl_vector *, gsl_matrix * );
	extern gsl_matrix *GPCA_SVD( gsl_matrix *, gsl_matrix *, gsl_vector * );
	extern gsl_matrix *dmatT_x_mat( gsl_matrix *, gsl_matrix *, gsl_matrix * );

	gsl_set_error_handler_off();

	/* get mean */
	p2 = gsl_vector_calloc( mat1->size2 );

	for ( j = 0; j < mat1->size2; j++ ) {
		sum = mx = 0;

		for ( i = 0; i < mat1->size1; i++ ) {
			sum += gsl_matrix_get( mat1, i, j );
			mx++;
		}

		if ( mx < 1 ) continue;

		gsl_vector_set( p2, j, sum / mx );
	}


	/* SVD */
	A    = mat1;
	ev   = gsl_vector_calloc( A->size1 );
	mean = GComputeMean( A, NULL ); /* get mean */
	B    = GSubMean( A, mean, NULL ); /* centering */
	E    = GPCA_SVD( B, NULL, ev );
	A    = dmatT_x_mat( E, B, A );

	/* get first principal component */
	p1 = gsl_vector_calloc( A->size2 );

	for ( j = 0; j < A->size2; j++ ) {
		u = gsl_matrix_get( A, 0, j );
		gsl_vector_set( p1, j, u );
	}


	/* get sign */
	d1 = d2 = 0;
	ptr1 = p1->data;
	ptr2 = p2->data;

	for ( i = 0; i < A->size2; i++ ) {
		u = *ptr1++;
		v = *ptr2++;
		d1 += SQR( u - v );
		d2 += SQR( u + v );
	}

	sign = 1;

	if ( d1 > d2 ) sign = -1;

	ptr1 = p1->data;

	for ( j = 0; j < A->size2; j++ ) {
		u = *ptr1++;
		u *= sign;
		gsl_matrix_set( mat2, id, j, u );
	}
}


VDictEntry TYPEDict[] = {
	{ "mean", 0 },
	{ "pca", 1 },
	{ NULL }
};



int
main ( int argc, char *argv[] )
{
	static VString  filename = "";
	static VString  rep_filename = "";
	static VShort   minval = 0;
	static VShort   type = 0;
	static VBoolean normalize = TRUE;
	static SPoint   addr;
	static VOptionDescRec  options[] = {
		{"mask", VStringRepn, 1, & filename, VOptionalOpt, NULL, "File containing segments" },
		{"report", VStringRepn, 1, & rep_filename, VOptionalOpt, NULL, "Report file" },
		{"minval", VShortRepn, 1, & minval, VOptionalOpt, NULL, "minval"},
		{"type", VShortRepn, 1, & type, VOptionalOpt, TYPEDict, "type"},
		{
			"normalize", VBooleanRepn, 1, & normalize, VOptionalOpt, NULL,
			"Whether to normalize the output to N(0,1)"
		},
		{"addr", VShortRepn, 3, &addr, VOptionalOpt, 0, "voxel address (if no mask is used)" }
	};
	FILE *in_file = NULL, *out_file = NULL, *fp = NULL, *fpe = NULL;
	VAttrList list = NULL, mlist = NULL, out_list = NULL;
	VAttrListPosn posn;
	VImage xsrc = NULL, src[NSLICES], mask = NULL, dest = NULL;
	Volumes volumes;
	Volume vol;
	VTrack tc;
	int i, j, k, id, size, maxid = 0;
	int b0, r0, c0, c1, b, r, c, nslices, nrows, ncols, len;
	double u, sum1, sum2, nx, mean, sigma;
	gsl_matrix *mat1 = NULL, *mat2 = NULL;

	char prg[50];
	sprintf( prg, "vtc V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg );

	VParseFilterCmd ( VNumber ( options ), options, argc, argv, &in_file, &out_file );


	if ( ! ( list = VReadFile ( in_file, NULL ) ) ) exit ( 1 );

	fclose( in_file );

	nslices = nrows = ncols = len = 0;
	i = -1;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & xsrc );

		if ( VPixelRepn( xsrc ) != VShortRepn ) continue;

		i++;
		src[i] = xsrc;

		if ( VImageNRows( xsrc ) < 2 ) continue;

		if ( VImageNBands( xsrc ) > len ) len = VImageNBands( xsrc );

		if ( VImageNRows( xsrc ) > nrows ) nrows = VImageNRows( xsrc );

		if ( VImageNColumns( xsrc ) > ncols ) ncols = VImageNColumns( xsrc );

		if ( i >= NSLICES ) VError( " too many slices %d", i );
	}

	nslices = i + 1;


	/*
	** read mask
	*/
	if ( strlen( filename ) > 2 ) {
		fp = VOpenInputFile ( filename, TRUE );
		mlist = VReadFile ( fp, NULL );

		if ( ! mlist )  VError( "Error reading mask image" );

		fclose( fp );

		for ( VFirstAttr ( mlist, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
			if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

			VGetAttrValue ( & posn, NULL, VImageRepn, & mask );

			if ( VPixelRepn( mask ) != VUByteRepn && VPixelRepn( mask ) != VBitRepn ) {
				mask = NULL;
				continue;
			}

			break;
		}

		if ( mask == NULL ) VError( " no mask found" );

		if ( VImageNColumns( mask ) != ncols ) VError( " inconsistent image dimensions" );

		if ( VImageNRows( mask ) != nrows ) VError( " inconsistent image dimensions" );

		if ( VImageNBands( mask ) != nslices ) VError( " inconsistent image dimensions" );
	} else {
		mask = VCreateImage( nslices, nrows, ncols, VBitRepn );
		VFillImage( mask, VAllBands, 0 );
		c0 = addr.x;
		r0 = addr.y;
		b0 = addr.z;

		if ( c0 < 0 || c0 >= ncols ) VError( "illegal coordinate" );

		if ( r0 < 0 || r0 >= nrows ) VError( "illegal coordinate" );

		if ( b0 < 0 || b0 >= nslices ) VError( "illegal coordinate" );

		VPixel( mask, b0, r0, c0, VBit ) = 1;
		fprintf( stderr, "# addr: %d %d %d\n", c0, r0, b0 );
	}

	if ( mask == NULL ) VError( " specify either mask or voxel addr" );


	/*
	** get max-id
	*/
	volumes = VImage2Volumes( mask );

	maxid = 0;

	for ( vol = volumes->first; vol != NULL; vol = vol->next ) {
		id = vol->label;

		if ( id > maxid ) maxid = id;
	}


	/*
	** loop thru volumes
	*/
	mat2 = gsl_matrix_calloc( maxid, len );

	id = 0;

	for ( vol = volumes->first; vol != NULL; vol = vol->next ) {

		size = VolumeSize( vol );

		if ( size < 2 && type == 1 ) VError( " ROI too small for PCA" );

		mat1 = gsl_matrix_calloc( size, len );

		i = 0;

		for ( k = 0; k < VolumeNBuckets( vol ); k++ ) {
			for ( tc = VFirstTrack( vol, k ); VTrackExists( tc ); tc = VNextTrack( tc ) ) {
				b  = tc->band;

				if ( VImageNRows( src[b] ) < 2 ) continue;

				r  = tc->row;
				c0 = tc->col;
				c1 = c0 + tc->length;

				for ( c = c0; c < c1; c++ ) {

					mean  = 0;
					sigma = 1;

					sum1 = sum2 = nx = 0;

					for ( j = 0; j < len; j++ ) {
						if ( VPixel( src[b], 0, r, c, VShort ) < minval ) continue;

						u = VPixel( src[b], j, r, c, VShort );
						sum1 += u;
						sum2 += u * u;
						nx++;
					}

					if ( nx < 3 ) continue;

					mean  = sum1 / nx;
					sigma = sqrt( ( sum2 - nx * mean * mean ) / ( nx - 1.0 ) );


					for ( j = 0; j < len; j++ ) {
						if ( VPixel( src[b], 0, r, c, VShort ) < minval ) continue;

						u = VPixel( src[b], j, r, c, VShort );

						u = ( u - mean );
						gsl_matrix_set( mat1, i, j, u );
					}

					i++;
				}
			}
		}

		switch ( type ) {
		case 0:  /* mean */
			GetMean( mat1, mat2, id );
			break;

		case 1:  /* pca */
			GetPCA( mat1, mat2, id );
			break;

		default:
			VError( " illegal type" );
		}

		gsl_matrix_free( mat1 );
		id++;
	}


	/*
	** normalize
	*/
	if ( normalize )
		Normalize( mat2 );


	/*
	** output to ASCII file or to terminal
	*/
	if ( strlen( rep_filename ) > 1 ) {
		fpe = fopen( rep_filename, "w" );

		if ( !fpe ) VError( " err opening report file %s", rep_filename );

		for ( i = 0; i < len; i++ ) {
			for ( id = 0; id < maxid; id++ ) {
				fprintf( fpe, " %10.5f", gsl_matrix_get( mat2, id, i ) );
			}

			fprintf( fpe, "\n" );
		}

		fclose( fpe );
	}


	/*
	** output to data file
	*/
	dest = VCreateImage( len, 2, maxid + 1, VShortRepn );
	VFillImage( dest, VAllBands, 0 );
	VCopyImageAttrs ( src[0], dest );

	for ( id = 0; id < maxid; id++ ) {
		for ( i = 0; i < len; i++ ) {
			u = gsl_matrix_get( mat2, id, i );

			if ( normalize ) u = 10000.0 + 1000.0 * u;

			VPixel( dest, i, 0, id, VShort ) = u;
		}
	}


	/*
	** write output image
	*/
	out_list = VCreateAttrList();
	VHistory( VNumber( options ), options, prg, &list, &out_list );
	VAppendAttr( out_list, "image", NULL, VImageRepn, dest );

	if ( ! VWriteFile ( out_file, out_list ) ) VError( " error writing output file" );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return ( 0 );
}
