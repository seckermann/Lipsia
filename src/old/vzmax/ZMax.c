/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <via.h>


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern void VPixel2Tal( float [3], float [3], float [3], int, int, int, float *, float *, float * );

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

void
VPaintVolume( Volume vol, VImage zmap, VImage dest, int id, int *band, int *row, int *col )
{
	VTrack t;
	int   b, r, c, c0, c1, i;
	int   bx = 0, rx = 0, cx = 0, bb, rr, cc;
	float u, xmin, xmax;
	float sumx, sumy, sumz, nx, tiny = 1.0e-3;

	*band = *row = *col = 0;
	bx = rx = cx = 0;

	/* get blob center, and paint dest image */
	xmax = xmin = 0;

	for ( i = 0; i < vol->nbuckets; i++ ) {
		for ( t = vol->bucket[i].first; t != NULL; t = t->next )  {
			c0 = t->col;
			c1 = t->col + t->length;

			for ( c = c0; c < c1; c++ ) {
				u = VPixel( zmap, t->band, t->row, c, VFloat );
				VPixel( dest, t->band, t->row, c, VFloat ) = u;

				if ( u > xmax && u > 0 ) {
					xmax = u;
				}

				if ( u < xmin && u < 0 ) {
					xmin = u;
				}
			}
		}
	}

	/* get blob center */
	sumx = sumy = sumz = nx = 0;

	for ( i = 0; i < vol->nbuckets; i++ ) {
		for ( t = vol->bucket[i].first; t != NULL; t = t->next )  {
			c0 = t->col;
			c1 = t->col + t->length;

			for ( c = c0; c < c1; c++ ) {
				u = VPixel( zmap, t->band, t->row, c, VFloat );

				if ( u >= xmax - tiny && u > 0 ) {
					bx = t->band;
					rx = t->row;
					cx = c;
					sumx += cx;
					sumy += rx;
					sumz += bx;
					nx++;
				}

				if ( u <= xmin + tiny && u < 0 ) {
					bx = t->band;
					rx = t->row;
					cx = c;
					sumx += cx;
					sumy += rx;
					sumz += bx;
					nx++;
				}
			}
		}
	}

	if ( nx < 0.5 ) VError( " err in VZMax" );

	bx = VRint( sumz / nx );
	rx = VRint( sumy / nx );
	cx = VRint( sumx / nx );

	b = bx;
	r = rx;
	c = cx;

	xmax = VPixel( zmap, bx, rx, cx, VFloat );
	xmin = xmax;

	for ( bb = b - 1; bb <= b + 1; bb++ ) {
		for ( rr = r - 1; rr <= r + 1; rr++ ) {
			for ( cc = c - 1; cc <= c + 1; cc++ ) {
				if ( ABS( b - bb ) + ABS( r - rr ) + ABS( c - cc ) > 2 ) continue;

				u = VPixel( zmap, bb, rr, cc, VFloat );

				if ( ( u > xmax && u > 0 ) || ( u < xmin && u < 0 ) ) {
					xmax = u;
					xmin = u;
					bx = bb;
					rx = rr;
					cx = cc;
				}
			}
		}
	}

	*band = bx;
	*row  = rx;
	*col  = cx;
}



