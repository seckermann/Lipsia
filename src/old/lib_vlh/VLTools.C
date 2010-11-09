/****************************************************************
 *
 * vlview:
 * 3D-visualization of anatomical and raw data
 * beginning of new better version of best vlview
 *
 * Copyright (C) 2002 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Heiko Mentzel, Jan. 2002, <mentzel@cns.mpg.de>
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
 *****************************************************************/

#include <qfont.h> 
#include <qimage.h>

#include "VLTools.h"

#define ABS(x) ((x) > 0 ? (x) : -(x))

extern "C" {
  extern void VPixel2Tal(float [3],float [3],float [3],int,int,int,float *,float *,float *);
  extern void VTal2Pixel(float *,float *,float *,float,float,float,int *,int *,int *);
}

extern VImage vlhTriLinearScale3d (VImage,VImage,int,int,int,double,double,double,int);
extern VImage VNNScale3d (VImage,VImage,int,int,int,double,double,double);

/* resize image (interpolate) */
void VLTools::vlhInterpolate( VImage & image, double nbands_mult, double nrows_mult, double ncols_mult, int type ) {  

  /* move image to tempfile */
  VImage tmpimg=VCopyImage(image,NULL,VAllBands);
  VFree(image);

  /* create and interpolate image */
  if (type==0) {
    image = vlhTriLinearScale3d(tmpimg,NULL,(int)rint(VImageNFrames(tmpimg)*nbands_mult),(int)rint(VImageNRows(tmpimg)*nrows_mult),(int)rint(VImageNColumns(tmpimg)*ncols_mult),nbands_mult,nrows_mult,ncols_mult,0);
  } else {
    image = VNNScale3d(tmpimg,NULL,(int)rint(VImageNFrames(tmpimg)*nbands_mult),(int)rint(VImageNRows(tmpimg)*nrows_mult),(int)rint(VImageNColumns(tmpimg)*ncols_mult),nbands_mult,nrows_mult,ncols_mult);
  }
}

/* resize image (inflate) */
void VLTools::vlhInflate( VImage & image, double nbands_mult, double nrows_mult, double ncols_mult) {

  /* move image to tempfile */
  VImage tmpimg=VCopyImage(image,NULL,VAllBands);
  VFree(image);

  /* create image */
  image = VCreateImage((int)rint(VImageNFrames(tmpimg)*nbands_mult),(int)rint(VImageNRows(tmpimg)*nrows_mult),(int)rint(VImageNColumns(tmpimg)*ncols_mult),VPixelRepn(tmpimg) );

  /* copy (scale) image */
  int maxbands= VImageNFrames(tmpimg);
  int maxrows = VImageNRows(tmpimg);
  int maxcols = VImageNColumns(tmpimg);
  for (int l=0; l<(int)rint(VImageNFrames(tmpimg)*nbands_mult); l++) {
    for (int j=0; j<(int)rint(VImageNRows(tmpimg)*nrows_mult); j++) {
      for (int k=0; k<(int)rint(VImageNColumns(tmpimg)*ncols_mult); k++) {
	int aa = (int)rint((double)l/nbands_mult);
	int bb = (int)rint((double)j/nrows_mult);
	int cc = (int)rint((double)k/ncols_mult);
	if (aa >= maxbands ) aa=maxbands-1;
	if (bb >= maxrows )  bb=maxrows-1;
	if (cc >= maxcols )  cc=maxcols-1;
	if (aa < 0 ) aa=0;
	if (bb < 0 ) bb=0;
	if (cc < 0 ) cc=0;
	if (VPixelRepn(tmpimg)==VUByteRepn)
	  VPixel( image, l, j, k, VUByte ) = VPixel( tmpimg, aa, bb, cc, VUByte );
	if (VPixelRepn(tmpimg)==VFloatRepn)
	  VPixel( image, l, j, k, VFloat ) = VPixel( tmpimg, aa, bb, cc, VFloat );
      }
    }
  }
}


prefs * VLTools::vlhContrast( prefs *pr, VImage Src ) {

  if (VPixelRepn(Src)!=VShortRepn) VError("Src must be short in vlhContrast.C");

  int nbands = VImageNBands(Src);
  int nrows  = VImageNRows(Src);
  int ncols  = VImageNColumns(Src);
  int npixels = nbands * nrows * ncols;
  int smin = (int)VRepnMinValue(VShortRepn);
  int smax = (int)VRepnMaxValue(VShortRepn);

  double percent1 = 0.01;  /* unten */
  double percent2 = 0.01;  /* oben  */

  int i=0, j=0;
  int dim = 2*smax + 1;
  float *histo = (float *) VMalloc(sizeof(float)*dim);
  for (j=0; j<dim; j++) histo[j] = 0;

  VShort *src_pp = (VShort *) VImageData(Src);
  for (i=0; i<(int)(npixels/4.0); i++) {
    j = *src_pp;
    src_pp += 4;
    //if (j!=0) {
    if (j < pr->background0 || j > pr->background1) {
      j -= smin;
      histo[j]++;
    }
  } 

  float sum = 0;
  for (j=0; j<dim; j++) sum += histo[j];
  for (j=0; j<dim; j++) histo[j] /= sum;

  sum  = 0;
  for (j=0; j<dim; j++) {
    sum += histo[j];
    if (sum > percent1) break;
  }
  int xmin = j+smin;

  sum = 0;
  for (j=dim-1; j>=0; j--) {
    sum += histo[j];
    if (sum > percent2) break;
  }
  int xmax = j+smin;

  //
  // Initialisierung von pr
  //
  pr->minwert  = (int)xmin;
  pr->maxwert  = (int)xmax;
  pr->minwert1 = (int)rint(xmin);
  pr->maxwert1 = (int)rint(xmax);
  pr->anaalpha = (float)255.0 / (float)(xmax - xmin);
  pr->anamean  = (float)xmin;
  //fprintf(stderr,"xmin: %d, xmax: %d, alpha: %f\n",pr->minwert, pr->maxwert,  pr->anaalpha);


  return pr;
}


