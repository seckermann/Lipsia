/*
** CCM for two data sets
**
** G.Lohmann, Jan 2011
*/
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_sort_double.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NSLICES 2000

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

#ifdef _OPENMP
#include <omp.h>
#endif /*_OPENMP*/

extern double VKendallDist(double *arr1,double *arr2,int n);
extern double CCC(double *arr1,double *arr2,int n);
extern void gsl_sort_vector_index (gsl_permutation *,gsl_vector *);

double EuclDist(double *arr1,double *arr2,int n)
{
  int i;
  double d,sum;

  sum = 0;
  for (i=0; i<n; i++) {
    d = arr1[i] - arr2[i];
    sum += d*d;
  }
  sum = sqrt(sum);
  return sum;
}


double kendall(double *arr1,double *arr2,int n)
{ 
  static gsl_vector *vec = NULL;
  static gsl_permutation *perm=NULL,*rank1=NULL,*rank2=NULL;
  static double *r=NULL;
  int i;
  double S,W,R;
  double nx=0;

  if (vec == NULL) {
    vec   = gsl_vector_calloc(n);
    perm  = gsl_permutation_alloc(n);
    rank1 = gsl_permutation_alloc(n);
    rank2 = gsl_permutation_alloc(n);
    r = (double *) VCalloc(n,sizeof(double));
  }

  for (i=0; i<n; i++) gsl_vector_set(vec,i,arr1[i]);
  gsl_sort_vector_index (perm, vec);
  gsl_permutation_inverse (rank1, perm);

  for (i=0; i<n; i++) gsl_vector_set(vec,i,arr2[i]);
  gsl_sort_vector_index (perm, vec);
  gsl_permutation_inverse (rank2, perm);

  for (i=0; i<n; i++) r[i] = (double)(rank1->data[i] + rank2->data[i]);

  nx = (double)n;
  R = 0;
  for (i=0; i<n; i++) R += r[i];
  R /= nx;

  S = 0;
  for (i=0; i<n; i++) S += SQR(r[i] - R);

  W = 12.0*S/(4.0*(nx*nx-1.0)*nx);
  return W;
}


/* re-implementation of cblas_sspmv,  cblas_sspmv causes problems */
void
my_sspmv(float *A,float *x,float *y,int n)
{
  size_t i,j,k,kk;
  float tmp1=0,tmp2=0;
 
  for (j=0; j<n; j++) y[j] = 0;
  kk = k = 0;
  for (j=0; j<n; j++) {
    tmp1 = x[j];
    tmp2 = 0;
    k = kk;
    for (i=0; i<j; i++) {
      y[i] += tmp1 * A[k];
      tmp2 += A[k] * x[i];
      k++;
    }
    y[j] += tmp1*A[kk+j] + tmp2;
    kk += (j+1);
  }
}


double
Correlation(const float * arr1,const float * arr2,int n)
{
  int i;
  double sx,sy,sxx,syy,sxy,rx;
  double tiny=1.0e-4;

  sxx = syy = sxy = sx = sy = 0;
  for (i=0; i<n; i++) {
    const double u = (double)arr1[i];
    const double v = (double)arr2[i];
    sx  += u;
    sy  += v;
    sxx += u*u;
    syy += v*v;
    sxy += u*v;
  }
  const double nx= n;
  const double u = nx*sxx - sx*sx;
  const double v = nx*syy - sy*sy;
  rx = 0;
  if (u*v > tiny)
    rx = (nx*sxy - sx*sy)/sqrt(u*v);

  return rx;
}

