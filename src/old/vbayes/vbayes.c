/****************************************************************
 *
 * vbayes: fixed effects 2nd level bayes
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Authors: Gabriele Lohmann, Jane Neumann, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: vbayes.c 3519 2009-03-26 15:04:06Z lohmann $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_integration.h>

#define N 200 /* max number of input images */

#define ABS(x) ((x) > 0 ? (x) : -(x))

extern double p2z(double);
extern VAttrList VBayes(VImage [],VImage [],int,VBoolean,VBoolean);
extern char * getLipsiaVersion();


struct my_params {
  double mean;
  double sigma;
};


int main (int argc, char *argv[])
{
  static VArgVector in_files;
  static VString  out_filename;
  static VBoolean level  = FALSE;
  static VBoolean zscore = FALSE;
  static VBoolean in_found, out_found;
  static VOptionDescRec options[] = {
    { "in", VStringRepn, 0, & in_files,  & in_found, NULL,
      "Contrast images" },
    { "out", VStringRepn, 1, & out_filename, & out_found, NULL,
      "Output file"},
    { "level", VBooleanRepn, 1, & level, VOptionalOpt, NULL,
      "Whether to produce output to be used for 3rd level analysis"}
    /*    { "zscore", VBooleanRepn, 1, & zscore, VOptionalOpt, NULL,
	  "Whether to produce z-scores as output"} */
  };

  FILE *f;
  VStringConst in_filename;
  VAttrList list,out_list;
  VAttrListPosn posn;
  VImage src=NULL;
  VImage cbeta_images[N],sd_images[N];

  int i,nimages=0;
  VString str=NULL;
  char prg_name[50];	
  sprintf(prg_name,"vbayes V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);

  /* Parse command line arguments: */
  if (! VParseCommand (VNumber (options), options, & argc, argv) ||
      ! VIdentifyFiles (VNumber (options), options, "in", & argc, argv, 0) ||
      ! VIdentifyFiles (VNumber (options), options, "out", & argc, argv, -1))
    goto Usage;
  if (argc > 1) {
    VReportBadArgs (argc, argv);
  Usage:	VReportUsage (argv[0], VNumber (options), options, NULL);
  exit (EXIT_FAILURE);
  }
  nimages = in_files.number;
  fprintf(stderr,"Processing %d input files\n\n",nimages);

  if (nimages >= N)
    VError("Too many input images, max: %d",N);
  if(nimages == 0)  VError("Input images missing");

  /* loop through all input files */
  for (i = 0; i < nimages; i++) {
    in_filename = ((VStringConst *) in_files.vector)[i];
    fprintf(stderr,"%s\n",in_filename);

    f = fopen ((char *)in_filename, "r");
    if (! f) VError ("Failed to open input file %s", in_filename);
    if (! (list = VReadFile (f, NULL))) exit (EXIT_FAILURE);
    fclose (f);

    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) != VFloatRepn) continue;

      VGetAttr(VImageAttrList(src),"modality",NULL,VStringRepn,&str);

      if (strcmp(str,"conimg") == 0) {
	cbeta_images[i] = VCopyImage(src,NULL,VAllBands);
      }
      else if (strcmp(str,"std_dev") == 0 || strcmp(str,"sd") == 0) {
	sd_images[i] = VCopyImage(src,NULL,VAllBands);
      }
      else
	VError(" Illegal input file! Make sure to use con-images as input"); 
    }
  }


  /* calculate probabilities */
  out_list = VBayes(cbeta_images,sd_images,nimages,level,zscore);
  if (out_list == NULL) VError(" no result");

  
  /* 
  ** output
  */
  VHistory(VNumber(options),options,prg_name,&list,&out_list); 
  if (strcmp (out_filename, "-") == 0) VError("Output file required");
 
  f = fopen (out_filename, "w");
  if (! f)
    VError ("Failed to open output file %s", out_filename);
  if (! VWriteFile (f, out_list)) exit (EXIT_FAILURE);

  fprintf(stderr,"\n%s: done \n",argv[0]);
  return EXIT_SUCCESS;
}


/* Gaussian function */
double gauss(double x,void *p)
{
  struct my_params *par = p;
  double y,z,a=2.506628273;
  double mean=0,sigma=0;
  
  mean  = par->mean;
  sigma = par->sigma;

  z = (x-mean) / sigma;
  y = exp((double)-z*z*0.5)/(sigma * a);
  return y;
}


/* Integrate over distribution */
double CumulativeNormal(double mean,double sigma)
{
  double a=0,b=0;
  double y=0,error=0;
  static gsl_integration_workspace *work=NULL;
  struct my_params params;
  gsl_function F;

  F.function = &gauss;
  F.params = &params;
  params.mean = mean;
  params.sigma = sigma;

  if (work == NULL)
    work = gsl_integration_workspace_alloc (1000);

  if (mean >= 0) {
    a = 0;
    b = mean+5.0*sigma;
  }
  else {
    a = mean-5.0*sigma;
    b = 0;
  }
  gsl_integration_qag (&F, a, b, 0, 1e-7, 1000,GSL_INTEG_GAUSS15,work, &y, &error);
  return y;
}



