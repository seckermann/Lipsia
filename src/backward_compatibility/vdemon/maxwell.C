/****************************************************************
 *
 * vdemon: maxwell.C
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
 * $Id: maxwell.C 3181 2008-04-01 15:19:44Z karstenm $
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

#include "util.H"
#include "gradient.H"
#include "gauss.H"
#include "trilinear.H"
#include "maxwell.H"


/*------------------------------------------------------------------------------

DemonMatch
==========

This function implements a non-linear image matching with Maxwell's demons.

Note: The algorithm is described as "Demons 1" with bijective extension in

      Thirion, J.-P.
      "Image matching as a diffusion process: an analogy with Maxwell's demons"
      Medical Image Analysis 2(3), 243-260, 1998

S         anatomical source image as fixed reference
M         anatomical model image to be deformed
Dx        resulting x-deformation field with voxels of type VFloat (created)
Dy        resulting y-deformation field with voxels of type VFloat (created)
Dz        resulting z-deformation field with voxels of type VFloat (created)
Sigma     standard deviation of Gaussian filter (recommendation is Sigma = 1)
Iter      number of iterations at finest scale (recommendation is Iterations = 4)
Scale     coarsest scale of matching (recommendation is Scale = 3)
Verbose   verbose flag enables status messages to stderr
          (if skipped the default is FALSE)

------------------------------------------------------------------------------*/

template <class T> void OpticalFlow (VImage S, VImage M, VImage G, VImage G2, VImage& V);

