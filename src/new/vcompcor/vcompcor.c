/*
** regress out physiological noise using tCompCor
** Ref: Behzadi et al. (2007). Neuroimage,37:90-101.
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

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit.h>

#define NSLICES 2000

extern int isnan(double);
extern int isinf(double);
extern gsl_matrix *VPCA(gsl_matrix *,int,int);

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


double
Variance(const double *data,int n,double *ave)
{
  int i;
  double sum1,sum2,nx,mean,var;

  nx = (double)n;
  sum1 = sum2 = 0;
  for (i=0; i<n; i++) {
    const double u = data[i];
    sum1 += u;
    sum2 += u*u;
  }
  mean = sum1/nx;
  var = (sum2 - nx * mean * mean) / (nx - 1.0);
  *ave = mean;
  return var;
}


void
ScaleX(gsl_matrix *X)
{
  int i,j;
  double mean,var,sigma,*data;
  
  data = (double *) VCalloc(X->size1,sizeof(double));
  for (i=0; i<X->size2-1; i++) {
    for (j=0; j<X->size1; j++) data[j] = gsl_matrix_get(X,j,i);
    var = Variance(data,X->size1,&mean);
    sigma = sqrt(var);
    if (sigma < 1.0e-6) continue;
    for (j=0; j<X->size1; j++) 
      gsl_matrix_set(X,j,i,(data[j] - mean)/sigma);
  }    
}



VAttrList
VCompCor(VAttrList list,VShort minval,int ncomp,int first,VFloat fraction,VFloat low,VFloat high)
{
  VAttrList r2_list=NULL;
  VImage src[NSLICES],map=NULL,dest0=NULL,dest1=NULL;
  VAttrListPosn posn;
  int b,r,c;
  size_t i,j,n=0,k=0;
  int nslices,nrows,ncols,nt,ntimesteps;
  double u=0,threshold=0,smin,smax;
  double sum1,sum2,mean,sigma,nx,t,s;
  double *data=NULL,*tc=NULL;
  gsl_matrix *A=NULL,*B=NULL,*X=NULL;
  gsl_matrix_float *mat=NULL;
  gsl_vector *y=NULL,*beta=NULL,*residuals=NULL;
  double chisq;
  gsl_matrix *cov=NULL;
  gsl_multifit_linear_workspace *work=NULL;
  extern void VFreqFilter(VAttrList, VFloat, VFloat, VBoolean, VFloat);
  extern void VApplyMinval(VAttrList, VShort);


  /*
  ** get image dimensions
  */
  i = ntimesteps = nrows = ncols = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src[i]);
    if (VPixelRepn(src[i]) != VShortRepn) continue;

    if (VImageNBands(src[i]) > ntimesteps) ntimesteps = VImageNBands(src[i]);
    if (VImageNRows(src[i])  > nrows)  nrows = VImageNRows(src[i]);
    if (VImageNColumns(src[i]) > ncols) ncols = VImageNColumns(src[i]);
    i++;
    if (i >= NSLICES) VError(" too many slices");
  }
  nslices = i;

  if(minval > 0) VApplyMinval(list, minval);
  if (high > 0) {
    float sharp = 0.8;
    VFreqFilter(list, high, low, FALSE, sharp);
  }


  /* get time steps to include */
  nt = ntimesteps;
  if (nt < 2) VError(" not enough timesteps, nt= %d",nt);
  if (ncomp >= nt/4) VError(" ncomp too large (%d), should be < %d",ncomp,nt/4);

  /* count number of voxels */
  n = 0;
  for (b=0; b<nslices; b++) {
    if (VImageNRows(src[b]) < 2) continue;
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	if (VPixel(src[b],0,r,c,VShort) < 1) continue;
	n++;
      }
    }
  }
  fprintf(stderr,"# nvoxels: %ld,  ntimesteps: %ld\n",(long)n,(long)nt);


  /*
  ** map of voxel addresses
  */
  map = VCreateImage(1,5,n,VFloatRepn);
  if (map == NULL) VError(" error allocating addr map");
  VFillImage(map,VAllBands,0);

  i = 0;
  for (b=0; b<nslices; b++) {
    if (VImageNRows(src[b]) < 2) continue;
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	if (VPixel(src[b],0,r,c,VShort) < 1) continue;
	VPixel(map,0,0,i,VFloat) = b;
	VPixel(map,0,1,i,VFloat) = r;
	VPixel(map,0,2,i,VFloat) = c;
	i++;
      }
    }
  }


  /*
  ** avoid casting to float, copy data to matrix
  */
  fprintf(stderr,"# data matrix, size: %.3f Mbyte...\n",(float)(n*nt*4)/(float)(1024.0*1024.0));
  mat = gsl_matrix_float_calloc(n,nt);
  for (i=0; i<n; i++) {
    b = VPixel(map,0,0,i,VFloat);
    r = VPixel(map,0,1,i,VFloat);
    c = VPixel(map,0,2,i,VFloat);
    if (VPixel(src[b],0,r,c,VShort) < 1) continue;

    float *ptr = gsl_matrix_float_ptr(mat,i,0);
    for (k=0; k<nt; k++) {
      *ptr++ = (float) VPixel(src[b],k,r,c,VShort);
    }
  }

  /* copy to data matrix */
  data = (double *) VCalloc(n,sizeof(double));
  tc   = (double *) VCalloc(nt,sizeof(double));
  k = 0;
  for (i=0; i<n; i++) {
    if (i%10000 == 0) fprintf(stderr," %7ld of %ld\r",(long)i,(long)n);
    float *ptr = gsl_matrix_float_ptr(mat,i,0);
    for (j=0; j<nt; j++) tc[j] = (double)ptr[j];    
    /* if (high > 1) HighPassFilter(tc,nt,tr,high); */
    u = Variance(tc,nt,&mean);
    u = sqrt(u);
    VPixel(map,0,3,i,VFloat) = u;
    data[k++] = u;
    for (j=0; j<nt; j++) {
      gsl_matrix_float_set(mat,i,j,(float)tc[j]);
    }
  }
  
  
  /* get upper fraction of variance */
  fprintf(stderr,"# fraction of variance...\n");
  gsl_sort (data,1,n);
  k = (size_t)(fraction * (double)n);
  if (k < nt) k = nt;
  if (k < ncomp || k >= n-1) VError(" too few voxels found, k= %ld, ncomp= %d, nvoxels= %ld",
					  (long)k,ncomp,(long)n);
  threshold = data[n-k-1];
  fprintf(stderr,"# fraction= %.3f, nvoxels= %ld\n",fraction,(long)k);

    
  /* put into data matrix */
  A = gsl_matrix_calloc(k,nt);
  k = 0;
  for (i=0; i<n; i++) {
    if (VPixel(map,0,3,i,VFloat) < threshold) continue;
    b = VPixel(map,0,0,i,VFloat);
    r = VPixel(map,0,1,i,VFloat);
    c = VPixel(map,0,2,i,VFloat);
    
    if (k >= A->size1) continue;
    for (j=0; j<nt; j++) {
      gsl_matrix_set(A,k,j,(double)gsl_matrix_float_get(mat,i,j));
    }
    k++;
  }

  /* perform pca */
  fprintf(stderr,"# pca...\n");
  B = VPCA(A,ncomp,first);

  /* construct design matrix as transpose of B, add unity column */
  X = gsl_matrix_calloc(B->size2,ncomp+1);
  gsl_matrix_set_all (X, (double) 1.0);
  for (i=0; i<ncomp; i++) {
    for (j=0; j<nt; j++) {
      gsl_matrix_set(X,j,i,gsl_matrix_get(B,i,j));
    }
  }
  gsl_matrix_free(B);
  ScaleX(X);

  /*
    FILE *fp=NULL;
  if (strlen(txtfilename) > 2) {
    fp = fopen(txtfilename,"w");
    if (!fp) VError(" err opening file %s",txtfilename);
    for (i=0; i<X->size2-1; i++) {
      for (j=0; j<X->size1; j++) {
	fprintf(fp," %ld  %f\n",(long)j,gsl_matrix_get(X,j,i));
      }
      fprintf(fp,"\n\n");
    }
    fclose(fp);
  }  
  */

  /* regress out nuisance components */
  fprintf(stderr,"# glm...\n");
  work = gsl_multifit_linear_alloc ((size_t)nt,(size_t)(ncomp+1));
  beta = gsl_vector_calloc(ncomp+1);
  residuals = gsl_vector_calloc(nt);
  y = gsl_vector_calloc(nt);
  cov = gsl_matrix_calloc(ncomp+1,ncomp+1);

  smin = VPixelMinValue(src[0]);
  smax = VPixelMaxValue(src[0]);

  sum1 = sum2 = nx = 0;
  for (i=0; i<n; i++) {
    if (i%5000 == 0) fprintf(stderr," %7ld of %ld\r",(long)i,(long)n);
    float *tc = gsl_matrix_float_ptr(mat,i,0);
    for (j=0; j<nt; j++) gsl_vector_set(y,j,(double)tc[j]);
    gsl_multifit_linear (X,y,beta,cov,&chisq,work);
    gsl_multifit_linear_residuals (X,y,beta,residuals);

    /* get R^2 of fit */
    t = 0;
    s = gsl_stats_tss(y->data,1,nt);
    if (s > 1.0e-6) t = 1.0 - chisq/s;
    VPixel(map,0,4,i,VFloat) = t;

    for (j=0; j<nt; j++) {
      u = gsl_vector_get(residuals,j);
      if (isnan(u) || isinf(u)) u = 0;
      gsl_matrix_float_set(mat,i,j,(float)u);
      sum1 += u;
      sum2 += u*u;
      nx++;
    }
  }
  fprintf(stderr,"                                  \r");
  if (nx < 2) VError(" nx= %f\n",nx);
  mean = sum1/nx;
  sigma = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));


  /* image containing R^2 of fit to noise components */
  dest0 = VCreateImage(nslices,nrows,ncols,VFloatRepn);
  VFillImage(dest0,VAllBands,0);
  VCopyImageAttrs (src[0], dest0);
  VSetAttr(VImageAttrList(dest0),"modality",NULL,VStringRepn,"R2_of_noise");
  VSetAttr(VImageAttrList(dest0),"name",NULL,VStringRepn,"R2_of_noise");

  dest1 = VCreateImage(nslices,nrows,ncols,VFloatRepn);
  VFillImage(dest1,VAllBands,0);
  VCopyImageAttrs (src[0], dest1);
  VSetAttr(VImageAttrList(dest1),"modality",NULL,VStringRepn,"sigma");
  VSetAttr(VImageAttrList(dest1),"name",NULL,VStringRepn,"sigma");

  for (i=0; i<n; i++) {
    b = VPixel(map,0,0,i,VFloat);
    r = VPixel(map,0,1,i,VFloat);
    c = VPixel(map,0,2,i,VFloat);
    
    u = VPixel(map,0,3,i,VFloat);
    if (u > threshold) VPixel(dest1,b,r,c,VFloat) = u;
 
    if (VPixel(src[b],0,r,c,VShort) < 1) continue;
    if (VPixel(src[b],0,r,c,VShort) < minval) continue;
    VPixel(dest0,b,r,c,VFloat) = VPixel(map,0,4,i,VFloat);

    for (j=0; j<nt; j++) {
      u = gsl_matrix_float_get(mat,i,j);
      u = (u-mean)/sigma;
      u = u*1000.0 + 15000;
      if (u < 0) u = 0;
      if (u >= smax) u = smax-1;
      VPixel(src[b],j,r,c,VShort) = (VShort)u;
    }
  }

  r2_list = VCreateAttrList();
  VAppendAttr(r2_list,"r2",NULL,VImageRepn,dest0);
  VAppendAttr(r2_list,"sigma",NULL,VImageRepn,dest1);
  return r2_list;
}



