/****************************************************************
 *
 * vdemon: gradient.H
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
 * $Id: gradient.H 704 2004-02-16 09:41:44Z karstenm $
 *
 *****************************************************************/

#ifndef GRADIENT_H_INCLUDED
#define GRADIENT_H_INCLUDED

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

1D Gradient
===========

Src    source image with voxels of type T
Dir    direction of gradient (X = 0, Y = 1 and Z = 2)
Grad   gradient image with voxels of type VFloat (created)

------------------------------------------------------------------------------*/

const int XGradient = 0;
const int YGradient = 1;
const int ZGradient = 2;

template <class T> void Gradient (VImage Src, int Dir, VImage& Grad)
{
   int Bands;     /* number of bands   */
   int Rows;      /* number of rows    */
   int Columns;   /* number of columns */

   T*      src;         /* source data pointer   */
   T       vn, v, vp;   /* data values           */
   VFloat* grad;        /* gradient data pointer */

   int  stride = 0;              /* stride     */
   int  I = 0,  J = 0,  K = 0;   /* sizes      */
   int  i = 0,  j = 0,  k = 0;   /* indices    */
   int *x = 0, *y = 0, *z = 0;   /* references */


   /* get source image size */
   Bands   = VImageNBands   (Src);
   Rows    = VImageNRows    (Src);
   Columns = VImageNColumns (Src);

   /* create gradient image */
   Grad = VCreateImage (Bands, Rows, Columns, VFloatRepn);
   VImageAttrList (Grad) = VCopyAttrList (VImageAttrList (Src));


   /* set gradient parameters */
   if (Dir == XGradient) {K = Bands;   z = &k; J = Rows;    y = &j; I = Columns; x = &i; stride = 1;             }
   if (Dir == YGradient) {K = Columns; x = &k; J = Bands;   z = &j; I = Rows;    y = &i; stride = Columns;       }
   if (Dir == ZGradient) {K = Rows;    y = &k; J = Columns; x = &j; I = Bands;   z = &i; stride = Rows * Columns;}

   /* compute gradient */
   for (k = 0; k < K; ++k)
      for (j = 0; j < J; ++j)
      {
         /* setup gradient line */
         i    = 0;
         src  = (T*)      VPixelPtr (Src,  *z, *y, *x);
         grad = (VFloat*) VPixelPtr (Grad, *z, *y, *x);
         v    = *src; src += stride;
         vp   = *src; src += stride;

         /* compute first entry */
         *grad = (VFloat) (vp - v); grad += stride;

         /* compute center entries */
         for (i = 1; i < I - 1; ++i)
         {
            vn = v; v = vp; vp = *src; src += stride;
            *grad = (VFloat) ((vp - vn) / 2.0); grad += stride;
         }

         /* compute last entry */
         *grad = (VFloat) (vp - v);
      }

} /* Gradient */

/*------------------------------------------------------------------------------

3D Gradient
===========

Src     source image with voxels of type T
GradX   x-gradient image with voxels of type VFloat (created)
GradY   y-gradient image with voxels of type VFloat (created)
GradZ   z-gradient image with voxels of type VFloat (created)

------------------------------------------------------------------------------*/

template <class T> void Gradient (VImage Src, VImage& GradX, VImage& GradY, VImage& GradZ)
{
   /* compute gradients */
   Gradient<T> (Src, XGradient, GradX);
   Gradient<T> (Src, YGradient, GradY);
   Gradient<T> (Src, ZGradient, GradZ);

} /* Gradient */

/*------------------------------------------------------------------------------

Squared 3D Gradient
===================

GradX   x-gradient image with voxels of type VFloat
GradY   y-gradient image with voxels of type VFloat
GradZ   z-gradient image with voxels of type VFloat
Grad2   squared gradient image with voxels of type VFloat (created)

------------------------------------------------------------------------------*/

void SquaredGradient (VImage GradX, VImage GradY, VImage GradZ, VImage& Grad2)
{
   int Pixels;   /* number of pixels */

   VFloat *gradx, *grady, *gradz;   /* gradient data pointer         */
   VFloat *grad2;                   /* squared gradient data pointer */
   VFloat gx, gy, gz;               /* data values                   */

   int n;   /* index */


   /* get gradient size */
   Pixels = VImageNPixels (GradX);

   /* create squared gradient */
   Grad2 = VCreateImage (VImageNBands (GradX), VImageNRows (GradX), VImageNColumns (GradX), VPixelRepn (GradX));
   VImageAttrList (Grad2) = VCopyAttrList (VImageAttrList (GradX));


   /* compute magnitude */
   gradx = (VFloat*) VPixelPtr (GradX, 0, 0, 0);
   grady = (VFloat*) VPixelPtr (GradY, 0, 0, 0);
   gradz = (VFloat*) VPixelPtr (GradZ, 0, 0, 0);
   grad2 = (VFloat*) VPixelPtr (Grad2, 0, 0, 0);
   for (n = 0; n < Pixels; ++n)
   {
      gx = *(gradx++);
      gy = *(grady++);
      gz = *(gradz++);
      *(grad2++) = gx * gx + gy * gy + gz * gz;
   }

} /* SquaredGradient */

/*----------------------------------------------------------------------------*/

#endif
