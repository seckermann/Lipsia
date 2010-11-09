
/****************************************************************
 *
 * Program: v2ndlevel
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2003, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * $Id: v2ndlevel.c 3675 2009-11-04 14:37:50Z lohmann $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include "gsl_utils.h"


#define ABS(x) ((x) > 0 ? (x) : -(x))
#define TINY   1.0e-10
#define ETMP   50     /* max number of temporary images for smoothness estim */

extern float VSmoothnessEstim(VImage *,int);
extern VImage VRead2ndLevel (FILE *);
extern char * getLipsiaVersion();

/*
** convert a gsl matrix to a 2D vista raster image
*/
VImage
Mat2Vista(gsl_matrix_float *A)
{
  VImage dest=NULL;
  int i,j;
  float x;
  dest = VCreateImage(1,A->size1,A->size2,VFloatRepn);

  for (i=0; i<A->size1; i++) {
    for (j=0; j<A->size2; j++) {
      x = fmget(A,i,j);
      VPixel(dest,0,i,j,VFloat) = x;
    }
  }
  return dest;
}


/*
** compute pseudoinverse: V * D^-1 * U^T
*/
void
PseudoInverse(gsl_matrix *U,gsl_matrix *V,gsl_vector *w,gsl_matrix_float *C)
{
  int k,l,j,n,m;
  double x,u,wmin,wmax,tiny=1.0e-6;

  n = C->size1;
  m = C->size2;
  gsl_matrix_float_set_zero(C);

  wmax = 0;
  for (j=0; j<n; j++) {
    u = ABS(dvget(w,j));
    if (u > wmax) wmax = u;
  }
  wmin = wmax * tiny;
  if (wmin < 1.0e-8) wmin = 1.0e-8;

  for (k=0; k<n; k++) {
    for (l=0; l<m; l++) {
      for (j=0; j<n; j++) {
	if (dvget(w,j) != 0) {
	  x = fmget(C,k,l);
	  u = dvget(w,j);
	  if (ABS(u) > wmin)
	    x += dmget(V,k,j)*dmget(U,l,j)/u;
	  fmset(C,k,l,x);
	}
      }
    }
  }
}


