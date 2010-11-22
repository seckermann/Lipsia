
/****************************************************************
 *
 * Program: vtwosample_ttest
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vtwosample_ttest.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/
#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_errno.h>

#include <math.h>
#include <stdio.h>
#include <string.h>


#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

extern void VGaussianize(VImage *,int);
extern double t2z(double,double);
extern float t2z_approx(float,float);
extern char * getLipsiaVersion();

void 
avevar(float *data,int n,float *a,float *v)
{
  int j;
  float ave,var,nx,s,u;

  nx = (float)n;
  ave = 0;
  for (j=0; j<n; j++) ave += data[j];
  ave /= nx;

  var = u = 0;
  for (j=0; j<n; j++) {
    s = data[j]-ave;
    u   += s;
    var += s*s;
  }
  var=(var-u*u/nx)/(nx-1);
  
  *v = var;
  *a = ave;
}


VImage
TwoSampleTest(VImage *src1,VImage *src2,VImage dest,int n1,int n2,VShort type)
{
  int i,k,b,r,c,nslices,nrows,ncols,idf;
  float ave1,ave2,var1,var2,nx1,nx2;
  float sum,nx,smooth=0;
  float u,t,z,df,*data1=NULL,*data2=NULL;
  float tiny=1.0e-10;

  nslices = VImageNBands(src1[0]);
  nrows   = VImageNRows(src1[0]);
  ncols   = VImageNColumns(src1[0]);


  dest = VCopyImage(src1[0],NULL,VAllBands);
  VFillImage(dest,VAllBands,0);
  VSetAttr(VImageAttrList(dest),"num_images",NULL,VShortRepn,(VShort)(n1+n2));
  VSetAttr(VImageAttrList(dest),"patient",NULL,VStringRepn,"two_sample_ttest");

  if (type == 0)
    VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"tmap");
  else
    VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"zmap");


  /* get smoothness estimates */
  sum = nx = 0;
  for (i=0; i<n1; i++) {
    if (VGetAttr(VImageAttrList(src1[i]),"smoothness",NULL,VFloatRepn,&smooth) == VAttrFound) {
      sum += smooth;
      nx++;
    }
  }
  for (i=0; i<n2; i++) {
    if (VGetAttr(VImageAttrList(src2[i]),"smoothness",NULL,VFloatRepn,&smooth) == VAttrFound) {
      sum += smooth;
      nx++;
    }
  }
  if (nx > 1) {
    VSetAttr(VImageAttrList(dest),"smoothness",NULL,VFloatRepn,sum/nx);
  }


  /* degrees of freedom */
  df  = n1+n2-2;
  idf = df;
  VSetAttr(VImageAttrList(dest),"df",NULL,VShortRepn,(VShort)idf);
  
  data1 = (float *) VCalloc(n1,sizeof(float));
  data2 = (float *) VCalloc(n2,sizeof(float));

  nx1 = (float)n1;
  nx2 = (float)n2;

  for (b=0; b<nslices; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {

	k = 0;
	for (i=0; i<n1; i++) {
	  data1[i] = 0;
	  u = VPixel(src1[i],b,r,c,VFloat);
	  if (isnanf(u) || isinff(u)) continue;
	  if (ABS(u) > tiny) k++;
	  data1[i] = u;
	}
	if (k < n1-3) continue;

	k = 0;
	for (i=0; i<n2; i++) {
	  data2[i] = 0;
	  u = VPixel(src2[i],b,r,c,VFloat);
	  if (isnanf(u) || isinff(u)) continue;
	  if (ABS(u) > tiny) k++;
	  data2[i] = u;
	}
	if (k < n2-3) continue;

	avevar(data1,n1,&ave1,&var1);
	avevar(data2,n2,&ave2,&var2);
	if (var1 < tiny || var2 < tiny) continue;

	t=(ave1-ave2)/sqrt(var1/nx1+var2/nx2);
	if (isnanf(t) || isinff(t)) continue;

	switch(type) {
	case 0:
	  VPixel(dest,b,r,c,VFloat) = t;
	  break;

	case 1:
	  df = SQR(var1/nx1+var2/nx2)/(SQR(var1/nx1)/(nx1-1)+SQR(var2/nx2)/(nx2-1));
	  z  = t2z((double) t,(double) df);
	  if (t < 0) z = -z;
	  VPixel(dest,b,r,c,VFloat) = z;
	  break;

	default:
	  VError(" illegal type");
	}
      }
    }
  }

  return dest;
}


