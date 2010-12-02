/****************************************************************
 *
 * vseltimesteps:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2000, <lipsia@cbs.mpg.de>
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
 * $Id: vseltimesteps.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include "viaio/VImage.h"



/* Later in this file: */
static VImage SelectBands ( VImage );
static VBoolean Included ( int, VArgVector *, VDictEntry * );
static int Decode ( VStringConst, VDictEntry * );
extern char *getLipsiaVersion();

/* Command line options: */
static VArgVector band_args = {NULL};
static VBoolean band_found = FALSE;
static VArgVector frame_args = {NULL}, viewpt_args = {NULL}, color_args = {NULL}, comp_args = {NULL};
static VBoolean frame_found = FALSE, viewpt_found = FALSE, color_found = FALSE, comp_found = FALSE;

static VOptionDescRec options[] = {
	{
		"step", VStringRepn, 0, & band_args, & band_found, NULL,
		"Timesteps to be selected"
	}
};

/* Dictionaries of names that can be used in place of band numbers: */
static VDictEntry viewptDict[] = {
	{ "left", 0 }, { "right", 1 },
	NULL
};
static VDictEntry colorDict[] = {
	{ "red", 0 }, { "green", 1 }, { "blue", 2 },
	NULL
};
static VDictEntry compDict[] = {
	{ "real", 0 }, { "imaginary", 1 },
	{ "x", 0 }, { "y", 1 }, { "z", 2 },
	NULL
};

void
UpdateScanBorders ( VImage src )
{
	VString tmpstring = NULL, string = NULL, str = NULL;
	char *token = NULL;
	double tr = 0;

	if ( VGetAttr( VImageAttrList( src ), "MPIL_vista_0", 0, VStringRepn, &string ) == VAttrFound ) {
		tmpstring = VNewString( string );

		token = strtok( tmpstring, "=" );

		if ( strncmp( token, " repetition_time", ( size_t ) 10 ) == 0 ) {
			token = strtok( NULL, " " );
			tr = ( double ) atoi( token );
		}

		str = VMalloc( 80 );
		sprintf( str, " repetition_time=%g packed_data=1 %d ", tr, VImageNBands( src ) );

		VSetAttr( VImageAttrList( src ), "MPIL_vista_0", NULL, VStringRepn, str );
		VSetAttr( VImageAttrList( src ), "ntimesteps", NULL, VShortRepn, ( VShort )VImageNBands( src ) );
	}

	return;
}


/*
 *  Program entry point.
 */

int main ( int argc, char *argv[] )
{
	FILE *in_file, *out_file;
	VAttrList list;
	VAttrListPosn posn;
	VImage src, result;
	int nimages = 0, slice = 0;

	char prg_name[50];
	sprintf( prg_name, "vseltimesteps V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );


	/* Parse command line arguments: */
	VParseFilterCmd ( VNumber ( options ), options, argc, argv,
					  & in_file, & out_file );

	/* Require EITHER -band OR some of -frame, -viewpoint, etc.: */
	if ( ! ( band_found ) )
		VError ( "parameter '-step' must be specified." );

	/* Read source image(s): */
	if ( ! ( list = VReadFile ( in_file, NULL ) ) )
		exit ( EXIT_FAILURE );

	/* Select specified bands out of each image: */
	slice = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) != VShortRepn ) continue;

		slice++;

		if ( VImageNRows( src ) > 2 ) {
			result = SelectBands ( src );
			UpdateScanBorders ( result );
			VDestroyImage ( src );
			VSetAttrValue ( & posn, NULL, VImageRepn, result );
		} else {
			UpdateScanBorders ( src );
			VSetAttrValue ( & posn, NULL, VImageRepn, src );
		}

		nimages++;
	}

	/* Make History */
	VHistory( VNumber( options ), options, prg_name, &list, &list );

	/* Write the results to the output file: */
	if ( ! VWriteFile ( out_file, list ) )
		exit ( EXIT_FAILURE );

	fprintf ( stderr, "%s: Processed %d image%s.\n", argv[0], nimages,
			  nimages == 1 ? "" : "s" );

	return EXIT_SUCCESS;
}


/*
 *  SelectBands
 *
 *  Create a new image containing just the bands specified by the
 *  program options.
 */

