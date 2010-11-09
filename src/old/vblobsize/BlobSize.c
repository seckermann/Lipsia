/****************************************************************
 *
 * vregionsize:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: BlobSize.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <via.h>


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))

extern void VPixel2Tal(float [3],float [3],float [3],int,int,int,float *,float *,float *);


void
VolumeZmapStats(VImage zmap,Volume v,float *mean,float *sigma,float *vmax,
		int *band,int *row,int *col)
{
  int i,b,r,c,c0,c1;
  VTrack t;
  float u,sum1,sum2,ave,nx;

  *mean = *sigma = *vmax = 0;
  *band = *row = *col = 0;

  nx = 0;
  sum1 = sum2 = nx = 0;
  for (i=0; i<VolumeNBuckets(v); i++) {
    for (t = VFirstTrack(v,i); VTrackExists(t); t = VNextTrack(t)) {
      b  = t->band;
      r  = t->row;
      c0 = t->col;
      c1 = c0 + t->length;
      for (c=c0; c<c1; c++) {
	u = VPixel(zmap,b,r,c,VFloat);
	if (ABS(u) > (*vmax)) {
	  (*vmax) = ABS(u);
	  (*band) = b;
	  (*row)  = r;
	  (*col)  = c;
	}
	sum1 += u;
	sum2 += u*u;
	nx++;
      }
    }
  }
  if (nx < 1) return;
  ave      = sum1/nx;
  (*mean)  = ave;

  if (nx > 1) {
    u = (sum2 - nx * ave * ave) / (nx - 1.0);
    (*sigma) = sqrt((double)u);
  }
}




VImage
VBlobSize(VImage zmap,VImage dest,VDouble pos,VDouble neg,
	  VShort minsize,VShort type,VBoolean clear_median,VString filename)
{
  VImage label_image=NULL,tmp=NULL;
  Volumes volumes=NULL;
  Volume v;
  VDouble xmin,xmax;
  int i,id=0,nl,iter,b,r,c,size,nbands,nrows,ncols,poslabels;
  float x0,x1,x2,x,y,z;
  float ave,sigma,vmax,xsize;
  float voxel[3],ca[3],extent[3];
  char hemi[2];
  VFloat *float_pp;
  VShort *short_pp;
  FILE *fp=NULL;
  VString str;
  int b0=0,r0=0,c0=0;
  int minvoxelsize=0;

  nrows  = VImageNRows (zmap);
  ncols  = VImageNColumns (zmap);
  nbands = VImageNBands (zmap);



  /*
  ** separate hemisphers
  */
  if (clear_median == TRUE) {
    c = ncols/2;
    for (b=0; b<nbands; b++)
      for (r=0; r<nrows; r++)
	VPixel(zmap,b,r,c,VFloat) = 0;
  }


  /*
  ** read image parameters
  */
  if (VGetAttr(VImageAttrList(zmap),"ca",NULL,VStringRepn,(VPointer) &str) != VAttrFound)
    VError(" attribute 'ca' not found");
  sscanf(str,"%f %f %f",&x0, &x1, &x2);
  ca[0] = x0;
  ca[1] = x1;
  ca[2] = x2;

  if (VGetAttr(VImageAttrList(zmap),"voxel",NULL,VStringRepn,(VPointer) &str) != VAttrFound)
    VError(" attribute 'voxel' not found");
  sscanf(str,"%f %f %f",&x0, &x1, &x2);
  voxel[0] = x0;
  voxel[1] = x1;
  voxel[2] = x2;

  minvoxelsize = VRint((float)minsize /(voxel[0] * voxel[1] * voxel[2]));
  if (minvoxelsize < 1 && minsize > 0) {
    VWarning(" parameter 'minsize' has no effect (0 voxels). Units are in mm^3 !");
  }

  if (VGetAttr(VImageAttrList(zmap),"extent",NULL,VStringRepn,(VPointer) &str) != VAttrFound)
    VError(" attribute 'extent' not found");
  sscanf(str,"%f %f %f",&x0, &x1, &x2);
  extent[0] = x0;
  extent[1] = x1;
  extent[2] = x2;


  /*
  ** create output image
  */
  dest = VSelectDestImage("VBlobSize",dest,nbands,nrows,ncols,VFloatRepn);
  if (! dest) VError(" err creating dest image");
  VCopyImageAttrs (zmap, dest);
  VFillImage(dest,VAllBands,0);


  /*
  ** write activations area statistics
  */
  if (strlen(filename) > 2) {
    fp = fopen(filename,"w");
    if (fp == NULL) VError("err opening %s",filename);
  }
  else
    fp = stderr;
  
  fprintf(fp,"\n   id  :   [mm]^3     mean   sigma     max      location  \n");
  fprintf(fp," ----------------------------------------------------------------\n");
  

  poslabels = 0;
  for (iter=0; iter < 2; iter++) {

    if (iter == 0) {   /* positive activations */
      xmin = pos;
      xmax = VRepnMaxValue(VFloatRepn);
    }
    else {            /* negative activations */
      xmin = VRepnMinValue(VFloatRepn);
      xmax = neg;
    }

    tmp = VBinarizeImage (zmap,tmp,(VDouble) xmin,(VDouble) xmax);
    label_image = VLabelImage3d(tmp,label_image,(int)26,VShortRepn,&nl);
    if (nl < 1) continue;

    if (minvoxelsize > 0) {
      tmp = VDeleteSmall (label_image,tmp,minvoxelsize);
      label_image = VLabelImage3d(tmp,label_image,(int)26,VShortRepn,&nl);
      if (nl < 1) continue;
    }

    volumes = VImage2Volumes(label_image);

    float_pp = (VFloat *) VImageData(dest);
    short_pp = (VShort *) VImageData(label_image);
    for (i=0; i<VImageNPixels(dest); i++) {
      if (*short_pp > 0 && iter == 0) *float_pp =  (float) (*short_pp);
      if (*short_pp > 0 && iter == 1) *float_pp =  (float) ((*short_pp) + poslabels);
      float_pp++;
      short_pp++;
    }

    x = y = z = b = r = c = 0;
    hemi[1]= '\0';

    for (v = volumes->first; v != NULL; v = v->next) {
      size = VolumeSize(v);
      xsize = size * voxel[0] * voxel[1] * voxel[2];
      if (xsize < minsize || size == 0) continue;

      id = v->label + poslabels;

      VolumeZmapStats(zmap,v,&ave,&sigma,&vmax,&b,&r,&c);
      if (iter > 0) vmax = -vmax;

      hemi[0]='R';
      if (c < ncols/2) hemi[0]='L';
    
      switch (type) {
      case 0:
	c0 = c;
	r0 = r;
	b0 = b;
	break;

      case 1:
	c0 = VRint((float)c * voxel[2]);
	r0 = VRint((float)r * voxel[1]);
	b0 = VRint((float)b * voxel[0]);
	break;

      case 2:
	VPixel2Tal(ca,voxel,extent,b,r,c,&x,&y,&z);
	c0 = VRint(x);
	r0 = VRint(y);
	b0 = VRint(z);
	break;

      case 3:
	VPixel2MNI(ca,voxel,b,r,c,&x,&y,&z);
	c0 = VRint(x);
	r0 = VRint(y);
	b0 = VRint(z);
	break;

      default:
	VError("illegal coord type");
      }

      fprintf(fp," %4d :   %6.0f    %6.2f  %6.2f  %7.2f   %2s (%4d %4d %4d)\n",
	      id,xsize,ave,sigma,vmax,hemi,c0,r0,b0);

    }
    if (iter == 0) poslabels = id;
  }
  fprintf(fp," ----------------------------------------------------------------\n");
  if (fp != stderr) fclose(fp);

  return dest;
}

