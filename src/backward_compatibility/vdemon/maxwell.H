/****************************************************************
 *
 * vdemon: maxwell.H
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
 * $Id: maxwell.H 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#ifndef MAXWELL_H_INCLUDED
#define MAXWELL_H_INCLUDED

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

VBoolean DemonMatch (VImage S, VImage M, VImage& Dx, VImage& Dy, VImage& Dz, VFloat Sigma, VLong Iter, VLong Scale, VBoolean Verbose = FALSE);

/*----------------------------------------------------------------------------*/

#endif