VImage
GetMap(VAttrList list1,VAttrList list2,VImage mask,VShort minval)
{
  VAttrListPosn posn;
  VImage src1[NSLICES],src2[NSLICES],map=NULL;
  size_t b,r,c,i,n;
  int nrows,ncols,nslices,nrows2,ncols2,nslices2;

  /*
  ** get image dimensions
  */
  i = nrows = ncols = 0;
  for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src1[i]);
    if (VPixelRepn(src1[i]) != VShortRepn) continue;
    if (VImageNRows(src1[i])  > nrows)  nrows = VImageNRows(src1[i]);
    if (VImageNColumns(src1[i]) > ncols) ncols = VImageNColumns(src1[i]);
    i++;
    if (i >= NSLICES) VError(" too many slices");
  }
  nslices = i;

  i = nrows2 = ncols2 = 0;
  for (VFirstAttr (list2, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src2[i]);
    if (VPixelRepn(src2[i]) != VShortRepn) continue;
    if (VImageNRows(src2[i])  > nrows2)  nrows2 = VImageNRows(src2[i]);
    if (VImageNColumns(src2[i]) > ncols2) ncols2 = VImageNColumns(src2[i]);
    i++;
    if (i >= NSLICES) VError(" too many slices");
  }
  nslices2 = i;
  if (nslices2 != nslices) VError(" inconsistent number of slices: %d %d",nslices,nslices2);
  if (nrows2 != nrows) VError(" inconsistent number of rows: %d %d",nrows,nrows2);
  if (ncols2 != ncols) VError(" inconsistent number of cols: %d %d",ncols,ncols2);


  /* count number of voxels */
  n = 0;
  for (b=0; b<nslices; b++) {
    if (VImageNRows(src1[b]) < 2) continue;
    if (VImageNRows(src2[b]) < 2) continue;
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	if (VPixel(src1[b],0,r,c,VShort) < minval) continue;
	if (VPixel(src2[b],0,r,c,VShort) < minval) continue;
	if (VGetPixel(mask,b,r,c) < 0.5) continue;
	n++;
      }
    }
  }
  fprintf(stderr," nvoxels: %ld\n",(long)n);


  /*
  ** voxel addresses
  */
  map = VCreateImage(1,5,n,VShortRepn);
  if (map == NULL) VError(" error allocating addr map");
  VFillImage(map,VAllBands,0);
  VCopyImageAttrs (src1[0],map);

  VPixel(map,0,3,0,VShort) = nslices;
  VPixel(map,0,3,1,VShort) = nrows;
  VPixel(map,0,3,2,VShort) = ncols;

  i = 0;
  for (b=0; b<nslices; b++) {
    if (VImageNRows(src1[b]) < 2) continue;
    if (VImageNRows(src2[b]) < 2) continue;
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	if (VPixel(src1[b],0,r,c,VShort) < minval) continue;
	if (VPixel(src2[b],0,r,c,VShort) < minval) continue;
	if (VGetPixel(mask,b,r,c) < 0.5) continue;

	VPixel(map,0,0,i,VShort) = b;
	VPixel(map,0,1,i,VShort) = r;
	VPixel(map,0,2,i,VShort) = c;
	i++;
      }
    }
  }
  return map;
}


