/****************************************************************
 *
 * vdemon: trilinear.H
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Thomas Arnold, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: trilinear.H 704 2004-02-16 09:41:44Z karstenm $
 *
 *****************************************************************/

#ifndef TRILINEAR_H_INCLUDED
#define TRILINEAR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <assert.h>

extern "C"
{
   #include <Vlib.h>
   #include <VImage.h>
   #include <option.h>
   #include <mu.h>
}


/*------------------------------------------------------------------------------

TrilinearScale
==============

Src    source image with voxels of type T
Fx     x-scaling factor
Fy     y-scaling factor
Fz     z-scaling factor
Dest   scaled image with voxels of type T (created)

ATTENTION!

Be aware that downscaling (F < 1) leads to ALIASING ARTEFACTS if no appropriate
LOWPASS FILTERING has been previously applied to the data!

------------------------------------------------------------------------------*/

template <class T> void TrilinearScale (VImage Src, float Fx, float Fy, float Fz, VImage& Dest)
{
   int SrcBands;      /* number of bands   */
   int SrcRows;       /* number of rows    */
   int SrcColumns;    /* number of columns */
   int DestBands;     /* number of bands   */
   int DestRows;      /* number of rows    */
   int DestColumns;   /* number of columns */

   T* src;    /* source data pointer      */
   T* dest;   /* destination data pointer */

   float cx, cy, cz;   /* interpolation coordinates */
   int   sx, sy, sz;   /* origin of subcube         */
   float px, py, pz;   /* fractions of subcube      */
   float qx, qy, qz;   /* fractions of subcube      */

   float value;   /* value of voxel */

   int lx, ly, lz;   /* lengths */
   int ox, oy, oz;   /* offsets */
   int  x,  y,  z;   /* indices */


   /* get source image size */
   SrcBands   = VImageNBands   (Src);
   SrcRows    = VImageNRows    (Src);
   SrcColumns = VImageNColumns (Src);

   /* compute destination image size */
   DestBands   = (int) floor (SrcBands   * Fz);
   DestRows    = (int) floor (SrcRows    * Fy);
   DestColumns = (int) floor (SrcColumns * Fx);

   /* create destination image */
   Dest = VCreateImage (DestBands, DestRows, DestColumns, VPixelRepn (Src));
   VImageAttrList (Dest) = VCopyAttrList (VImageAttrList (Src));


   /* scale source image */
   dest = (T*) VPixelPtr (Dest, 0, 0, 0);
   for (z = 0; z < DestBands; ++z)
      for (y = 0; y < DestRows; ++y)
         for (x = 0; x < DestColumns; ++x)
         {
            /* compute interpolation coordinates */
            cx = x / Fx;
            cy = y / Fy;
            cz = z / Fz;

            /* compute origin of subcube */
            sx = (int) cx;
            sy = (int) cy;
            sz = (int) cz;

            /* compute fractions of subcube */
            qx = cx - sx; px = 1 - qx;
            qy = cy - sy; py = 1 - qy;
            qz = cz - sz; pz = 1 - qz;

            /* compute lengths and offsets */
            lx = (cx < SrcColumns - 1) ? 1 : 0;
            ly = (cy < SrcRows    - 1) ? (SrcColumns) : 0;
            lz = (cz < SrcBands   - 1) ? (SrcRows * SrcColumns) : 0;
            ox = lx;
            oy = ox + ly - 2 * lx;
            oz = oy + lz - 2 * ly;

            /* interpolate value */
            value  = 0;
            src    = (T*) VPixelPtr (Src, sz, sy, sx);
            value += (float) pz * py * px * *src; src += ox;
            value += (float) pz * py * qx * *src; src += oy;
            value += (float) pz * qy * px * *src; src += ox;
            value += (float) pz * qy * qx * *src; src += oz;
            value += (float) qz * py * px * *src; src += ox;
            value += (float) qz * py * qx * *src; src += oy;
            value += (float) qz * qy * px * *src; src += ox;
            value += (float) qz * qy * qx * *src;

            /* assign value */
            *(dest++) = (T) value;
         }

} /* TrilinearScale */

template <class T> void TrilinearScale (VImage& Src, float Fx, float Fy, float Fz)
{
   VImage Dest;   /* destination image */


   /* scale image */
   TrilinearScale<T> (Src, Fx, Fy, Fz, Dest);
   VDestroyImage (Src);
   Src = Dest;

} /* TrilinearScale */

