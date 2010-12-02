#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x)*(x))

extern VImage VMedianImage2d( VImage, VImage, int );
extern VImage VFilterGauss2d ( VImage, VImage, double );
extern VImage VChamferDist2d ( VImage, VImage, VBand );
extern VImage VDTErode2d( VImage, VImage, VDouble );
extern VImage VDTClose2d( VImage, VImage, VDouble );



VImage
CloseGaps( VImage src, VImage dest, VDouble edist )
{
	VImage tmp = NULL, tmp1 = NULL;
	VFloat *src_pp;
	VBit *bin_pp;
	float sum, u, nx, tiny = 0.0001;
	int i, iter, b, r, c, rr, cc, wn = 1;
	int nslices, nrows, ncols;

	nslices = VImageNBands( src );
	nrows   = VImageNRows( src );
	ncols   = VImageNColumns( src );

	tmp = VCreateImage( nslices, nrows, ncols, VBitRepn );

	if ( edist > 0 ) {
		bin_pp = VImageData( tmp );
		src_pp = VImageData( src );

		for ( i = 0; i < VImageNPixels( src ); i++ ) {
			*bin_pp = 1;

			if ( ABS( *src_pp ) < 0.0001 ) *bin_pp = 0;

			src_pp++;
			bin_pp++;
		}

		tmp1 = VDTClose2d( tmp, NULL, ( VDouble )4 );
		tmp = VDTErode2d( tmp1, tmp, edist );

		bin_pp = VImageData( tmp );
		src_pp = VImageData( src );

		for ( i = 0; i < VImageNPixels( src ); i++ ) {
			if ( *bin_pp == 0 ) *src_pp = 0;

			src_pp++;
			bin_pp++;
		}
	}

	dest = VCopyImage( src, dest, VAllBands );


	for ( iter = 0; iter < 3; iter++ ) {

		for ( b = 0; b < nslices; b++ ) {
			for ( r = wn; r < nrows - wn; r++ ) {
				for ( c = wn; c < ncols - wn; c++ ) {
					u = VPixel( src, b, r, c, VFloat );

					if ( ABS( u ) >= tiny ) continue;

					sum = nx = 0;

					for ( rr = r - wn; rr <= r + wn; rr++ ) {
						for ( cc = c - wn; cc <= c + wn; cc++ ) {
							u = VPixel( src, b, rr, cc, VFloat );

							if ( ABS( u ) < tiny ) continue;

							sum += u;
							nx++;
						}
					}

					if ( nx < 1 ) continue;

					u = sum / nx;
					VPixel( dest, b, r, c, VFloat ) = u;
				}
			}
		}

		src = VCopyImage( dest, src, VAllBands );
	}

	return dest;
}






