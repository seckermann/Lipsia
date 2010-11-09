/*! \file
  Talairach coordinates.

Conversion from voxel to Talairach coordinates,
and vice versa.
The reference brain size is: 136 x 172 x 118 mm
(see J.L.Lancaster, et. al.).

\par Reference:
J.L.Lancaster, et. al.,...,P.T.Fox.
Automated Talairach Labels for functional brain mapping.
Human Brain Mapping, 10:120-131, 2000.

\par Author:
Gabriele Lohmann, MPI-CBS
*/


/****************************************************************
 *
 * Talairach.c
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2000, <lipsia@cbs.mpg.de>
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
 * $Id: Talairach.c 2134 2007-04-23 15:02:10Z karstenm $
 *
 *****************************************************************/

#include <via.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))


/*!
\fn void VPixel2Tal_Flt(float ca[3],float voxel[3],float extent[3],
	   int band,int row,int col,float *x,float *y,float *z)
\param ca     CA address in mm.
\param voxel  spatialresolution in mm (x,y,z)
\param extent brain size in mm
\param band   voxel slice address
\param row    voxel row address
\param col    voxel column address
\param *x     output x-Talairach coordinate
\param *y     output y-Talairach coordinate
\param *z     output z-Talairach coordinate
*/
void
VPixel2Tal_Flt(float ca[3],float voxel[3],float extent[3],
	       float band,float row,float col,float *x,float *y,float *z)
{
  float bx,rx,cx;
  float u=0,v=0,w=0;
  float xscale=0,yscale=0,zscale=0;
  float vox[3];
  int i;

  for (i=0; i<3; i++) vox[i] = voxel[i];
 
  *x = 0;
  *y = 0;
  *z = 0;

  /* check consistency, is CA/CP given in voxel-coordinates or in mm ? */
  /* Attention!!!!!! This has to be changed in the future !!!!!!!!!!!!!!!!!!!!!! */
  if (ca[0] > 70 && voxel[0] > 1.7)
    for (i=0; i<3; i++) vox[i] = 1;

  /* do transformation */
  cx = (float)col  * voxel[0];
  rx = (float)row  * voxel[1];
  bx = (float)band * voxel[2];

  xscale =  135.0 / extent[0];
  yscale =  175.0 / extent[1];
  zscale =  120.0 / extent[2]; 

  u = (cx  - ca[0]*vox[0]) * xscale;
  v = (ca[1]*vox[1] - rx)  * yscale;
  w = (ca[2]*vox[2] - bx)  * zscale;

  *x = u;
  *y = v;
  *z = w;
}



/*!
\fn void VTal2Pixel_Flt(float ca[3],float voxel[3],float extent[3],
                    float x,float y,float z,int *band,int *row,int *col)
\param ca     CA address in mm.
\param voxel  spatial resolution in mm (x,y,z)
\param extent brain size in mm (x,y,z)
\param x      x-Talairach coordinate
\param y      y-Talairach coordinate
\param z      z-Talairach coordinate
\param *band  voxel slice address
\param *row   voxel row address
\param *col   voxel column address
*/
void VTal2Pixel_Flt(float ca[3],float voxel[3],float extent[3],
		    float x,float y,float z,
		    float *band,float *row,float *col)
{
  float xscale,yscale,zscale;
  float u,v,w;
  float vox[3];
  int i;

  for (i=0; i<3; i++) vox[i] = voxel[i];
  
  /* check consistency, is CA/CP given in voxel-coordinates or in mm ? */
  /* Attention!!!!!! This has to be changed in the future !!!!!!!!!!!!!!!!!!!!!! */
  if (ca[0] > 70 && voxel[0] > 1.7)
    for (i=0; i<3; i++) vox[i] = 1;


  xscale =  135.0 / extent[0];
  yscale =  175.0 / extent[1];
  zscale =  120.0 / extent[2];

  u = x/xscale + ca[0]*vox[0];
  v = ca[1]*vox[1] - y/yscale;
  w = ca[2]*vox[2] - z/zscale;

  u /= voxel[0];
  v /= voxel[1];
  w /= voxel[2];

  *col  = u;
  *row  = v;
  *band = w;
}


