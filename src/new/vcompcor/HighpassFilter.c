/*
** Highpass filter using FFT
**
** G.Lohmann, MPI-CBS, May 2011
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <fftw3.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))

void
HighPassFilter(double *z,int n,float tr,VFloat high)
{
  int i,j,nn,nc,tail;
  static double *in=NULL;
  static double *highp=NULL;
  static fftw_complex *out=NULL;
  double sharp=0.8,x,alpha;
  fftw_plan p1,p2;

  tail = n/10;
  if (tail < 50) tail = 50;
  if (tail >= n/2) tail = n/2-1;
  if (tail >= n-1) tail = n-2;
  if (tail < 0) tail = 0;

  if(n <= tail) tail = n - 2;
  nn  = n + 2*tail;

  nc  = (nn / 2) + 1;
  if (out == NULL) {
    in  = (double *) VCalloc(nn,sizeof(double));
    out = fftw_malloc (sizeof (fftw_complex ) * nc);
  }

  i = 0;
  for(j=0; j<tail; j++) in[i++] = z[tail-j];
  for(j=0; j<n; j++) in[i++] = z[j];
  j = n-2;
  while (i < n && j >= 0) in[i++] = z[j];

  /*
  for (i=0; i<n; i++) in[i] = z[i];
  */

  /* make plans */
  p1 = fftw_plan_dft_r2c_1d (nn,in,out,FFTW_ESTIMATE);
  p2 = fftw_plan_dft_c2r_1d (nn,out,in,FFTW_ESTIMATE);

  alpha = (double)n * tr;
  if (highp == NULL) highp = (double *) VCalloc(nc,sizeof(double));

  sharp = 0.8;
  for (i=1; i <nc; i++) {
    highp[i] = 1.0 / (1.0 +  exp( (alpha/high -(double)i)*sharp ));
  }

  /* forward fft */
  fftw_execute(p1);

  /* highpass */
  for (i=1; i<nc; i++) {
    x = highp[i];
    out[i][0] *= x;
    out[i][1] *= x;
  }

  /* inverse fft */
  fftw_execute(p2);
  for (i=0; i<n; i++) z[i] = in[i+tail]/(double)n;
}