float *
VCorrMatrix(VAttrList list,VImage map,VShort first,VShort length)
{
  VAttrListPosn posn;
  VImage src[NSLICES];
  size_t b,r,c,i,j,i1,n,m,nt,last,ntimesteps,nrows,ncols,nslices;
  gsl_matrix_float *mat=NULL;
  float *A=NULL;

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


  /* get time steps to include */
  if (length < 1) length = ntimesteps-2;
  last = first + length -1;
  if (last >= ntimesteps) last = ntimesteps-1;
  if (first < 0) first = 1;

  nt = last - first + 1;
  i1 = first+1;
  if (nt < 2) VError(" not enough timesteps, nt= %d",nt);

  /* number of voxels */
  n = VImageNColumns(map);

  fprintf(stderr," ntimesteps: %d, first= %d, last= %d, nt= %d, nvoxels: %ld\n",
	  (int)ntimesteps,(int)first,(int)last,(int)nt,(long)n);


  /*
  ** avoid casting to float, copy data to matrix
  */
  mat = gsl_matrix_float_calloc(n,nt);
  if (!mat) VError(" err allocating mat");
  for (i=0; i<n; i++) {

    b = VPixel(map,0,0,i,VShort);
    r = VPixel(map,0,1,i,VShort);
    c = VPixel(map,0,2,i,VShort);

    float *ptr = gsl_matrix_float_ptr(mat,i,0);
    int k;
    j = 0;
    for (k=first; k<=last; k++) {
      if (j >= mat->size2) VError(" j= %d %d",j,mat->size2);
      if (k >= VImageNBands(src[b])) VError(" k= %d %d",k, VImageNBands(src[b]));
      *ptr++ = (float) VPixel(src[b],k,r,c,VShort);
      j++;
    }
  }


  /*
  ** compute similarity matrix
  */
  m = (n*(n+1))/2;
  fprintf(stderr," compute correlation matrix...\n");
  A = (float *) VCalloc(m,sizeof(float));
  if (!A) VError(" err allocating correlation matrix");
  memset(A,0,m*sizeof(float));
  size_t progress=0;

#pragma omp parallel for shared(progress) private(j) schedule(guided) firstprivate(mat,A)
 
  for (i=0; i<n; i++) {
    if (i%100 == 0) fprintf(stderr," %d00\r",(int)(++progress));

    const float *arr1 = gsl_matrix_float_const_ptr(mat,i,0);
    for (j=0; j<i; j++) {
      const float *arr2 = gsl_matrix_float_const_ptr(mat,j,0);
      const double v = Correlation(arr1,arr2,nt);
      const size_t k=j+i*(i+1)/2;
      if (k >= m) VError(" illegal addr k= %d, m= %d",k,m);
      A[k] = v;
    }
  }
  fprintf(stderr," matrix done.\n");
  gsl_matrix_float_free(mat);
  return A;
}


/* 
** concordance mapping
*/
VImage VCCM2(float *A1,float *A2,VImage map,int type)
{
  VImage dest=NULL;
  size_t b,r,c,i,j,k,kk,nvoxels;
  int nslices=0,nrows=0,ncols=0;
  double *arr1=NULL,*arr2=NULL,u;

  nslices = VPixel(map,0,3,0,VShort);
  nrows = VPixel(map,0,3,1,VShort);
  ncols = VPixel(map,0,3,2,VShort);

  nvoxels = VImageNColumns(map);
  dest = VCreateImage(nslices,nrows,ncols,VFloatRepn);
  VFillImage(dest,VAllBands,0);
  VCopyImageAttrs (map, dest);
  VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"conimg");

  arr1 = (double *) VCalloc(nvoxels,sizeof(double));
  arr2 = (double *) VCalloc(nvoxels,sizeof(double));

  fprintf(stderr," concordance mapping...\n");

  for (i=0; i<nvoxels; i++) {
    if (i%50 == 0) fprintf(stderr," %6d  of %d\r",i,nvoxels);
    b = (int)VPixel(map,0,0,i,VShort);
    r = (int)VPixel(map,0,1,i,VShort);
    c = (int)VPixel(map,0,2,i,VShort);
    for (j=0; j<nvoxels; j++) arr1[j] = arr2[j] = 0;

    kk = 0;
    for (j=0; j<i; j++) {
      k=j+i*(i+1)/2;
      arr1[kk] = (double)A1[k];
      arr2[kk] = (double)A2[k];
      kk++;
    }
    for (j=i+1; j<nvoxels; j++) {
      k=i+j*(j+1)/2;
      arr1[kk] = (double)A1[k];
      arr2[kk] = (double)A2[k];
      kk++;
    }
    u = 0;
    switch (type) {
    case 0:
      u = kendall(arr1,arr2,nvoxels);
      break;
    case 1:
      u = CCC(arr1,arr2,nvoxels);
      break;
    default:
      VError("illegal type");
    }

    VPixel(dest,b,r,c,VFloat) = (VFloat)u;
  }
  fprintf(stderr,"\n");
  return dest;
}


VDictEntry TYPDict[] = {
  { "kendall", 0 },
  { "CCC", 1 },
  { NULL }
};

