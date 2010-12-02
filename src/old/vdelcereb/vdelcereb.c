/****************************************************************
 *
 * vdelcereb:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author G Lohmann, 1998, <lipsia@cbs.mpg.de>
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
 * $Id: vdelcereb.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <via.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



extern VImage VCerebellum( VImage );
extern void VDeleteCereb( VImage, VImage *, int, int );
extern char *getLipsiaVersion();

static int verbose = 0;

static VBoolean in_found, out_found;
static  FILE *in_file, *out_file;
static  VString in_filename, out_filename;
static VOptionDescRec options[] = {
	{ "in",      VStringRepn, 1, &in_filename,        &in_found,    NULL, "Input File (white matter segmentation)" },
	{ "out",     VStringRepn, 1, & out_filename,      & out_found,  NULL, "Output file" },
	{ "verbose", VShortRepn,  1, ( VPointer ) &verbose, VOptionalOpt, NULL, "Show program messages"}
};


#define ABS(x) (((x) > 0) ? (x) : -(x))
#define SQR(x) ((x) * (x))

int main ( int argc, char *argv[] )
{

	VAttrList list, out_list;
	VImage src;
	VImage dest = NULL;
	VAttrListPosn posn;
	VString str;
	int nimages;
	char historystr[] = "history";
	char prg_name[50];
	sprintf( prg_name, "vdelcereb V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );

	/* Parse command line arguments: */
	if ( !VParseCommand ( VNumber ( options ), options, &argc, argv ) ||
		 ! VIdentifyFiles ( VNumber ( options ), options, "in", & argc, argv, 0 ) ||
		 ! VIdentifyFiles ( VNumber ( options ), options, "out", & argc, argv, -1 ) )
		goto Usage;

	if ( argc > 1 ) {
		VReportBadArgs ( argc, argv );
Usage:
		VReportUsage ( argv[0], VNumber ( options ), options, NULL );
		exit ( EXIT_FAILURE );
	}

	/* Read source image(s): */
	if ( strcmp ( in_filename, "-" ) == 0 )
		in_file = stdin;
	else {
		in_file = fopen ( in_filename, "r" );

		if ( ! in_file )
			VError ( "Failed to open input file %s", in_filename );
	}

	if ( ! ( list = VReadFile ( in_file, NULL ) ) )
		exit ( EXIT_FAILURE );

	fclose ( in_file );

	/* Process image */
	nimages = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn )
			continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );
		dest = VCerebellum( src );

		VSetAttrValue ( & posn, NULL, VImageRepn, dest );
		VDestroyImage ( src );
		nimages++;
		break;
	}

	/* Create outlist */
	out_list = VCreateAttrList();

	/* Make History */
	VHistory( VNumber( options ), options, prg_name, &list, &out_list );

	/*  Alle Attribute (ausser history:) in die neue Liste schreiben: */
	for ( VFirstAttr ( list, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
		if ( strncmp( VGetAttrName( &posn ), historystr, strlen( historystr ) ) == 0 )
			continue;

		switch ( VGetAttrRepn( &posn ) ) {
		case VImageRepn:
			VGetAttrValue( &posn, NULL, VImageRepn, &src );
			VAppendAttr( out_list, VGetAttrName( &posn ), NULL, VImageRepn, src );
			break;

		case VStringRepn:
			VGetAttrValue( &posn, NULL, VStringRepn, &str );
			VAppendAttr( out_list, VGetAttrName( &posn ), NULL, VImageRepn, str );
			break;

		default:
			break;
		}
	}


	/* Write the results to the output file: */
	if ( strcmp ( out_filename, "-" ) == 0 )
		out_file = stdout;
	else {
		out_file = fopen ( out_filename, "w" );

		if ( ! out_file )
			VError ( "Failed to open output file %s", out_filename );
	}

	if ( VWriteFile ( out_file, out_list ) ) {
		if ( verbose >= 1 ) fprintf ( stderr, "%s: processed %d image%s.\n",
										  argv[0], nimages, nimages == 1 ? "" : "s" );

		fprintf( stderr, "%s: done.\n", argv[0] );
	}

	return 0;
}


