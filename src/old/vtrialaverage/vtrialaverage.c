/****************************************************************
 *
 * Program: vtrialaverage
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
 * $Id: vtrialaverage.c 3191 2008-04-01 16:11:38Z karstenm $
 *
 *****************************************************************/
 
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_spline.h>
#include "gsl_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <via.h>

extern void VGetVoxelCoord(VImage,float,float,float,float *,float *,float *);
extern char * getLipsiaVersion();


#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x)*(x))

#define NSLICES   256   /* max number of slices */
#define LEN     10000   /* buffer length        */
#define NTRIALS  5000   /* max number of trials */
#define MAXCOND   100   /* max number of experimental conditions */

typedef struct FpointStruct{
  VFloat x;
  VFloat y;
  VFloat z;
} FPoint;



typedef struct TrialStruct{
  int   id;
  float onset;
  float duration;
  float height;
} Trial;



Trial trial[NTRIALS];
int ntrials=0;
int nevents=0;

int
test_ascii(int val)
{
  if (val >= 'a' && val <= 'z') return 1;
  if (val >= 'A' && val <= 'Z') return 1;
  if (val >= '0' && val <= '9') return 1;
  if (val ==  ' ') return 1;
  if (val == '\0') return 1;
  if (val == '\n') return 1;
  if (val == '\r') return 1;
  if (val == '\t') return 1;
  if (val == '\v') return 1;
  return 0;
}


/*
** parse design file
*/
void
ReadDesign(VString designfile)
{
  FILE *fp=NULL;
  int  i,j,k,id;
  char buf[LEN];
  float onset=0,duration=0,height=0;

  fp = fopen(designfile,"r");
  if (!fp) VError(" error opening design file %s",designfile);

  i = ntrials = nevents = 0;
  while (!feof(fp)) {
    for (j=0; j<LEN; j++) buf[j] = '\0';
    fgets(buf,LEN,fp);
    if (strlen(buf) < 2) continue;
    if (buf[0] == '%' || buf[0] == '#') continue;
    if (! test_ascii((int)buf[0])) VError(" input file must be a text file");


    /* remove non-alphanumeric characters */
    for (j=0; j<strlen(buf); j++) {
      k = (int)buf[j];
      if (!isgraph(k) && buf[j] != '\n' && buf[j] != '\r' && buf[j] != '\0') {
	buf[j] = ' ';
      }
      if (buf[j] == '\v') buf[j] = ' '; /* remove tabs */
      if (buf[j] == '\t') buf[j] = ' ';
    }

    if (sscanf(buf,"%d %f %f %f",&id,&onset,&duration,&height) != 4)
      VError(" line %d: illegal input format",i+1);
    
    if (duration < 0.5 && duration >= -0.0001) duration = 0.5;
    trial[i].id       = id-1;
    trial[i].onset    = onset;
    trial[i].duration = duration;
    trial[i].height   = height;
    i++;
    if (i > NTRIALS) VError(" too many trials %d",i);

    if (id > nevents) nevents = id;
  }
  fclose(fp);

  ntrials = i;
}




VDictEntry TYPDict[] = {
  { "single", 0 },
  { "6adj",   1 },
  { "26adj",  2 },
  { "blob",   3 },
  { NULL }
};

VDictEntry CoordDict[] = {
  { "voxel", 0 },
  { "talairach", 1 },
  { NULL }
};


