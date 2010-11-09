/*
** motion correction using 6 degrees of freedom
**
** (3 translational + 3 rotational)
**
** G.Lohmann, MPI-CBS, April 2007
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x)*(x))

extern void ApplyTransform2d(VImage *,int,int,int,int,gsl_vector *);

int verbose=0;

#define NSLICES 256
VImage src[NSLICES];

struct my_params {
  int    i1;           /* reference time step */
  int    i2;           /* current time step */
  int    nslices;
  int    nrows;
  int    ncols;
  int    *slicetable;  /* slices to be used */
};


/*
** goal function to minimize
*/
double
my_f (const gsl_vector *vec, void *params)
{


  struct my_params *par = params;
  double t[2];
  double rot[2][2],alpha;
  int    b,r,c,rr,cc,nslices,nrows,ncols;
  int    i1,i2,step,q;
  double u,v;
  double cx,rx;
  double sum,nx;
  double c1,c2,r1,r2;
  double rp,cp;
  double xcos,xsin;
  double deg,pi=3.14159265;
  double xmax = VRepnMaxValue(VDoubleRepn);

  t[0]   = gsl_vector_get(vec, 0);
  t[1]   = gsl_vector_get(vec, 1);
  alpha  = gsl_vector_get(vec, 2);

  deg =  180.0f / pi;
  alpha  /= deg;

  xcos = cos(alpha);
  xsin = sin(alpha);
  rot[0][0] =  xcos;
  rot[0][1] = -xsin;
  rot[1][0] =  xsin;
  rot[1][1] =  xcos;

  i1     = par->i1;
  i2     = par->i2;

  nslices = par->nslices;
  nrows   = par->nrows;
  ncols   = par->ncols;

  sum = nx = 0;

  step = 2;
  for (b=0; b<nslices; b++) {
    if (VImageNRows(src[b]) < 2) continue;
    if (par->slicetable[b] == 0) continue;
    q = 1;
    if (b%2 == 0) q = 0;

    for (r=q+1; r<nrows-1; r+=step) {
      for (c=q+1; c<ncols-1; c+=step) {

	u = VPixel(src[b],i1,r,c,VShort);

	rx = (double) r;
	cx = (double) c;

	rp = rot[0][0] * rx + rot[0][1] * cx  +  t[0];
        cp = rot[1][0] * rx + rot[1][1] * cx  +  t[1];

	rr = (int) rp;
	cc = (int) cp;


	/* check subcube */
	if (rr < 0 || rr+1 >= nrows-1) continue;
	if (cc < 0 || cc+1 >= ncols-1) continue;


	/* compute fractions of subcube */
	c1 = cp  - (double)cc;
        c2 = 1.0 - c1;
        r1 = rp  - (double)rr;
        r2 = 1.0 - r1;


	v  = r2 * c2 * VPixel(src[b],i2,rr,cc,VShort); 
	v += r2 * c1 * VPixel(src[b],i2,rr,cc+1,VShort);
	v += r1 * c2 * VPixel(src[b],i2,rr+1,cc,VShort);
	v += r1 * c1 * VPixel(src[b],i2,rr+1,cc+1,VShort);

	sum += (u-v) * (u-v);
	nx++;
      }
    }
  }
  if (nx < 10) return xmax;
  return sum/nx;
}


/*
** select slices to be used for estimating motion parameters
*/
int *
SelectSlices(VImage *src,int nslices,VShort minval,int i0)
{
  int b,r,c,n,*table;
  float nx,mx,threshold=0.2;

  table = (int *)VCalloc(nslices,sizeof(int));

  for (b=0; b<nslices; b++) {
    table[b] = 0;
    if (VImageNRows(src[b]) < 2) continue;

    nx = mx = 0;
    for (r=0; r<VImageNRows(src[b]); r++) {
      for (c=0; c<VImageNColumns(src[b]); c++) {
	if (VPixel(src[b],i0,r,c,VShort) > minval) mx++;
	nx++;
      }
    }
    if (mx/nx > threshold) table[b] = 1;
  }
  if (nslices >= 5)
    table[0] = table[nslices-1] = 0;

  if (nslices >= 7)
    table[1] = table[nslices-2] = 0;


  n = 0;
  for (b=0; b<nslices; b++) {
    if (table[b] > 0) n++;
  }
  if (n < 2) VError(" not enough slices above minval threshold");

  return table;
}



