/****************************************************************
 *
 * vextractparam:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Jane Neumann, 2004, <lipsia@cns.mpg.de>
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
 * $Id: vextractparam.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via/via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAXTIMESTEPS 2000

VDictEntry TALDict[] = {
	{ "maxtime", 0 },
	{ "mintime", 1 },
	{ "peaktime", 2 },
	{ "onsettime", 3 },
	{ "slope", 4 },
	{ "amp", 5 },
	{ "disp", 6 },
	{ NULL }
};

extern char *getLipsiaVersion();

int main ( int argc, char *argv[] )
{
	/* Command line options: */
	static VShort  type    =  0;
	static VString reportfile = "report.txt";
	static VShort onstart = 0;
	static VShort onend = 5;
	static VShort offstart = 5;
	static VShort offend = 15;
	static VFloat minval = 2000;
	static VFloat reso = 0.2;
	static VBoolean psc = TRUE;

	static VOptionDescRec options[] = {
		{
			"param", VShortRepn, 1, ( VPointer ) &type, VRequiredOpt, TALDict,
			"Parameter to be extracted (maxtime,mintime,peaktime,onsettime,slope,amp,disp)"
		},
		{
			"reso", VFloatRepn, 1, ( VPointer ) &reso,
			VOptionalOpt, NULL, "temporal resolution of time course (in sec)"
		},
		{
			"onstart", VShortRepn, 1, ( VPointer ) &onstart,
			VOptionalOpt, NULL, "first possible time for onset (in sec)"
		},
		{
			"onend", VShortRepn, 1, ( VPointer ) &onend,
			VOptionalOpt, NULL, "last possible time for onset (in sec)"
		},
		{
			"offstart", VShortRepn, 1, ( VPointer ) &offstart,
			VOptionalOpt, NULL, "first possible time for peak (in sec)"
		},
		{
			"offend", VShortRepn, 1, ( VPointer ) &offend,
			VOptionalOpt, NULL, "last possible time for peak (in sec)"
		},
		{
			"minval", VFloatRepn, 1, ( VPointer ) &minval,
			VRequiredOpt, NULL, "minval of first timestep"
		},
		{
			"psc", VBooleanRepn, 1, &psc,
			VOptionalOpt, NULL, "Whether to report percent signal change"
		},
		{
			"report", VStringRepn, 1, ( VPointer ) &reportfile,
			VOptionalOpt, NULL, "report file"
		}
	};


	FILE *in_file, *out_file, *rep_file;
	VAttrList in_list, out_list;
	VAttrListPosn posn;
	VImage src = NULL, map = NULL;

	int j = 0;
	int rr = 0, cc = 0, bb = 0, nrows = 0, ncols = 0, slice = 0, nslice = 0, fullslice = 0;
	int ntimesteps = 0;

	/* for determining minval */
	double mean, grandmean, rowmean, colmean, imagemean;

	double onset, peak, min, max;
	double derivmin, derivmax, deriv1[MAXTIMESTEPS], deriv2[MAXTIMESTEPS];
	double timecourse[MAXTIMESTEPS], glatt[MAXTIMESTEPS];
	int onsettime, peaktime, maxtime, mintime;

	/* for dispersion */
	double halfpeak;
	int firsttime, secondtime;

	/* for slope */
	double x, a, b, beta;

	/* for filtering */
	double u, ubefore, uafter, ufirst;
	double g, gbefore, gtwobefore, gafter, gtwoafter;

	/* for percent signal change */
	double sum, nxx, ave;

	char prg_name[50];
	sprintf( prg_name, "vextractparam V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );

	/* Parse command line arguments and identify files: */
	VParseFilterCmd( VNumber ( options ), options, argc, argv, &in_file, &out_file );

	if ( reso > 100 )
		VError( " reso parameter must be in seconds - not milliseconds" );

	/* Read the input file, return as attribute list */
	in_list = VReadFile ( in_file, NULL );

	if ( !in_list ) exit( 1 );

	fclose( in_file );

	/* Create attribute list for output image and r-value image */
	/* Make History */
	out_list = VCreateAttrList();
	VHistory( VNumber( options ), options, prg_name, &in_list, &out_list );


	nslice = 0;

	for ( VFirstAttr ( in_list, & posn ); VAttrExists( & posn ); VNextAttr( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) == VShortRepn ) {

			nslice++;

			if ( ( int )VImageNRows( src ) < 2 ) continue;

			fullslice++;


			/* compare rows, colums, timesteps of current slice with previous slice */
			if ( ntimesteps == 0 ) {
				ntimesteps = ( int )VImageNBands( src );
				nrows  = ( int )VImageNRows( src );
				ncols  = ( int )VImageNColumns( src );
			} else {
				if ( ntimesteps != VImageNBands( src ) ) VError( "Different number of timesteps in slices" );

				if ( nrows  != VImageNRows( src ) )      VError( "Different number of rows in slices" );

				if ( ncols  != VImageNColumns( src ) )   VError( "Different number of columns in slices" );
			}

			/* calculate grand mean */
			if ( minval == 0 ) {
				mean = 0;
				colmean = 0;
				rowmean = 0;
				imagemean = 0;

				for ( rr = 0; rr < nrows; rr++ ) {
					for ( cc = 0; cc < ncols; cc++ ) {
						for ( bb = 0; bb < ntimesteps; bb++ ) {
							mean = mean + VPixel( src, bb, rr, cc, VShort );
						}

						mean = mean / ntimesteps;
						colmean = colmean + mean;
					}

					colmean = colmean / ncols;
					rowmean = rowmean + colmean;
				}

				imagemean = rowmean / nrows;
			}
		}

		if ( minval == 0 ) grandmean += imagemean;
	}

	/*  if (minval == 0) minval = grandmean/(fullslice*8); */
	if ( minval == 0 ) minval = grandmean / fullslice;

	/* Check if there are functional slices */
	if ( nslice == 0 ) VError( "No functional slices" );

	/* write reportfile */
	rep_file = NULL;

	if ( strlen( reportfile ) > 2 ) {
		rep_file = fopen( reportfile, "w" );

		if ( rep_file == NULL ) VError( "Error opening report file %s", reportfile );

		fprintf( rep_file, "# Number of time points in timecourse:  %d\n", ntimesteps );
		fprintf( rep_file, "# Temporal resolution                :  %.2f\n", reso );
		fprintf( rep_file, "# Minval                             :  %.2f\n", minval );

		fprintf( rep_file, "# Area for onset time  (onstart/end) :  %d to %d\n", onstart, onend );
		fprintf( rep_file, "# Area for peak  time (offstart/end) :  %d to %d\n", offstart, offend );
		fprintf( rep_file, "\n" );
	} else
		VError( "Report filename not valid" );


	slice = -1;

	/* create map file, set voxel attribute */
	map = VCreateImage( nslice, nrows, ncols, VFloatRepn );
	VFillImage( map, VAllBands, 0 );

	/* get timecourse */
	for ( VFirstAttr ( in_list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) != VShortRepn ) continue;

		slice++;

		if ( ( int )VImageNRows( src ) < 2 ) continue;

		/* loop through all pixels */
		for ( rr = 0; rr < nrows; rr++ ) {
			for ( cc = 0; cc < ncols; cc++ ) {

				onset = 30000;
				min   = 30000;
				peak  = 0;
				max   = 0;
				onsettime = onstart / reso;
				mintime   = onstart / reso;
				peaktime  = offstart / reso;
				maxtime   = offstart / reso;

				/* for dispersion */
				halfpeak   = 0;
				firsttime  = 0;
				secondtime = 0;

				/* for percent signal change in amp */
				sum = nxx = ave = 0;

				ufirst = ( double ) VPixel( src, 0, rr, cc, VShort );


				/* find maxtime */
				for ( j = 0; j < ntimesteps; j++ ) {

					if ( psc ) {
						sum += ( double )VPixel( src, j, rr, cc, VShort );
						nxx++;
					}

					/* ignore timesteps outside range */
					if ( ( j * reso < offstart ) || ( j * reso > offend ) ) continue;

					u = ( double )VPixel( src, j, rr, cc, VShort );

					/* get new max und min time of timecourse */
					if ( u >  max )  {max = u ; maxtime = j;}
				}

				if ( psc )  ave = sum / nxx;


				/* find mintime */
				for ( j = 0; j < ntimesteps; j++ ) {

					/* ignore timesteps outside range */
					if ( ( j * reso < onstart ) || ( j * reso > onend ) ) continue;

					u = ( double )VPixel( src, j, rr, cc, VShort );

					/* get new max und min time of timecourse */
					if ( u <= min )  {min = u; mintime = j;}
				}

				/* time to minimum is after time to maximum */
				if ( maxtime <= mintime ) continue;


				/* gauss filter 1/16 (1 4 6 4 1) for smoothing */
				for ( j = 0; j < ntimesteps; j++ ) {

					glatt[j] = 0;
					timecourse[j] = 0;
					deriv1[j] = 0;
					deriv2[j] = 0;

					g = ( double )VPixel( src, j, rr, cc, VShort );

					/* save timecourse for printout */
					timecourse[j] = g;


					/* left boundary */
					if ( j == 0 ) {gbefore = g; gtwobefore = g;}
					else if ( j == 1 ) {
						gbefore = ( double )VPixel( src, j - 1, rr, cc, VShort );
						gtwobefore = gbefore;
					} else  {
						gbefore =   ( double )VPixel( src, j - 1, rr, cc, VShort );
						gtwobefore = ( double )VPixel( src, j - 2, rr, cc, VShort );
					}

					/* right boundary */
					if ( j == ntimesteps - 1 ) {gafter = g; gtwoafter = g;}
					else if ( j == ntimesteps - 2 ) {
						gafter = ( double )VPixel( src, j + 1, rr, cc, VShort );
						gtwoafter = gafter;
					} else  {
						gafter =   ( double )VPixel( src, j + 1, rr, cc, VShort );
						gtwoafter = ( double )VPixel( src, j + 2, rr, cc, VShort );
					}


					glatt[j] = ( double )( gtwobefore + 4 * gbefore + 6 * g + 4 * gafter + gtwoafter ) / 16;
				}


				derivmin = 30000;
				peaktime = maxtime;


				/* calculate derivatives, find peak */

				for ( j = 0; j < maxtime; j++ ) {

					/* ignore timesteps outside range */
					if ( ( j * reso < offstart ) || ( j * reso > offend ) ) continue;

					u = glatt[j];

					if ( j == 0 ) ubefore = u;
					else      ubefore = glatt[j - 1];

					if ( j == maxtime - 1 )  uafter = u;
					else               uafter = glatt[j + 1];


					deriv1[j] = ( double ) ( uafter - ubefore ) / 2;
					deriv2[j] = ( double ) uafter - 2 * u + ubefore;

					/* find minimum of second derivative corresponding to peak */
					if ( ( deriv2[j] < derivmin ) && ( deriv1[j] >= 0 ) )
						{derivmin =  deriv2[j] ; peaktime = j;}

				}


				derivmax = -30000;
				onsettime = mintime;

				/* find onset */
				for ( j = mintime; j < peaktime; j++ ) {

					/* ignore timesteps outside range */
					if ( ( j * reso < onstart ) || ( j * reso > onend ) ) continue;

					u = glatt[j];

					if ( j == 0 ) ubefore = u;
					else      ubefore = glatt[j - 1];

					if ( j == ntimesteps - 1 )  uafter = u;
					else                  uafter = glatt[j + 1];


					deriv1[j] = ( double ) ( uafter - ubefore ) / 2;
					deriv2[j] = ( double ) uafter - 2 * u + ubefore;

					if ( j == 1 ) derivmax = deriv2[1];

					/* find maximum of second derivative corresponding to onset */
					if ( ( deriv2[j] > derivmax ) && ( deriv1[j] >= 0 ) )
						{derivmax =  deriv2[j] ; onsettime = j;}

				}

				onset = glatt[onsettime];
				peak  = glatt[peaktime];



				/* calculate parameters of interest */
				if ( ufirst > minval )

					switch ( type ) {
					case 0: /* maxtime */

						VPixel( map, slice, rr, cc, VFloat ) = ( VFloat ) maxtime * reso;
						break;

					case 1: /* mintime */

						VPixel( map, slice, rr, cc, VFloat ) = ( VFloat ) mintime * reso;
						break;

					case 2: /* peaktime */

						VPixel( map, slice, rr, cc, VFloat ) = ( VFloat ) peaktime * reso;
						break;

					case 3: /* onsettime */

						VPixel( map, slice, rr, cc, VFloat ) = ( VFloat ) onsettime * reso;
						break;

					case 4: /* slope */

						a = ( peak - onset ) * ( peak - onset );
						b = ( peaktime - onsettime ) * ( peaktime - onsettime );
						x = sqrt( a + b );

						beta = asin( ( peak - onset ) / x ) / 3.14 * 180;

						VPixel( map, slice, rr, cc, VFloat ) = ( VFloat ) beta;

						break;

					case 5: /* amp */

						if ( psc ) {
							max  = 100.0f * ( max - ave ) / ave;
							min  = 100.0f * ( min - ave ) / ave;
						}

						VPixel( map, slice, rr, cc, VFloat ) = ( VFloat ) ( max - min );
						break;

					case 6: /* disp */

						halfpeak = min + ( ( max - min ) / 2 );

						/* get second time for halfpeak */
						for ( j = maxtime; j < ntimesteps; j++ ) {

							u = ( double )VPixel( src, j, rr, cc, VShort );

							if ( u <= halfpeak ) {secondtime = j; break;}
						}

						/* get first time for halfpeak */
						for ( j = maxtime; j > mintime; j-- ) {

							u = ( double )VPixel( src, j, rr, cc, VShort );

							if ( u <= halfpeak ) {firsttime = j; break;}
						}

						if ( ( secondtime - firsttime )*reso > 0 )
							VPixel( map, slice, rr, cc, VFloat ) = ( VFloat ) ( ( secondtime - firsttime ) * reso );

						break;

					default:
						VError( "Illegal parameter to be extracted" );
					}


			}
		}
	}

	/* add ttp-map to out-list */
	VCopyImageAttrs ( src, map );
	VSetAttr( VImageAttrList( map ), "modality", NULL, VStringRepn, "conimg" );
	VSetAttr( VImageAttrList( map ), "name", NULL, VStringRepn, "parameter map" );

	VAppendAttr( out_list, "image", NULL, VImageRepn, map );

	if ( ! VWriteFile ( out_file, out_list ) ) exit ( 1 );

	fprintf( stderr, "done\n" );

	fclose( rep_file );
	fclose( out_file );

	return 0;

}