VBoolean DemonMatch (VImage S, VImage M, VImage& Dx, VImage& Dy, VImage& Dz, VFloat Sigma, VLong Iter, VLong Scale, VBoolean Verbose)
{
   int Factor = 4;   /* scaling factor of iterations at each step */

   int scale;   /* scaling level       */
   int iter;    /* number of iteration */

   VImage *s, *m;                /* source and model pyramid   */
   VImage gsx, gsy, gsz,  gs2;   /* source gradient field      */
   VImage gmx, gmy, gmz,  gm2;   /* model gradient field       */
   VImage dsx, dsy, dsz;         /* source deformation field   */
   VImage dmx, dmy, dmz;         /* model deformation field    */
   VImage vx,  vy,  vz;          /* optical flow field         */
   VImage rx,  ry,  rz;          /* residual deformation field */

   int width = 2 * (2 * (int) floor (Sigma) + 1) + 1;   /* Lohmann, G. "Volumetric Image Analysis", p. 141, Teubner, 1998 */

   VImage tmp;   /* temporal storage */


   /* create multi-scale pyramids */
   s = (VImage*) malloc ((Scale + 1) * sizeof (VImage));
   m = (VImage*) malloc ((Scale + 1) * sizeof (VImage));

   /* compute multi-scale pyramids */
   s[0] = S;
   m[0] = M;
   for (scale = 1; scale <= Scale; scale++, Iter *= Factor)
   {
      RTTI (S, GaussianFilter, (s[scale-1], 0.5, 3, s[scale]));
      RTTI (S, TrilinearScale, (s[scale], 0.5, 0.5, 0.5));

      RTTI (M, GaussianFilter, (m[scale-1], 0.5, 3, m[scale]));
      RTTI (M, TrilinearScale, (m[scale], 0.5, 0.5, 0.5));
   }

   /* create deformation fields */
   dsx = VCreateImage (VImageNBands (s[Scale]), VImageNRows (s[Scale]), VImageNColumns (s[Scale]), VFloatRepn);
   dsy = VCreateImage (VImageNBands (s[Scale]), VImageNRows (s[Scale]), VImageNColumns (s[Scale]), VFloatRepn);
   dsz = VCreateImage (VImageNBands (s[Scale]), VImageNRows (s[Scale]), VImageNColumns (s[Scale]), VFloatRepn);
   dmx = VCreateImage (VImageNBands (m[Scale]), VImageNRows (m[Scale]), VImageNColumns (m[Scale]), VFloatRepn);
   dmy = VCreateImage (VImageNBands (m[Scale]), VImageNRows (m[Scale]), VImageNColumns (m[Scale]), VFloatRepn);
   dmz = VCreateImage (VImageNBands (m[Scale]), VImageNRows (m[Scale]), VImageNColumns (m[Scale]), VFloatRepn);

   /* initialize deformation fields */
   VFillImage (dsx, VAllBands, 0);
   VFillImage (dsy, VAllBands, 0);
   VFillImage (dsz, VAllBands, 0);
   VFillImage (dmx, VAllBands, 0);
   VFillImage (dmy, VAllBands, 0);
   VFillImage (dmz, VAllBands, 0);


   /* multi-scale scheme */
   for (scale = Scale; scale >= 0; scale--, Iter /= Factor)
   {
      /* print status */
      if (Verbose) {fprintf (stderr, "Working hard at scale %d ...                 \n", scale + 1); fflush (stderr);}

      /* compute gradients */
      RTTI (S, Gradient, (s[scale], gsx, gsy, gsz));
      SquaredGradient (gsx, gsy, gsz, gs2);
      RTTI (M, Gradient, (m[scale], gmx, gmy, gmz));
      SquaredGradient (gmx, gmy, gmz, gm2);

      /* diffusion process */
      for (iter = 1; iter <= Iter; iter++)
      {
         /* print status */
         if (Verbose) {fprintf (stderr, "Iteration %d of %d ...\r", iter, Iter); fflush (stderr);}


         /* FORWARD */

         /* interpolate deformed image */
         RTTI (S, TrilinearInverseDeform, (s[scale], dmx, dmy, dmz, tmp));

         /* compute deformation */
         RTTI (M, OpticalFlow, (m[scale], tmp, gmx, gm2, vx));
         RTTI (M, OpticalFlow, (m[scale], tmp, gmy, gm2, vy));
         RTTI (M, OpticalFlow, (m[scale], tmp, gmz, gm2, vz));
         Subtract<VFloat> (dmx, vx);
         Subtract<VFloat> (dmy, vy);
         Subtract<VFloat> (dmz, vz);

         /* clean-up */
         VDestroyImage (tmp);
         VDestroyImage (vx);
         VDestroyImage (vy);
         VDestroyImage (vz);


         /* BACKWARD */

         /* interpolate deformed image */
         RTTI (M, TrilinearInverseDeform, (m[scale], dsx, dsy, dsz, tmp));

         /* compute deformation */
         RTTI (S, OpticalFlow, (s[scale], tmp, gsx, gs2, vx));
         RTTI (S, OpticalFlow, (s[scale], tmp, gsy, gs2, vy));
         RTTI (S, OpticalFlow, (s[scale], tmp, gsz, gs2, vz));
         Subtract<VFloat> (dsx, vx);
         Subtract<VFloat> (dsy, vy);
         Subtract<VFloat> (dsz, vz);

         /* clean-up */
         VDestroyImage (tmp);
         VDestroyImage (vx);
         VDestroyImage (vy);
         VDestroyImage (vz);


         /* BIJECTIVE */

         /* compute residual deformation */
         TrilinearInverseDeform<VFloat> (dsx, dmx, dmy, dmz, rx);
         TrilinearInverseDeform<VFloat> (dsy, dmx, dmy, dmz, ry);
         TrilinearInverseDeform<VFloat> (dsz, dmx, dmy, dmz, rz);
         Add<VFloat> (rx, dmx);
         Add<VFloat> (ry, dmy);
         Add<VFloat> (rz, dmz);
         Multiply<VFloat> (rx, 0.5);
         Multiply<VFloat> (ry, 0.5);
         Multiply<VFloat> (rz, 0.5);

         /* update deformation fields */
         Subtract<VFloat> (dmx, rx);
         Subtract<VFloat> (dmy, ry);
         Subtract<VFloat> (dmz, rz);
         TrilinearInverseDeform<VFloat> (rx, dsx, dsy, dsz);
         TrilinearInverseDeform<VFloat> (ry, dsx, dsy, dsz);
         TrilinearInverseDeform<VFloat> (rz, dsx, dsy, dsz);
         Subtract<VFloat> (dsx, rx);
         Subtract<VFloat> (dsy, ry);
         Subtract<VFloat> (dsz, rz);

         /* clean-up */
         VDestroyImage (rx);
         VDestroyImage (ry);
         VDestroyImage (rz);


         /* regularize deformation fields */
         GaussianFilter<VFloat> (dsx, Sigma, width);
         GaussianFilter<VFloat> (dsy, Sigma, width);
         GaussianFilter<VFloat> (dsz, Sigma, width);
         GaussianFilter<VFloat> (dmx, Sigma, width);
         GaussianFilter<VFloat> (dmy, Sigma, width);
         GaussianFilter<VFloat> (dmz, Sigma, width);
      }

      if (scale > 0)
      {
         /* downscale deformation fields */
         TrilinearScale<VFloat> (dsx, 2.0, 2.0, 2.0); Multiply<VFloat> (dsx, 2.0);
         TrilinearScale<VFloat> (dsy, 2.0, 2.0, 2.0); Multiply<VFloat> (dsy, 2.0);
         TrilinearScale<VFloat> (dsz, 2.0, 2.0, 2.0); Multiply<VFloat> (dsz, 2.0);
         TrilinearScale<VFloat> (dmx, 2.0, 2.0, 2.0); Multiply<VFloat> (dmx, 2.0);
         TrilinearScale<VFloat> (dmy, 2.0, 2.0, 2.0); Multiply<VFloat> (dmy, 2.0);
         TrilinearScale<VFloat> (dmz, 2.0, 2.0, 2.0); Multiply<VFloat> (dmz, 2.0);

         /* clean-up */
         VDestroyImage (s[scale]);
         VDestroyImage (m[scale]);
      }

      /* clean-up */
      VDestroyImage (gsx);
      VDestroyImage (gsy);
      VDestroyImage (gsz);
      VDestroyImage (gs2);
      VDestroyImage (gmx);
      VDestroyImage (gmy);
      VDestroyImage (gmz);
      VDestroyImage (gm2);
   }

   /* return inverse deformation field */
   Dx = dsx;
   Dy = dsy;
   Dz = dsz;

   /* clean-up */
   VDestroyImage (dmx);
   VDestroyImage (dmy);
   VDestroyImage (dmz);
   free (s);
   free (m);

   return TRUE;

} /* DemonMatch */