/*
** motion correction using 6 degrees of freedom
*/
VImage 
VMotionCorrection2d(VAttrList list,VShort minval,VShort i0,VShort maxiter)
{
  VAttrListPosn posn;
  VImage xsrc=NULL,motion=NULL;
  int i,j;
  int n,nslices,nrows,ncols;
  double f0=0;    
  const gsl_multimin_fminimizer_type *T =
    gsl_multimin_fminimizer_nmsimplex;
  gsl_multimin_fminimizer *s = NULL;
  gsl_vector *stepsizes=NULL, *x=NULL;
  gsl_multimin_function minex_func;
  int status,iter;
  double size;
  size_t np = 3;
  struct my_params params;


  /* 
  ** get image dimensions 
  */
  n = i = nrows = ncols = nslices = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (i >= NSLICES) VError(" too many slices");
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & xsrc);
    if (VPixelRepn(xsrc) != VShortRepn) continue;
    if (VImageNBands(xsrc) > n) n = VImageNBands(xsrc);
    if (VImageNRows(xsrc) > nrows) nrows = VImageNRows(xsrc);
    if (VImageNColumns(xsrc) > ncols) ncols = VImageNColumns(xsrc);
    src[i] = xsrc;
    i++;
  }
  nslices = i;

  if (i0 < 0 || i0 >= n) 
    VError(" illegal choice of ref scan (%d), must be in [%d,%d]",i0,0,n-1);



  /*
  ** get relevant slices
  */
  params.slicetable = SelectSlices(src,nslices,minval,i0);



  /*
  ** ini minimization method
  */
  stepsizes = gsl_vector_calloc (np);
  gsl_vector_set_all (stepsizes, 1.0);
  x = gsl_vector_calloc (np);
     
  minex_func.f = &my_f;
  minex_func.n = np;
  minex_func.params = (void *)&params;
  s = gsl_multimin_fminimizer_alloc (T, np);


  /* 
  ** alloc storage for motion parameters
  */
  motion = VCreateImage(1,n,np,VDoubleRepn);
  VFillImage(motion,VAllBands,0);


  /*
  ** set parameters of reference time step
  */
  params.i1      = i0;
  params.nslices = nslices;
  params.nrows   = nrows;
  params.ncols   = ncols;


  /*
  ** for each time step...
  */
  fprintf(stderr,"# estimating motion parameters...\n");
  for (i=0; i<n; i++) {
    if (i%20 == 0) fprintf(stderr,"#   %6d  of %6d\r",i,n);
    

    /* set parameter of current time step */
    params.i2 = i;


    /* optimization using simplex method */
    if (verbose) {
      gsl_vector_set_zero(x);
      f0 = my_f(x,&params);
    }


    gsl_vector_set_zero(x);
    gsl_vector_set_all (stepsizes,1.0);
    gsl_multimin_fminimizer_set (s, &minex_func, x, stepsizes);

    for (iter=0; iter<maxiter; iter++) {	
      status = gsl_multimin_fminimizer_iterate(s);
      if (status) break;     
      size = gsl_multimin_fminimizer_size (s);
      status = gsl_multimin_test_size (size, 1e-4);
      if (status == GSL_SUCCESS) break;
    }


    /* reverse motion */
    gsl_vector_scale(s->x,(double)-1.0);


    /* save motion parameters */
    for (j=0; j<np; j++)
      VPixel(motion,0,i,j,VDouble) = gsl_vector_get(s->x,j);
    

    if (verbose) {
      fprintf(stderr," %5d  %5d  %10.3f  %10.3f   %10.3f\n",i,iter,f0,s->fval,f0-s->fval);
    }
  }
  return motion;
}



void
VApplyMotionCorrection2d(VAttrList list,VImage motion,VString reportfile)
{
  VAttrListPosn posn;
  VImage xsrc=NULL;
  FILE *fp=NULL;
  VString buf;
  int i,j,n=0,np=3;
  int nslices,nrows,ncols;
  double t[2],angle;
  double u,xmax,vox_x,vox_y,vox_z;
  gsl_vector *x;

  x = gsl_vector_calloc(np);

  if (strlen(reportfile) > 1) {
    fp = fopen(reportfile,"w");
    if (!fp) VError(" error opening report file");
  }
  else if (verbose)
    fp = stderr;

  if (fp) {
    fprintf(fp,"#   scan         shift          angle\n");
    fprintf(fp,"#------------------------------------\n");
  }

  n = i = nrows = ncols = nslices = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (i >= NSLICES) VError(" too many slices");
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & xsrc);
    if (VPixelRepn(xsrc) != VShortRepn) continue;
    if (VImageNBands(xsrc) > n) n = VImageNBands(xsrc);
    if (VImageNRows(xsrc) > nrows) nrows = VImageNRows(xsrc);
    if (VImageNColumns(xsrc) > ncols) ncols = VImageNColumns(xsrc);
    src[i] = xsrc;
    i++;
  }
  nslices = i;
  fprintf(stderr,"# apply motion correction...\n");


  vox_x = vox_y = vox_z = 1;
  if (VGetAttr(VImageAttrList(src[0]),"voxel",NULL,VStringRepn,&buf) == VAttrFound)
    sscanf(buf,"%lf %lf %lf",&vox_x,&vox_y,&vox_z);
  else
    VWarning(" parameter 'voxel' missing in header");


  for (i=0; i<n; i++) {

    xmax = 0;
    for (j=0; j<np; j++) {
      u = VPixel(motion,0,i,j,VDouble);
      if (ABS(u) > xmax) xmax = ABS(u);
      gsl_vector_set(x,j,u);
    }

    /* print output */
    if (fp) {
      t[0]  = VPixel(motion,0,i,0,VDouble);
      t[1]  = VPixel(motion,0,i,1,VDouble);
      angle = VPixel(motion,0,i,2,VDouble);

      fprintf(fp,"  %5d    %7.3f %7.3f    %7.3f\n",
              i,t[1]*vox_x,t[0]*vox_y,angle);
    }
    if (xmax < 0.0005) continue;

    /* apply reverse motion */
    ApplyTransform2d(src,nslices,nrows,ncols,i,x);
  }
  if (fp) fclose(fp);
}