/*------------------------------------------------------------------------------

TrilinearInverseDeform
======================

Src    source image with voxels of type T
Dx     x-deformation field with voxels of type VFloat (from destination to source)
Dy     y-deformation field with voxels of type VFloat (from destination to source)
Dz     z-deformation field with voxels of type VFloat (from destination to source)
Dest   deformed image with voxels of type T (created)

------------------------------------------------------------------------------*/

template <class T> void TrilinearInverseDeform (VImage Src, VImage Dx, VImage Dy, VImage Dz, VImage& Dest)
{
   int Bands;     /* number of bands   */
   int Rows;      /* number of rows    */
   int Columns;   /* number of columns */

   T*     src;             /* source data pointer       */
   T*     dest;            /* destination data pointer  */
   VFloat *dx, *dy, *dz;   /* deformation field pointer */

   float cx, cy, cz;   /* interpolation coordinates */
   int   sx, sy, sz;   /* origin of subcube         */
   float px, py, pz;   /* fractions of subcube      */
   float qx, qy, qz;   /* fractions of subcube      */

   float value;   /* value of voxel */

   int lx, ly, lz;   /* lengths */
   int ox, oy, oz;   /* offsets */
   int  x,  y,  z;   /* indices */


   /* get source image size */
   Bands   = VImageNBands   (Src);
   Rows    = VImageNRows    (Src);
   Columns = VImageNColumns (Src);

   /* create destination image */
   Dest = VCreateImage (Bands, Rows, Columns, VPixelRepn (Src));
   VImageAttrList (Dest) = VCopyAttrList (VImageAttrList (Src));


   /* inverse deform image */
   dest = (T*)      VPixelPtr (Dest, 0, 0, 0);
   dx   = (VFloat*) VPixelPtr (Dx,   0, 0, 0);
   dy   = (VFloat*) VPixelPtr (Dy,   0, 0, 0);
   dz   = (VFloat*) VPixelPtr (Dz,   0, 0, 0);
   for (z = 0; z < Bands; ++z)
      for (y = 0; y < Rows; ++y)
         for (x = 0; x < Columns; ++x)
         {
            /* compute interpolation coordinates */
            cx = x + (float) *(dx++);
            cy = y + (float) *(dy++);
            cz = z + (float) *(dz++);

            /* check coordinates */
            if ((cx < 0) || (cx > Columns - 1)) {*(dest++) = 0; continue;}
            if ((cy < 0) || (cy > Rows    - 1)) {*(dest++) = 0; continue;}
            if ((cz < 0) || (cz > Bands   - 1)) {*(dest++) = 0; continue;}

            /* compute coordinates of subcube */
            sx = (int) (cx);
            sy = (int) (cy);
            sz = (int) (cz);

            /* compute fractions of subcube */
            qx = cx - sx; px = 1 - qx;
            qy = cy - sy; py = 1 - qy;
            qz = cz - sz; pz = 1 - qz;

            /* compute lengths and offsets */
            lx = (cx < Columns - 1) ? 1 : 0;
            ly = (cy < Rows    - 1) ? Columns : 0;
            lz = (cz < Bands   - 1) ? Rows * Columns : 0;
            ox = lx;
            oy = ox + ly - 2 * lx;
            oz = oy + lz - 2 * ly;

            /* interpolate value */
            value  = 0;
            src    = (T*) VPixelPtr (Src, sz, sy, sx);
            value += (float) pz * py * px * *src; src += ox;
            value += (float) pz * py * qx * *src; src += oy;
            value += (float) pz * qy * px * *src; src += ox;
            value += (float) pz * qy * qx * *src; src += oz;
            value += (float) qz * py * px * *src; src += ox;
            value += (float) qz * py * qx * *src; src += oy;
            value += (float) qz * qy * px * *src; src += ox;
            value += (float) qz * qy * qx * *src;

            /* assign value */
            *(dest++) = (T) value;
         }

} /* TrilinearInverseDeform */

template <class T> void TrilinearInverseDeform (VImage& Src, VImage Dx, VImage Dy, VImage Dz)
{
   VImage Dest;   /* destination image */


   /* inverse deform image */
   TrilinearInverseDeform<T> (Src, Dx, Dy, Dz, Dest);
   VDestroyImage (Src);
   Src = Dest;

} /* TrilinearInverseDeform */

/*----------------------------------------------------------------------------*/

#endif
