/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <stdio.h>
#include <math.h>
#include <via.h>



VImage
IniShift( VImage ref, VImage src, int minval, float shift[], int band1, int band2 )
{
	VImage dest;
	int b = 0, r = 0, c = 0;
	int bb, nbands3;
	int i, npixels;
	int nbands1, nrows1, ncols1;
	int nbands2, nrows2, ncols2;
	VUByte *dest_pp, *src_pp;
	float sumr = 0, sumc = 0, len = 0;
	float meanr, meanc;


	nbands1 = VImageNBands( ref );
	nrows1  = VImageNRows( ref );
	ncols1  = VImageNColumns( ref );

	nbands2 = VImageNBands( src );
	nrows2  = VImageNRows( src );
	ncols2  = VImageNColumns( src );

	if ( band1 < 0 || band2 >= nbands2 ) VError( "illegal choice of bands" );

	if ( band2 < band1 ) VError( "illegal choice of bands" );

	if ( VPixelRepn( ref ) != VUByteRepn ) VError( " ref must be ubyte" );

	if ( VPixelRepn( src ) != VUByteRepn ) VError( " src must be ubyte" );


	/*
	** get center of gravity of foreground voxels
	*/
	sumr = sumc = len = 0;

	for ( b = band1; b <= band2; b++ ) {
		for ( r = 0; r < nrows2; r++ ) {
			for ( c = 0; c < ncols2; c++ ) {
				if ( VPixel( src, b, r, c, VUByte ) > minval ) {
					sumr += r;
					sumc += c;
					len++;
				}
			}
		}
	}

	meanr = sumr / len;
	meanc = sumc / len;


	/*
	** initial shift: adjust center of gravity
	*/
	shift[0] = 0;
	shift[1] = ( float )nrows1 * 0.5  -  meanr;
	shift[2] = ( float )ncols1 * 0.5  -  meanc;

	/*
	** create output image, reshuffle slices
	*/
	nbands3 = band2 - band1 + 1;

	dest = VCreateImage( nbands3, nrows2, ncols2, VUByteRepn );
	VFillImage( dest, VAllBands, 0 );
	npixels = nrows2 * ncols2;

	bb = nbands3 - 1;

	for ( b = band1; b <= band2; b++ ) {
		dest_pp = ( VUByte * ) VPixelPtr( dest, bb, 0, 0 );
		src_pp  = ( VUByte * ) VPixelPtr( src, b, 0, 0 );

		for ( i = 0; i < npixels; i++ ) {
			*dest_pp++ = *src_pp++;
		}

		bb--;
	}

	return dest;
}