int
main (int argc,char *argv[])
{
  static VString  filename = "";
  static VString  filename1 = "";
  static VString  filename2 = "";
  static VShort   first  = 2;
  static VShort   length = 0;
  static VShort   type = 0;
  static VShort   minval = 0;
  static VShort   nproc = 4;
  static VOptionDescRec  options[] = {
    {"in1",VStringRepn,1,(VPointer) &filename1,VRequiredOpt,NULL,"first input file"},
    {"in2",VStringRepn,1,(VPointer) &filename2,VRequiredOpt,NULL,"second input file"},
    {"mask",VStringRepn,1,(VPointer) &filename,VRequiredOpt,NULL,"mask"},
    {"type",VShortRepn,1,(VPointer) &type,VOptionalOpt,TYPDict,"type of concordance measure"},
    {"minval",VShortRepn,1,(VPointer) &minval,VOptionalOpt,NULL,"signal threshold"},
    {"first",VShortRepn,1,(VPointer) &first,VOptionalOpt,NULL,"first timestep to use"},
    {"length",VShortRepn,1,(VPointer) &length,VOptionalOpt,NULL,
     "length of time series to use, '0' to use full length"},
    {"j",VShortRepn,1,(VPointer) &nproc,VOptionalOpt,NULL,"number of processors to use, '0' to use all"}
  };
  FILE *out_file,*fp;
  VAttrList list=NULL,list1=NULL,list2=NULL,out_list=NULL;
  VAttrListPosn posn;
  VImage mask=NULL,map=NULL,dest=NULL,disc=NULL;
  float *A1=NULL,*A2=NULL,u=0,tiny=1.0e-6;
  int b,r,c;
  char *prg = "vccm2";

  VParseFilterCmd (VNumber (options),options,argc,argv,NULL,&out_file);
  if (type > 3) VError("illegal type");


  /*
  ** read mask
  */
  fp = VOpenInputFile (filename, TRUE);
  list = VReadFile (fp, NULL);
  if (! list) VError("Error reading mask file");
  fclose(fp);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & mask);
    if (VPixelRepn(mask) == VFloatRepn || VPixelRepn(mask) == VDoubleRepn) {
      mask = NULL;
      continue;
    }
  }
  if (mask == NULL) VError(" no mask found");


  /* read files */
  fp = VOpenInputFile (filename1, TRUE);
  list1 = VReadFile (fp, NULL);
  if (! list1) VError("Error reading 1st input file");
  fclose(fp);

  fp = VOpenInputFile (filename2, TRUE);
  list2 = VReadFile (fp, NULL);
  if (! list2) VError("Error reading 2nd input file");
  fclose(fp);


  /* get voxel map */
  map = GetMap(list1,list2,mask,minval);


 /* omp-stuff */
#ifdef _OPENMP
  int num_procs=omp_get_num_procs();
  if (nproc > 0 && nproc < num_procs) num_procs = nproc;
  printf("using %d cores\n",(int)num_procs);
  omp_set_num_threads(num_procs);
#endif /* _OPENMP */


  /* compute corr matrices */
  A1 = VCorrMatrix(list1,map,first,length);
  A2 = VCorrMatrix(list2,map,first,length);


  /* get concordance between two corr matrices */
  dest = VCCM2(A1,A2,map,(int)type);


  /* invert to get discordant map */
  disc = VCreateImageLike(dest);
  VFillImage(disc,VAllBands,0);

  if (type < 2) {
    for (b=0; b<VImageNBands(dest); b++) {
      for (r=0; r<VImageNRows(dest); r++) {
	for (c=0; c<VImageNColumns(dest); c++) {
	  if (VGetPixel(mask,b,r,c) < 1) continue;
	  u = VPixel(dest,b,r,c,VFloat);
	  if (u < tiny) continue;
	  VPixel(disc,b,r,c,VFloat) = 1-u;
	} 
      }
    }
  }


  /* output */
  out_list = VCreateAttrList();
  VHistory(VNumber(options),options,prg,&list,&out_list);
  VAppendAttr(out_list,"concordant",NULL,VImageRepn,dest);
  if (type < 2) VAppendAttr(out_list,"discordant",NULL,VImageRepn,disc);
  if (! VWriteFile (out_file, out_list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