VImage
VCerebellum( VImage src )
{
	VImage coronal, label_coronal;
	VImage sagital, label_sagital;
	VImage axial, label_axial;
	VImage bin0_image, bin1_image, label_image;
	VBit *bin0_pp, *src_pp;
	VUByte *ubyte_pp;
	int i, nbands, nrows, ncols, npixels, nl = 0;
	float x0, x1, x2;
	VString str;
	int b, r, c, cp, rp, bp, slice_extent, slice_origin, b0;

	nbands  = VImageNBands( src );
	nrows   = VImageNRows( src );
	ncols   = VImageNColumns( src );
	npixels = nbands * nrows * ncols;

	str = VMalloc( 80 );

	if ( VGetAttr( VImageAttrList( src ), "cp", NULL, VStringRepn, ( VPointer ) &str ) != VAttrFound )
		VError( " attribute 'cp' not found" );

	sscanf( str, "%f %f %f", &x0, &x1, &x2 );
	bp = ( int ) VRint( ( double ) x2 );
	rp = ( int ) VRint( ( double ) x1 );
	cp = ( int ) VRint( ( double ) x0 );

	if ( VGetAttr( VImageAttrList( src ), "extent", NULL, VStringRepn, ( VPointer ) &str ) != VAttrFound )
		VError( " attribute 'extent' not found" );

	sscanf( str, "%f %f %f", &x0, &x1, &x2 );
	slice_extent = ( int ) VRint( ( double ) x2 );

	if ( VGetAttr( VImageAttrList( src ), "origin", NULL, VStringRepn, ( VPointer ) &str ) != VAttrFound )
		VError( " attribute 'origin' not found" );

	sscanf( str, "%f %f %f", &x0, &x1, &x2 );
	slice_origin = ( int ) VRint( ( double ) x2 );

	/* erode */
	bin1_image = VDTErode( src, NULL, ( VDouble ) 2.0 );

	/* readdress to coronal slices and remove small 2D components */
	coronal = VCreateImage( 1, nbands, ncols, VBitRepn );
	label_coronal = VCreateImage( 1, nbands, ncols, VUByteRepn );

	ubyte_pp = ( VUByte * ) VImageData( label_coronal );

	for ( i = 0; i < ( nbands * ncols ); i++ ) *ubyte_pp++ = 0;

	for ( r = rp; r < nrows; r++ ) {

		bin0_pp = ( VBit * ) VImageData( coronal );

		for ( i = 0; i < ( nbands * ncols ); i++ ) *bin0_pp++ = 0;

		for ( b = 0; b < nbands; b++ ) {
			for ( c = 0; c < ncols; c++ ) {
				VPixel( coronal, 0, b, c, VBit ) = VPixel( bin1_image, b, r, c, VBit );
			}
		}

		label_coronal = VLabelImage2d( coronal, label_coronal, 8, VUByteRepn, &nl );

		VDeleteCereb ( label_coronal, &coronal, 110, ( int ) 0 );

		for ( b = 0; b < nbands; b++ ) {
			for ( c = 0; c < ncols; c++ ) {
				if ( VPixel( coronal, 0, b, c, VBit ) == 0 )
					VPixel( bin1_image, b, r, c, VBit ) = 0;
			}
		}
	}

	/* readdress to sagital slices and remove small 2D components */
	sagital = VCreateImage( 1, nbands, nrows, VBitRepn );
	label_sagital = VCreateImage( 1, nbands, nrows, VUByteRepn );
	ubyte_pp = ( VUByte * ) VImageData( label_sagital );

	for ( i = 0; i < ( nbands * nrows ); i++ ) *ubyte_pp++ = 0;

	for ( c = 0; c < ncols; c++ ) {
		if ( ABS( c - 80 ) < 10 ) continue;

		bin0_pp = ( VBit * ) VImageData( sagital );

		for ( i = 0; i < ( nbands * nrows ); i++ ) *bin0_pp++ = 0;

		for ( b = 0; b < nbands; b++ ) {
			for ( r = 0; r < nrows; r++ ) {
				VPixel( sagital, 0, b, r, VBit ) = VPixel( bin1_image, b, r, c, VBit );
			}
		}

		label_sagital = VLabelImage2d( sagital, label_sagital, ( int ) 8, VUByteRepn, &nl );
		VDeleteCereb ( label_sagital, &sagital, 115, cp );

		for ( b = 0; b < nbands; b++ ) {
			for ( r = 0; r < nrows; r++ ) {
				if ( VPixel( sagital, 0, b, r, VBit ) == 0 )
					VPixel( bin1_image, b, r, c, VBit ) = 0;
			}
		}
	}

	/* readdress to axial slices and remove small 2D components */
	axial = VCreateImage( 1, nrows, ncols, VBitRepn );
	label_axial = VCreateImage( 1, nrows, ncols, VUByteRepn );
	ubyte_pp = ( VUByte * ) VImageData( label_axial );

	for ( i = 0; i < ( nrows * ncols ); i++ ) *ubyte_pp++ = 0;

	/*  for (b=bp; b<nbands; b++) { */
	for ( b = 105; b < nbands; b++ ) {

		bin0_pp = ( VBit * ) VImageData( axial );

		for ( i = 0; i < ( nrows * ncols ); i++ ) *bin0_pp++ = 0;

		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				VPixel( axial, 0, r, c, VBit ) = VPixel( bin1_image, b, r, c, VBit );
			}
		}

		label_sagital = VLabelImage2d( axial, label_axial, ( int ) 8, VUByteRepn, &nl );
		VDeleteCereb ( label_axial, &axial, 0, 0 );

		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				if ( VPixel( axial, 0, r, c, VBit ) == 0 )
					VPixel( bin1_image, b, r, c, VBit ) = 0;
			}
		}
	}

	/* remove everything below slice extent */
	b0 = slice_extent + slice_origin;
	npixels = nrows * ncols;

	for ( b = b0; b < nbands; b++ ) {
		bin0_pp = ( VBit * ) VPixelPtr( bin1_image, b, 0, 0 );

		for ( i = 0; i < npixels; i++ )
			*bin0_pp++ = 0;
	}

	/* dilate */
	bin0_image = VDTDilate( bin1_image, NULL, ( VDouble ) 3.0 );
	npixels = nrows * ncols;

	for ( b = 0; b < bp; b++ ) {
		bin0_pp = ( VBit * ) VPixelPtr( bin0_image, b, 0, 0 );
		src_pp  = ( VBit * ) VPixelPtr( src, b, 0, 0 );

		for ( i = 0; i < npixels; i++ ) *bin0_pp++ = *src_pp++;
	}

	for ( b = bp; b < nbands; b++ ) {
		bin0_pp = ( VBit * ) VPixelPtr( bin0_image, b, 0, 0 );
		src_pp  = ( VBit * ) VPixelPtr( src, b, 0, 0 );

		for ( i = 0; i < npixels; i++ ) {
			if ( *src_pp == 0 ) *bin0_pp = 0;

			src_pp++;
			bin0_pp++;
		}
	}

	/*
	** remove small remaining components
	*/
	label_image = VLabelImage3d( bin0_image, NULL, ( int ) 26, VShortRepn, &nl );
	bin0_image = VSelectBig( label_image, bin0_image );

	VImageAttrList ( bin0_image ) = VCopyAttrList ( VImageAttrList ( src ) );
	return bin0_image;
}



