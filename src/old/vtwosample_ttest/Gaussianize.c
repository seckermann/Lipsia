/*
** transform data to make them Gaussian normal distributed
**
** Ref: van Albada, Robinson (2007).
**    J. Neuroscience Methods, 161,205-211.
**
** G.Lohmann, MPI-CBS, Oct 2009
*/

#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_cdf.h>

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

extern int isnan(double);
extern int isinf(double);

/*
double
inverse_erf(double x)
{
  if (x <= -1) return -1/sqrt(2.0);
  if (x >=  1) return 1/sqrt(2.0);
  return gsl_cdf_ugaussian_Pinv((x+1)*0.5)/sqrt(2.0);
}
*/

double
Pinv(double x)
{
  if (x <= -1) return -1;
  if (x >=  1) return 1;
  return gsl_cdf_ugaussian_Pinv(x);
}


void
VGaussianize(VImage *src,int n)
{
  int i,j,k,b,r,c,nslices,nrows,ncols;
  double u,nx,sum,sum1,sum2,mean,sigma;
  double smax,smin,tiny=1.0e-6;
  double *x=NULL,*edf=NULL;

  nslices = VImageNBands(src[0]);
  nrows   = VImageNRows(src[0]);
  ncols   = VImageNColumns(src[0]);

  smax = VPixelMaxValue(src[0]);
  smin = VPixelMinValue(src[0]);

  nx   = (double)n;
  x    = (double *) VCalloc(n,sizeof(double));
  edf  = (double *) VCalloc(n,sizeof(double));

  for (b=0; b<nslices; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {

	k = 0;
	for (i=0; i<n; i++) {
	  u = VGetPixel(src[i],b,r,c);
	  x[i] = u;
	  if (ABS(u) > tiny) k++;
	}
	if (k < n) continue;


	/* empirical mean, std */
	sum1 = sum2 = 0;
	for (i=0; i<n; i++) {
	  sum1 += x[i];
	  sum2 += x[i]*x[i];
	}
	mean = sum1/nx;
	sigma = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));


	/* empirical distribution function */
	for (i=0; i<n; i++) {
	  sum = 0;
	  u = x[i];
	  for (j=0; j<n; j++) if (x[j] <= u) sum++;
	  edf[i] = sum/nx;
	}

	/* apply transform to normal distribution */
	for (i=0; i<n; i++) {
	  u = mean + sigma*Pinv(edf[i]);
	  if (isnan(u)) u = 0;
	  if (isinf(u)) u = 0;
	  if (u < smin) u = smin;
	  if (u > smax) u = smax;
	  VSetPixel(src[i],b,r,c,u);
	}
      }
    }
  }
}
