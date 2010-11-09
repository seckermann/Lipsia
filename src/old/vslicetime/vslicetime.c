/****************************************************************
 *
 * Program: vslicetime
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
 * $Id: vslicetime.c 3354 2008-05-30 08:30:37Z karstenm $
 *
 *****************************************************************/

/* K. Mueller: Added option for reading slicetimes from an ASCII file */
 
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#define LEN 1024

extern char * getLipsiaVersion();



/*
** read slice onset times from ASCII file, if not in header
*/
float *
ReadSlicetimes(VString filename)
{
  FILE *fp;
  int   i,j,id,n=256;
  float onset;
  float *onset_array=NULL;
  char   buf[LEN];

  onset_array = (float *) VCalloc(n,sizeof(float));
  fp = fopen(filename,"r");
  if (!fp) VError(" error opening file %s",filename);
  /* fprintf(stderr," reading file: %s\n",filename); */
  i = 0;
  while (!feof(fp)) {
    if (i >= n) VError(" too many lines in file %s, max is %d",filename,n);
    for (j=0; j<LEN; j++) buf[j] = '\0';
    fgets(buf,LEN,fp);
    if (buf[0] == '%' || buf[0] == '#') continue;
    if (strlen(buf) < 2) continue;

    if (sscanf(buf,"%f",&onset) != 1)
      VError(" line %d: illegal input format",i+1);
    onset_array[i] = onset;
    i++;
  }
  fclose(fp);
  return onset_array;
}



/*
** slicetime correction for non-constant TR
*/
void
VSlicetime_NC(VAttrList list,VShort minval,VFloat tdel,
	      VBoolean slicetime_correction,float *onset_array,VString filename)
{
  FILE *fp=NULL;
  VImage src;
  VAttrListPosn posn;
  VString buf,str;
  int b,r,c,i,j,nt=0,mt=0,val,del=0;
  double xmin,xmax,sum,nx,estim_tr=0;
  double *xx=NULL,*yy=NULL,xi,yi,slicetime=0,u=0;
  gsl_interp_accel *acc=NULL;
  gsl_spline *spline=NULL;

  xmin = (VShort) VRepnMinValue(VShortRepn);
  xmax = (VShort) VRepnMaxValue(VShortRepn);
  buf = VMalloc(LEN);


  /*
  ** read scan times from file, non-constant TR
  */
  if (strlen(filename) > 2) {
    fp = fopen(filename,"r");
    if (!fp) VError(" error opening file %s",filename);
    i = 0;
    while (!feof(fp)) {
      for (j=0; j<LEN; j++) buf[j] = '\0';
      fgets(buf,LEN,fp);
      if (buf[0] == '%' || buf[0] == '#') continue;
      if (strlen(buf) < 2) continue;
      i++;
    }

    rewind(fp);
    nt = i;
    fprintf(stderr," num timesteps: %d\n",nt);
    xx = (double *) VCalloc(nt,sizeof(double));
    yy = (double *) VCalloc(nt,sizeof(double));
    i = 0;
    sum = 0;
    while (!feof(fp)) {
      for (j=0; j<LEN; j++) buf[j] = '\0';
      fgets(buf,LEN,fp);
      if (buf[0] == '%' || buf[0] == '#') continue;
      if (strlen(buf) < 2) continue;
      if (sscanf(buf,"%lf",&u) != 1) VError(" line %d: illegal input format",i+1);
      xx[i] = u * 1000.0;  /* convert to millisec */
      if (i > 1) sum += xx[i] - xx[i-1];
      i++;
    }
    fclose(fp);
    estim_tr = sum /(double)(nt-2);
    fprintf(stderr," average scan interval = %.3f sec\n",estim_tr/1000.0);
  }


  /*
  ** process data
  */
  b = -1;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VPixelRepn(src) != VShortRepn) continue;
    VSetAttr(VImageAttrList(src),"repetition_time",NULL,VLongRepn,(VLong)estim_tr);
    VExtractAttr (VImageAttrList(src),"MPIL_vista_0",NULL,VStringRepn,&str,FALSE);
    b++;
    if (VImageNRows(src) < 2) continue;


    /*
    ** get header info
    */
    if (VGetAttr (VImageAttrList (src), "slice_time", NULL,
		  VDoubleRepn, (VPointer) & slicetime) != VAttrFound && slicetime_correction && onset_array==NULL) 
      VError(" 'slice_time' info missing");
    if (onset_array!=NULL)
      slicetime = onset_array[b];
  
    
    if (nt != VImageNBands(src)) VError(" inconsistent number of time steps, %d %d",
					nt,VImageNBands(src));

    if (acc == NULL && slicetime_correction) {
      acc = gsl_interp_accel_alloc ();
      spline = gsl_spline_alloc (gsl_interp_akima,nt);

      for (i=0; i<5; i++) {
	if (xx[i]/1000.0 > tdel) break;
      }
      del = i;
      fprintf(stderr," The first %.2f secs (%d timesteps) will be replaced.\n",tdel,del);
    }


    /*
    ** loop through all voxels in current slice
    */
    if (slicetime_correction)
      fprintf(stderr," slice: %3d,  %10.3f ms\r",b,slicetime);

    for (r=0; r<VImageNRows(src); r++) {
      for (c=0; c<VImageNColumns(src); c++) {
	if (VPixel(src,0,r,c,VShort) < minval) continue;

	/* replace first few time steps by average */
	if (del > 0) {
	  mt = del+10;
	  if (mt > nt) mt = nt;
	  sum = nx = 0;
	  for (i=del; i<mt; i++) {
	    sum += VPixel(src,i,r,c,VShort);
	    nx++;
	  }
	  if (nx < 1) continue;
	  val = sum/nx;

	  for (i=0; i<del; i++) {
	    VPixel(src,i,r,c,VShort) = val;
	  }
	}
	if (!slicetime_correction) continue;

	/* correct for slicetime offsets using cubic spline interpolation */
	for (i=0; i<nt; i++) {
	  yy[i] = VPixel(src,i,r,c,VShort);
	}
	gsl_spline_init (spline,xx,yy,nt);


	for (i=1; i<nt; i++) {
	  xi = xx[i] - slicetime;
	  yi = gsl_spline_eval (spline,xi,acc);
	  val = (int)(yi+0.49);
	  if (val > xmax) val = xmax;
	  if (val < xmin) val = xmin;
	  VPixel(src,i,r,c,VShort) = val;
	}
      }
    }
  }
  fprintf(stderr,"\n");
}