/*!
\fn void VPixel2Tal(float ca[3],float voxel[3],float extent[3],
	   int band,int row,int col,float *x,float *y,float *z)
\param ca     CA address in mm.
\param voxel  spatialresolution in mm (x,y,z)
\param extent brain size in mm
\param band   voxel slice address
\param row    voxel row address
\param col    voxel column address
\param *x     output x-Talairach coordinate
\param *y     output y-Talairach coordinate
\param *z     output z-Talairach coordinate
*/
void
VPixel2Tal(float ca[3],float voxel[3],float extent[3],
	   int band,int row,int col,float *x,float *y,float *z)
{
  float bx,rx,cx;
  float u=0,v=0,w=0;
  float xscale=0,yscale=0,zscale=0;
  float vox[3];
  int i;

  for (i=0; i<3; i++) vox[i] = voxel[i];
 
  *x = 0;
  *y = 0;
  *z = 0;

  /* check consistency, is CA/CP given in voxel-coordinates or in mm ? */
  /* Attention!!!!!! This has to be changed in the future !!!!!!!!!!!!!!!!!!!!!! */
  if (ca[0] > 70 && voxel[0] > 1.7)
    for (i=0; i<3; i++) vox[i] = 1;

  /* do transformation */
  cx = (float)col  * voxel[0];
  rx = (float)row  * voxel[1];
  bx = (float)band * voxel[2];

  xscale =  135.0 / extent[0];
  yscale =  175.0 / extent[1];
  zscale =  120.0 / extent[2];

  u = (cx  - ca[0]*vox[0]) * xscale;
  v = (ca[1]*vox[1] - rx)  * yscale;
  w = (ca[2]*vox[2] - bx)  * zscale;

  *x = u;
  *y = v;
  *z = w;
}



/*!
\fn void VTal2Pixel(float ca[3],float voxel[3],float extent[3],
                    float x,float y,float z,int *band,int *row,int *col)
\param ca     CA address in mm.
\param voxel  spatial resolution in mm (x,y,z)
\param extent brain size in mm (x,y,z)
\param x      x-Talairach coordinate
\param y      y-Talairach coordinate
\param z      z-Talairach coordinate
\param *band  voxel slice address
\param *row   voxel row address
\param *col   voxel column address
*/
void VTal2Pixel(float ca[3],float voxel[3],float extent[3],
		float x,float y,float z,
		int *band,int *row,int *col)
{
  float xscale,yscale,zscale;
  float u,v,w;
  float vox[3];
  int i;

  for (i=0; i<3; i++) vox[i] = voxel[i];
  
  /* check consistency, is CA/CP given in voxel-coordinates or in mm ? */
  /* Attention!!!!!! This has to be changed in the future !!!!!!!!!!!!!!!!!!!!!! */
  if (ca[0] > 70 && voxel[0] > 1.7)
    for (i=0; i<3; i++) vox[i] = 1;


  xscale =  135.0 / extent[0];
  yscale =  175.0 / extent[1];
  zscale =  120.0 / extent[2];

  u = x/xscale + ca[0]*vox[0];
  v = ca[1]*vox[1] - y/yscale;
  w = ca[2]*vox[2] - z/zscale;

  u /= voxel[0];
  v /= voxel[1];
  w /= voxel[2];

  *col  = VRint(u);
  *row  = VRint(v);
  *band = VRint(w);
}


void
VPixel2MNI(float ca[3],float voxel[3],
	   int band,int row,int col,float *x,float *y,float *z)
{
  float bx,rx,cx;
  float u=0,v=0,w=0;
  float vox[3];
  int i;

  *x = *y = *z = 0;
  /* check consistency, is CA/CP given in voxel-coordinates or in mm ? */
  /* Attention!!!!!! This has to be changed in the future !!!!!!!!!!!!!!!!!!!!!! */
  for (i=0; i<3; i++) vox[i] = voxel[i];
  if (ca[0] > 70 && voxel[0] > 1.7)
    for (i=0; i<3; i++) vox[i] = 1;

  /* do transformation */
  cx = (float)col  * voxel[0];
  rx = (float)row  * voxel[1];
  bx = (float)band * voxel[2];

  u = (cx  - ca[0]*vox[0]);
  v = (ca[1]*vox[1] - rx);
  w = (ca[2]*vox[2] - bx);

  *x = u;
  *y = v;
  *z = w;
}