int
main(int argc, char *argv[])
{ 
  static VFloat fraction = 0.01;
  static VFloat high   = 100;
  static VFloat low    = 0;
  static VShort ncomp  = 4;
  static VShort first  = 0;
  static VShort minval = 0;
  static VString filename = "r2.v";
  static VOptionDescRec  options[] = {   
    {"fraction",VFloatRepn,1,(VPointer) &fraction,VOptionalOpt,NULL,"upper fraction in percent"},
    {"ncomp",VShortRepn,1,(VPointer) &ncomp,VOptionalOpt,NULL,"number of components to regress out"},
    {"first",VShortRepn,1,(VPointer) &first,VOptionalOpt,NULL,"first component to regress out"},
    {"minval",VShortRepn,1,(VPointer) &minval,VOptionalOpt,NULL,"Signal threshold"},
    {"low",VFloatRepn,1,(VPointer) &low,VOptionalOpt,NULL,"Temporal Filter: lower cutoff  in seconds"},
    {"high",VFloatRepn,1,(VPointer) &high,VOptionalOpt,NULL,"Temporal Filter: upper cutoff in seconds"},
    {"r2",VStringRepn,1,(VPointer) &filename,VOptionalOpt,NULL,"R2 image"}
  };
  FILE *in_file=NULL,*out_file=NULL,*fp=NULL;
  VAttrList list=NULL,r2_list=NULL;
  char *prg = "vcompcor";

  /* parse command line */
  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);
  if (fraction <= 0 || fraction >= 1) VError(" parameter 'fraction' must be in (0,1)");
  if (ncomp < 1) VError(" parameter 'ncomp' must be >= 1");


  /* read data */
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);
  

  /* CompCor */
  r2_list = VCompCor(list,minval,(int)ncomp,(int)first,fraction,low,high);


  /* write output */
  if (strlen(filename) > 2) {
    VHistory(VNumber(options),options,prg,&list,&r2_list);
    fp = fopen(filename,"w");
    if (!fp) VError(" err opening %s",filename);
    VWriteFile (fp, r2_list);
    fclose(fp);
  }

  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
