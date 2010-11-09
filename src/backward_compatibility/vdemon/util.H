/****************************************************************
 *
 * vdemon: util.H
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
 * $Id: util.H 704 2004-02-16 09:41:44Z karstenm $
 *
 *****************************************************************/


#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

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


/*----------------------------------------------------------------------------*/

/* macro for RTTI-dependent functions */
#define RTTI(Image, Function, Arguments)                  \
switch (VPixelRepn (Image))                               \
{                                                         \
   case VBitRepn:    Function <VBit>    Arguments; break; \
   case VUByteRepn:  Function <VUByte>  Arguments; break; \
   case VSByteRepn:  Function <VSByte>  Arguments; break; \
   case VShortRepn:  Function <VShort>  Arguments; break; \
   case VLongRepn:   Function <VLong>   Arguments; break; \
   case VFloatRepn:  Function <VFloat>  Arguments; break; \
   case VDoubleRepn: Function <VDouble> Arguments; break; \
   default: break;                                        \
}

/*------------------------------------------------------------------------------

Image Arithmetics (voxels of type T)
=================

Add:      Sum  += Add
Subtract: Diff -= Sub
Multiply: Prod *= Mult

------------------------------------------------------------------------------*/

template <class T> void Add (VImage Sum, VImage Add)
{
   T *sum, *add;   /* sum and addend data pointer */
   T *stop;        /* end of sum pointer          */

   /* add values */
   sum  = (T*) VPixelPtr (Sum, 0, 0, 0);
   add  = (T*) VPixelPtr (Add, 0, 0, 0);
   stop = sum + VImageNPixels (Sum);
   while (sum != stop)
      *(sum++) += *(add++);

} /* Add */

template <class T> void Subtract (VImage Diff, VImage Sub)
{
   T *diff, *sub;   /* difference and subtrahend data pointer */
   T *stop;         /* end of difference pointer              */

   /* subtract values */
   diff = (T*) VPixelPtr (Diff, 0, 0, 0);
   sub  = (T*) VPixelPtr (Sub,  0, 0, 0);
   stop = diff + VImageNPixels (Diff);
   while (diff != stop)
      *(diff++) -= *(sub++);

} /* Subtract */

template <class T> void Multiply (VImage Prod, float Mult)
{
   T *prod;   /* product data pointer   */
   T *stop;   /* end of product pointer */

   /* multiply values */
   prod = (T*) VPixelPtr (Prod, 0, 0, 0);
   stop = prod + VImageNPixels (Prod);
   while (prod != stop)
      *(prod++) *= Mult;

} /* Multiply */

/*----------------------------------------------------------------------------*/

#endif
