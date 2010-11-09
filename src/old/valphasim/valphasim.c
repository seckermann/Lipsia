/****************************************************************
 *
 * Program: valphasim
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2007, <lipsia@cbs.mpg.de>
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
 * $Id: valphasim.c 3191 2008-04-01 16:11:38Z karstenm $
 *
 *****************************************************************/

#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_statistics.h>


extern VImage VLabelImage3d(VImage,VImage,int,VRepnKind,int *);
extern char * getLipsiaVersion();

#define NSLICES 256   /* max number of slices */

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


/*
** convert z to p values
*/
double z2p(double z)
{
  if (z < 0)
    return gsl_cdf_ugaussian_Q(-z);
  else
    return gsl_cdf_ugaussian_Q(z);
}


/*
** convert p to z values
*/
double p2z(double p)
{
  return gsl_cdf_ugaussian_Qinv(p);
}


void
WriteTest(VImage src)
{
  FILE *fp;
  VAttrList out_list;
  fp = fopen("test.v","w");
  out_list = VCreateAttrList();
  VAppendAttr (out_list,"image",NULL,VImageRepn,src);
  VWriteFile (fp, out_list);
  fclose(fp);
  exit(0);
} 




VImage
VAlphaSim(VImage src,VImage dest,VLong seed,VLong numiter,
	  VDouble fwhm,VDouble z0,VString filename)
{
  FILE *fp=NULL;
  VImage tmp=NULL,bin_image=NULL,label_image=NULL,kernel=NULL,mask=NULL;
  VString buf;
  VFloat *dst_pp;
  VShort *short_pp;
  VBit *bin_pp,*bit_pp;
  int i,j=0,b,r,c,nbands,nrows,ncols,npixels,nl,iter,nvox;
  double u,pthr,zthr,sum,sum1,sum2,nx,mean,sig;
  double sigma=0;
  float *histogram=NULL,*maxhist=NULL,*table=NULL;
  float tiny=1.0e-7;
  float x,y,z,voxsize=1;
  int tabsize=0,maxsize;
  gsl_rng *rx=NULL;
  const gsl_rng_type *T=NULL;
  extern VImage VGauss3d (VImage,VImage,VImage);
  extern VImage VSGaussKernel(double);
  extern void VZeroBorders(VImage,VImage);

  /*
  ** read smoothness estimation, if not specified on command line 
  */
  /*
  if (fwhm < 0) {
    if (VGetAttr (VImageAttrList (src), "smoothness", NULL,
		  VFloatRepn, (VPointer) & smoothness) == VAttrFound) {
      fwhm = smoothness;
    }
    else
      VError(" Please specify smoothness in fwhm");
  }
  */


  /*
  ** ini data structs
  */
  if (strlen(filename) > 2) {
    fp = fopen(filename,"w");
    if (!fp) VError(" error opening report file '%s' ",filename);
  }

  pthr = z2p(z0);
  fprintf(stderr,"\n");
  fprintf(stderr,"  fwhm: %.3f mm,  seed: %d,  numiter: %d\n",fwhm,seed,numiter);
  fprintf(stderr,"  z: %.5f,  p: %.7f\n",z0,pthr);

  if (fp) {
    fprintf(fp,"\n");
    fprintf(fp,"  fwhm: %.3f mm,  seed: %d,  numiter: %d\n",fwhm,seed,numiter);
    fprintf(fp,"  z: %.5f,  p: %.7f\n",z0,pthr);
  }


  nbands = VImageNBands(src);
  nrows  = VImageNRows(src);
  ncols  = VImageNColumns(src);
  npixels = nbands * nrows * ncols;
  fprintf(stderr,"  image dims:  %d %d %d\n",ncols,nrows,nbands);
  if (fp) fprintf(fp,"  image dims:  %d %d %d\n",ncols,nrows,nbands);

  mask = VCreateImage(nbands,nrows,ncols,VBitRepn);
  VFillImage(mask,VAllBands,0);

  nvox = 0;
  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	u = VGetPixel(src,b,r,c);
	if (ABS(u) > tiny) {
	  VPixel(mask,b,r,c,VBit) = 1;
	  nvox++;
	}
      }
    }
  }
  fprintf(stderr,"  num voxels:  %d\n",nvox);
  if (fp) fprintf(fp,"  num voxels:  %d\n",nvox);

  voxsize = x = y = z = 1;
  if (VGetAttr (VImageAttrList (src), "voxel", NULL,
                VStringRepn, (VPointer) & buf) == VAttrFound) {
    sscanf(buf,"%f %f %f",&x,&y,&z);
    voxsize = x*y*z;
    fprintf(stderr,"  voxel size:  %.2f x %.2f x %.2f = %.2f mm^3\n",x,y,z,voxsize);
    if (fp) fprintf(fp,"  voxel size:  %.2f x %.2f x %.2f = %.2f mm^3\n",x,y,z,voxsize);
  }
  sigma = fwhm/2.35482;
  sigma /= x;


  fprintf(stderr,"\n");
  if (fp) fprintf(fp,"\n");


  tabsize = npixels / 2;
  table     = (float *) VCalloc(tabsize,sizeof(float));
  histogram = (float *) VMalloc(sizeof(float) * tabsize);
  maxhist   = (float *) VMalloc(sizeof(float) * tabsize);
  for (i=0; i<tabsize; i++) histogram[i] = table[i] = maxhist[i] = 0;

  
  dest = VCreateImage(nbands,nrows,ncols,VFloatRepn);
  tmp  = VCreateImage(nbands,nrows,ncols,VFloatRepn);
  bin_image = VCreateImage(nbands,nrows,ncols,VBitRepn);
  VFillImage(dest,VAllBands,0);
  VFillImage(tmp,VAllBands,0);
  VFillImage(bin_image,VAllBands,0);

  if (sigma > 0) {
    kernel = VSGaussKernel(sigma);
    VZeroBorders(mask,kernel);
  }


  /*
  ** ini random number generator 
  */
  gsl_rng_env_setup();
  T  = gsl_rng_default;
  rx = gsl_rng_alloc(T);
  gsl_rng_set(rx,(unsigned long int)seed);



  /*
  ** main loop
  */
  for (iter=0; iter<numiter; iter++) {
    if (iter%10 == 0) fprintf(stderr," iter: %5d\r",iter);


    /*
    ** fill with random numbers
    */
    dst_pp = VImageData(dest);
    for (i=0; i<npixels; i++) {
      *dst_pp++ = gsl_ran_ugaussian(rx);
    }

    /*
    ** Gauss filter
    */
    if (sigma > 0) 
      tmp = VGauss3d (dest,tmp,kernel);
    else
      tmp = VCopyImage(dest,tmp,VAllBands);


    /*
    ** sample mean, std
    */
    sum1 = sum2 = nx = 0;
    dst_pp = VImageData(tmp);
    bin_pp = VImageData(mask);
    for (i=0; i<npixels; i++) {
      u = *dst_pp++;
      if (*bin_pp > 0) {
	sum1 += u;
	sum2 += u*u;
	nx++;
      }
      bin_pp++;
    }
    mean = sum1/nx;
    sig  = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));
    

    /*
    ** get threshold zthr
    */
    zthr = gsl_cdf_gaussian_Qinv(pthr,sig);


    /*
    ** get connected components
    */
    dst_pp = VImageData(tmp);
    bin_pp = VImageData(bin_image);
    bit_pp = VImageData(mask);

    for (i=0; i<npixels; i++) {
      *bin_pp = 0;
      u = *dst_pp++;
      if (u > zthr && *bit_pp > 0) *bin_pp = 1;
      bin_pp++;
      bit_pp++;
    }


    label_image = VLabelImage3d(bin_image,label_image,26,VShortRepn,&nl);
    if (nl < 1) continue; /* no clusters found */
    if (nl >= tabsize) VError(" table too small");

    for (i=0; i<tabsize; i++) table[i] = 0;
    short_pp = VImageData(label_image);
    for (i=0; i<npixels; i++) {
      j = *short_pp++;
      if (j >= tabsize) VError(" hist too small, %d",j);
      if (j > 0) table[j]++;
    }

    /*
    ** get histograms
    */
    maxsize = 0;
    for (i=1; i<tabsize; i++) {
      j = (int)table[i];
      histogram[j]++;
      if (j > maxsize) 	maxsize = j;
    }
    if (maxsize >= tabsize) VError(" maxhist too small, %d",j);
    maxhist[maxsize]++;
  }
  fprintf(stderr,"\n\n\n");

  

  /*
  ** output
  */
  for (i=0; i<tabsize; i++) table[i] = 0;
  for (i=1; i<tabsize; i++) {
    table[i] = maxhist[i]/(float)numiter;
  }
  fprintf(stderr,"     mm^3      freq     maxfreq    corrected p\n");
  fprintf(stderr,"  --------------------------------------------\n");

  if (fp) {
    fprintf(fp,"     mm^3      freq     maxfreq    corrected p\n");
    fprintf(fp,"  --------------------------------------------\n");
  }
  for (i=1; i<tabsize; i++) {
    if (histogram[i] > 0) {

      sum = 0;
      for (j=i; j<tabsize; j++) sum += table[j];

      if (sum < 0.12) {

	u = (float)i * voxsize;
	fprintf(stderr," %8.2f  %8.0f  %8.0f     %10.5f\n",u,histogram[i],maxhist[i],sum);
	
	if (fp) {
	  fprintf(fp," %8.2f  %8.0f  %8.0f     %10.5f\n",u,histogram[i],maxhist[i],sum);
	}
      }
    }
  }
  fprintf(stderr,"\n");
  if (fp) fprintf(fp,"\n");
  if (fp) fclose(fp);


  /* 
  ** if wanted, output test image 
  */
 ende:
  dst_pp = VImageData(tmp);
  bin_pp = VImageData(mask);
  for (i=0; i<npixels; i++) {
    if (*bin_pp == 0) *dst_pp = 0;
    bin_pp++;
    dst_pp++;
  }
  VCopyImageAttrs (src,tmp);
  return tmp;
}


