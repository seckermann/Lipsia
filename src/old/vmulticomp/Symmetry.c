#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>


#define CSIZE   1200   /* max cluster size */

#define ABS(x) ((x) > 0 ? (x) : -(x))


void
VSymmetry( VImage label_image, VImage zimage, int nl, double zthr, float *clustersym )
{
	int i, j, k, m;
	int b, r, c, c2, nbands, nrows, ncols;
	float u, sumc = 0, cog = 0;
	VShort *ptr;

	nbands = VImageNBands( label_image );
	nrows = VImageNRows( label_image );
	ncols = VImageNColumns( label_image );

	c2 = ncols / 2;


	for ( k = 1; k <= nl; k++ ) {
		ptr = VImageData( label_image );

		sumc = 0;

		i = j = 0;

		for ( b = 0; b < nbands; b++ ) {
			for ( r = 0; r < nrows; r++ ) {
				for ( c = 0; c < ncols; c++ ) {
					m = *ptr++;

					if ( m != k ) continue;

					sumc += c;

					i++;

					if ( VPixel( zimage, b, r, ABS( ncols - c ), VFloat ) > zthr ) j++;
				}
			}
		}

		cog = sumc / ( float )i;

		if ( ABS( cog - c2 ) < 3 ) continue; /* ignore median wall */

		u = 0;

		if ( i > 10 ) u = ( float )j / ( float )i;

		clustersym[k] = u;
	}
}