typedef struct PointStruct {
	int b, r, c;
} Point;

/*
** identify conponents belonging to the cerebellum
*/
void
VDeleteCereb( VImage label_image, VImage *wm, int ta, int tb )
{
	VUByte *ubyte_pp;
	VBit *bit_pp;
	double *npix;
	int i, j, maxlabel = 0;
	Point *mean;
	int b, r, c, nbands, nrows, ncols, npixels;

	nbands  = VImageNBands ( ( *wm ) );
	nrows   = VImageNRows ( ( *wm ) );
	ncols   = VImageNColumns ( ( *wm ) );
	npixels = nbands * nrows * ncols;

	maxlabel = 0;
	ubyte_pp = ( VUByte * ) VImageData( label_image );

	for ( i = 0; i < npixels; i++ ) {
		if ( *ubyte_pp > maxlabel ) maxlabel = *ubyte_pp;

		ubyte_pp++;
	}

	maxlabel++;

	mean = ( Point * ) VMalloc( sizeof( Point ) * maxlabel );
	npix = ( double * ) VMalloc( sizeof( double ) * maxlabel );

	for ( i = 0; i < maxlabel; i++ ) {
		mean[i].b = 0;
		mean[i].r = 0;
		mean[i].c = 0;
		npix[i]   = 0;
	}

	/* get centroids */
	ubyte_pp = ( VUByte * ) VImageData( label_image );

	for ( b = 0; b < nbands; b++ ) {
		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {

				i = ( int ) * ubyte_pp++;

				if ( i != 0 ) {
					npix[i]++;
					mean[i].b += b;
					mean[i].r += r;
					mean[i].c += c;
				}
			}
		}
	}

	for ( i = 0; i < maxlabel; i++ ) {
		if ( npix[i] > 1 ) {
			mean[i].b /= npix[i];
			mean[i].r /= npix[i];
			mean[i].c /= npix[i];
		}
	}

	/* del comp */
	bit_pp = ( VBit * ) VImageData( ( *wm ) );
	ubyte_pp = ( VUByte * ) VImageData( label_image );

	for ( j = 0; j < npixels; j++ ) {
		i = ( int ) * ubyte_pp++;

		if ( i > 0 ) {
			if ( ta == 0 ) {    /* remove center blobs  (axial) */
				if ( npix[i] < 1000
					 && ABS( mean[i].r - ( nrows / 2 ) ) < 10
					 && ABS( mean[i].c - ( ncols / 2 ) ) < 10 )
					*bit_pp = 0;
			} else {
				if ( mean[i].r >= 100 && tb == 0
					 && ABS( mean[i].c - ( ncols / 2 ) ) < 10 ) /* coronal only */
					*bit_pp = 0;

				else if ( mean[i].r >= ta && mean[i].c >= tb ) /* sagittal, coronal */
					*bit_pp = 0;
			}
		}

		bit_pp++;
	}
}
