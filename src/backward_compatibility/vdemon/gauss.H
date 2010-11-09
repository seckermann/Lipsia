/****************************************************************
 *
 * vdemon: gauss.H
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
 * $Id: gauss.H 704 2004-02-16 09:41:44Z karstenm $
 *
 *****************************************************************/

#ifndef GAUSS_H_INCLUDED
#define GAUSS_H_INCLUDED

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

1D Gaussian Filter
==================

Src     source image with voxels of type T
Sigma   standard deviation of filter
Width   width of filter (must be odd)
Dir     direction of filter (X = 0, Y = 1 and Z = 2)
Dest    filtered image with voxels of type T (created)

------------------------------------------------------------------------------*/

const int XGaussian = 0;
const int YGaussian = 1;
const int ZGaussian = 2;

template <class T> void GaussianFilter (VImage Src, float Sigma, int Width, int Dir, VImage& Dest)
{
   int Bands;     /* number of bands   */
   int Rows;      /* number of rows    */
   int Columns;   /* number of columns */

   int    N;        /* half width of filter */
   float* filter;   /* filter coefficients  */
   float  sum;      /* sum of coefficients  */

   T*     src;     /* source data pointer      */
   T*     conv;    /* convolution data pointer */
   T*     dest;    /* destination data pointer */
   float* start;   /* filter data pointer      */
   float* stop;    /* filter data pointer      */

   float value;   /* value of voxel */

   int  stride = 0;                 /* stride     */
   int  I = 0,  J = 0,  K = 0;      /* sizes      */
   int  i = 0,  j = 0,  k = 0, w;   /* indices    */
   int *x = 0, *y = 0, *z = 0;      /* references */


   /* get source image size */
   Bands   = VImageNBands   (Src);
   Rows    = VImageNRows    (Src);
   Columns = VImageNColumns (Src);

   /* create destination image */
   Dest = VCreateImage (Bands, Rows, Columns, VPixelRepn (Src));
   VImageAttrList (Dest) = VCopyAttrList (VImageAttrList (Src));


   /* create filter coefficients */
   N = (Width - 1) / 2;
   filter = (float*) malloc (Width * sizeof (float));

   /* compute coefficients */
   sum = 0;
   for (w = 0; w < Width; ++w)
   {
      filter[w] = exp (- (w - N) * (w - N) / (2 * Sigma * Sigma));
      sum += filter[w];
   }

   /* normalize coefficients */
   for (w = 0; w < Width; ++w)
      filter[w] /= sum;


   /* set filter parameters */
   if (Dir == XGaussian) {K = Bands;   z = &k; J = Rows;    y = &j; I = Columns; x = &i; stride = 1;             }
   if (Dir == YGaussian) {K = Columns; x = &k; J = Bands;   z = &j; I = Rows;    y = &i; stride = Columns;       }
   if (Dir == ZGaussian) {K = Rows;    y = &k; J = Columns; x = &j; I = Bands;   z = &i; stride = Rows * Columns;}

   /* filter source image */
   for (k = 0; k < K; ++k)
      for (j = 0; j < J; ++j)
      {
         /* setup filter line */
         i    = 0;
         src  = (T*) VPixelPtr (Src,  *z, *y, *x);
         dest = (T*) VPixelPtr (Dest, *z, *y, *x);

         for (; i < I; ++i)
         {
            /* compute length of convolution */
            start = filter;
            stop  = filter + Width;
            if (i < N)         {start += (N - i);          }
            if (i > I - 1 - N) {stop  -= (N - (I - 1 - i));}

            /* compute convolution */
            value = 0;
            conv  = src;
            for (; start != stop; ++start)
            {
               value += (float) (*start * *conv);
               conv  += stride;
            }

            /* assign value */
            *dest = (T) value;

            /* proceed */
            if (i >= N) src += stride;
            dest += stride;
         }
      }


   /* clean-up */
   free (filter);

} /* GaussianFilter */

template <class T> void GaussianFilter (VImage& Src, float Sigma, int Width, int Dir)
{
   VImage Dest;   /* destination image */


   /* filter image */
   GaussianFilter<T> (Src, Sigma, Width, Dir, Dest);
   VDestroyImage (Src);
   Src = Dest;

} /* GaussianFilter */

/*------------------------------------------------------------------------------

3D Gaussian Filter
==================

Src     source image with voxels of type T
Sigma   standard deviation of filter
Width   width of filter (must be odd)
Dest    filtered image with voxels of type T (created)

------------------------------------------------------------------------------*/

template <class T> void GaussianFilter (VImage Src, float Sigma, int Width, VImage& Dest)
{
   VImage xtmp, ytmp;   /* temporal storage */


   /* filter image */
   GaussianFilter<T> (Src,  Sigma, Width, XGaussian, xtmp);
   GaussianFilter<T> (xtmp, Sigma, Width, YGaussian, ytmp);
   GaussianFilter<T> (ytmp, Sigma, Width, ZGaussian, Dest);

   /* clean-up */
   VDestroyImage (xtmp);
   VDestroyImage (ytmp);

} /* GaussianFilter */

template <class T> void GaussianFilter (VImage& Src, float Sigma, int Width)
{
   VImage Dest;   /* destination image */


   /* filter image */
   GaussianFilter<T> (Src, Sigma, Width, Dest);
   VDestroyImage (Src);
   Src = Dest;

} /* GaussianFilter */

/*----------------------------------------------------------------------------*/

#endif