VAttrList
VBayes(VImage cbeta_images[],VImage sd_images[],int nimages,VBoolean level,VBoolean zscore)
{
  VAttrList out_list=NULL;
  VImage dest=NULL,sigma_image=NULL;
  int    i,j,k,npixels;
  VFloat *dest_pp,*beta_pp[N],*sd_pp[N],*sigma_pp=NULL;
  VFloat pmin,pmax;
  double mean=0,sigma=0;
  float  rx,wsum,msum,w0,wx,s,msumold;
  float  result=0;
  float  gmean[N],gvar[N];
  float  tiny=1.0e-12;
 
  /* 
  ** create output image 
  */
  dest = VCopyImage(cbeta_images[0],NULL,VAllBands);
  if (!dest) return NULL;
  VFillImage(dest,VAllBands,0);
  
  VSetAttr(VImageAttrList(dest),"num_images",NULL,VShortRepn,(VShort)nimages);
  VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"bayes_map");
  VSetAttr(VImageAttrList(dest),"name",NULL,VStringRepn,"bayes");

  if (level == TRUE) {
    VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"mean");
    VSetAttr(VImageAttrList(dest),"name",NULL,VStringRepn,"mean");

    sigma_image = VCopyImage(dest,NULL,VAllBands);
    if (!sigma_image) return NULL;
    VFillImage(sigma_image,VAllBands,0);
    VSetAttr(VImageAttrList(sigma_image),"num_images",NULL,VShortRepn,(VShort)nimages);
    VSetAttr(VImageAttrList(sigma_image),"modality",NULL,VStringRepn,"std_dev");
    VSetAttr(VImageAttrList(sigma_image),"name",NULL,VStringRepn,"std_dev");
  }


  /*
  ** for each voxel 
  */
  pmax = VRepnMinValue(VFloatRepn);
  pmin = VRepnMaxValue(VFloatRepn);

  npixels = VImageNPixels(cbeta_images[0]);

  for (i=0; i<nimages; i++) {
    beta_pp[i] = (VFloat *) VImageData(cbeta_images[i]);
    sd_pp[i]   = (VFloat *) VImageData(sd_images[i]);
  }

  dest_pp = (VFloat *) VImageData(dest);
  if (level == TRUE) sigma_pp = (VFloat *) VImageData(sigma_image);

  for (j=0; j<npixels; j++) {
    if (j%10000 == 0) fprintf(stderr,"...%.2f%%\r",(float)100*j/(float)npixels);
    result = mean = sigma = 0;

    /* 
    ** read data from input images
    */
    for (k=0; k<nimages; k++) {
      gmean[k] = *beta_pp[k]++;  /* mean c*beta */
      rx       = *sd_pp[k]++;    /* standard deviation of c*beta*/
      gvar[k]  = rx*rx;          /* variation of c*beta*/
    }

    /* see Box,Tiao, pp.17-18     calculate probability distribution */
    wsum = 0;
    msum = 0;
    w0 = 0;

    for (k=0; k<nimages; k++) {  /* for each image */

      s = gvar[k];
      if (s < tiny) goto next;

      s = sqrt((double)s);
      if (s < tiny) goto next;

      wx = 1.0 / (s*s);
      wsum = w0 + wx;

      msumold = msum;
      msum = (w0 * msum  +  wx * gmean[k]) / wsum;
      w0 = wsum;
      sigma = 1.0 / sqrt(wsum);
    }
    if (wsum < tiny) goto next;

    /* resulting mean and std dev */
    mean = msum;                    
    sigma = 1.0 / sqrt(wsum);


    if (level == TRUE) {
      result = mean;
      goto next;
    }

    /* calculate probability under distribution */
    result = CumulativeNormal(mean,sigma);
    
    /* output */
    if (zscore) {
      /* CHECK HERE IF "1.0-" is correct */
      result = (float)p2z((double)(1.0-result));
      if (mean < 0) result = -result;
      if (result >  20) result =  20;
      if (result < -20) result = -20;
    }
    else {
      result *= 100.0;
      if (result >  100) result =  100;
      if (mean < 0) result = -result;
    }

    if (result < pmin) pmin = result;
    if (result > pmax) pmax = result;

  next:
    *dest_pp++ = result;
    if (level) *sigma_pp++ = sigma;
  }

  
  out_list = VCreateAttrList ();
  fprintf(stderr,"...100.00%%\n");
  if (level == FALSE) {
    fprintf(stderr,"\n min: %.3f, max: %.3f\n",pmin,pmax);
    VAppendAttr (out_list,"zmap",NULL,VImageRepn,dest);
    return out_list;
  }
  else {
    VAppendAttr (out_list,"mean",NULL,VImageRepn,dest);
    VAppendAttr (out_list,"std_dev",NULL,VImageRepn,sigma_image);
    return out_list;
  }
  return NULL;
}