int 
main (int argc, char *argv[])
{
  static VString test_filename = "";
  static VDouble z0      = 2.576;
  static VDouble fwhm    = 5;
  static VLong   seed    = 555;
  static VLong   numiter = 1000;
  static VString filename = "";
  static VOptionDescRec  options[] = {
    {"out",VStringRepn,1,(VPointer) &test_filename,VOptionalOpt,NULL,"Output test image"},
    {"z",VDoubleRepn,1,(VPointer) &z0,VOptionalOpt,NULL,"threshold"},
    {"fwhm",VDoubleRepn,1,(VPointer) &fwhm,VOptionalOpt,NULL,"fwhm of spatial smoothness in mm "},
    {"seed",VLongRepn,1,(VPointer) &seed,VOptionalOpt,NULL,"seed"},
    {"report",VStringRepn,1,(VPointer) &filename,VOptionalOpt,NULL,"report file"},
    {"iter",VLongRepn,1,(VPointer) &numiter,VOptionalOpt,NULL,"number of iterations"}
  };
  FILE *in_file,*fp=NULL;
  VAttrList list=NULL,out_list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;
  char prg[50];	
  sprintf(prg,"valphasim V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,NULL);


  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    dest = VAlphaSim(src,NULL,seed,numiter,fwhm,z0,filename);
    break;
  }
  if (src == NULL) VError(" no input image found");


  /* write test image */
  if (strlen(test_filename) > 2 && dest != NULL) {
    fp = fopen(test_filename,"w");
    out_list = VCreateAttrList();
    VAppendAttr (out_list,"image",NULL,VImageRepn,dest);
    VWriteFile (fp, out_list);
    fclose(fp);
  }
  return 0;
}
