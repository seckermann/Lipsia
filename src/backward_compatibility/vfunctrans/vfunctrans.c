/****************************************************************
 *
 * vfunctrans:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 1999, <lipsia@cbs.mpg.de>
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
 * $Id: vfunctrans.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/headerinfo.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern char *getLipsiaVersion();
extern void VImageInfoIni( VImageInfo * );
extern VBoolean ReadHeader( FILE * );
extern VBoolean VGetImageInfo( FILE *, VAttrList, int, VImageInfo * );
extern VAttrList ReadAttrList ( FILE * );
extern VImage *VFunctrans( VStringConst, VImageInfo *, int, VImage, VFloat,
						   int, VShort, VBoolean, int * );


int main ( int argc, char *argv[] )
{
	/* command line arguments */
	static VString out_filename;
	static VArgVector in_files;
	static VString trans_filename = "";
	static VBoolean in_found, out_found;
	static VShort minval = 0;
	static VBoolean compress = TRUE;
	static VFloat resolution = 3;
	static VOptionDescRec options[] = {
		{ "in", VStringRepn, 0, & in_files, & in_found, NULL, "Input file" },
		{ "out", VStringRepn, 1, & out_filename, & out_found, NULL, "Output file" },
		{
			"trans", VStringRepn, 1, &trans_filename, VRequiredOpt, NULL,
			"File containing transformation matrix"
		},
		{
			"resolution", VFloatRepn, 1, &resolution, VOptionalOpt, NULL,
			"Output voxel resolution in mm"
		},
		{
			"minval", VShortRepn, 1, &minval, VOptionalOpt, NULL,
			"Signal threshold"
		},
		{
			"compress", VBooleanRepn, 1, &compress, VOptionalOpt, NULL,
			"Whether to compress empty slices"
		}
	};

	VStringConst in_filename;
	FILE *in_file, *out_file, *fp;
	VAttrList list, list1, out_list;
	VAttrListPosn posn;
	VImage trans = NULL;
	VImage *dst_image;
	VImageInfo *imageInfo;
	int nobject = 0, ntimesteps = 0, nbands = 0, nrows = 0, ncols = 0;
	VString ca, cp, extent, str;
	int found = 0;
	int j, dest_nbands;
	char prg_name[50];
	sprintf( prg_name, "vfunctrans V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );

	/* Parse command line arguments: */
	if ( ! VParseCommand ( VNumber ( options ), options, & argc, argv ) ||
		 ! VIdentifyFiles ( VNumber ( options ), options, "in", & argc, argv, 0 ) ||
		 ! VIdentifyFiles ( VNumber ( options ), options, "out", & argc, argv, -1 ) )
		goto Usage;

	if ( argc > 1 ) {
		VReportBadArgs ( argc, argv );
Usage:
		VReportUsage ( argv[0], VNumber ( options ), options, NULL );
		exit ( EXIT_FAILURE );
	}

	if ( resolution <= 0 )
		VError( " 'resolution' must be an integer > 0" );


	/*
	** Read the transformation matrix:
	*/
	fp = VOpenInputFile ( trans_filename, TRUE );
	list1 = VReadFile ( fp, NULL );

	if ( ! list1 ) VError( "Error reading image" );

	fclose( fp );

	for ( VFirstAttr ( list1, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		if ( strncmp( VGetAttrName( &posn ), "transform", 9 ) != 0 ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & trans );
		break;
	}

	if ( trans == NULL ) VError( "transformation matrix not found" );


	/*
	** check attributes
	*/
	if ( VGetAttr ( VImageAttrList ( trans ), "ca", NULL,
					VStringRepn, ( VPointer ) & ca ) != VAttrFound )
		VError( " attribute 'ca' missing in transformation matrix " );

	if ( VGetAttr ( VImageAttrList ( trans ), "cp", NULL,
					VStringRepn, ( VPointer ) & cp ) != VAttrFound )
		VError( " attribute 'cp' missing in transformation matrix " );

	if ( VGetAttr ( VImageAttrList ( trans ), "extent", NULL,
					VStringRepn, ( VPointer ) & extent ) != VAttrFound )
		VError( " attribute 'extent' missing in transformation matrix " );


	/*
	** open in-file
	*/
	if ( in_files.number < 1 || in_files.number > 1 )
		VError( " incorrect number of input files: %d", in_files.number );

	in_filename = ( ( VStringConst * ) in_files.vector )[0];

	if ( strcmp ( in_filename, "-" ) == 0 )
		in_file = stdin;
	else {
		in_file = fopen ( ( char * )in_filename, "r" );

		if ( ! in_file )
			VError ( "Failed to open input file %s", in_filename );
	}


	/*
	** read file info
	*/
	if ( ! ReadHeader ( in_file ) ) VError( "error reading header" );

	if ( ! ( list = ReadAttrList ( in_file ) ) )
		VError( "error reading attr list" );

	j = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		j++;
	}

	imageInfo = ( VImageInfo * ) VMalloc( sizeof( VImageInfo ) * ( j + 1 ) );

	nobject = nbands = found = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {

		str = VGetAttrName( &posn );

		if ( strncmp( str, "history", 7 ) == 0 ) {
			nobject++;
			continue;
		}

		VImageInfoIni( &imageInfo[nbands] );

		if ( ! VGetImageInfo( in_file, list, nobject, &imageInfo[nbands] ) )
			VError( " error reading image info" );


		if ( imageInfo[nbands].repn == VShortRepn ) {
			found = 1;
			nrows = imageInfo[nbands].nrows;
			ncols = imageInfo[nbands].ncolumns;
			ntimesteps = imageInfo[nbands].nbands;
			nbands++;
		}

		nobject++;
	}

	fclose( in_file );

	if( !found ) VError( " couldn't find functional data" );


	/*
	** process each time step
	*/
	dst_image = VFunctrans( in_filename, imageInfo, nbands, trans, resolution,
							ntimesteps, minval, compress, &dest_nbands );


	/*
	** output
	*/
	out_list = VCreateAttrList ();
	VHistory( VNumber( options ), options, prg_name, &list, &out_list );

	for ( j = 0; j < dest_nbands; j++ ) {
		VAppendAttr( out_list, "image", NULL, VImageRepn, dst_image[j] );
	}


	/* Open and write the output file: */
	if ( strcmp ( out_filename, "-" ) == 0 )
		out_file = stdout;
	else {
		out_file = fopen ( out_filename, "w" );

		if ( ! out_file )
			VError ( "Failed to open output file %s", out_filename );
	}

	if ( !VWriteFile( out_file, out_list ) || fclose( out_file ) )
		VSystemError( "error writing output file" );

	fprintf( stderr, "\n%s: done.\n", argv[0] );

	return ( EXIT_SUCCESS );
}