int
main (int argc,char *argv[])
{
  static VString  designfile = "";
  static VString  mask_filename = "";
  static VString  rep_filename = "";
  static VString  zmap_filename = "";
  static VFloat   pos =  3.09;
  static VFloat   neg = -1000;
  static VShort   minval = 0;
  static VString  strcond = "";
  static VShort   adj = 0;
  static VShort   coord = 0;
  static VDouble  temporal_resolution = 0.25;
  static VDouble  length = 15;
  static FPoint   addr;
  static VBoolean psc=TRUE;
  static VOptionDescRec  options[] = {
    {"design", VStringRepn, 1, & designfile, VRequiredOpt, NULL,"Design file" },
    {"report", VStringRepn, 1, & rep_filename, VOptionalOpt, NULL,"Report file" },
    {"minval", VShortRepn, 1, & minval, VOptionalOpt, NULL,"minval"},
    {"addr", VFloatRepn, 3, &addr, VRequiredOpt, 0, "voxel address" },
    {"type", VShortRepn, 1, & adj, VOptionalOpt, TYPDict,"How to average across adjacent voxels"},
    {"mask", VStringRepn, 1, & mask_filename, VOptionalOpt, NULL,"File containing ROI mask" },
    {"zmap", VStringRepn, 1, & zmap_filename, VOptionalOpt, NULL,"Zmap file" },
    {"pos", VFloatRepn, 1, & pos, VOptionalOpt, NULL,"Positive threshold for zmap"},
    {"neg", VFloatRepn, 1, & neg, VOptionalOpt, NULL,"Negative threshold for zmap"},
    {"cond", VStringRepn, 1, & strcond, VRequiredOpt, NULL,"Experimental conditions"},
    {"system", VShortRepn, 1, & coord, VOptionalOpt, CoordDict,"coordinate system"},
    {"resolution", VDoubleRepn, 1, & temporal_resolution, VOptionalOpt, NULL,
     "temporal resolution in secs"},
    {"length", VDoubleRepn, 1, & length, VOptionalOpt, NULL, "trial length in seconds"},
    {"psc", VBooleanRepn, 1, & psc, VOptionalOpt, NULL, "Whether to report percent signal change"}
  };
  FILE *in_file=NULL,*fp=NULL,*fpe=NULL;
  VAttrList list=NULL,mlist=NULL,zlist=NULL;
  VAttrListPosn posn;
  VString str,buf;
  int nbeta=0;
  VBit *bin_pp;
  VFloat *flt_pp;
  VImage xsrc=NULL,src[NSLICES],mask=NULL,zmap=NULL,designmatrix=NULL;
  Volumes volumes;
  Volume vol=NULL,vol0=NULL;
  VTrack tc;
  int i,j,k,id,numcond,cond[MAXCOND];
  char *token;
  int b0,r0,c0,c1,b,r,c,nslices,nrows,ncols,ntimesteps=0,nt=0;
  float x,y,z;
  double xi,yi,*xx=NULL,*yy=NULL;
  double t,tstep,tr=0,experiment_duration=0;
  double sum,*sum1=NULL,*sum2=NULL,mean,sigma,xerr,u,v,w,nx;
  gsl_interp_accel *acc=NULL;
  gsl_spline *spline=NULL;

  char prg[50];	
  sprintf(prg,"vtrialaverage V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);


  /*
  ** parse command line
  */
  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,NULL);



  /*
  ** read design file
  */
  ReadDesign(designfile);


  /* 
  ** parse command string 
  */
  for (i=0; i<MAXCOND; i++) cond[i] = 0;
  numcond = 0;
  token = strtok(strcond," ");

  i = 0;
  while (token) {
    if (i > MAXCOND) VError(" too many conditions");
    cond[i] = atoi(token);
    if (cond[i] > nevents) VError(" illegal condition (%d), max is %d",cond[i],nevents);
    if (cond[i] < 1) VError(" illegal condition id, must be positive");
    token = strtok(NULL," ");
    i++;
  }
  numcond = i;



  /*
  ** read functional image data
  */
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  nslices = nrows = ncols = ntimesteps = nbeta = 0;
  i = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & xsrc);
    if (i >= NSLICES) VError(" too many slices %d",i);

    if (VGetAttr (VImageAttrList (xsrc), "modality", NULL,
                  VStringRepn, (VPointer) & str) != VAttrFound)
      VError(" attribute 'modality' missing");


    if (strcmp(str,"X") == 0) {
      designmatrix = xsrc;
    }
    else if (strcmp(str,"BETA") == 0 || VPixelRepn(xsrc) == VShortRepn) {
      src[i++] = xsrc;
      if (VImageNBands(xsrc) > ntimesteps) ntimesteps = VImageNBands(xsrc);
      if (VImageNRows(xsrc) > nrows) nrows = VImageNRows(xsrc);
      if (VImageNColumns(xsrc) > ncols) ncols = VImageNColumns(xsrc);
    }
  }
  nslices = i;

  if (designmatrix != NULL) {
    nbeta = nslices;
    nslices = VImageNBands(src[0]);
    ntimesteps = VImageNRows(designmatrix);
    if (nbeta != VImageNColumns(designmatrix))
      VError(" inconsistent dimensions: design matrix does not match beta file");
  }



  /*
  ** get voxel address
  */
  if (coord == 0) {
    c0 = (int)(addr.x + 0.499);
    r0 = (int)(addr.y + 0.499);
    b0 = (int)(addr.z + 0.499);
  }
  else {
    VGetVoxelCoord(src[0],addr.x,addr.y,addr.z,&z,&y,&x);
    c0 = (int)(x + 0.499);
    r0 = (int)(y + 0.499);
    b0 = (int)(z + 0.499);
  }

  if (c0 < 0 || c0 >= ncols) VError("illegal voxel coordinate");
  if (r0 < 0 || r0 >= nrows) VError("illegal voxel coordinate");
  if (b0 < 0 || b0 >= nslices) VError("illegal voxel coordinate");



  /*
  ** read repetition time
  */
  tr = 0;
  if (VGetAttr (VImageAttrList (src[0]), "repetition_time", NULL,
		VDoubleRepn, (VPointer) & tr) != VAttrFound) {
    tr = 0;
    buf = VMalloc(256);
    if (VGetAttr (VImageAttrList (src[0]), "MPIL_vista_0", NULL,
		  VStringRepn, (VPointer) & str) == VAttrFound) {
      sscanf(str," repetition_time=%lf %s",&tr,buf);
    }
  }
  if (tr < 1) VError(" attribute 'repetition_time' missing");
  tr /= 1000.0;
  experiment_duration = tr * (double)ntimesteps;




  /*
  ** read mask
  */
  if (strlen(mask_filename) > 2) {
    fp = VOpenInputFile (mask_filename, TRUE);
    mlist = VReadFile (fp, NULL);
    if (! mlist)  VError("Error reading mask image");
    fclose(fp);

    for (VFirstAttr (mlist, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & mask);
      if (VPixelRepn(mask) != VUByteRepn && VPixelRepn(mask) != VBitRepn) {
	mask = NULL;
	continue;
      }
      break;
    }
    if (mask == NULL) VError(" no mask found");
    if (VImageNColumns(mask) != ncols) VError(" inconsistent image dimensions");
    if (VImageNRows(mask) != nrows) VError(" inconsistent image dimensions");
    if (VImageNBands(mask) != nslices) VError(" inconsistent image dimensions");
  }

  else {
    mask = VCreateImage(nslices,nrows,ncols,VBitRepn);
    VFillImage(mask,VAllBands,0);
    VPixel(mask,b0,r0,c0,VBit) = 1;

    if (adj == 1) {    /* 6-adjacency */
      if (b0 < nslices-1) VPixel(mask,b0+1,r0,c0,VBit) = 1;
      if (b0 > 1) VPixel(mask,b0-1,r0,c0,VBit) = 1;
      if (r0 < nrows-1) VPixel(mask,b0,r0+1,c0,VBit) = 1;
      if (r0 > 1) VPixel(mask,b0,r0-1,c0,VBit) = 1;
      if (c0 < ncols-1) VPixel(mask,b0,r0,c0+1,VBit) = 1;
      if (c0 > 1) VPixel(mask,b0,r0,c0-1,VBit) = 1;
    }

    if (adj == 2) {  /* 26-adjacency */
      for (b=b0-1; b<= b0+1; b++) {
	if (b < 0 || b >= nslices) continue;

	for (r=r0-1; r<= r0+1; r++) {
	  if (r < 0 || r >= nrows) continue;

	  for (c=c0-1; c<= c0+1; c++) {
	    if (c < 0 || c >= ncols) continue;
	    VPixel(mask,b,r,c,VBit) = 1;
	  }
	}
      }
    }

  }
  if (mask == NULL) VError(" no mask image found");


  /*
  ** read zmap, if present, and use for thresholding
  */
  if (strlen(zmap_filename) > 2) {
    fp = VOpenInputFile (zmap_filename, TRUE);
    zlist = VReadFile (fp, NULL);
    if (! zlist)  VError("Error reading zmap");
    fclose(fp);

    for (VFirstAttr (zlist, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & zmap);
      if (VPixelRepn(zmap) != VFloatRepn) {
	zmap = NULL;
	continue;
      }
      break;
    }
    if (zmap == NULL) VError(" zmap not found");

    if (mask == NULL) {
      mask = VCreateImage(nslices,nrows,ncols,VBitRepn);
      VFillImage(mask,VAllBands,0);
    }

    if (VImageNPixels(mask) != VImageNPixels(zmap))
      VError(" inconsistent image dimensions in zmap");
    
    /* thresholding */
    bin_pp = VImageData(mask);
    flt_pp = VImageData(zmap);
    for (i=0; i<VImageNPixels(zmap); i++) {
      u = *flt_pp++;
      if (adj == 3) {
	if (u < neg || u > pos) *bin_pp = 1;
      }
      if (u > neg && u < 0) *bin_pp = 0;
      if (u < pos && u > 0) *bin_pp = 0;
      bin_pp++;
    }
  }
  VPixel(mask,b0,r0,c0,VBit) = 1;



  /*
  ** get ROI volume that includes point c0,r0,b0;
  */
  volumes = VImage2Volumes(mask);
  vol0 = NULL;
  for (vol = volumes->first; vol != NULL; vol = vol->next) {
    if (VolumeInside(vol, (short) b0, (short) r0, (short) c0)) {
      vol0 = vol;
      break;
    }
  }
  if (vol0 == NULL) VError(" no voxels meeting the specifications were found");
  i = VolumeSize(vol0);
  if (i == 0) VError(" no voxels in ROI");
  if (i > 1000) VWarning(" more than 1000 voxels in ROI (%d voxels) !",i);



  /*
  ** set up data structs for interpolation
  */
  acc = gsl_interp_accel_alloc ();
  spline = gsl_spline_alloc (gsl_interp_akima,ntimesteps);
  xx = (double *) VCalloc(ntimesteps,sizeof(double));
  yy = (double *) VCalloc(ntimesteps,sizeof(double));

  nt = (int)(length/temporal_resolution + 0.5);
  sum1 = (double *) VCalloc(nt,sizeof(double));
  sum2 = (double *) VCalloc(nt,sizeof(double));
  tstep = temporal_resolution;



  /*
  ** average data across ROI
  */
  for (j=0; j<ntimesteps; j++) {

    sum = nx = 0;    
    for (k=0; k<VolumeNBuckets(vol0); k++) {
      for (tc = VFirstTrack(vol0,k); VTrackExists(tc); tc = VNextTrack(tc)) {
	b  = tc->band;	
	r  = tc->row;
	c0 = tc->col;
	c1 = c0 + tc->length;

	for (c=c0; c<c1; c++) {

	  if (nbeta == 0) {    /* use functional data */
	    if (VImageNRows(src[b]) < 2) continue;
	    if (VPixel(src[b],0,r,c,VShort) < minval) continue;
	    u = VPixel(src[b],j,r,c,VShort);
	  }

	  else {    /* use beta approx: Y = X*beta */
	    u = 0;
	    for (i=0; i<nbeta; i++) {
	      v = VGetPixel(src[i],b,r,c);
	      w = VGetPixel(designmatrix,0,j,i);
	      u += v*w;
	    }
	  }
	  
	  sum += u;
	  nx++;
	}
      }
    }
    u = 0;
    if (nx > 0) u = sum/nx;
    yy[j] = u;
  }



  /*
  ** percent signal change
  */
  if (psc) {
    sum = nx = 0;
    for (j=0; j<ntimesteps; j++) {
      sum += yy[j];
      nx++;
    }
    mean = sum/nx;
    for (j=0; j<ntimesteps; j++) {
      yy[j] = 100.0 * (yy[j] - mean) / mean;
    }
  }


  /* 
  ** prepare spline interpolation
  */
  for (j=0; j<ntimesteps; j++) {
    xi = (double)j;
    xx[j] = xi * tr;
  }
  gsl_spline_init (spline,xx,yy,ntimesteps);


  /*
  ** begin output
  */
  if (strlen(rep_filename) > 2) {
    fpe = fopen(rep_filename,"w");
    if (!fpe) VError(" error opening report file %s",rep_filename);
  }
  else
    fpe = stderr;

  fprintf(fpe,"#\n");
  fprintf(fpe,"# design file: %s\n",designfile);
  if (strlen(mask_filename) > 1)
    fprintf(fpe,"# mask file: %s\n",mask_filename);
  fprintf(fpe,"# number of voxels in ROI: %d\n",(int)VolumeSize(vol0));
  fprintf(fpe,"# voxel address: %.2f %.2f %.2f\n",addr.x,addr.y,addr.z);
  fprintf(fpe,"#\n");


  /*
  ** do spline interpolation and average across trials for each event
  */
  for (id=0; id<numcond; id++) {

    for (j=0; j<nt; j++) sum1[j] = sum2[j] = 0;

    nx = 0;
    for (j=0; j<ntrials; j++) {
      if (trial[j].id != cond[id]-1) continue;
      if (trial[j].onset + length >= experiment_duration) continue;
      nx++;

      k=0;
      for (t=0; t<length; t += tstep) {
	if (k >= nt) break;
	xi = trial[j].onset + t;
	yi = 0;
	if (xi < experiment_duration && xi >= 0)
	  yi = gsl_spline_eval (spline,xi,acc);

	sum1[k] += yi;
	sum2[k] += yi*yi;
	k++;
      }
    }

    /*
    ** output
    */
    fprintf(fpe,"#                                  \n");
    fprintf(fpe,"# experimental condition: %d\n",cond[id]);
    fprintf(fpe,"# number of trials in this condition: %.0f\n",nx);
    
    if (nx < 1) {
      VWarning("# no trials in condition %d",cond[id]);
      continue;
    }

    fprintf(fpe,"#----------------------------------\n");
    fprintf(fpe,"#     sec         mean      std-err\n");
    fprintf(fpe,"#..................................\n");


    k = 0;
    for (t=0; t<length; t += tstep) {
      mean  = sum1[k]/nx;
      sigma = sqrt((sum2[k] - nx * mean * mean) / (nx - 1.0));
      xerr  = sigma/sqrt(nx);
      fprintf(fpe," %10.5f  %10.5f  %10.5f\n",t,mean,xerr);
      k++;
    }
    fprintf(fpe,"\n\n");
  }
  fclose(fpe);

  exit(0);
}
