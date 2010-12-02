/*
** get transformation matrix for co-registration
** refined version: arbitrary slices (not only axial)
**
** G.Lohmann, 1999
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/os.h>
#include <via.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>



#define ABS(x) ((x) > 0 ? (x) : -(x))
#define PI 3.14159265

#define N 128   /* max size of MI arrays */


struct my_params {
	int    nslices;
	int    nrows;
	int    ncols;
};



VImage s1, s2;
int nbands1, nrows1, ncols1;
int nbands2, nrows2, ncols2;
float slicedist;

float xminval = 0, xmaxval = 0;
float rot[3][3];

int bmin, rmin, cmin;
int bmax, rmax, cmax;
float ave1, ave2;
float center[3];
int verbose = 1;

int itype = 0;



void
MatMult( void )
{
	int i, j, k;
	float sum;

	if ( verbose >= 1 ) fprintf( stderr, "\n mult: \n" );

	for ( i = 0; i < 3; i++ ) {
		for ( k = 0; k < 3; k++ ) {
			sum = 0;

			for ( j = 0; j < 3; j++ ) {
				sum += rot[i][j] * rot[k][j];
			}

			if ( verbose >= 1 ) fprintf( stderr, " %6.3f ", sum );
		}

		if ( verbose >= 1 ) fprintf( stderr, "\n" );
	}

	if ( verbose >= 1 ) fprintf( stderr, "\n" );
}


/*
** compute rotation matrix from roll, pitch and yaw
*/
void
rotation_matrix( float roll, float pitch, float yaw )
{
	float cr, cy, cp, sr, sy, sp;

	cr = cos( ( double )roll );
	cp = cos( ( double )pitch );
	cy = cos( ( double )yaw );

	sr = sin( roll );
	sp = sin( pitch );
	sy = sin( yaw );

	rot[0][0] = cr * cy + sr * sp * sy;
	rot[0][1] = sr * cp;
	rot[0][2] = sr * sp * cy - sy * cr;
	rot[1][0] = cr * sp * sy - sr * cy;
	rot[1][1] = cr * cp;
	rot[1][2] = sr * sy + cy * cr * sp;
	rot[2][0] = cp * sy;
	rot[2][1] = - sp;
	rot[2][2] = cp * cy;
}



