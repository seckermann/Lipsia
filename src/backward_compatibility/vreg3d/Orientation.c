/*
 *
 *  convert a axial image to a coronal or sagittal (for vreg3d)
 *
 *  G.Lohmann, <lohmann@cns.mpg.de>, Dec 1998
 */

#include <stdio.h>
#include <stdlib.h>


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/VImage.h>

VImage
VAxial2Coronal( VImage src )
{
	VImage dest = NULL;
	int nbands, nrows, ncols, b, r, c;
	VDouble v = 0;

	nbands  = VImageNBands( src );
	nrows   = VImageNRows( src );
	ncols   = VImageNColumns( src );

	dest = VCreateImage( nrows, nbands, ncols, VPixelRepn( src ) );

	if ( dest == NULL ) VError( " error converting ax to cor" );

	for ( b = 0; b < nbands; b++ ) {
		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				v = VGetPixel( src, b, r, c );
				VSetPixel( dest, nrows - r - 1, b, c, v );
			}
		}
	}

	return dest;
}



VImage
VAxial2Sagittal( VImage src )
{
	VImage dest = NULL;
	int nbands, nrows, ncols, b, r, c;
	VDouble v = 0;

	nbands  = VImageNBands( src );
	nrows   = VImageNRows( src );
	ncols   = VImageNColumns( src );

	dest = VCreateImage( ncols, nbands, nrows, VPixelRepn( src ) );

	if ( dest == NULL ) VError( " error converting axial to sagittal" );

	for ( b = 0; b < nbands; b++ ) {
		for ( r = 0; r < nrows; r++ ) {
			for ( c = 0; c < ncols; c++ ) {
				v = VGetPixel( src, b, r, c );
				VSetPixel( dest, ncols - c - 1, b, r, v );
			}
		}
	}

	return dest;
}


