#ifndef _CONTRAST_H_
#define _CONTRAST_H_

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

LinearContrast
==============

Src     source image with voxels of type T
Min     minimum value of contrasted image
Max     maximum value of contrasted image
Dest    contrasted image with voxels of type T (created)
Black   amount of lower histogram that is set to black (0 <= Black <= 1, default 0.01)
White   amount of upper histogram that is set to white (0 <= White <= 1, default 0.01)

Note: This function returns an error message for images of type VLong, VFloat
      and VDouble. These types require a sparse histogram, which is not yet
      implemented.

------------------------------------------------------------------------------*/

template <class T> void LinearContrast (VImage Src, int Min, int Max, VImage& Dest, 
					float Black = 0.001, float White = 0.001)
{
   int Voxels;   /* number of voxels */

   long  min, max;        /* min/max voxel values  */
   long  size;            /* size of histogram     */
   float* hist;           /* image histogram       */
//   long  lower, upper;    /* histogram borders     */
//   float scale;           /* linear scaling factor */
   float xmin,xmax,sum1,sum2,ave,sigma,nx,sum,u,v,slope;
   float background = 1;
   int   nbands,nrows,ncols;
   float sig2,/*alpha,norm,*/sumw,w,d;
   int   i,j,k,ws;
   float *histo;
   float *lut;
   VString str;
   VBoolean angio=FALSE;
   int type=0;  // contrast stretching method: 0=linear, 1=histo equalization


   T*    src;     /* source data pointer      */
   T*    dest;    /* destination data pointer */
   float value;   /* voxel value              */

   long n;   /* index */


   /* check image representation */
   if ((VPixelRepn (Src) == VLongRepn) || (VPixelRepn (Src) == VFloatRepn) || (VPixelRepn (Src) == VDoubleRepn))
   {
      VError ("LinearContrast does not support images of type VLong, VFloat and VDouble");
      return;
   }

   /* get source image size */
   Voxels = VImageNPixels (Src);

   /* create contrasted image */
   Dest = VCreateImage (VImageNBands (Src), VImageNRows (Src), VImageNColumns (Src), VPixelRepn (Src));
   VImageAttrList (Dest) = VCopyAttrList (VImageAttrList (Src));


   // check if it is an angio sequence (G.L., 14.7.2003)
   if (VGetAttr (VImageAttrList (Src), "protocol", NULL,
		 VStringRepn, (VPointer) & str) == VAttrFound) {
     angio = FALSE;
     if (str[0] == 't' && str[1] == 'o' && str[2] == 'f' && str[3] == '_')
       angio = TRUE;
   }     


   /* create histogram */
   min  = (long) VPixelMinValue (Src);
   max  = (long) VPixelMaxValue (Src);
   size = (long) (VPixelMaxValue (Src) - VPixelMinValue (Src) + 1);
   hist = new float[size];
   hist -= min;

   histo = new float[size];
   histo -= min;
   lut   = new float[size];
   lut -= min;


   // 
   // estimate background grey values, assume that background is dark
   //
   ncols  = VImageNColumns(Src);
   nrows  = VImageNRows(Src);
   nbands = VImageNBands(Src);

   sum1 = sum2 = nx = 0;
   n = 12;
   if (ncols < n) n=ncols/2;
   if (nrows < n) n=nrows/2;
   for (i=1; i<n; i++) {
     for (j=1; j<n; j++) {
       value = VGetPixel(Src,0,i,j);
       sum1 += value;
       sum2 += (value*value);
       nx++;
     }
   }
   ave = sum1/nx;
   sigma = sqrt((double)((sum2 - nx * ave * ave) / (nx - 1.0)));
   background = ave + 2.0*sigma;



   //
   // get image histogram
   //
   for (n = min; n <= max; n++) hist[n] = histo[n] = lut[n] = 0;
   src = (T*) VPixelPtr (Src, 0, 0, 0);
   for (n = 0; n < Voxels; n++) {
     j = *(src++);
     if (type == 1 && j > background) hist[j]++;
     if (type == 0) hist[j]++;
   }

   
   //
   // histogram equalization with Gaussian blurring of the histogram
   //
   sigma = 90;
   ws    = (int)(2.5*sigma);
   sig2  = 2.0 * sigma * sigma;
    
   for (j=min; j<=max; j++) {
     sum = sumw = 0;
     for (k=j-ws; k<=j+ws; k++) {
       if (k < min || k >= max) continue;
       d = j-k;
       w =  exp((double)(- d*d/sig2));
       sum  += w*hist[k];
       sumw += w;
     }
     if (sumw > 0 && j > background) histo[j] = sum / sumw;
   }

   sum = 0;
   for (j=min; j<=max; j++) sum += histo[j];
   for (j=min; j<=max; j++) histo[j] /= sum;
  
   for (i=min; i<=max; i++) {
     sum = 0;
     for (j=min; j<=i; j++) sum += histo[j];
     if (sum < 0) sum = 0;
     if (sum > 1) sum = 1;
     lut[i] = sum * 255.0f;
   }
   // if (type == 0) goto next;


   // 
   // standard linear contrast
   //
   Black = 0.001;
   White = 0.001;

   if (angio) {
     Black = 0.01;
     White = 0.00065;
   }

   sum = 0;
   for (n=min; n<=max; n++) sum += hist[n];
   for (n=min; n<=max; n++) hist[n] /= sum;

   xmin = 0;
   sum  = 0;
   for (n=min; n<max; n++) {
     if (n > background) sum += hist[n];
     if (sum > Black) break;
   }
   xmin = n;

   xmax = 255.0;
   sum = 0;
   for (n=max; n>min; n--) {
     if (n > background) sum += hist[n];
     if (sum > White) break;
   }
   xmax = n;

   slope = 255.0 / (xmax - xmin);


   //
   // get transfer function
   // histo equalization plus linear contrast stretch
   //
   for (n=min; n<max; n++) {
     value = n;
     v = slope * (value - xmin);
     u = lut[n];
     lut[n] = rint((double)(0.0*u  + 1.0*v));
   }


   //
   // apply contrast
   //
   src  = (T*) VPixelPtr (Src,  0, 0, 0);
   dest = (T*) VPixelPtr (Dest, 0, 0, 0);
   for (n = 0; n < Voxels; n++)
     {
       value = (float) *(src++);
       j = (int) value;
       v = rint((double)lut[j]);
       if (v < 0)     v = 0;
       if (v > 255)   v = 255;

       *(dest++) = (T) v;
     }


   /* clean-up */
   hist += min;
   delete[] hist;

} /* LinearContrast */


template <class T> void LinearContrast (VImage& Src, int Min, int Max, float Black = 0.01, float White = 0.01)
{
   VImage Dest;   /* destination image */


   /* apply linear contrast */
   LinearContrast<T> (Src, Min, Max, Dest, Black, White);
   VDestroyImage (Src);
   Src = Dest;

} /* LinearContrast */

/*----------------------------------------------------------------------------*/

#endif /*_CONTRAST_H_*/
