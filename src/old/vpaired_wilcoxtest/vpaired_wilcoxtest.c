/****************************************************************
 *
 * Program: vpaired_wilcoxtest.c
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
 * $Id: vpaired_wilcoxtest.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/
 
#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>


#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_cdf.h>


#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


/* two-tailed significance levels */
extern double LevelOfSignificanceWXMPSR(double Winput, long int N);
extern float *getTable(int);
extern double p2z(double);
extern char * getLipsiaVersion();


VImage
PairedWilcoxTest(VImage *src1,VImage *src2,VImage dest, int n)
{
  int i,m,k,b,r,c,nslices,nrows,ncols;
  int sumpos,sumneg,w;
  double wx,u,v,z,p,tiny=1.0e-10;
  double *ptr1,*ptr2;
  float *table=NULL;
  gsl_vector *vec1=NULL,*vec2=NULL;
  gsl_permutation *perm=NULL,*rank=NULL;
  extern void gsl_sort_vector_index(gsl_permutation *,gsl_vector *);


  nslices = VImageNBands(src1[0]);
  nrows   = VImageNRows(src1[0]);
  ncols   = VImageNColumns(src1[0]);


  dest = VCopyImage(src1[0],NULL,VAllBands);
  VFillImage(dest,VAllBands,0);
  VSetAttr(VImageAttrList(dest),"num_images",NULL,VShortRepn,(VShort)n);
  VSetAttr(VImageAttrList(dest),"patient",NULL,VStringRepn,"paired_wilcoxtest");
  VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"zmap");


  m = 0;
  for (i=1; i<=n; i++) m += i;

  if (n > 18) {
    table = getTable(n);
    for (i=0; i<m; i++) {
      p = table[i];
      p *= 0.5;
      if (p < tiny) p = tiny;
      z = p2z(p);
      if (z < 0) z = 0;
      table[i] = z;
    }
  }
  else {
    table = (float *) VMalloc(sizeof(float) * m);
    for (i=0; i<m; i++) {

      for (i=0; i<m; i++) {
	wx = i;
	p = LevelOfSignificanceWXMPSR(wx,(long int)n);
	p *= 0.5;
	z = p2z(p);
	table[i] = z;
      }
    }
  }

  vec1 = gsl_vector_calloc(n);
  vec2 = gsl_vector_calloc(n);
  perm = gsl_permutation_alloc(n);
  rank = gsl_permutation_alloc(n);

  for (b=0; b<nslices; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	
	k = 0;
	ptr1 = vec1->data;
	ptr2 = vec2->data;
	for (i=0; i<n; i++) {
	  u = VPixel(src1[i],b,r,c,VFloat);
	  v = VPixel(src2[i],b,r,c,VFloat);
	  if (ABS(u) > tiny && ABS(v) > tiny) k++;
	  *ptr1++ = ABS(u-v);
	  *ptr2++ = u-v;
	}
	if (k < n/2) continue;

	gsl_sort_vector_index(perm,vec1);
	gsl_permutation_inverse(rank,perm);

	sumpos = sumneg = 0;
	ptr2 = vec2->data;
	for (i=0; i<n; i++) {
	  u = *ptr2++;
	  if (u > 0)
	    sumpos += rank->data[i]; 
	  else if (u < 0)
	    sumneg += rank->data[i];
	}

	w = sumpos;
	if (sumpos > sumneg) w = sumneg;

	if (w >= m) z = 0;
	else z = table[w];
	   
	if (sumneg > sumpos) z = -z;
	VPixel(dest,b,r,c,VFloat) = z;

      }
    }
  }

  return dest;
}




int main (int argc, char *argv[])
{
  static VArgVector in_files1;
  static VArgVector in_files2;
  static VString out_filename;
  static VOptionDescRec options[] = {
    { "in1", VStringRepn, 0, & in_files1,VRequiredOpt, NULL,"Input files 1" },
    { "in2", VStringRepn, 0, & in_files2, VRequiredOpt, NULL,"Input files 2" },
    { "out", VStringRepn, 1, & out_filename, VRequiredOpt, NULL,"Output file" }
  };
  FILE *fp=NULL;
  VStringConst in_filename,buf1,buf2;
  VAttrList list1,list2,out_list;
  VAttrListPosn posn;
  VImage src,*src1,*src2,dest=NULL;
  int i,nimages,npix=0;
  char prg_name[50];	
  sprintf(prg_name,"vpaired_wilcoxtest V%s", getLipsiaVersion());
  
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


  /* number of images */
  nimages = in_files1.number;
  if (in_files2.number != nimages) VError(" inconsistent number of files ");


  for (i = 0; i < nimages; i++) {
    buf1 = ((VStringConst *) in_files1.vector)[i];
    buf2 = ((VStringConst *) in_files2.vector)[i];
    fprintf(stderr,"%3d:  %s  %s\n",i,buf1,buf2);
  }
  fprintf(stderr,"\n");


  /* 
  ** read images 1 
  */
  src1 = (VImage *) VCalloc(nimages,sizeof(VImage));
  for (i = 0; i < nimages; i++) {
    src1[i] = NULL;
    in_filename = ((VStringConst *) in_files1.vector)[i];
    fp = VOpenInputFile (in_filename, TRUE);
    list1 = VReadFile (fp, NULL);
    if (! list1)  VError("Error reading image");
    fclose(fp);

    for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) != VFloatRepn) continue;
      src1[i] = src;
      break;
    }
    if (i == 0) npix = VImageNPixels(src1[i]);
    else if (npix != VImageNPixels(src1[i])) VError(" inconsistent image dimensions");
    if (src1[i] == NULL) VError(" no image found in %s",in_filename);
  }


  /* 
  ** read images 2 
  */
  src2 = (VImage *) VCalloc(nimages,sizeof(VImage));
  for (i = 0; i < nimages; i++) {
    src2[i] = NULL;
    in_filename = ((VStringConst *) in_files2.vector)[i];
    fp = VOpenInputFile (in_filename, TRUE);
    list2 = VReadFile (fp, NULL);
    if (! list2)  VError("Error reading image");
    fclose(fp);

    for (VFirstAttr (list2, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) != VFloatRepn) continue;
      src2[i] = src;
      break;
    }
    if (npix != VImageNPixels(src2[i])) VError(" inconsistent image dimensions");
    if (src2[i] == NULL) VError(" no image found in %s",in_filename);
  }


  /* 
  ** paired wilcoxon test 
  */
  dest = PairedWilcoxTest(src1,src2,dest,nimages);


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
