/****************************************************************
 *
 * Program: vattrcopy
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
 * $Id: vattrcopy.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>


/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>

extern char *getLipsiaVersion();

int main ( int argc, char *argv[] )
{
	static VString filename = "";
	static VOptionDescRec options[] = {
		{
			"image", VStringRepn, 1, ( VPointer ) &filename, VRequiredOpt, NULL,
			"image whose header info is copied"
		}
	};

	FILE *in_file, *out_file, *fp = NULL;
	VAttrList list1, list2;
	VAttrListPosn posn;
	VImage src1, src2;
	char prg_name[50];
	sprintf( prg_name, "vattrcopy V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );

	/* Parse command line arguments and identify files: */
	VParseFilterCmd ( VNumber ( options ), options, argc, argv, &in_file, & out_file );



	/* Read 2nd image: */
	fp = VOpenInputFile ( filename, TRUE );
	list2 = VReadFile ( fp, NULL );

	if ( ! list2 )  VError( "Error reading %s", filename );

	fclose( fp );

	for ( VFirstAttr ( list2, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src2 );
		break;
	}



	/* Read 1st image: */
	if ( ! ( list1 = VReadFile ( in_file, NULL ) ) ) VError( " error reading file" );

	for ( VFirstAttr ( list1, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src1 );
		VDestroyAttrList( VImageAttrList ( src1 ) );
		VImageAttrList ( src1 ) = VCopyAttrList ( VImageAttrList ( src2 ) );
		break;
	}



	/* Write out the results: */
	VHistory( VNumber( options ), options, prg_name, &list1, &list2 );

	if ( ! VWriteFile ( out_file, list1 ) ) exit ( 1 );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return 0;
}