/*
** general linear regression, 2nd level designs
*/
VAttrList
V2ndLevelRegression(VImage *src,int nimages,VImage design,float vx,float vy,float vz)
{
  VAttrList out_list;
  int nslices=0,nrows=0,ncols=0,slice,row,col;
  VImage res_image=NULL; 
  VImage *beta_image,BCOV=NULL,KX_image=NULL;
  VImage res_map[ETMP];
  VFloat *float_pp,df;
  float d,err;
  float  smooth_fwhm=0;
  int   i,k,l,n,m=0;
  float x,trace=0,trace2=0,var=0;
  float *ptr1,*ptr2;
  double *double_pp;
  gsl_matrix_float *X=NULL,*XInv=NULL,*SX=NULL;
  gsl_vector_float *y,*z,*beta;
  gsl_matrix *U=NULL,*V=NULL;
  gsl_vector *w;
  gsl_matrix_float *S=NULL,*Vc=NULL,*F=NULL,*P=NULL,*Q=NULL;
  gsl_matrix_float *R=NULL,*RV=NULL;

  VBoolean smooth = FALSE;  /* no smoothness estimation */


  /* 
  ** get image dimensions
  */
  nslices = VImageNBands(src[0]);
  nrows   = VImageNRows(src[0]);
  ncols   = VImageNColumns(src[0]);


  /*
  ** get design dimensions 
  */
  m = VImageNRows(design);      /* number of timesteps   */
  n = VImageNColumns(design);   /* number of covariates */
  fprintf(stderr," nimages=%d,   num covariates=%d\n",m,n);


  beta_image = (VImage *) VMalloc(sizeof(VImage) * (n+1));
  if (m != nimages) VError(" inconsistent number of input images: %d %d",m,nimages);
  fprintf(stderr," working...\n");


  /* 
  ** read design matrix
  */
  X = gsl_matrix_float_alloc (m, n);

  for (k=0; k<m; k++) {
    for (l=0; l<n; l++) {
      x = VGetPixel(design,0,k,l);
      fmset(X,k,l,x);
    }
  }


  /* 
  ** no pre-coloring for 2nd level designs
  */
  S  = gsl_matrix_float_alloc (m, m);
  gsl_matrix_float_set_identity(S);

  Vc = gsl_matrix_float_alloc (m, m);
  gsl_matrix_float_set_identity(Vc);



  /* 
  ** compute pseudoinverse: singular value decomp, and V * D^-1 * U^T
  */
  U    = gsl_matrix_alloc (m, n);
  V    = gsl_matrix_alloc (n, n);
  XInv = gsl_matrix_float_alloc (n, m);
  w    = gsl_vector_alloc (n);

  SX = fmat_x_mat(S,X,NULL);

  float_pp  = SX->data;
  double_pp = U->data;
  for (i=0; i<m*n; i++) *double_pp++ = *float_pp++;

  gsl_linalg_SV_decomp_jacobi(U,V,w);
  PseudoInverse(U,V,w,XInv);

  gsl_matrix_free(U);
  gsl_matrix_free(V);



  /* 
  ** get variance estimate 
  */
  Q = fmat_x_mat(XInv,Vc,Q);
  F = fmat_x_matT(Q,XInv,F);

  BCOV = VCreateImage(1,n,n,VFloatRepn);
  float_pp = VImageData(BCOV);
  ptr1 = F->data;
  for (i=0; i<n*n; i++) *float_pp++ = *ptr1++;

  gsl_matrix_float_free(Q);
  gsl_matrix_float_free(F);


  /* 
  ** get effective degrees of freedom 
  */
  R  = gsl_matrix_float_alloc (m, m);

  P = fmat_x_mat(SX,XInv,P);
  gsl_matrix_float_set_identity(R);
  gsl_matrix_float_sub(R,P);

  RV = fmat_x_mat(R,Vc,NULL);

  trace = 0;
  for (i=0; i<m; i++)
    trace += fmget(RV,i,i);

  P = fmat_x_mat(RV,RV,P);
  trace2 = 0;
  for (i=0; i<m; i++)
    trace2 += fmget(P,i,i);

  df = (trace*trace) / trace2;

  fprintf(stderr," df= %.3f\n",df);



  /*
  ** create output images
  */
  out_list = VCreateAttrList();
  res_image = VCreateImage(nslices,nrows,ncols,VFloatRepn);
  VFillImage(res_image,VAllBands,0);
  VCopyImageAttrs (src[0],res_image);

  VSetAttr(VImageAttrList(res_image),"patient",NULL,VStringRepn,"group statistics");
  VSetAttr(VImageAttrList(res_image),"name",NULL,VStringRepn,"RES/trRV");
  VSetAttr(VImageAttrList(res_image),"modality",NULL,VStringRepn,"RES/trRV");
  VSetAttr(VImageAttrList(res_image),"df",NULL,VFloatRepn,df);
  VAppendAttr (out_list, "image", NULL, VImageRepn,res_image);

  for (i=0; i<n; i++) {
    beta_image[i] = VCreateImage(nslices,nrows,ncols,VFloatRepn);
    VFillImage(beta_image[i],VAllBands,0);
    VCopyImageAttrs (src[0],beta_image[i]);
    VSetAttr(VImageAttrList(beta_image[i]),"patient",NULL,VStringRepn,"group statistics");
    VSetAttr(VImageAttrList(beta_image[i]),"name",NULL,VStringRepn,"BETA");
    VSetAttr(VImageAttrList(beta_image[i]),"modality",NULL,VStringRepn,"BETA");
    VSetAttr(VImageAttrList(beta_image[i]),"beta",NULL,VShortRepn,i+1);
    VSetAttr(VImageAttrList(beta_image[i]),"df",NULL,VFloatRepn,df);
    VAppendAttr (out_list, "image", NULL, VImageRepn,beta_image[i]);
  }
  
  
  KX_image = Mat2Vista(SX);
  VSetAttr(VImageAttrList(KX_image),"patient",NULL,VStringRepn,"group statistics");
  VSetAttr(VImageAttrList(KX_image),"name",NULL,VStringRepn,"KX");
  VSetAttr(VImageAttrList(KX_image),"modality",NULL,VStringRepn,"KX");
  VAppendAttr (out_list, "image", NULL, VImageRepn,KX_image);
  

  VSetAttr(VImageAttrList(BCOV),"patient",NULL,VStringRepn,"group statistics");
  VSetAttr(VImageAttrList(BCOV),"name",NULL,VStringRepn,"BCOV");
  VSetAttr(VImageAttrList(BCOV),"modality",NULL,VStringRepn,"BCOV");
  VAppendAttr (out_list, "image", NULL, VImageRepn,BCOV);


  /*
  ** create temporary images for smoothness estimation
  */ 
  if (smooth) {
    for (i=0; i<m; i++) {
      if (m >= ETMP) VError(" too many tmp images");
      res_map[i] = VCreateImage(nslices,nrows,ncols,VFloatRepn);
      VFillImage(res_map[i],VAllBands,0);
    }
  }


  /*
  ** process
  */
  y    = gsl_vector_float_alloc (m);
  z    = gsl_vector_float_alloc (m);
  beta = gsl_vector_float_alloc (n);

  
  for (slice=0; slice<nslices; slice++) {
    if (slice%5 == 0) fprintf(stderr," slice: %3d\r",slice);

    for (row=0; row<nrows; row++) {
      for (col=0; col<ncols; col++) {


	/* read time series data */
	ptr1 = y->data;

	for (i=0; i<nimages; i++) {
	  x = VPixel(src[i],slice,row,col,VFloat);
	  if (ABS(x) < TINY) goto next;
	  (*ptr1++) = x;
	}


	/* compute beta's */
	fmat_x_vector(XInv,y,beta);


	/* residuals */
	fmat_x_vector(SX,beta,z);

	err  = 0;
	ptr1 = y->data;
	ptr2 = z->data;
	for (i=0; i<m; i++) {
	  d = ((*ptr1++) - (*ptr2++));
	  err += d*d;
	}


	/* sigma^2 */
	var = err / trace;


	/* write residuals output */
	VPixel(res_image,slice,row,col,VFloat) = (VFloat)var;

	/* save residuals of several timesteps for smoothness estimation */
	if (smooth) {
	  ptr1 = y->data;
	  ptr2 = z->data;
	  err = 0;
	  for (i=0; i<m; i++) {
	    d = ((*ptr1++) - (*ptr2++));
	    err += d*d;
	    VPixel(res_map[i],slice,row,col,VFloat) = d;
	  }
	  err = sqrt(err);

	  for (i=0; i<m; i++) {
	    d = VPixel(res_map[i],slice,row,col,VFloat);
	    VPixel(res_map[i],slice,row,col,VFloat) = d/err;
	  }	
	}

	/* write beta output */
	ptr1 = beta->data;
	for (i=0; i<n; i++)
	  VPixel(beta_image[i],slice,row,col,VFloat) = (VFloat)(*ptr1++);

      next: ;
      }
    }
  }


  /* 
  ** Smoothness estimation based on residual images 
  */
  if (smooth) {
    smooth_fwhm = VSmoothnessEstim(res_map,m);
    vx = (vx + vy + vz) / 3.0;    /* voxels should be isotropic */
    smooth_fwhm *= vx;
    VSetAttr(VImageAttrList(res_image),"smoothness",NULL,VFloatRepn,smooth_fwhm);
    for (i=0; i<n; i++) {
      VSetAttr(VImageAttrList(beta_image[i]),"smoothness",NULL,VFloatRepn,smooth_fwhm);
    }
    for (i=0; i<m; i++) VDestroyImage(res_map[i]);
  }


  return out_list;
}





