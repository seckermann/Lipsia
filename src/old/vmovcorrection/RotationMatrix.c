/*
** input : rotation angles: pitch,yaw,roll
** output: inverse of rotation matrix
**
** G.Lohmann, Sept 2010
*/

#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


void InverseRotationMatrix( double pitch, double yaw, double roll, double ainv[3][3] )
{
	int i, j;
	double rot[3][3], detA;
	double cr, cy, cp, sr, sp, sy;

	cr = cos( pitch );
	cp = cos( yaw );
	cy = cos( roll );
	sr = sin( pitch );
	sp = sin( yaw );
	sy = sin( roll );

	rot[0][0] = cr * cy + sr * sp * sy;
	rot[0][1] = sr * cp;
	rot[0][2] = sr * sp * cy - sy * cr;
	rot[1][0] = cr * sp * sy - sr * cy;
	rot[1][1] = cr * cp;
	rot[1][2] = sr * sy + cy * cr * sp;
	rot[2][0] = cp * sy;
	rot[2][1] = -sp;
	rot[2][2] = cp * cy;

	/*
	** invert matrix
	*/
	ainv[0][0] =  rot[1][1] * rot[2][2] - rot[1][2] * rot[2][1];
	ainv[1][0] = -rot[1][0] * rot[2][2] + rot[1][2] * rot[2][0];
	ainv[2][0] =  rot[1][0] * rot[2][1] - rot[1][1] * rot[2][0];

	ainv[0][1] = -rot[0][1] * rot[2][2] + rot[0][2] * rot[2][1];
	ainv[1][1] =  rot[0][0] * rot[2][2] - rot[0][2] * rot[2][0];
	ainv[2][1] = -rot[0][0] * rot[2][1] + rot[0][1] * rot[2][0];

	ainv[0][2] =  rot[0][1] * rot[1][2] - rot[0][2] * rot[1][1];
	ainv[1][2] = -rot[0][0] * rot[1][2] + rot[0][2] * rot[1][0];
	ainv[2][2] =  rot[0][0] * rot[1][1] - rot[0][1] * rot[1][0];

	/* determinant */
	detA = rot[0][0] * ainv[0][0] + rot[0][1] * ainv[1][0] + rot[0][2] * ainv[2][0];

	if ( detA == 0 ) VError( " transformation matrix is singular" );

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			ainv[i][j] /= detA;
		}
	}
}
