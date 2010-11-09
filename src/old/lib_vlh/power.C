/*
**
** Powerspectrum
*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <fftw3.h>


float * VPowerSpectrum(float *data,int ntimesteps)
{
  float powermin=10000000, powermax=-10000000;
  float *powspec=NULL;
  double *in=NULL;
  int k,n,n2;
  fftw_complex *out=NULL;
  fftw_plan p1=NULL;

  n   = ntimesteps;
  n2  = (int)(n/2 + 1);

  in  = (double *)fftw_malloc(sizeof(double) * n);
  out = (fftw_complex *)fftw_malloc (sizeof (fftw_complex) * n2);

  // fill data
  for (k=0; k<n; k++) in[k] = (double)(data[k]);

  if (p1 == NULL)
      p1 = fftw_plan_dft_r2c_1d (n,in,out,FFTW_ESTIMATE);

  // rfftw
  fftw_execute(p1);

  // result
  powspec = (float *) malloc(sizeof(float) * (n2 + 2));

  // powerspec
  for (k = 1; k < n2; k++) {
    powspec[k] = (float)sqrt(out[k][0] * out[k][0] + out[k][1] * out[k][1]);
    if (powermin>powspec[k]) powermin=powspec[k];
    if (powermax<powspec[k]) powermax=powspec[k];
  }
  powspec[0]    = (float)0.0;
  powspec[n2]   = powermin;
  powspec[n2+1] = powermax;

  // return
  return powspec;
}