/*
**  goal function 2, mutual information
*/
double
func2 ( const gsl_vector *vec, void *params )
{
	struct my_params *par = params;
	VDouble reject = VRepnMaxValue( VFloatRepn );
	int   i, j, n, m;
	float nx1, nx2;
	int   u, v;
	int   b = 0, r, c, bb, rr, cc;
	float bx, rx, cx;
	float bx1, bx2, bx3, rx1, rx2, rx3;
	float phi, psi, theta, sb, sr, sc;
	int   step;
	float nn[N][N];
	float sumi[N], sumj[N];
	float hxy, hx, hy, ixy, px, sum;
	float tiny = 1.0e-30;


	m = 16;
	m = 8;
	n = 256 / m;

	if ( n > N ) VError( " MI arrays too large" );

	step = 2 + nbands2;

	if ( step > 8 ) step = 8;

	sb    = gsl_vector_get( vec, 0 );
	sr    = gsl_vector_get( vec, 1 );
	sc    = gsl_vector_get( vec, 2 );
	psi   = gsl_vector_get( vec, 3 );
	phi   = gsl_vector_get( vec, 4 );
	theta = gsl_vector_get( vec, 5 );

	rotation_matrix( psi, phi, theta );

	for ( i = 0; i < n; i++ )
		for ( j = 0; j < n; j++ ) nn[i][j] = 0;

	nx1 = nx2 = 0;

	for ( b = 0; b < nbands2; b++ ) {
		bx = ( float ) b * slicedist;
		bx -= center[0];
		bx1 = rot[0][0] * bx;
		bx2 = rot[1][0] * bx;
		bx3 = rot[2][0] * bx;

		for ( r = rmin; r < rmax; r += step ) {
			rx = ( float ) r;
			rx -= center[1];
			rx1 = rot[0][1] * rx;
			rx2 = rot[1][1] * rx;
			rx3 = rot[2][1] * rx;

			for ( c = cmin; c < cmax; c += step ) {
				cx = ( float ) c;
				cx -= center[2];

				nx2++;
				bb = ( int ) VRint( bx1 + rx1 + rot[0][2] * cx + sb );

				if ( bb < 0 || bb >= nbands1 ) continue;

				rr = ( int ) VRint( bx2 + rx2 + rot[1][2] * cx + sr );

				if ( rr < 0 || rr >= nrows1 ) continue;

				cc = ( int ) VRint( bx3 + rx3 + rot[2][2] * cx + sc );

				if ( cc < 0 || cc >= ncols1 ) continue;

				u = ( int ) VPixel( s1, bb, rr, cc, VUByte );
				i = u / m;

				v = ( int ) VPixel( s2, b, r, c, VUByte );
				j = v / m;

				nn[i][j]++;
				nx1++;
			}
		}
	}

	if ( nx1 < nx2 * 0.33 ) return reject;

	sum = 0;

	for ( i = 0; i < n; i++ ) {
		sumi[i] = 0;

		for ( j = 0; j < n; j++ ) {
			sumi[i] += nn[i][j];
			sum += nn[i][j];
		}
	}

	if ( sum < tiny ) return reject;

	for ( j = 0; j < n; j++ ) {
		sumj[j] = 0;

		for ( i = 0; i < n; i++ )
			sumj[j] += nn[i][j];
	}

	hx = hy = 0;

	for ( i = 0; i < n; i++ ) {
		if ( sumi[i] > 0 ) {
			px  = sumi[i] / sum;
			hx -= px * log( px );
		}

		if ( sumj[i] > 0 ) {
			px  = sumj[i] / sum;
			hy -= px * log( px );
		}
	}

	hxy = 0;

	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < n; j++ ) {
			if ( nn[i][j] > 0 ) {
				px   = nn[i][j] / sum;
				hxy -= px * log( px );
			}
		}
	}

	ixy = hx + hy - hxy;
	return ( double )( -ixy );
}



/*
**  goal function 1, linear correlation
*/
double
func1 ( const gsl_vector *vec, void *params )
{
	struct my_params *par = params;
	float u, v, nx1, nx2;
	float sum1, sum2, sum3;
	int b = 0, r, c, bb, rr, cc;
	float bx, rx, cx;
	float bx1, bx2, bx3, rx1, rx2, rx3;
	float phi, psi, theta, sb, sr, sc;
	int step;
	double corr;
	VDouble reject = VRepnMaxValue( VFloatRepn );

	step = 2 + nbands2;

	if ( step > 8 ) step = 8;


	sb    = gsl_vector_get( vec, 0 );
	sr    = gsl_vector_get( vec, 1 );
	sc    = gsl_vector_get( vec, 2 );
	psi   = gsl_vector_get( vec, 3 );
	phi   = gsl_vector_get( vec, 4 );
	theta = gsl_vector_get( vec, 5 );

	rotation_matrix( psi, phi, theta );

	corr = 0;
	sum1 = sum2 = sum3 = 0;
	nx1 = nx2 = 0;

	for ( b = 0; b < nbands2; b++ ) {
		bx = ( float ) b * slicedist;
		bx -= center[0];
		bx1 = rot[0][0] * bx;
		bx2 = rot[1][0] * bx;
		bx3 = rot[2][0] * bx;

		for ( r = rmin; r < rmax; r += step ) {
			rx = ( float ) r;
			rx -= center[1];
			rx1 = rot[0][1] * rx;
			rx2 = rot[1][1] * rx;
			rx3 = rot[2][1] * rx;

			for ( c = cmin; c < cmax; c += step ) {
				cx = ( float ) c;
				cx -= center[2];

				nx2++;
				bb = ( int ) VRint( bx1 + rx1 + rot[0][2] * cx + sb );

				if ( bb < 0 || bb >= nbands1 ) continue;

				rr = ( int ) VRint( bx2 + rx2 + rot[1][2] * cx + sr );

				if ( rr < 0 || rr >= nrows1 ) continue;

				cc = ( int ) VRint( bx3 + rx3 + rot[2][2] * cx + sc );

				if ( cc < 0 || cc >= ncols1 ) continue;

				u = VPixel( s1, bb, rr, cc, VUByte );

				if ( u < xminval || u > xmaxval ) continue;

				u -= ave1;
				v = ( float ) VPixel( s2, b, r, c, VUByte ) - ave2;

				sum1 += u * v;
				sum2 += u * u;
				sum3 += v * v;
				nx1++;
			}
		}
	}

	if ( nx1 < nx2 * 0.33 ) return reject;


	if ( sum2 *sum3 != 0 )
		corr = ( sum1 * sum1 ) / ( sum2 * sum3 );
	else corr = 1.0;

	return -corr;
}



