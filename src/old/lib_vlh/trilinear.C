/****************************************************************
 *
 * trilinear.C
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Thomas Arnold, <lipsia@cbs.mpg.de>
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
  *
 *****************************************************************/

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
   #include <viaio/Vlib.h>
   #include <viaio/VImage.h>
   #include <viaio/option.h>
   #include <viaio/mu.h>
}


/*----------------------------------------------------------------------------*/

/* rewritten by T. Arnold using old interface from H. Mentzel */

VImage vlhTriLinearScale3d (VImage src, VImage dest,
                            int dest_nbands, int dest_nrows, int dest_ncolumns,
                            double scale_band, double scale_row, double scale_col, int slevel)
{
   int src_nbands;     /* number of bands   */
   int src_nrows;      /* number of rows    */
   int src_ncolumns;   /* number of columns */

   VFloat* psrc;    /* source data pointer      */
   VFloat* pdest;   /* destination data pointer */

   float dx, dy, dz;   /* displacement of origin    */
   float cx, cy, cz;   /* interpolation coordinates */
   int   sx, sy, sz;   /* origin of subcube         */
   float px, py, pz;   /* fractions of subcube      */
   float qx, qy, qz;   /* fractions of subcube      */

   VFloat value;   /* value of voxel */

   int lx, ly, lz;   /* lengths */
   int ox, oy, oz;   /* offsets */
   int  x,  y,  z;   /* indices */


   /* get source image size */
   src_nbands   = VImageNFrames   (src);
   src_nrows    = VImageNRows    (src);
   src_ncolumns = VImageNColumns (src);

   /* create destination image */
   if (!dest) dest = VCreateImage (dest_nbands, dest_nrows, dest_ncolumns, VFloatRepn);
   VImageAttrList (dest) = VCopyAttrList (VImageAttrList (src));


   /* compute displacements */
   dx = 0.5 * (dest_ncolumns - scale_col  * src_ncolumns);
   dy = 0.5 * (dest_nrows    - scale_row  * src_nrows);
/* dz = 0.5 * (dest_nbands   - scale_band * src_nbands); */
   dz = 0;

   /* scale source image */
   pdest = (VFloat*) VPixelPtr (dest, 0, 0, 0);
   for (z = 0; z < dest_nbands; ++z)
      for (y = 0; y < dest_nrows; ++y)
         for (x = 0; x < dest_ncolumns; ++x)
         {
            /* compute interpolation coordinates */
            cx = (x - dx) / scale_col;
            cy = (y - dy) / scale_row;
            cz = (z - dz) / scale_band;

            /* compute origin of subcube */
            sx = (int) cx;
            sy = (int) cy;
            sz = (int) cz;

            /* check subcube */
            if ((sx < 0) || (sx >= src_ncolumns)) {*(pdest++) = 0.0; continue;}
            if ((sy < 0) || (sy >= src_nrows))    {*(pdest++) = 0.0; continue;}
            if ((sz < 0) || (sz >= src_nbands))   {*(pdest++) = 0.0; continue;}

            /* compute fractions of subcube */
            qx = cx - sx; px = 1 - qx;
            qy = cy - sy; py = 1 - qy;
            qz = cz - sz; pz = 1 - qz;

            /* compute lengths and offsets */
            lx = (cx < src_ncolumns - 1) ? 1 : 0;
            ly = (cy < src_nrows    - 1) ? (src_ncolumns) : 0;
            lz = (cz < src_nbands   - 1) ? (src_nrows * src_ncolumns) : 0;
            ox = lx;
            oy = ox + ly - 2 * lx;
            oz = oy + lz - 2 * ly;

            /* interpolate value */
            value  = 0;
            psrc   = (VFloat*) VPixelPtr (src, sz, sy, sx);
            value += (float) pz * py * px * *psrc; psrc += ox;
            value += (float) pz * py * qx * *psrc; psrc += oy;
            value += (float) pz * qy * px * *psrc; psrc += ox;
            value += (float) pz * qy * qx * *psrc; psrc += oz;
            value += (float) qz * py * px * *psrc; psrc += ox;
            value += (float) qz * py * qx * *psrc; psrc += oy;
            value += (float) qz * qy * px * *psrc; psrc += ox;
            value += (float) qz * qy * qx * *psrc;

            /* assign value */
            *(pdest++) = value;
         }

   return dest;

} /* vlhTriLinearScale3d */

/*----------------------------------------------------------------------------*/

/* rewritten by T. Arnold using old interface from H. Mentzel */

VImage VNNScale3d (VImage src, VImage dest,
                   int dest_nbands, int dest_nrows, int dest_ncolumns,
                   double scale_band, double scale_row, double scale_col)
{
   int src_nbands;     /* number of bands   */
   int src_nrows;      /* number of rows    */
   int src_ncolumns;   /* number of columns */

   VFloat* psrc;    /* source data pointer      */
   VFloat* pdest;   /* destination data pointer */

   float dx, dy, dz;   /* displacement of origin    */
   float cx, cy, cz;   /* interpolation coordinates */
   int   sx, sy, sz;   /* origin of subcube         */

   int x, y, z;   /* indices */


   /* get source image size */
   src_nbands   = VImageNFrames  (src);
   src_nrows    = VImageNRows    (src);
   src_ncolumns = VImageNColumns (src);

   /* create destination image */
   if (!dest) dest = VCreateImage (dest_nbands, dest_nrows, dest_ncolumns, VFloatRepn);
   VImageAttrList (dest) = VCopyAttrList (VImageAttrList (src));


   /* compute displacements */
   dx = 0.5 * (dest_ncolumns - scale_col  * src_ncolumns);
   dy = 0.5 * (dest_nrows    - scale_row  * src_nrows);
/* dz = 0.5 * (dest_nbands   - scale_band * src_nbands); */
   dz = 0;

   /* scale source image */
   pdest = (VFloat*) VPixelPtr (dest, 0, 0, 0);
   for (z = 0; z < dest_nbands; ++z)
      for (y = 0; y < dest_nrows; ++y)
         for (x = 0; x < dest_ncolumns; ++x)
         {
            /* compute interpolation coordinates */
            cx = (x + dx) / scale_col;
            cy = (y + dy) / scale_row;
            cz = (z + dz) / scale_band;

            /* compute origin of subcube */
            sx = (int) cx;
            sy = (int) cy;
            sz = (int) cz;

            /* check subcube */
            if ((sx < 0) || (sx >= src_ncolumns)) {*(pdest++) = 0.0; continue;}
            if ((sy < 0) || (sy >= src_nrows))    {*(pdest++) = 0.0; continue;}
            if ((sz < 0) || (sz >= src_nbands))   {*(pdest++) = 0.0; continue;}

            /* assign value */
            psrc       = (VFloat*) VPixelPtr (src, sz, sy, sx);
            *(pdest++) = *psrc;
         }

   return dest;

} /* VNNScale3d */

/*----------------------------------------------------------------------------*/