/*
** slicetime correction with constant TR
*/
void
VSlicetime(VAttrList list,VShort minval,VFloat tdel,
	   VBoolean slicetime_correction,float *onset_array)
{
  VImage src;
  VAttrListPosn posn;
  VString str,buf;
  int b,r,c,i,nt,mt,val,del=0;
  double xmin,xmax,sum,nx;
  double *xx=NULL,*yy=NULL,xi,yi,tr=0,xtr=0,slicetime=0;
  gsl_interp_accel *acc=NULL;
  gsl_spline *spline=NULL;

  xmin = (VShort) VRepnMinValue(VShortRepn);
  xmax = (VShort) VRepnMaxValue(VShortRepn);
  buf = VMalloc(256);


  /*
  ** process data
  */
  b = -1;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VPixelRepn(src) != VShortRepn) continue;
    b++;
    if (VImageNRows(src) < 2) continue;
    
    /*
    ** get header info
    */
    if (VGetAttr (VImageAttrList (src), "slice_time", NULL,
		  VDoubleRepn, (VPointer) & slicetime) != VAttrFound && slicetime_correction && onset_array==NULL) 
      VError(" 'slice_time' info missing");;
    if (onset_array!=NULL) 
      slicetime = onset_array[b];


    tr = 0;
    if (VGetAttr (VImageAttrList (src), "repetition_time", NULL,
		  VDoubleRepn, (VPointer) & tr) != VAttrFound) {
      tr = 0;
      if (VGetAttr (VImageAttrList (src), "MPIL_vista_0", NULL,
		    VStringRepn, (VPointer) & str) == VAttrFound) {
	sscanf(str," repetition_time=%lf %s",&tr,buf);
      }
    }
    if (tr < 1)
      VError(" attribute 'repetition_time' missing");

    xtr = tr/1000.0; 
    del = (int)(tdel/xtr + 0.5); /* num timesteps to be ignored */

    nt = VImageNBands(src);
    if (acc == NULL) {
      acc = gsl_interp_accel_alloc ();
      spline = gsl_spline_alloc (gsl_interp_akima,nt);
      xx = (double *) VCalloc(nt,sizeof(double));
      yy = (double *) VCalloc(nt,sizeof(double));
      fprintf(stderr," The first %.2f secs (%d timesteps) will be replaced.\n",tdel,del);
    }


    /*
    ** loop through all voxels in current slice
    */
    if (slicetime_correction)
      fprintf(stderr," slice: %3d,  %10.3f ms,  TR: %.3f\r",b,slicetime,xtr);

    for (r=0; r<VImageNRows(src); r++) {
      for (c=0; c<VImageNColumns(src); c++) {
	if (VPixel(src,0,r,c,VShort) < minval) continue;

	/* replace first few time steps by average */
	if (del > 0) {
	  mt = del+10;
	  if (mt > nt) mt = nt;
	  sum = nx = 0;
	  for (i=del; i<mt; i++) {
	    sum += VPixel(src,i,r,c,VShort);
	    nx++;
	  }
	  if (nx < 1) continue;
	  val = sum/nx;

	  for (i=0; i<del; i++) {
	    VPixel(src,i,r,c,VShort) = val;
	  }
	}
	if (!slicetime_correction) continue;


	/* correct for slicetime offsets using cubic spline interpolation */
	for (i=0; i<nt; i++) {
	  xi = i;
	  xx[i] = xi*tr;
	  yy[i] = VPixel(src,i,r,c,VShort);
	}
	gsl_spline_init (spline,xx,yy,nt);


	for (i=1; i<nt; i++) {
	  xi = xx[i] - slicetime;
	  yi = gsl_spline_eval (spline,xi,acc);

	  val = (int)(yi+0.49);
	  if (val > xmax) val = xmax;
	  if (val < xmin) val = xmin;
	  VPixel(src,i,r,c,VShort) = val;
	}
      }
    }
  }
  fprintf(stderr,"\n");
}