VImage
VGetTrans( VImage ref, VImage src, int minval, int maxval,
		   float pitch, float roll, float yaw, float shift[3], float shiftcorr,
		   float offset, VShort scanwidth, VShort type )
{
	VImage transform = NULL;
	VString str;
	long i, j, k, b, r, c, n = 6;
	float sb, sr, sc, range, range2;
	float theta, phi, psi;
	float theta0, phi0, psi0, step1, step2, angle;
	float theta1 = 0, phi1 = 0, psi1 = 0, sb1 = 0, sr1 = 0, sc1 = 0;
	float nx, u, deg;
	int    iter, maxiter;
	double d, dmin;
	char typename[40];

	const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex;
	gsl_multimin_fminimizer *s = NULL;
	gsl_vector *stepsizes = NULL, *p = NULL;
	gsl_multimin_function minex_func;
	size_t np = 6;
	struct my_params params;
	int status;
	double size, fret = 0;




	deg =  ( float ) 180.0 / ( float ) PI;
	xminval = minval;
	xmaxval = maxval;

	itype = type;

	if ( type == 0 ) sprintf( typename, "corr" );

	if ( type == 1 ) sprintf( typename, "MI" );

	s1 = ref;
	s2 = src;
	nbands1 = VImageNBands( ref );
	nrows1  = VImageNRows( ref );
	ncols1  = VImageNColumns( ref );

	nbands2 = VImageNBands( src );
	nrows2  = VImageNRows( src );
	ncols2  = VImageNColumns( src );

	if ( VPixelRepn( ref ) != VUByteRepn ) VError( " ref must be ubyte" );

	if ( VPixelRepn( src ) != VUByteRepn ) VError( " src must be ubyte" );

	if ( offset < 1 ) offset = 1;

	slicedist = offset;    /* distance between slices */


	center[0] = ( float ) nbands1 * 0.5;  /* center of rotation */
	center[1] = ( float ) nrows1 * 0.5;
	center[2] = ( float ) ncols1 * 0.5;

	bmin = rmin = cmin = VRepnMaxValue( VLongRepn );
	bmax = rmax = cmax = 0;
	ave2 = nx = 0;

	for ( b = 0; b < nbands2; b++ ) {
		for ( r = 0; r < nrows2; r++ ) {
			for ( c = 0; c < ncols2; c++ ) {
				u = VPixel( src, b, r, c, VUByte );

				if ( u > xminval && u < xmaxval ) {
					if ( r > rmax ) rmax = r;

					if ( c > cmax ) cmax = c;

					if ( r < rmin ) rmin = r;

					if ( c < cmin ) cmin = c;

					ave2 += u;
					nx++;
				}
			}
		}
	}

	ave2 /= nx;

	ave1 = nx = 0;
	b = ( int ) VRint( ( float ) nbands1 * 0.5 );

	step2 = nbands2 + 2;

	for ( b = 0; b < nbands1; b += ( int )offset ) {
		for ( r = 0; r < nrows1; r += step2 ) {
			for ( c = 0; c < ncols1; c += step2 ) {
				u = VPixel( ref, b, r, c, VUByte );

				if ( u > minval && u < xmaxval ) {
					ave1 += u;
					nx++;
				}
			}
		}
	}

	ave1 /= nx;

	/*
	** get initial shift
	*/
	range  = 30;
	range2 = 10;
	step1  = 2;

	psi0   = pitch / deg; /* set initial guesses of angles */
	phi0   = roll / deg;
	theta0 = yaw / deg;

	n = 6;
	p = gsl_vector_calloc( n );

	shift[0]  = center[0];
	shift[1] += center[1];
	shift[2] += center[2];

	dmin = VRepnMaxValue( VFloatRepn );

	for ( sb = 0; sb < nbands1; sb += step1 ) {
		for ( sr = -range2 + shift[1]; sr <= range2 + shift[1]; sr += step1 ) {
			for ( sc = -range2 + shift[2]; sc <= range2 + shift[2]; sc += step1 ) {

				gsl_vector_set( p, 0, sb );
				gsl_vector_set( p, 1, sr );
				gsl_vector_set( p, 2, sc );
				gsl_vector_set( p, 3, psi0 );
				gsl_vector_set( p, 4, psi0 );
				gsl_vector_set( p, 5, theta0 );


				if ( type == 0 ) d = func1( p, &params );
				else d = func2( p, &params );

				if ( d < dmin ) {
					dmin = d;
					sb1 = sb;
					sr1 = sr;
					sc1 = sc;
				}
			}
		}
	}

	shift[0] = sb1;
	shift[1] = sr1;
	shift[2] = sc1;


	/*
	** get both shift and rotation
	*/
	if ( scanwidth == 0 ) {
		maxiter = 2;
		angle = 9.0 / deg;
		step2 = 3.0 / deg;

		range = 6;
		step1 = 3;
	} else {
		maxiter = 3;
		angle = 12.0 / deg;
		step2 =  3.0 / deg;

		range = 12;
		step1 =  4;
	}


	psi0 = pitch / deg;
	phi0 = 0;
	theta0 = 0;

	psi = psi1 = psi0;
	phi = phi1 = phi0;
	theta = theta1 = theta0;


	gsl_vector_set( p, 0, shift[0] );
	gsl_vector_set( p, 1, shift[1] );
	gsl_vector_set( p, 2, shift[2] );
	gsl_vector_set( p, 3, psi0 );
	gsl_vector_set( p, 4, phi0 );
	gsl_vector_set( p, 5, theta0 );



	if ( type == 0 ) d = func1( p, &params );
	else d = func2( p, &params );

	if ( verbose >= 1 ) fprintf( stderr, " %6.2f  %6.2f  %6.2f,  %6.2f  %6.2f  %6.2f,  %s: %7.4f\n",
									 shift[0], shift[1], shift[2], psi0 * deg, phi0 * deg, theta0 * deg, typename, sqrt( ABS( d ) ) );

	dmin = d;

	for ( k = 0; k < maxiter; k++ ) {
		if ( verbose >= 1 ) fprintf( stderr, "  shift range: %g (step=%g),  angle range: %g (step=%g)\n",
										 range, step1, angle * deg, step2 * deg );

		for ( sb = -range + shift[0]; sb <= range + shift[0]; sb += step1 ) {
			for ( sr = -range + shift[1]; sr <= range + shift[1]; sr += step1 ) {
				for ( sc = -range + shift[2]; sc <= range + shift[2]; sc += step1 ) {
					for ( psi = psi0 - angle; psi <= psi0 + angle; psi += step2 ) {
						for ( phi = phi0 - angle; phi <= phi0 + angle; phi += step2 ) {
							for ( theta = theta0 - angle; theta <= theta0 + angle; theta += step2 ) {

								gsl_vector_set( p, 0, sb );
								gsl_vector_set( p, 1, sr );
								gsl_vector_set( p, 2, sc );
								gsl_vector_set( p, 3, psi );
								gsl_vector_set( p, 4, phi );
								gsl_vector_set( p, 5, theta );

								if ( type == 0 ) d = func1( p, &params );
								else d = func2( p, &params );

								if ( d < dmin ) {
									dmin = d;
									sb1  = sb;
									sr1  = sr;
									sc1  = sc;
									psi1 = psi;
									phi1 = phi;
									theta1 = theta;

									if ( verbose >= 1 )
										fprintf( stderr, " %6.2f  %6.2f  %6.2f,  %6.2f  %6.2f  %6.2f,  %s: %7.4f\n",
												 sb, sr, sc, psi * deg, phi * deg, theta * deg, typename, sqrt( ABS( dmin ) ) );

								}
							}
						}
					}
				}
			}
		}

		angle -= 3.0 / deg;
		step2 *= 0.5;

		if ( step2 < 2.0 / deg ) step2 = 2.0 / deg;

		if ( scanwidth == 0 ) range -= 2;
		else range -= 3;

		step1 -= 1;

		if ( step1 < 2 ) step1 = 2;

		psi0 = psi1;
		phi0 = phi1;
		theta0 = theta1;

		shift[0] = sb1;
		shift[1] = sr1;
		shift[2] = sc1;
	}



	gsl_vector_set( p, 0, sb1 );
	gsl_vector_set( p, 1, sr1 );
	gsl_vector_set( p, 2, sc1 );
	gsl_vector_set( p, 3, psi1 );
	gsl_vector_set( p, 4, phi1 );
	gsl_vector_set( p, 5, theta1 );



	if ( verbose >= 1 ) fprintf( stderr, " %6.2f  %6.2f  %6.2f,  %6.2f  %6.2f  %6.2f,  %s: %7.4f\n",
									 sb1, sr1, sc1, psi1 * deg, phi1 * deg, theta1 * deg, typename, sqrt( ABS( dmin ) ) );



	/* set up optimization procedure */
	np = 6;
	stepsizes = gsl_vector_calloc ( np );
	gsl_vector_set_all ( stepsizes, 1.0 );

	if ( type == 1 ) minex_func.f = &func2;
	else minex_func.f = &func1;

	minex_func.n = np;
	minex_func.params = ( void * )&params;
	s = gsl_multimin_fminimizer_alloc ( T, np );

	gsl_vector_set_all ( stepsizes, 1.0 );
	gsl_multimin_fminimizer_set ( s, &minex_func, p, stepsizes );


	/* do optimization */
	maxiter = 100;

	for ( iter = 0; iter < maxiter; iter++ ) {
		status = gsl_multimin_fminimizer_iterate( s );

		if ( status ) break;

		size = gsl_multimin_fminimizer_size ( s );
		status = gsl_multimin_test_size ( size, 1e-6 );

		if ( status == GSL_SUCCESS ) break;
	}

	fret = s->fval;

	if ( fret < dmin ) {
		sb1    = gsl_vector_get( s->x, 0 );
		sr1    = gsl_vector_get( s->x, 1 );
		sc1    = gsl_vector_get( s->x, 2 );
		psi1   = gsl_vector_get( s->x, 3 );
		phi1   = gsl_vector_get( s->x, 4 );
		theta1 = gsl_vector_get( s->x, 5 );
	}

	if ( verbose >= 1 ) fprintf( stderr, " %6.2f  %6.2f  %6.2f,  %6.2f  %6.2f  %6.2f,  %s: %7.4f\n",
									 sb1, sr1, sc1, psi1 * deg, phi1 * deg, theta1 * deg, typename, sqrt( ( double )ABS( fret ) ) );

	/*
	** output
	*/
ende:
	rotation_matrix( psi1, phi1, theta1 );

	transform = VCreateImage( 1, 3, 4, VDoubleRepn );
	VCopyImageAttrs ( ref, transform );

	shift[0] = sb1;
	shift[1] = sr1;
	shift[2] = sc1;
	shift[0] -= shiftcorr;

	for ( i = 0; i < 3; i++ ) VPixel( transform, 0, i, 0, VDouble ) = shift[i];

	for ( i = 0; i < 3; i++ )
		for ( j = 1; j < 4; j++ )
			VPixel( transform, 0, i, j, VDouble ) = rot[i][j - 1];

	str = ( VString ) VMalloc( 60 );
	sprintf( str, "%g %g %g", psi1 * deg, phi1 * deg, theta1 * deg );
	VAppendAttr( VImageAttrList( transform ), "rotation", NULL, VStringRepn, str );

	return transform;
}


