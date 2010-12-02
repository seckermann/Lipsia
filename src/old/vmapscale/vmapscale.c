/****************************************************************
 *
 * Program: vmapscale
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
 * $Id: vmapscale.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>

#include <string.h>
#include <stdlib.h>
#include <via.h>

extern char *getLipsiaVersion();

VDictEntry ITYPDict[] = {
	{ "trilinear", 0 },
	{ "NN", 1 },
	{ NULL }
};

#define M 256

int main ( int argc, char *argv[] )
{
	static VFloat   xreso = 1.0;
	static VFloat   yreso = 1.0;
	static VFloat   zreso = 1.0;

	static VShort   xdim = 0;
	static VShort   ydim = 0;
	static VShort   zdim = 0;

	static VShort   type = 0;
	static VOptionDescRec  options[] = {
		{ "xreso", VFloatRepn, 1, & xreso, VOptionalOpt, NULL, "New voxel resolution in x-dir (mm)" },
		{ "yreso", VFloatRepn, 1, & yreso, VOptionalOpt, NULL, "New voxel resolution in y-dir (mm)" },
		{ "zreso", VFloatRepn, 1, & zreso, VOptionalOpt, NULL, "New voxel resolution in z-dir (mm)" },
		{ "xdim", VShortRepn, 1, & xdim, VOptionalOpt, NULL, "Matrix size in x-dir" },
		{ "ydim", VShortRepn, 1, & ydim, VOptionalOpt, NULL, "Matrix size in y-dir" },
		{ "zdim", VShortRepn, 1, & zdim, VOptionalOpt, NULL, "Matrix size in z-dir" },
		{
			"type", VShortRepn, 1, & type, VOptionalOpt, ITYPDict,
			"Type of interpolation: trilinear | NN (nearest neighbour)"
		}
	};

	FILE *in_file, *out_file;
	VAttrList list;
	VAttrListPosn posn;
	VImage src = NULL, dest = NULL;
	int dst_nbands, dst_nrows, dst_ncols;
	VString str, str1;
	float x, y, z;
	float xscale = 1, yscale = 1, zscale = 1;
	float scale[3], shift[3];
	char prg_name[50];
	sprintf( prg_name, "vmapscale V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );


	/* Parse command line arguments: */
	VParseFilterCmd ( VNumber ( options ), options, argc, argv, & in_file, & out_file );


	/* Read source image(s): */
	if ( ! ( list = VReadFile ( in_file, NULL ) ) )  exit ( EXIT_FAILURE );

	fclose( in_file );

	str1 = VCalloc( M, 1 );


	/* Scale each object: */
	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );


		if ( VGetAttr ( VImageAttrList ( src ), "voxel", NULL,
						VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
			sscanf( str, "%f %f %f", &x, &y, &z );
			xscale = x / xreso;
			yscale = y / yreso;
			zscale = z / zreso;
		}

		dst_nbands = VFloor( ( float )VImageNBands( src ) * zscale );
		dst_nrows  = VFloor( ( float )VImageNRows( src ) * yscale );
		dst_ncols  = VFloor( ( float )VImageNColumns( src ) * xscale );

		if ( xdim > 0 ) dst_ncols  = xdim;

		if ( ydim > 0 ) dst_nrows  = ydim;

		if ( zdim > 0 ) dst_nbands = zdim;

		shift[0] = shift[1] = shift[2] = 0;


		fprintf( stderr, " output matrix size: %3d %3d %3d\n", dst_nbands, dst_nrows, dst_ncols );
		fprintf( stderr, "    scaling factors: %.3f %.3f %.3f\n", xscale, yscale, zscale );

		scale[0] = zscale;
		scale[1] = yscale;
		scale[2] = xscale;

		if ( VPixelRepn( src ) == VFloatRepn && type == 2 ) {
			VWarning( " no spline interpolation for float images, using trilinear" );
			type = 0;
		}


		switch ( type ) {
		case 0:   /* trilinear interpolation resampling */
			dest = VTriLinearScale3d ( src, NULL, dst_nbands, dst_nrows, dst_ncols, shift, scale );
			break;

		case 1:   /* nearest neighbour resampling */
			dest = VNNScale3d ( src, NULL, dst_nbands, dst_nrows, dst_ncols, shift, scale );
			break;

		case 2:   /* cubic spline resampling */
			dest = VCubicSplineScale3d ( src, NULL, dst_nbands, dst_nrows, dst_ncols, shift, scale );
			break;

		default:
			VError( "illegal interpolation type" );
		}

		if ( ! dest ) exit ( EXIT_FAILURE );


		/*
		** update header info
		*/

		/* update voxel */
		memset( str1, 0, M );
		sprintf( str1, "%.3f %.3f %.3f", xreso, yreso, zreso );
		VSetAttr( VImageAttrList( dest ), "voxel", NULL, VStringRepn, str1 );

		/* update ca */
		if ( VGetAttr ( VImageAttrList ( src ), "ca", NULL,
						VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
			sscanf( str, "%f %f %f", &x, &y, &z );
			x = x * xscale;
			y = y * yscale;
			z = z * zscale;

			memset( str1, 0, M );
			sprintf( str1, "%.3f %.3f %.3f", x, y, z );
			VSetAttr( VImageAttrList( dest ), "ca", NULL, VStringRepn, str1 );
		}

		/* update cp */
		if ( VGetAttr ( VImageAttrList ( src ), "cp", NULL,
						VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
			sscanf( str, "%f %f %f", &x, &y, &z );
			x = x * xscale;
			y = y * yscale;
			z = z * zscale;
			memset( str1, 0, M );
			sprintf( str1, "%.3f %.3f %.3f", x, y, z );
			VSetAttr( VImageAttrList( dest ), "cp", NULL, VStringRepn, str1 );
		}

		/* update fixpoint */
		if ( VGetAttr ( VImageAttrList ( src ), "fixpoint", NULL,
						VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
			sscanf( str, "%f %f %f", &x, &y, &z );
			x = x * xscale;
			y = y * yscale;
			z = z * zscale;
			memset( str1, 0, M );
			sprintf( str1, "%.3f %.3f %.3f", x, y, z );
			VSetAttr( VImageAttrList( dest ), "fixpoint", NULL, VStringRepn, str1 );
		}

		VSetAttrValue ( & posn, NULL, VImageRepn, dest );
	}


	VHistory( VNumber( options ), options, prg_name, &list, &list );

	if ( ! VWriteFile ( out_file, list ) ) exit ( 1 );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return 0;
}