VDictEntry TypeDict[] = {
  { "tmap", 0 },
  { "zmap", 1 },
  { NULL }
};


int main (int argc, char *argv[])
{
  static VArgVector in_files1;
  static VArgVector in_files2;
  static VString out_filename;
  static VShort type = 1;
  static VBoolean gauss=FALSE;
  static VOptionDescRec options[] = {
    { "in1", VStringRepn, 0, & in_files1, VRequiredOpt, NULL,"Input files 1" },
    { "in2", VStringRepn, 0, & in_files2, VRequiredOpt, NULL,"Input files 2" },
    { "type",VShortRepn,1,(VPointer) &type,VOptionalOpt,TypeDict,"output type"},
    { "gaussianize",VBooleanRepn,1,(VPointer) &gauss,VOptionalOpt,NULL,"Whether to gaussianize"},
    { "out", VStringRepn, 1, & out_filename, VRequiredOpt, NULL,"Output file" }
  };
  FILE *fp=NULL;
  VStringConst in_filename;
  VAttrList list1,list2,out_list;
  VAttrListPosn posn;
  VString str;
  VImage src,*src1,*src2,dest=NULL;
  int i,n1,n2,npix=0;

  char prg_name[50];	
  sprintf(prg_name,"vtwosample_ttest V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);

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
  if (type < 0 || type > 1) VError(" illegal type");




  /*
  ** read images 1 
  */
  fprintf(stderr," group 1:\n");
  n1 = in_files1.number;
  src1 = (VImage *) VMalloc(sizeof(VImage) * n1);
  for (i = 0; i < n1; i++) {
    src1[i] = NULL;
    in_filename = ((VStringConst *) in_files1.vector)[i];
    fprintf(stderr," %3d:  %s\n",i,in_filename);
    fp = VOpenInputFile (in_filename, TRUE);
    list1 = VReadFile (fp, NULL);
    if (! list1)  VError("Error reading image");
    fclose(fp);

    for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) != VFloatRepn) continue;

      if (VGetAttr(VImageAttrList(src),"modality",NULL,VStringRepn,&str) == VAttrFound) {
	if (strcmp(str,"conimg") != 0) continue;
      }
      if (i == 0) npix = VImageNPixels(src);
      else if (npix != VImageNPixels(src)) VError(" inconsistent image dimensions");
      
      src1[i] = src;
      break;
    }
    if (src1[i] == NULL) VError(" no contrast image found in %s",in_filename);
  }


  /* 
  ** read images 2 
  */
  fprintf(stderr,"\n group 2:\n");
  n2 = in_files2.number;
  src2 = (VImage *) VMalloc(sizeof(VImage) * n2);
  for (i = 0; i < n2; i++) {
    src2[i] = NULL;
    in_filename = ((VStringConst *) in_files2.vector)[i];
    fprintf(stderr," %3d:  %s\n",i,in_filename);
    fp = VOpenInputFile (in_filename, TRUE);
    list2 = VReadFile (fp, NULL);
    if (! list2)  VError("Error reading image");
    fclose(fp);

    for (VFirstAttr (list2, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) != VFloatRepn) continue;

      if (VGetAttr(VImageAttrList(src),"modality",NULL,VStringRepn,&str) == VAttrFound) {
	if (strcmp(str,"conimg") != 0) continue;
      }
      if (npix != VImageNPixels(src)) VError(" inconsistent image dimensions");

      src2[i] = src;
      break;
    }
    if (src2[i] == NULL) VError(" no contrast image found in %s",in_filename);
  }


  /* 
  ** make normally distributed 
  */
  if (gauss) {
    VGaussianize(src1,n1);
    VGaussianize(src2,n2);
  }


  /* 
  ** two sample t-test 
  */
  dest = TwoSampleTest(src1,src2,dest,n1,n2,type);


  /* 
  ** output
  */
  out_list = VCreateAttrList ();
  VHistory(VNumber(options),options,prg_name,&list1,&out_list); 
  VAppendAttr (out_list,"image",NULL,VImageRepn,dest);

  fp = VOpenOutputFile (out_filename, TRUE);
  if (! VWriteFile (fp, out_list)) exit (1);
  fclose(fp);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
