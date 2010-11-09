#ifndef _CONVERT_H_
#define _CONVERT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include <Vlib.h>
#include <VImage.h>
#include <option.h>
#include <mu.h>


/*------------------------------------------------------------------------------

ConvertType
===========

Src    source image with voxels of type T
Dest   converted image with voxels of type U (created)
Type   type of created image (MUST correspond to type U)

------------------------------------------------------------------------------*/

template <class T, class U> void ConvertType (VImage Src, VImage& Dest, VRepnKind Type)
{
   int Voxels;   /* number of voxels */

   T* src;    /* source data pointer      */
   U* dest;   /* destination data pointer */

   long n;   /* index */


   /* get source image size */
   Voxels = VImageNPixels (Src);

   /* create converted image */
   Dest = VCreateImage (VImageNBands (Src), VImageNRows (Src), VImageNColumns (Src), Type);
   VImageAttrList (Dest) = VCopyAttrList (VImageAttrList (Src));


   /* convert image */
   src  = (T*) VPixelPtr (Src,  0, 0, 0);
   dest = (U*) VPixelPtr (Dest, 0, 0, 0);
   for (n = 0; n < Voxels; n++)
      *(dest++) = (U) *(src++);

} /* ConvertType */

template <class T, class U> void ConvertType (VImage& Src, VRepnKind Type)
{
   VImage Dest;   /* destination image */


   /* convert image */
   ConvertType<T,U> (Src, Dest, Type);
   VDestroyImage (Src);
   Src = Dest;

} /* ConvertType */

/*----------------------------------------------------------------------------*/

#endif /*_CONVERT_H_*/