static VImage SelectBands ( VImage src )
{
	int band, *bands, nbands = 0;
	int frame, viewpt, color, comp, nframes, nviewpts, ncolors, ncomps;
	VImage dest;
	VBandInterp frame_interp, viewpt_interp, color_interp, comp_interp;

	/* Make a list of bands to be copied to the destination image: */
	bands = VMalloc ( VImageNBands ( src ) * sizeof ( int ) );

	if ( band_found ) {
		for ( band = 0; band < VImageNBands ( src ); band++ )
			if ( Included ( band, & band_args, NULL ) )
				bands[nbands++] = band;
	} else {
		for ( frame = 0; frame < VImageNFrames ( src ); frame++ ) {
			if ( ! Included ( frame, & frame_args, NULL ) )
				continue;

			for ( viewpt = 0; viewpt < VImageNViewpoints ( src ); viewpt++ ) {
				if ( ! Included ( viewpt, & viewpt_args, viewptDict ) )
					continue;

				for ( color = 0; color < VImageNColors ( src ); color++ ) {
					if ( ! Included ( color, & color_args, colorDict ) )
						continue;

					for ( comp = 0; comp < VImageNComponents ( src ); comp++ )
						if ( Included ( comp, & comp_args, compDict ) )
							bands[nbands++] =
								VBandIndex ( src, frame, viewpt, color, comp );
				}
			}
		}
	}

	/* It could be that no bands were selected: */
	if ( nbands == 0 ) {
		VFree ( bands );
		return NULL;
	}

	/* Create the destination image and copy over the bands: */
	dest = VCreateImage ( nbands, VImageNRows ( src ), VImageNColumns ( src ),
						  VPixelRepn ( src ) );

	if ( ! dest )
		exit ( EXIT_FAILURE );

	for ( band = 0; band < nbands; band++ )
		if ( ! VCopyBand ( src, bands[band], dest, band ) )
			exit ( EXIT_FAILURE );

	/* Copy over image attributes that are still valid: */
	VCopyImageAttrs ( src, dest );

	if ( ! band_found ) {

		/* Determine how many frames, viewpoints, etc. have been copied
		   from the source image. If all frames, etc., have been copied,
		   also copy the frame interpretation attribute. */
		for ( nframes = frame = 0; frame < VImageNFrames ( src ); frame++ )
			nframes += Included ( frame, & frame_args, NULL );

		frame_interp = ( nframes == VImageNFrames ( src ) ) ?
					   VImageFrameInterp ( src ) : VBandInterpNone;

		for ( nviewpts = viewpt = 0; viewpt < VImageNViewpoints ( src ); viewpt++ )
			nviewpts += Included ( viewpt, & viewpt_args, viewptDict );

		viewpt_interp = ( nviewpts == VImageNViewpoints ( src ) ) ?
						VImageViewpointInterp ( src ) : VBandInterpNone;

		for ( ncolors = color = 0; color < VImageNColors ( src ); color++ )
			ncolors += Included ( color, & color_args, colorDict );

		color_interp = ( ncolors == VImageNColors ( src ) ) ?
					   VImageColorInterp ( src ) : VBandInterpNone;

		for ( ncomps = comp = 0; comp < VImageNComponents ( src ); comp++ )
			ncomps += Included ( comp, & comp_args, compDict );

		comp_interp = ( ncomps == VImageNComponents ( src ) ) ?
					  VImageComponentInterp ( src ) : VBandInterpNone;

		VSetBandInterp ( dest,
						 frame_interp, nframes,
						 viewpt_interp, nviewpts,
						 color_interp, ncolors,
						 comp_interp, ncomps );
	}

	VFree ( bands );
	return dest;
}


/*
 *  Included
 *
 *  Returns TRUE if a particular index value is, according to the command
 *  line, to be included in the output image.
 */

static VBoolean Included ( int v, VArgVector *vec, VDictEntry *dict )
{
	int i, lb, ub;
	VStringConst str;

	if ( vec->number == 0 )
		return TRUE;

	for ( i = 0; i < vec->number; i++ ) {
		ub = lb = Decode ( ( ( VStringConst * ) vec->vector )[i], dict );

		if ( i < vec->number - 2 ) {
			str = ( ( VStringConst * ) vec->vector )[i + 1];

			if ( strcmp ( str, "to" ) == 0 ||
				 strcmp ( str, "-" ) == 0 ||
				 strcmp ( str, ":" ) == 0 ) {
				ub = Decode ( ( ( VStringConst * ) vec->vector )[i + 2], dict );

				if ( lb > ub )
					VError ( "\"%s %s %s\" isn't a valid range",
							 ( ( VStringConst * ) vec->vector )[i],
							 ( ( VStringConst * ) vec->vector )[i + 1],
							 ( ( VStringConst * ) vec->vector )[i + 2] );

				i += 2;
			}
		}

		if ( v >= lb && v <= ub )
			return TRUE;
	}

	return FALSE;
}


/*
 *  Decode
 *
 *  Decode an index value from the command line, with the help of a dictionary.
 */

static int Decode ( VStringConst str, VDictEntry *dict )
{
	VLong value;

	if ( ! VDecodeAttrValue ( str, dict, VLongRepn, & value ) )
		VError ( "Invalid index argument \"%s\"", str );

	return value;
}
