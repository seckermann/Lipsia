#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <via.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))



void
ApplyTransform2d(VImage *src,int nslices,int nrows,int ncols,int i2,gsl_vector *vec)
{
  static VImage tmp=NULL;
  VShort *ptr1,*ptr2;
  int    b,r,c,i;
  int    rq,cq;
  double ra,ca,rb,cb,val;
  double rp,cp,rx,cx;
  double a[2][2],x[2][2],detA;
  double shift[2],alpha;
  double xcos,xsin;
  double deg,pi=3.14159265;


  /* get parameters */
  shift[0] = gsl_vector_get (vec, 0);
  shift[1] = gsl_vector_get (vec, 1);
  alpha    = gsl_vector_get (vec, 2);

  deg    =  180.0f / pi;
  alpha /= deg;
  xcos   = cos(alpha);
  xsin   = sin(alpha);

  x[0][0] =  xcos;
  x[0][1] = -xsin;
  x[1][0] =  xsin;
  x[1][1] =  xcos;


  /* invert transformation matrix, Fischer, p. 152 */
  detA = (x[0][0]*x[1][1] - x[0][1]*x[1][0]);
  if (detA == 0) VError(" Resampling: matrix not invertible");

  a[0][0] =  x[1][1]/detA;
  a[0][1] = -x[0][1]/detA;
  a[1][0] = -x[1][0]/detA;
  a[1][1] =  x[0][0]/detA;



  /* Determines the value of each pixel in the destination image: */
  if (tmp == NULL)
    tmp = VCreateImage(nslices,nrows,ncols,VShortRepn);
  VFillImage(tmp,VAllBands,0);


  for (b=0; b<nslices; b++) {
    if (VImageNRows(src[b]) < 2) continue;

    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {

	rx = (double) r - shift[0];
        cx = (double) c - shift[1];

        rp = a[0][0] * rx + a[0][1] * cx;
        cp = a[1][0] * rx + a[1][1] * cx;

        rq = (int) rp;
        cq = (int) cp;

        /* check subcube */
        if ((cq < 0) || (cq >= ncols  - 1)) continue;
        if ((rq < 0) || (rq >= nrows  - 1)) continue;


        /* compute fractions of subcube */
        ca = cp  - (double)cq;
        cb = 1.0 - ca;
        ra = rp  - (double)rq;
        rb = 1.0 - ra;

        val  = rb * cb * VPixel(src[b],i2,rq,cq,VShort);
        val += rb * ca * VPixel(src[b],i2,rq,cq+1,VShort);
        val += ra * cb * VPixel(src[b],i2,rq+1,cq,VShort);
        val += ra * ca * VPixel(src[b],i2,rq+1,cq+1,VShort);

	VPixel(tmp,b,r,c,VShort) = (int)(val + 0.4999);
      }
    }
  }

  ptr1 = (VShort *) VImageData(tmp);
  for (b=0; b<nslices; b++) {
    ptr2 = (VShort *) VPixelPtr(src[b],i2,0,0);
    for (i=0; i<nrows*ncols; i++) {
      (*ptr2++) = (*ptr1++);
    }
  }
}