void
VDistortionCorrection( VAttrList list, VAttrList list_shift, VDouble sigma, VDouble edist )
{
	VImage src = NULL;
	VImage shift_image = NULL, tmp = NULL;
	VAttrListPosn posn;
	int b, r, c, i, nt, n, val, xmin, xmax;
	int nslices, nrows, ncols;

	double *xx = NULL, *yy = NULL, xi, yi, d;
	gsl_interp_accel *acc = NULL;
	gsl_spline *spline = NULL;



	/* get shift image*/
	for ( VFirstAttr ( list_shift, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & shift_image );

		if ( VPixelRepn( shift_image ) != VFloatRepn ) continue;

		break;
	}

	if ( shift_image == NULL ) VError( " no shift image found" );

	tmp = CloseGaps( shift_image, NULL, edist );
	shift_image = VFilterGauss2d ( tmp, shift_image, sigma );
	/* shift_image =  VMedianImage2d(tmp,shift_image,3); */

	/*
	{
	  FILE *fp;
	  VAttrList out_list;
	  fp = fopen("test.v","w");
	  out_list = VCreateAttrList();
	  VAppendAttr(out_list,"image",NULL,VImageRepn,shift_image);
	  VWriteFile (fp, out_list);
	  fclose(fp);
	}
	*/


	/*
	** process EPI-T1 image(s)
	*/
	nslices = nrows = ncols = i = n = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) != VUByteRepn ) continue;

		if ( VImageNBands( shift_image ) != VImageNBands( src ) ) continue;

		if ( VImageNRows( shift_image ) != VImageNRows( src ) ) continue;

		if ( VImageNColumns( shift_image ) != VImageNColumns( src ) ) continue;

		xmin = VRepnMinValue( VUByteRepn );
		xmax = VRepnMaxValue( VUByteRepn );

		nslices = VImageNBands( src );
		nrows   = VImageNRows( src );
		ncols   = VImageNColumns( src );

		if ( acc == NULL && nrows > 0 ) {
			acc = gsl_interp_accel_alloc ();
			n = nrows;
			spline = gsl_spline_alloc ( gsl_interp_akima, n );
			xx = ( double * ) VCalloc( n, sizeof( double ) );
			yy = ( double * ) VCalloc( n, sizeof( double ) );
		}

		for ( b = 0; b < nslices; b++ ) {
			for ( c = 0; c < ncols; c++ ) {

				for ( r = 0; r < nrows; r++ ) {
					xx[r] = r;
					yy[r] = VPixel( src, b, r, c, VUByte );
				}

				gsl_spline_init ( spline, xx, yy, n );

				for ( r = 0; r < nrows; r++ ) {
					d = VPixel( shift_image, b, r, c, VFloat );

					if ( ABS( d ) < 0.001 ) continue;

					xi = xx[r] - d;

					if ( xi < 0 ) xi = 0;

					if ( xi >= nrows ) xi = nrows;

					yi = gsl_spline_eval ( spline, xi, acc );
					val = ( int )( yi + 0.5 );

					if ( val < xmin ) val = xmin;

					if ( val > xmax ) val = xmax;

					VPixel( src, b, r, c, VUByte ) = val;
				}
			}
		}

		i++;
	}

	if ( i > 0 ) fprintf( stderr, " processed %d structural images\n", i );


	/*
	** process functional slices
	*/
	b = -1;
	nrows = ncols = 0;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		if ( VPixelRepn( src ) != VShortRepn ) continue;

		if ( VImageNRows( shift_image ) != VImageNRows( src ) ) continue;

		if ( VImageNColumns( shift_image ) != VImageNColumns( src ) ) continue;

		b++;
		fprintf( stderr, " slice: %3d\r", b );

		nt      = VImageNBands( src );  /* num timesteps */
		nrows   = VImageNRows( src );
		ncols   = VImageNColumns( src );

		if ( acc == NULL && nrows > 0 ) {
			acc = gsl_interp_accel_alloc ();
			spline = gsl_spline_alloc ( gsl_interp_akima, nrows );
			xx = ( double * ) VCalloc( nrows, sizeof( double ) );
			yy = ( double * ) VCalloc( nrows, sizeof( double ) );
		}

		xmin = VRepnMinValue( VShortRepn );
		xmax = VRepnMaxValue( VShortRepn );

		for ( c = 0; c < ncols; c++ ) {
			for ( i = 0; i < nt; i++ ) {

				for ( r = 0; r < nrows; r++ ) {
					xx[r] = r;
					yy[r] = VPixel( src, i, r, c, VShort );
				}

				gsl_spline_init ( spline, xx, yy, nrows );

				for ( r = 0; r < nrows; r++ ) {
					d = VPixel( shift_image, b, r, c, VFloat );

					if ( ABS( d ) < 0.001 ) continue;

					xi = xx[r] - d;

					if ( xi < 0 ) xi = 0;

					if ( xi >= nrows ) xi = nrows;

					yi = gsl_spline_eval ( spline, xi, acc );
					val = ( int )( yi + 0.5 );

					if ( val < xmin ) val = xmin;

					if ( val > xmax ) val = xmax;

					VPixel( src, i, r, c, VShort ) = val;
				}
			}
		}
	}

	if ( b >= 0 )
		fprintf( stderr, " processed %d functional slices\n", b + 1 );
}
