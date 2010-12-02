/****************************************************************
 *
 * visotrop:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 1998, <lipsia@cbs.mpg.de>
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
 * $Id: visotrop.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <via.h>

/*------------------------------------------------------------------------------

visotrop makes isotropic voxels and transposes a sagital 3D dataset to axial
direction

------------------------------------------------------------------------------*/


extern char *getLipsiaVersion();

VDictEntry ITYPDict[] = {
	{ "linear", 0 },
	{ "NN", 1 },
	{ "spline", 2 },
	{ NULL }
};


/*
 *  Program entry point.
 */

int main ( int argc, char *argv[] )
{
	static VFloat reso   = -1.0;
	static VLong itype   = 0;
	static VBoolean flip = TRUE;
	static VBoolean reorder = TRUE;

	static VOptionDescRec  options[] = {
		{
			"reso", VFloatRepn, 1, ( VPointer ) &reso,
			VOptionalOpt, NULL, "New voxel resolution in mm, def: -1 means min(1.0,\"best source resolution\")"
		},

		{
			"flip", VBooleanRepn, 1, ( VPointer ) &flip,
			VOptionalOpt, NULL, "Whether to flip to natural convention"
		},

		{
			"reorder", VBooleanRepn, 1, ( VPointer ) &reorder,
			VOptionalOpt, NULL, "Whether to reorder axial slices from axial source image"
		},

		{
			"interpolation", VLongRepn, 1, & itype, VOptionalOpt, ITYPDict,
			"Type of interpolation (0: linear, 1: nearest neighbour, 2: cubic spline)"
		}
	};

	FILE *in_file, *out_file;
	VAttrList list;
	VAttrListPosn posn;
	int nobjects = 0;
	VImage src = NULL, dest = NULL, result = NULL;
	int i, b, r, c, nbands, nrows, ncols;
	VString str, newstr, fixpointString, caString, cpString;
	float fix_c, fix_r, fix_b;
	float ca_c, ca_r, ca_b;
	float cp_c, cp_r, cp_b;
	float x, y, z, min;
	VDouble v, scale_band, scale_row, scale_col;
	float scale[3], shift[3];

	/* print information */
	char prg_name[50];
	sprintf( prg_name, "visotrop V%s", getLipsiaVersion() );
	fprintf ( stderr, "%s\n", prg_name );
	fflush ( stderr );

	/* Parse command line arguments: */
	VParseFilterCmd ( VNumber ( options ), options, argc, argv,
					  & in_file, & out_file );

	/* Read source image(s): */
	if ( ! ( list = VReadFile ( in_file, NULL ) ) )
		exit ( EXIT_FAILURE );

	/* Scale each object: */
	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		switch ( VGetAttrRepn ( & posn ) ) {

		case VImageRepn:
			VGetAttrValue ( & posn, NULL, VImageRepn, & src );

			if ( VGetAttr ( VImageAttrList ( src ), "voxel", NULL,
							VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
				sscanf( str, "%f %f %f", &x, &y, &z );
				fprintf( stderr, " voxel: %f %f %f\n", x, y, z );

				min = x < y ? x : y;
				min = z < min ? z : min;

				/* if resolution is not set, use default value 1 or
				   smaler value if the image resolution is better */
				if ( reso < 0.0 )
					reso = min < 1.0 ? min : 1.0;

				if ( reso <= 0.0 ) exit ( EXIT_FAILURE );

				fprintf( stderr, " new resolution: %f \n", reso );

				scale_col  = x / reso;
				scale_row  = y / reso;
				scale_band = z / reso;

				nbands = VImageNBands( src ) * scale_band;
				nrows = VImageNRows( src ) * scale_row;
				ncols = VImageNColumns( src ) * scale_col;

				if ( VImageNBands( src ) == nbands
					 && VImageNRows( src ) == nrows
					 && VImageNColumns( src ) == ncols ) itype = 3;

				fprintf( stderr, " interpolation type: %s\n", ITYPDict[itype] );
				fprintf( stderr, " old dim: %3d %3d %3d\n",
						 VImageNBands( src ), VImageNRows( src ), VImageNColumns( src ) );


				for ( i = 0; i < 3; i++ ) shift[i] = scale[i] = 0;

				scale[0] = scale_band;
				scale[1] = scale_row;
				scale[2] = scale_col;

				switch ( itype ) {

					/* trilinear interpolation resampling */
				case 0:
					dest = VTriLinearScale3d ( src, NULL, ( int )nbands, ( int )nrows, ( int )ncols,
											   shift, scale );
					break;


					/* nearest neightbour resampling */
				case 1:
					dest = VNNScale3d ( src, NULL, ( int )nbands, ( int )nrows, ( int )ncols,
										shift, scale );
					break;


					/* cubic spline */
				case 2:
					dest = VCubicSplineScale3d ( src, NULL, ( int )nbands, ( int )nrows, ( int )ncols,
												 shift, scale );
					break;

				case 3: /* no interpolation, just reshuffle */
					dest = VCopyImage( src, NULL, VAllBands );
					break;

				default:
					VError( " unknown resampling type %d", itype );
				}

				if ( ! dest ) exit ( EXIT_FAILURE );

				/*aa 2003/09/11 added function not to rotate siemens data*/
				if ( ! VGetAttr ( VImageAttrList ( src ), "orientation", NULL,
								  VStringRepn, ( VPointer ) & str ) == VAttrFound ) VError( " attribute 'orientation' missing" );

				if ( strcmp ( str, "axial" ) == 0 ) {
					fprintf( stderr, " new dim: %3d %3d %3d\n", nbands, nrows, ncols );
					result = VCreateImage( nbands, nrows, ncols, VPixelRepn( src ) );
					VFillImage( result, VAllBands, 0 );

					for ( b = 0; b < nbands; b++ )
						for ( r = 0; r < nrows; r++ )
							for ( c = 0; c < ncols; c++ ) {
								v = VGetPixel( dest, b, r, c );

								if ( (     flip == FALSE ) && ( reorder == FALSE ) )
									VSetPixel( result, b, r, ncols - c - 1, v );

								else if ( ( flip == TRUE )  && ( reorder == FALSE ) )
									VSetPixel( result, b, r, c, v );

								else if ( ( flip == FALSE ) && ( reorder == TRUE ) )
									VSetPixel( result, nbands - b - 1, r, ncols - c - 1, v );

								else if ( ( flip == TRUE )  && ( reorder == TRUE ) )
									VSetPixel( result, nbands - b - 1, r, c, v );
							}
				} else if ( strcmp ( str, "sagittal" ) == 0 ) {

					/* re-arrange from sagittal to axial orientation */
					fprintf( stderr, " new dim: %3d %3d %3d\n", nrows, ncols, nbands );
					result = VCreateImage( nrows, ncols, nbands, VPixelRepn( src ) );
					VFillImage( result, VAllBands, 0 );


					for ( b = 0; b < nbands; b++ )
						for ( r = 0; r < nrows; r++ )
							for ( c = 0; c < ncols; c++ ) {
								v = VGetPixel( dest, b, r, c );

								if ( flip == FALSE )
									VSetPixel( result, r, c, nbands - b - 1, v );
								else
									VSetPixel( result, r, c, b, v );
							}
				}

				else if ( strcmp ( str, "coronal" ) == 0 ) {

					/* re-arrange from coronal to axial orientation */
					fprintf( stderr, " new dim: %3d %3d %3d\n", nrows, nbands, ncols );
					result = VCreateImage( nrows, nbands, ncols, VPixelRepn( src ) );
					VFillImage( result, VAllBands, 0 );


					for ( b = 0; b < nbands; b++ )
						for ( r = 0; r < nrows; r++ )
							for ( c = 0; c < ncols; c++ ) {
								v = VGetPixel( dest, b, r, c );

								if ( flip == FALSE )
									VSetPixel( result, r, b, ncols - c - 1, v );
								else
									VSetPixel( result, r, b, c, v );
							}
				}

				else {
					VError( " unknown resampling type %d", itype );
					exit ( EXIT_FAILURE );
				}

				/* copy attributes from source image */
				VCopyImageAttrs ( src, result );

				// [TS] 08/03/27
				// correct 'fixpoint', 'ca' and 'cp' if they exist in the source image
				//
				// NOTE:
				// this is only done when no flipping or reordering is requested :-(
				// (WARNING!!!!) '-flip true' actually means that no flipping is done (WHAAAAT ????)
				// and therefore we test for reorder = false and flip = true
				fixpointString = VMalloc( 80 );
				caString       = VMalloc( 80 );
				cpString       = VMalloc( 80 );
				VBoolean _issueWarning = FALSE;

				if( VGetAttr( VImageAttrList( src ), "fixpoint", NULL, VStringRepn, ( VPointer )&fixpointString ) == VAttrFound ) {
					if( reorder == FALSE && flip == TRUE ) {
						sscanf( fixpointString, "%f %f %f", &fix_c, &fix_r, &fix_b );
						fix_c *= scale_col;
						fix_r *= scale_row;
						fix_b *= scale_band;
						sprintf( ( char * )fixpointString, "%f %f %f", fix_c, fix_r, fix_b );
						VSetAttr( VImageAttrList( result ), "fixpoint", NULL, VStringRepn, fixpointString );
					} else {
						_issueWarning = TRUE;
					}
				}

				if( VGetAttr( VImageAttrList( src ), "ca", NULL, VStringRepn, ( VPointer )&caString ) == VAttrFound ) {
					if( reorder == FALSE && flip == TRUE ) {
						sscanf( caString, "%f %f %f", &ca_c, &ca_r, &ca_b );
						ca_c *= scale_col;
						ca_r *= scale_row;
						ca_b *= scale_band;
						sprintf( ( char * )caString, "%f %f %f", ca_c, ca_r, ca_b );
						VSetAttr( VImageAttrList( result ), "ca", NULL, VStringRepn, caString );
					} else {
						_issueWarning = TRUE;
					}
				}

				if( VGetAttr( VImageAttrList( src ), "cp", NULL, VStringRepn, ( VPointer )&cpString ) == VAttrFound ) {
					if( reorder == FALSE && flip == TRUE ) {
						sscanf( cpString, "%f %f %f", &cp_c, &cp_r, &cp_b );
						cp_c *= scale_col;
						cp_r *= scale_row;
						cp_b *= scale_band;
						sprintf( ( char * )cpString, "%f %f %f", cp_c, cp_r, cp_b );
						VSetAttr( VImageAttrList( result ), "cp", NULL, VStringRepn, cpString );
					} else {
						_issueWarning = TRUE;
					}
				}

				if( _issueWarning ) {
					VWarning( "Attributes 'fixpoint', 'ca' and 'cp' exist but were not corrected and are therefore likely to be wrong" );
					VWarning( "This was caused by setting -flip to false or -reorder to true" );
					VWarning( "Please correct the values manually using vattredit" );
				}



				/* set the attributes to the changed values */
				newstr = VMalloc( 80 );
				sprintf( ( char * )newstr, "%f %f %f", reso, reso, reso );
				VSetAttr( VImageAttrList ( result ), "voxel", NULL, VStringRepn, newstr );
				VSetAttr( VImageAttrList ( result ), "orientation", NULL, VStringRepn, "axial" );

				if ( flip )
					VSetAttr( VImageAttrList ( result ), "convention", NULL, VStringRepn, "natural" );
				else
					VSetAttr( VImageAttrList ( result ), "convention", NULL, VStringRepn, "radiologic" );
			}

			VSetAttrValue ( & posn, NULL, VImageRepn, result );
			VDestroyImage ( src );
			break;

		default:
			continue;
		}

		nobjects++;
	}

	/* Make History */
	VHistory( VNumber( options ), options, prg_name, &list, &list );

	/* Write the results to the output file: */
	if ( ! VWriteFile ( out_file, list ) )
		exit ( EXIT_FAILURE );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return EXIT_SUCCESS;
}

