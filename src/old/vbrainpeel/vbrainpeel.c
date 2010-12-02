/****************************************************************
 *
 * Program: vbrainpeel
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2005, <lipsia@cbs.mpg.de>
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
 * $Id: vbrainpeel.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/VImage.h"
#include "viaio/mu.h"
#include "viaio/option.h"


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


extern VImage VPeel( VImage, VImage, VDouble, VDouble, VFloat, VShort, VShort );
extern char *getLipsiaVersion();

int main ( int argc, char *argv[] )
{
	static VDouble d1   = 0;
	static VDouble d2   = 3.0;
	static VShort threshold = 155;
	static VShort background = 30;
	static VFloat edge  = 20;
	static VOptionDescRec options[] = {
		{ "d1", VDoubleRepn, 1, &d1, VOptionalOpt, 0, "erosion" },
		{ "d2", VDoubleRepn, 1, &d2, VOptionalOpt, 0, "dilation" },
		{ "t", VShortRepn, 1, &threshold, VOptionalOpt, 0, "threshold" },
		{ "background", VShortRepn, 1, &background, VOptionalOpt, 0, " image background" },
		{ "edge", VFloatRepn, 1, &edge, VOptionalOpt, 0, "edge strength" }
	};
	VAttrList list;
	VAttrListPosn posn;
	VImage src = NULL, dst = NULL;
	FILE *in_file, *out_file;
	char prg[50];
	sprintf( prg, "vbrainpeel V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg );

	VParseFilterCmd( VNumber( options ), options, argc, argv, &in_file, &out_file );

	/* Read source image(s): */
	if ( ! ( list = VReadFile( in_file, NULL ) ) ) return 1;

	fclose( in_file );

	/* Operate on each source image: */
	for ( VFirstAttr( list, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
		if ( VGetAttrRepn( &posn ) == VImageRepn )  {
			VGetAttrValue( &posn, NULL, VImageRepn, &src );
			dst = VPeel( src, NULL, d1, d2, edge, threshold, background );

			if ( dst ) VSetAttrValue( &posn, NULL, VImageRepn, dst );
			else VError( " error in vbrainpeel" );
		}
	}

	/* Write the results to the output file: */
	VHistory( VNumber( options ), options, prg, &list, &list );

	if ( !VWriteFile( out_file, list ) ) return 1;

	fprintf( stderr, " %s: done.\n", argv[0] );
	return 0;
}