int
main(int argc, char *argv[])
{
  static VShort minval = 0;
  static VFloat tdel = 5;
  static VBoolean slicetime_correction = TRUE;
  static VString slicetime = "";
  static VString filename = "";

  static VOptionDescRec  options[] = {
    {"minval",VShortRepn,1,(VPointer) &minval,VOptionalOpt,NULL,"Signal threshold"},
    {"correction",VBooleanRepn,1,(VPointer) &slicetime_correction,VOptionalOpt,NULL,
     "Whether to perform slicetime correction"},
    {"slicetime",VStringRepn,1,(VPointer) &slicetime,VOptionalOpt,NULL,
     "ASCII file containing slice times in milliseconds"},
    {"scanfile",VStringRepn,1,(VPointer) &filename,VOptionalOpt,NULL,
     "ASCII file containing scan times in seconds"},
    {"del",VFloatRepn,1,(VPointer) &tdel,VOptionalOpt,NULL,"First few seconds to be ignored"}
  };
  FILE *in_file=NULL,*out_file=NULL;  
  VAttrList list=NULL;
  float *onset_array=NULL;
  char prg[50];	

  sprintf(prg,"vslicetime V%s", getLipsiaVersion());  
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  if (strlen(slicetime) > 2) 
    onset_array = ReadSlicetimes(slicetime);

  if (strlen(filename) < 2) {
    VSlicetime(list,minval,tdel,slicetime_correction,onset_array);
  }
  else {
    VSlicetime_NC(list,minval,tdel,slicetime_correction,onset_array,filename);
  }

  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