/*----------------------------------------------------------------------------*/

template <class T> void OpticalFlow (VImage S, VImage M, VImage G, VImage G2, VImage& V)
{
   const float EPSILON = 1;   /* threshold for denominator of optical flow */

   int Pixels;   /* number of pixels */

   T      *s, *m;    /* source and model data pointer */
   VFloat *g, *g2;   /* gradient data pointer         */
   VFloat *v;        /* optical flow data pointer     */

   float nom, denom;   /* fraction */

   int n;   /* index */


   /* get image size */
   Pixels = VImageNPixels (S);

   /* create optical flow */
   V = VCreateImage (VImageNBands (S), VImageNRows (S), VImageNColumns (S), VFloatRepn);


   /* compute optical flow */
   s  = (T*)      VPixelPtr (S,  0, 0, 0);
   m  = (T*)      VPixelPtr (M,  0, 0, 0);
   g  = (VFloat*) VPixelPtr (G,  0, 0, 0);
   g2 = (VFloat*) VPixelPtr (G2, 0, 0, 0);
   v  = (VFloat*) VPixelPtr (V,  0, 0, 0);
   for (n = 0; n < Pixels; ++n)
   {
      /* compute fraction */
      nom   = (float) (*(m++) - *(s++));
      denom = (float) (*(g2++) + (nom * nom));

      /* assign optical flow */
      if (denom < EPSILON) *(v++) = 0;
      else                 *(v++) = (VFloat) ((nom / denom) * *g);
      ++g;
   }

} /* OpticalFlow */

/*----------------------------------------------------------------------------*/
