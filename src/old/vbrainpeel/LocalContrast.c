#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/VImage.h>

/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ((a) < (b) ? (a) : (b))



VImage
VLocalContrast( VImage src, VImage dest, VShort wsize, VDouble sfactor, int background )
{
	int b = 0, r = 0, c = 0, nbands, nrows, ncols, npixels;
	int bb, rr, cc, b0, r0, c0, b1, r1, c1;
	int wn2, step;
	double sum1, sum2, nx, ave, u, u0, v, maxval;
	double var, sigma, factor;
	double sqrt( double ), rint( double );
	VRepnKind repn;
	VUByte *ubyte_pp;

	nbands = VImageNBands( src );
	nrows  = VImageNRows( src );
	ncols  = VImageNColumns( src );
	npixels = nbands * nrows * ncols;
	repn = VPixelRepn( src );

	factor = sfactor;
	maxval = VRepnMaxValue( repn );


	if ( dest == NULL )
		dest = VCreateImage( nbands, nrows, ncols, VPixelRepn( src ) );

	step = 3;

	if ( wsize < 10 && nbands == 1 ) step = 1;

	if ( wsize > 30 && nbands > 10 ) step = 5;

	wn2 = wsize / 2;

	for ( b = 0; b < nbands; b++ ) {
		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {

				u0 = VPixel( src, b, r, c, VUByte );

				if ( u0 < 1 ) {
					VPixel( dest, b, r, c, VUByte ) = 0;
					continue;
				}

				ave = var = sigma = sum1 = sum2 = nx = 0;

				b0 = b - wn2;

				if ( b0 < 0 ) b0 = 0;

				b1 = b + wn2;

				if ( b1 >= nbands ) b1 = nbands - 1;

				for ( bb = b0; bb <= b1; bb += step ) {

					r0 = r - wn2;

					if ( r0 < 0 ) r0 = 0;

					r1 = r + wn2;

					if ( r1 >= nrows ) r1 = nrows - 1;

					for ( rr = r0; rr <= r1; rr += step ) {

						c0 = c - wn2;

						if ( c0 < 0 ) c0 = 0;

						c1 = c + wn2;

						if ( c1 >= ncols ) c1 = ncols - 1;

						ubyte_pp = VPixelPtr( src, bb, rr, c0 );

						for ( cc = c0; cc <= c1; cc += step ) {
							u = *ubyte_pp;
							ubyte_pp += step;

							if ( u <= background ) continue;

							sum1 += u;
							sum2 += u * u;
							nx++;
						}
					}
				}

				if ( nx > 0 ) ave = sum1 / nx;

				if ( nx > 2 ) {
					var = ( sum2 - nx * ave * ave ) / ( nx - 1.0 );
					sigma = sqrt( var );
				}

				v = u0;

				if ( sigma > 0 ) {
					v = ( u0 - ave ) / ( sfactor * sigma ) + 1.0f;
					v = 128.0f * v;
				}

				v = rint( v );

				if ( v > 255 ) v = 255;

				if ( v < 0 )   v = 0;

				/* don't change background/foreground topology */
				if ( VPixel( src, b, r, c, VUByte ) > 0 && v == 0 ) v = 1;
				else if ( VPixel( src, b, r, c, VUByte ) == 0 && v != 0 ) v = 0;

				VPixel( dest, b, r, c, VUByte ) = v;
			}
		}
	}

	VCopyImageAttrs ( src, dest );
	return dest;
}