void VLTools::interpolate( VImage & mysrc, VImage & myfnc, double ncols_mult, double nrows_mult, double nbands_mult, double scaleb, double scaler, double scalec, int whichimage, int intpolswitch ) {

  if (whichimage==1) {
    VError("Anatomical scaling. Please report this problem");
    /* INTERPOLATE ANATOMICAL */
    if (intpolswitch==1) 
      vlhInterpolate( mysrc, nbands_mult/scaleb, nrows_mult/scaler, ncols_mult/scalec, 0);
    else
      vlhInterpolate( mysrc, nbands_mult/scaleb, nrows_mult/scaler, ncols_mult/scalec, 1 );
  } else {
    /* INTERPOLATE ZMAP */
    if ( myfnc ) {
      if (intpolswitch==1) 
	vlhInterpolate( myfnc, scaleb/nbands_mult, scaler/nrows_mult, scalec/ncols_mult, 0 );
      else 
	vlhInterpolate( myfnc, scaleb/nbands_mult, scaler/nrows_mult, scalec/ncols_mult, 1 );
    }
  }
}

void VLTools::VPixel3Tal( double &x, double &y, double &z, double *extent, double *ca , double *cp, int files, double *pixelmult) {    

  int ifile=0;
  float u=0,v=0,w=0;
  
  float ca_[3];
  ca_[0]=(float)ca[0*files+ifile];
  ca_[1]=(float)ca[1*files+ifile];
  ca_[2]=(float)ca[2*files+ifile];
  
  float voxel_[3];
  voxel_[0]=(float)pixelmult[0];
  voxel_[1]=(float)pixelmult[1];
  voxel_[2]=(float)pixelmult[2];
  
  float extent_[3];
  extent_[0]=(float)extent[0*files+ifile];
  extent_[1]=(float)extent[1*files+ifile];
  extent_[2]=(float)extent[2*files+ifile];
  
  // Talairach aus VIA
  VPixel2Tal(ca_,voxel_,extent_,(int)rint(z),(int)rint(y),(int)rint(x),&u,&v,&w);
  x=(double)u; y=(double)v; z=(double)w;
}

void VLTools::VTal3Pixel( int &x, int &y, int &z, double *voxel, double *extent, double *ca, int files, double *pixelmult ) {

  int ifile=0;    
  int u=0, v=0, w=0;

  float ca_[3];
  ca_[0]=(float)ca[0*files+ifile];
  ca_[1]=(float)ca[1*files+ifile];
  ca_[2]=(float)ca[2*files+ifile];

  float voxel_[3];
  voxel_[0]=(float)pixelmult[0];
  voxel_[1]=(float)pixelmult[1];
  voxel_[2]=(float)pixelmult[2];
  
  float extent_[3];
  extent_[0]=(float)extent[0*files+ifile];
  extent_[1]=(float)extent[1*files+ifile];
  extent_[2]=(float)extent[2*files+ifile];

  // Talairach aus VIA
  VTal2Pixel(ca_,voxel_,extent_,(float)x,(float)y,(float)z,&u,&v,&w);
  z = u;
  y = v;
  x = w;	       

}




prefs * VLTools::GetRadiometricMax( VImage fnc, prefs *pr, int npixels ) 
{
  VDouble tiny=0.0001;
  VFloat *float_pp;
  double pos=0, neg=0, pscale=1.0, pmin=0, nscale=1.0,nmin=0;
  double maxp=pr->pmax, maxn=pr->nmax;
  double posmax=0, negmax=0, x=0;
  
  
  if (pr->verbose>=1) fprintf(stderr,"get radiometic maximum...");
	
  // get radiometric scaling factors //
  if (posmax < tiny || negmax > -tiny) {
    
    pr->pmax = 0;
    pmin = pos;
    pr->nmax = 0;
    nmin = neg;
    float_pp = (VFloat *) VPixelPtr(fnc,0,0,0);
    for (int i=0; i<npixels; i++) {
      if (*float_pp >= pos) {
	if (*float_pp > pr->pmax) pr->pmax = *float_pp;
	if (*float_pp < pmin) pmin = *float_pp;
      }
      if (*float_pp <= neg) {
	x = - (*float_pp);
	if (x > pr->nmax) pr->nmax = x;
	if (x < nmin) nmin = x;
      }
      float_pp++;
    }
    pscale = pr->pmax - pmin;
    if (pscale < 0.00001) pscale = 1.0;
    
    if (pr->nmax < 0) pr->nmax = -pr->nmax;
    if (nmin < 0) nmin = -nmin;
    nscale = pr->nmax - nmin;
    if (nscale < 0.00001) nscale = 1.0;
  }   
  if (posmax >= tiny) {
    pr->pmax = posmax;
    pmin = pos;
    pscale = pr->pmax - pmin;
    if (pscale < 0.00001) pscale = 1.0;
  }
  if (negmax <= -tiny) {
    pr->nmax = - (negmax);
    nmin = - (neg);
    nscale = pr->nmax - nmin;
    if (nscale < 0.00001) nscale = 1.0;
  }
  if (pr->pmax<maxp) pr->pmax=maxp;
  if (pr->nmax<maxn) pr->nmax=maxn;
  

  if (pr->verbose>=1) {
    fprintf(stderr,"ready\n");
    fprintf(stderr," pos: %6.2f  max: %6.2f\n",pmin,pr->pmax);
    fprintf(stderr," neg: %6.2f  max: %6.2f\n",-nmin,-pr->nmax);
  }

  return pr;
}