VImage
VZMax( VImage zmap, VImage dest, VDouble pos, VDouble neg, VDouble threshold,
	   VShort minsize, VFloat radius, VShort type, VString filename )
{
	FILE *fp = NULL;
	VImage label_image = NULL, tmp = NULL, tmp1 = NULL, dest_tmp = NULL;
	Volumes volumes = NULL;
	Volume vol = NULL;
	VDouble xmin, xmax;
	VBit  *bin_pp;
	VShort *short_pp;
	VString str;
	int   nbands, nrows, ncols, b, r, c, bb, rr, cc, rad;
	int   b0, r0, c0;
	int   nl = 0, id = 0, iter, i0 = 0, i;
	int   minvoxelsize = 0;
	float voxel[3], ca[3], extent[3];
	float x0, x1, x2, x, y, z, u, v;
	float d, rad2;


	nrows  = VImageNRows ( zmap );
	ncols  = VImageNColumns ( zmap );
	nbands = VImageNBands ( zmap );

	/*
	** create output image
	*/
	dest = VSelectDestImage( "VZMax", dest, nbands, nrows, ncols, VFloatRepn );

	if ( ! dest ) VError( " err creating dest image" );

	VCopyImageAttrs ( zmap, dest );
	VFillImage( dest, VAllBands, 0 );

	dest_tmp = VCreateImage( nbands, nrows, ncols, VBitRepn );
	VFillImage( dest_tmp, VAllBands, 0 );


	/*
	** read image parameters
	*/
	str = VMalloc( 80 );

	if ( VGetAttr( VImageAttrList( zmap ), "ca", NULL, VStringRepn, ( VPointer ) &str ) != VAttrFound )
		VError( " attribute 'ca' not found" );

	sscanf( str, "%f %f %f", &x0, &x1, &x2 );
	ca[0] = x0;
	ca[1] = x1;
	ca[2] = x2;

	if ( VGetAttr( VImageAttrList( zmap ), "voxel", NULL, VStringRepn, ( VPointer ) &str ) != VAttrFound )
		VError( " attribute 'voxel' not found" );

	sscanf( str, "%f %f %f", &x0, &x1, &x2 );
	voxel[0] = x0;
	voxel[1] = x1;
	voxel[2] = x2;
	minvoxelsize = VRint( ( float )minsize / ( voxel[0] * voxel[1] * voxel[2] ) );

	if ( minvoxelsize < 1 && minsize > 0 ) {
		VWarning( " parameter 'minsize' has no effect (0 voxels). Units are in mm^3 !" );
	}

	if ( VGetAttr( VImageAttrList( zmap ), "extent", NULL, VStringRepn, ( VPointer ) &str ) != VAttrFound )
		VError( " attribute 'extent' not found" );

	sscanf( str, "%f %f %f", &x0, &x1, &x2 );
	extent[0] = x0;
	extent[1] = x1;
	extent[2] = x2;


	radius = radius / voxel[1];
	rad  = ( int ) ceil( ( double ) radius );
	rad2 = radius * radius;

	/*
	** process
	*/
	if ( strlen( filename ) > 2 ) {
		fp = fopen( filename, "w" );

		if ( !fp ) VError( " error opening %s", filename );
	} else
		fp = stderr;

	fprintf( fp, "    id       location              z-val\n" );
	fprintf( fp, "-----------------------------------------\n" );

	id = 1;

	for ( iter = 0; iter < 2; iter++ ) {
		VFillImage( dest_tmp, VAllBands, 0 );

		if ( iter == 0 ) { /* positive activations */
			xmin = pos;
			xmax = VRepnMaxValue( VFloatRepn );
		} else {          /* negative activations */
			xmin = VRepnMinValue( VFloatRepn );
			xmax = neg;
		}

		/* delete small areas */
		tmp = VBinarizeImage ( zmap, tmp, ( VDouble ) xmin, ( VDouble ) xmax );

		if ( minvoxelsize > 0 ) {
			label_image = VLabelImage3d( tmp, label_image, ( int )26, VShortRepn, &nl );
			tmp = VDeleteSmall ( label_image, tmp, minvoxelsize );
		}

		for ( b = rad; b < nbands - rad; b++ ) {
			for ( r = rad; r < nrows - rad; r++ ) {
				for ( c = rad; c < ncols - rad; c++ ) {

					if ( VPixel( tmp, b, r, c, VBit ) == 0 ) continue;

					v = 0;
					u = VPixel( zmap, b, r, c, VFloat );

					if ( iter == 0 && u < pos ) continue;

					if ( iter == 1 && u > neg ) continue;

					for ( bb = b - radius; bb <= b + radius; bb++ ) {
						for ( rr = r - radius; rr <= r + radius; rr++ ) {
							for ( cc = c - radius; cc <= c + radius; cc++ ) {
								d = SQR( b - bb ) + SQR( r - rr ) + SQR( c - cc );

								if ( d > rad2 ) continue;

								v = VPixel( zmap, bb, rr, cc, VFloat );

								if ( iter == 0 && v > u ) goto next;

								if ( iter == 1 && v < u ) goto next;
							}
						}
					}

					VPixel( dest_tmp, b, r, c, VBit ) = 1;

					if ( threshold > 0 ) {
						if ( iter == 0 )
							tmp1 = VBinarizeImage ( zmap, tmp1, ( VDouble ) u - threshold, ( VDouble ) u );
						else
							tmp1 = VBinarizeImage ( zmap, tmp1, ( VDouble ) u, ( VDouble ) u + threshold );

						for ( bb = b - radius; bb <= b + radius; bb++ ) {
							for ( rr = r - radius; rr <= r + radius; rr++ ) {
								for ( cc = c - radius; cc <= c + radius; cc++ ) {
									d = SQR( b - bb ) + SQR( r - rr ) + SQR( c - cc );

									if ( d >= rad2 ) VPixel( tmp1, bb, rr, cc, VBit ) = 0;
								}
							}
						}

						label_image = VLabelImage3d( tmp1, label_image, ( int )26, VShortRepn, &nl );

						if ( nl == 0 ) continue;

						i0 = VPixel( label_image, b, r, c, VShort );
						short_pp = VImageData( label_image );
						bin_pp = VImageData( dest_tmp );

						for ( i = 0; i < VImageNPixels( zmap ); i++ ) {
							if ( *short_pp == i0 ) *bin_pp = 1;

							bin_pp++;
							short_pp++;
						}
					}

next:
					;
				}
			}
		}

		label_image = VLabelImage3d( dest_tmp, label_image, ( int )26, VShortRepn, &nl );

		if ( nl == 0 ) continue;

		volumes = VImage2Volumes( label_image );

		for ( vol = volumes->first; vol != NULL; vol = vol->next ) {
			VPaintVolume( vol, zmap, dest, id, &b, &r, &c );

			u = VPixel( zmap, b, r, c, VFloat );

			switch ( type ) {
			case 0:
				c0 = c;
				r0 = r;
				b0 = b;
				break;

			case 1:
				c0 = VRint( ( float )c * voxel[2] );
				r0 = VRint( ( float )r * voxel[1] );
				b0 = VRint( ( float )b * voxel[0] );
				break;

			case 2:
				VPixel2Tal( ca, voxel, extent, b, r, c, &x, &y, &z );
				c0 = VRint( x );
				r0 = VRint( y );
				b0 = VRint( z );
				break;

			default:
				VError( " vzmax: illegal type" );
			}

			fprintf( fp, " %5d:  %5d %5d %5d,     %8.3f\n", id, c0, r0, b0, u );
			id++;
		}
	}

	if ( fp != stderr ) fclose( fp );

	return dest;
}