int
main (int argc,char *argv[])
{
  static VArgVector in_files;
  static VString out_filename;
  static VString filename;
  static VOptionDescRec  options[] = {
    {"in", VStringRepn, 0, & in_files,VRequiredOpt, NULL,"Input files" },
    {"out", VStringRepn, 1, & out_filename,VRequiredOpt, NULL,"Output file" },
    {"design",VStringRepn,1,(VPointer) &filename,VRequiredOpt,NULL,"Design file"}
  };
  FILE *fp=NULL,*f=NULL;
  VStringConst in_filename;
  VAttrList list=NULL,out_list=NULL;
  VAttrListPosn posn;
  VImage design=NULL;
  VString str;
  VImage *src,tmp;
  float vx,vy,vz;
  int  i,nimages,found;
  char prg[50];	
  sprintf(prg,"v2ndlevel V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);

  /*
  ** parse command line
  */
  if (! VParseCommand (VNumber (options), options, & argc, argv)) {
    VReportUsage (argv[0], VNumber (options), options, NULL);
    exit (EXIT_FAILURE);
  }
  if (argc > 1) {
    VReportBadArgs (argc, argv);
    exit (EXIT_FAILURE);
  }



  /* 
  ** read design matrix 
  */
  fp = VOpenInputFile (filename, TRUE);
  design = VRead2ndLevel (fp);
  if (!design) VError(" error reading design file");


  /* 
  ** Read each input file 
  */
  nimages = in_files.number;
  src = (VImage *) VMalloc(sizeof(VImage) * nimages);


  for (i=0; i<nimages; i++) {
    in_filename = ((VStringConst *) in_files.vector)[i];
    fprintf(stderr," %3d  %s\n",i,in_filename);
 
    fp = VOpenInputFile (in_filename, TRUE);
    list = VReadFile (fp, NULL);
    if (! list) VError("Error reading file %s",in_filename);
    fclose(fp);

    found = 0;
    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & tmp);
      if (VPixelRepn(tmp) != VFloatRepn) continue;
      if (VGetAttr (VImageAttrList (tmp), "modality", NULL,VStringRepn, (VPointer) & str)
	  != VAttrFound)
	VError(" attribute 'modality' missing in input file.");
      if (strcmp(str,"conimg") != 0) continue;
      if (VGetAttr (VImageAttrList (tmp), "voxel", NULL,VStringRepn, (VPointer) & str)
	  != VAttrFound)
	VError(" attribute 'voxel' missing in input file.");  
      sscanf(str,"%f %f %f",&vx,&vy,&vz);
      src[i] = VCopyImage(tmp,NULL,VAllBands);
      found = 1;
      break;
    }
    if (found != 1) VError(" no contrast image found in file %s",in_filename);
  }


  /* 
  ** GLM 
  */
  out_list = V2ndLevelRegression(src,nimages,design,vx,vy,vz);



  /* 
  ** Output: 
  */
  VHistory(VNumber(options),options,prg,&list,&out_list);
  f = VOpenOutputFile (out_filename, TRUE);
  if (!f) VError(" error opening output file %s",out_filename);
  if (! VWriteFile (f, out_list)) exit (1);

  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}


