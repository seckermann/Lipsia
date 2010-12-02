/*
** 3D Lee filter
**
** G.Lohmann, MPI-CBS
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))


VImage
VLeeImage3d( VImage src, VImage dest, VLong wsize, VDouble sigma )
{
	int b, r, c, nbands, nrows, ncols;
	int bb, rr, cc, b0, b1, r0, r1, c0, c1;
	int wn, u, v;
	float sum, nx;

	nrows = VImageNRows ( src );
	ncols = VImageNColumns ( src );
	nbands = VImageNBands ( src );

	dest = VCopyImage( src, dest, VAllBands );

	wn = wsize / 2;

	for ( b = 0; b < nbands; b++ ) {
		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {

				u = VPixel( src, b, r, c, VUByte );

				sum = nx = v = 0;
				b0 = b - wn;

				if ( b0 < 0 ) b0 = 0;

				b1 = b + wn;

				if ( b1 >= nbands ) b1 = nbands - 1;

				for ( bb = b0; bb <= b1; bb++ ) {

					r0 = r - wn;

					if ( r0 < 0 ) r0 = 0;

					r1 = r + wn;

					if ( r1 >= nrows ) r1 = nrows - 1;

					for ( rr = r0; rr <= r1; rr++ ) {

						c0 = c - wn;

						if ( c0 < 0 ) c0 = 0;

						c1 = c + wn;

						if ( c1 >= ncols ) c1 = ncols - 1;

						for ( cc = c0; cc <= c1; cc++ ) {

							v = VPixel( src, bb, rr, cc, VUByte );

							if ( v == 0 ) continue;

							if ( ABS( v - u ) > sigma ) continue;

							sum += v;
							nx++;
						}
					}
				}

				if ( nx > 0 ) v = VRint( ( sum / nx ) );

				/* topology preservation */
				if ( u > 0 && v == 0 ) v = 1;
				else if ( u == 0 && v > 0 ) v = 0;

				VPixel( dest, b, r, c, VUByte ) = v;
			}
		}
	}

	VCopyImageAttrs ( src, dest );
	return dest;
}
