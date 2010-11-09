/*
** Linear Pearson correlation
** Spearman rank correlation
**
** G.Lohmann, MPI-CBS, March 2007
*/
#include <viaio/Vlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

extern int isnanf(float);
extern int isinff(float);
extern int isnan(double);
extern int isinf(double);

extern double gsl_sf_beta_inc(double,double,double);


/* Fisher's z-transform */
double
FisherTransform(double rx)
{
  double u;
  static double z=0;

  u = 0.999;
  if (rx >  u) rx = u;
  if (rx < -u) rx = -u;
  z = 0.5 * log((1.0 + rx)/(1.0 - rx));
  return z;
}


double
CorrelationPearson(gsl_vector *array1,gsl_vector *array2,int type)
{
  int j,n;
  double xt,yt,sxx,syy,sxy,rx;
  double ave1,ave2;
  double df,t,a,b,x,p,z;
  double *ptr1,*ptr2;
  double tiny=1.0e-20;

  gsl_set_error_handler_off();

  n = array1->size;
  if (array2->size != n) VError(" inconsistent length of vectors");

  ptr1 = array1->data;
  ptr2 = array2->data;
  ave1 = ave2 = 0;
  for (j=0; j<n; j++) {
    ave1 += *ptr1++;
    ave2 += *ptr2++;
  }
  ave1 /= (double)n;
  ave2 /= (double)n;

  sxx = syy = sxy = xt = yt = rx = 0;
  ptr1 = array1->data;
  ptr2 = array2->data;
  for (j=0; j<n; j++) {
    xt = (*ptr1++) - ave1;
    yt = (*ptr2++) - ave2;
    sxx += xt * xt;
    syy += yt * yt;
    sxy += xt * yt;
  }
  
  rx = 0;
  if (sxx > tiny && syy > tiny) rx = sxy / sqrt(sxx * syy);

  switch(type) {

  case 0:
    return rx;

  case 1:
    z = FisherTransform(rx);
    if (isnan(z) || isinf(z)) return 0;
    return z;

  case 2:
    df = n-2;
    t = rx*sqrt(df/((1.0-rx+tiny)*(1.0+rx+tiny)));
    a = 0.5*df;
    b = 0.5;
    x = df/(df+(t*t));
    p = gsl_sf_beta_inc(a,b,x);
    if (isnan(p) || isinf(p)) return 0;
    return p;

  default:
    VError(" illegal output type");
  }
  return 0;
}



double
CorrelationSpearman(gsl_vector *array1,gsl_vector *array2,int type)
{
  int i,j,n;
  double d,n1,n3,rx;
  double a,b,x,t,df,p;
  static gsl_permutation *perm1=NULL,*rank1=NULL;
  static gsl_permutation *perm2=NULL,*rank2=NULL;
  extern void gsl_sort_vector_index(gsl_permutation *,gsl_vector *);

  gsl_set_error_handler_off();


  n = array1->size;
  if (array2->size != n) VError(" inconsistent length of vectors");

  if (perm1 == NULL) {
    perm1 = gsl_permutation_calloc(n);
    rank1 = gsl_permutation_calloc(n);
    perm2 = gsl_permutation_calloc(n);
    rank2 = gsl_permutation_calloc(n);
  }

  gsl_sort_vector_index(perm1,array1);
  gsl_permutation_inverse(rank1,perm1);

  gsl_sort_vector_index(perm2,array2);
  gsl_permutation_inverse(rank2,perm2);

  d = 0;
  for (i=0; i<n; i++) {
    j = rank1->data[i] - rank2->data[i];
    d += (double)(j*j);
  }
  if (isnan(d) || isinf(d)) return 0;

  n1 = (double)n;
  n3 = (double)(n*n*n);
  rx = 1.0 - (6.0 * d) / (n3 - n1);

  switch(type) {

  case 0:
    return rx;

  case 1:
    return FisherTransform(rx);

  case 2:
    df=n-2;
    t = 0;
    if (rx < 1) 
      t = rx * sqrt((n1-2.0) / (1.0-rx*rx));
    a = 0.5*df;
    b = 0.5;
    x = df/(df+(t*t));
    p = gsl_sf_beta_inc(a,b,x);
    return p;

  case 3:
    return d;

  default:
    VError(" illegal output type");
  }
  return 0;
}
