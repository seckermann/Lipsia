/****************************************************************
 *
 * vconvcoord:
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
 * $Id: vconvcoord.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <via.h>

VDictEntry ConvDict[] = {
	{ "vox2tal", 0 },
	{ "tal2vox", 1 },
	{ NULL }
};

extern void VTal2Pixel( float [3], float [3], float [3], float, float, float, int *, int *, int * );
extern void VPixel2Tal( float [3], float [3], float [3], int, int, int, float *, float *, float * );
extern char *getLipsiaVersion();

int
main ( int argc, char *argv[] )
{
	static VFloat reso = 1;
	static VShort type = 0;
	static VOptionDescRec  options[] = {
		{"resolution", VFloatRepn, 1, ( VPointer ) &reso, VOptionalOpt, NULL, "Voxel resolution in mm"},
		{
			"type", VShortRepn, 1, ( VPointer ) &type, VOptionalOpt, ConvDict,
			"Type of conversion, 'voxel to Talairach' vs. 'Talairach to voxel'"
		}
	};
	FILE *in_file;
	char *line;
	int   n = 80;
	int   b, r, c;
	float x, y, z;
	float ca[3], extent[3], voxel[3];
	char prg_name[50];
	sprintf( prg_name, "vconvcoord V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );


	VParseFilterCmd ( VNumber ( options ), options, argc, argv, &in_file, NULL );

	extent[0] = 135;
	extent[1] = 175;
	extent[2] = 118;

	voxel[0] = reso;
	voxel[1] = reso;
	voxel[2] = reso;

	ca[0] = 80;
	ca[1] = 81;
	ca[2] = 90;

	line = ( char * ) VMalloc( 80 );

	while ( fgets( line, n, in_file ) ) {
		if ( strlen( line ) < 2 ) continue;

		if ( line[0] == '#' ) continue;

		switch ( type ) {
		case 0:
			sscanf( line, "%d %d %d", &c, &r, &b );
			VPixel2Tal( ca, voxel, extent, b, r, c, &x, &y, &z );
			fprintf( stderr, " %3d  %3d  %3d:   %.0f, %.0f, %.0f\n", c, r, b, x, y, z );
			break;

		case 1:
			sscanf( line, "%f %f %f", &x, &y, &z );
			VTal2Pixel( ca, voxel, extent, x, y, z, &b, &r, &c );
			fprintf( stderr, " %8.2f %8.2f %8.2f:   %3d  %3d  %3d\n", x, y, z, c, r, b );
			break;
		default:
			VError( "illegal conversion type" );
		}
	}

	fclose( in_file );

	return 0;
}
