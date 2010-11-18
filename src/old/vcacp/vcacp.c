/****************************************************************
 *
 * vcacp:
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Authors: Heiko Temming, Gabriele Lohmann, <lipsia@cbs.mpg.de>
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
 * $Id: vcacp.c 3615 2009-07-13 22:22:34Z proeger $
 *
 *****************************************************************/

/*  From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/VImage.h"
#include "viaio/mu.h"
#include "viaio/option.h"

/* From the GSL library */
#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_sort.h>

/*  From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/*
** fast implementation of rounding
*/
#define VRintPos(x) ((int)((x) + 0.5))
#define VRintNeg(x) ((int)((x) - 0.5))
#define VRint(x)  ((x) >= 0 ? VRintPos(x) : VRintNeg(x))

#define VFloor(x) ((x) >= 0 ? (int)((x+0.000001)) : (int)((x) - 0.999999))

#define VCeilPos(x) (VRint((x)) == (x) ? (x) : (int)(x+0.99999))
#define VCeilNeg(x) (VRint((x)) == (x) ? (x) : (int)(x+0.00001))
#define VCeil(x) ((x) >= 0 ? VCeilPos(x) : VCeilNeg(x))



extern VImage VGaussianConv (VImage,VImage,VBand,double,int);


/* global definitions */
typedef struct pointStruct{
        float x;
        float y;
        float z;
        float ro;
        float pi;
        float ya;
        float co;
} Point;

typedef struct dpointStruct{
  double x;
  double y;
  double z;
} dpoint;

typedef struct ipointStruct{
  int x;
  int y;
  int z;
} ipoint;

typedef struct msppointStruct{
  double corr;
  double roh;
  double pnt;
} mspPoint;

VImage src_m,ca_m,cp_m,ca_rot,cp_rot;
int sbands,srows,scols,cabands,carows,cacols,cpbands,cprows,cpcols;
VRepnKind s_repn,ca_repn,cp_repn;
VAttrList dest_list;
int ncom;
float *pcom,*xicom,(*nrfunc)(float[]);
extern char * getLipsiaVersion();

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))
#define GRD (180/3.1415931)



/*
** linear correlation
*/
double
pearson(VUByte *xx, VUByte *yy, int anz)
{
  int i;
  double sum1,sum2,sum3,ave1,ave2,nx;
  double a,b,r;
  double sqrt(double);
  double tiny=1.0e-20;

  r = 0;
  sum1 = sum2 = nx = 0;
  for (i=0; i<anz; i+=1) {
    if (xx[i] == 0 || yy[i] == 0) continue;
    sum1 += (double)xx[i];
    sum2 += (double)yy[i];
    nx++;
  }
  ave1 = sum1 / nx;
  ave2 = sum2 / nx;

  sum1 = sum2 = sum3 = 0;
  for (i=0; i<anz; i+=1) {
    if (xx[i] == 0 || yy[i] == 0) continue;
    a = (double) xx[i] - ave1;
    b = (double) yy[i] - ave2;
    sum1 += a * b;
    sum2 += a * a;
    sum3 += b * b;
  }
  if (ABS(sum2) < tiny || ABS(sum3) < tiny)
    r = 0;
  else
    r = sum1 / sqrt(sum2 * sum3);

  return r;
}
/*
** 3D-Image resampling
*/
VImage VReSample3d(VImage src, VImage dest, int dest_nbands, int dest_nrows, int dest_ncolumns, 
		   double transmatrix[3][3], double shift[3], double fix[3])
{
  int src_nrows, src_ncols, src_nbands;
  VRepnKind src_repn;
  int b,r,c;
  double bx,rx,cx;
  double ainv[3][3],detA;
  int i,j;
  double bp,rp,cp,x,y,z;
  int north,south,left,right,top,bottom;
  double p1,p2,p3,p4,p5,p6,p7,p8;
  double w1,w2,w3,w4,w5,w6,w7,w8;
  double interpolation;

  /* Extract data from source image */
  src_nrows    = VImageNRows(src);
  src_ncols    = VImageNColumns(src);
  src_nbands   = VImageNBands(src);
  src_repn     = VPixelRepn(src);

  if (dest == NULL)
    dest = VCreateImage(dest_nbands,dest_nrows,dest_ncolumns,src_repn);
    VImageAttrList(dest) = VCopyAttrList(VImageAttrList(src));
  if (! dest)return NULL;


  /* get inverse of transformation matrix */
  ainv[0][0] =  transmatrix[1][1]*transmatrix[2][2] - transmatrix[1][2]*transmatrix[2][1];
  ainv[1][0] = -transmatrix[1][0]*transmatrix[2][2] + transmatrix[1][2]*transmatrix[2][0];
  ainv[2][0] =  transmatrix[1][0]*transmatrix[2][1] - transmatrix[1][1]*transmatrix[2][0];

  ainv[0][1] = -transmatrix[0][1]*transmatrix[2][2] + transmatrix[0][2]*transmatrix[2][1];
  ainv[1][1] =  transmatrix[0][0]*transmatrix[2][2] - transmatrix[0][2]*transmatrix[2][0];
  ainv[2][1] = -transmatrix[0][0]*transmatrix[2][1] + transmatrix[0][1]*transmatrix[2][0];

  ainv[0][2] =  transmatrix[0][1]*transmatrix[1][2] - transmatrix[0][2]*transmatrix[1][1];
  ainv[1][2] = -transmatrix[0][0]*transmatrix[1][2] + transmatrix[0][2]*transmatrix[1][0];
  ainv[2][2] =  transmatrix[0][0]*transmatrix[1][1] - transmatrix[0][1]*transmatrix[1][0];


  /*  determinant */
  detA = transmatrix[0][0]*ainv[0][0] + transmatrix[0][1]*ainv[1][0] + transmatrix[0][2]*ainv[2][0];
  if (detA == 0) VError(" transformation matrix matrix is singular");

  for (i=0; i<3; i++) {
    for (j=0; j<3; j++) {
      ainv[i][j] /= detA;
    }
  }


  /* Determines the value of each pixel in the destination image: */
  for (b=0; b<src_nbands; b++) {
    for (r=0; r<src_nrows; r++) {
      for (c=0; c<src_ncols; c++) {

	bx = (double) b - fix[0];
	rx = (double) r - fix[1];
	cx = (double) c - fix[2];

	bp = ainv[0][0] * bx + ainv[0][1] * rx + ainv[0][2] * cx;
	rp = ainv[1][0] * bx + ainv[1][1] * rx + ainv[1][2] * cx;
	cp = ainv[2][0] * bx + ainv[2][1] * rx + ainv[2][2] * cx;
	bp += shift[0];
	rp += shift[1];
	cp += shift[2];

	if (bp <= -1.0 || bp >= src_nbands) continue;
	if (rp <= -1.0 || rp >= src_nrows) continue;
	if (cp <= -1.0 || cp >= src_ncols) continue;
	
	bottom = VCeil(rp);
	top    = VFloor(rp);
	right  = VCeil(cp);
	left   = VFloor(cp);
	south  = VCeil(bp);
	north  = VFloor(bp);

	if (north >= src_nbands) north = -1;
	if (top   >= src_nrows) top = -1;
	if (left  >= src_ncols) left = -1;

	z = VCeil(bp) - bp;
	x = VCeil(cp) - cp;
	y = VCeil(rp) - rp;

	w1 = x * y * z;
	w2 = y * (1.0 - x) * z;
	w3 = (1.0 - y) * x * z;
	w4 = (1.0 - x) * (1.0 - y) * z;

	w5 = x * y * (1.0 - z);
	w6 = y * (1.0 - x) * (1.0 - z);
	w7 = (1.0 - y) * x * (1.0 - z);
	w8 = (1.0 - x) * (1.0 - y) * (1.0 - z);

	p1=p2=p3=p4=p5=p6=p7=p8=0;
	if (north>=0 && top>=0 && left>=0)
	  p1 = VPixel(src,north,top,left,VUByte);
	else w1 = 0;

 	if (north>=0 && top>=0 && right<src_ncols)
	  p2 = VPixel(src,north,top,right,VUByte);
	else w2 = 0;

 	if (north>=0 && bottom<src_nrows && left>=0)
	  p3 = VPixel(src,north,bottom,left,VUByte);
	else w3 = 0;

 	if (north>=0 && bottom<src_nrows && right<src_ncols)
	  p4 = VPixel(src,north,bottom,right,VUByte);
	else w4 = 0;

 	if (south<src_nbands && top>=0 && left>=0)
	  p5 = VPixel(src,south,top,left,VUByte);
	else w5 = 0;

 	if (south<src_nbands && top>=0 && right<src_ncols)
	  p6 = VPixel(src,south,top,right,VUByte);
	else w6 = 0;

 	if (south<src_nbands && bottom<src_nrows && left>=0)
	  p7 = VPixel(src,south,bottom,left,VUByte);
	else w7 = 0;

 	if (south<src_nbands && bottom<src_nrows && right<src_ncols)
	  p8 = VPixel(src,south,bottom,right,VUByte);
	else w8 = 0;

	interpolation = w1*p1 + w2*p2 + w3*p3 +
	  w4*p4 + w5*p5 + w6*p6 + w7*p7 + w8*p8;
        VPixel(dest,b,r,c,VUByte) = VRint (interpolation);
      };
    };
  };
return dest;
}


/*
** Count back to original image
*/
dpoint computePoint(dpoint point, ipoint fixpoint, double *angle)
{
  double rot[3][3],ainv[3][3],detA,b,r,c;
  float roll,pitch,yaw,cor,cop,coy,sir,sip,siy;
  int i,j;

  roll     = (float)angle[0];
  pitch    = (float)angle[1];
  yaw      = (float)angle[2];

  cor = cos(roll);
  cop = cos(pitch);
  coy = cos(yaw);
  sir = sin(roll);
  sip = sin(pitch);
  siy = sin(yaw);
  rot[0][0] = cor * coy + sir * sip * siy;
  rot[0][1] = sir * cop;
  rot[0][2] = sir * sip * coy - siy * cor;
  rot[1][0] = cor * sip * siy - sir * coy;
  rot[1][1] = cor * cop;
  rot[1][2] = sir * siy + coy * cor * sip;
  rot[2][0] = cop * siy;
  rot[2][1] = - sip;
  rot[2][2] = cop * coy;


  /*  get inverse of transformation matrix : */
  ainv[0][0] =  rot[1][1]*rot[2][2] - rot[1][2]*rot[2][1];
  ainv[1][0] = -rot[1][0]*rot[2][2] + rot[1][2]*rot[2][0];
  ainv[2][0] =  rot[1][0]*rot[2][1] - rot[1][1]*rot[2][0];

  ainv[0][1] = -rot[0][1]*rot[2][2] + rot[0][2]*rot[2][1];
  ainv[1][1] =  rot[0][0]*rot[2][2] - rot[0][2]*rot[2][0];
  ainv[2][1] = -rot[0][0]*rot[2][1] + rot[0][1]*rot[2][0];

  ainv[0][2] =  rot[0][1]*rot[1][2] - rot[0][2]*rot[1][1];
  ainv[1][2] = -rot[0][0]*rot[1][2] + rot[0][2]*rot[1][0];
  ainv[2][2] =  rot[0][0]*rot[1][1] - rot[0][1]*rot[1][0];


  /*  determinant */
  detA = rot[0][0]*ainv[0][0] + rot[0][1]*ainv[1][0] + rot[0][2]*ainv[2][0];
  if (detA == 0) VError(" rotation matrix is singular");

  for (i=0; i<3; i++) {
    for (j=0; j<3; j++) {
      ainv[i][j] /= detA;
    }
  }

  b=(double)point.z-(double)fixpoint.z;
  r=(double)point.y-(double)fixpoint.y;
  c=(double)point.x-(double)fixpoint.x;

  point.z=(double)ainv[0][0]*b+(double)ainv[0][1]*r+(double)ainv[0][2]*c+(double)fixpoint.z;
  point.y=(double)ainv[1][0]*b+(double)ainv[1][1]*r+(double)ainv[1][2]*c+(double)fixpoint.y;
  point.x=(double)ainv[2][0]*b+(double)ainv[2][1]*r+(double)ainv[2][2]*c+(double)fixpoint.x;

  return point;
}
/*################################################################################################
**read in corrarray
*/
void
CorrArray(VUByte *xx, int x0, int y0, int z0 ,int flag, int rot)
{

  int i,x,y,z,zs,ys,xs,d;
  int tnx,tny,tnz;

  d=2;

  switch (flag){
  case 0: /* src_arr_ca */
            tnz = (int)floor((double)cabands/2);
            tny = (int)floor((double)carows/2);
            tnx = (int)floor((double)cacols/2);
            xs  = (int)x0;
            ys  = (int)y0;
            zs  = (int)z0;
            i=0;
            for (z=(zs-tnz+d);z<=(zs+tnz-d);z++){
              for (y=(ys-tny+d);y<=(ys+tny-d);y++){
                for (x=(xs-tnx+d);x<=(xs+tnx-d);x++){
                  xx[i]= VPixel(src_m,z,y,x,VUByte);
                  i++;
                }
              }
            }
            break;
  case 1: /* src_arr_cp */
            tnz = (int)floor((double)cpbands/2);
            tny = (int)floor((double)cprows/2);
            tnx = (int)floor((double)cpcols/2);
            xs  = (int)x0;
            ys  = (int)y0;
            zs  = (int)z0;
            i=0;
            for (z=(zs-tnz+d);z<=(zs+tnz-d);z++){
              for (y=(ys-tny+d);y<=(ys+tny-d);y++){
                for (x=(xs-tnx+d);x<=(xs+tnx-d);x++){
                  xx[i]= VPixel(src_m,z,y,x,VUByte);
                  i++;
                }
              }
            }
            break;
  case 2: /* ca_arr */
            i=0;
            for (z=0+d;z<cabands-d;z++){
              for (y=0+d;y<carows-d;y++){
                for (x=0+d;x<cacols-d;x++){
                  if (rot==0)xx[i]= VPixel(ca_m,z,y,x,VUByte);
                  else       xx[i]= VPixel(ca_rot,z,y,x,VUByte);
                  i++;
                }
              }
            }
            break;
  case 3: /* cp_arr */
            i=0;
            for (z=0+d;z<cpbands-d;z++){
              for (y=0+d;y<cprows-d;y++){
                for (x=0+d;x<cpcols-d;x++){
                  if (rot==0)xx[i]= VPixel(cp_m,z,y,x,VUByte);
                  else       xx[i]= VPixel(cp_rot,z,y,x,VUByte);
                  i++;
                }
              }
            }
            break;
    default: ;
  }
}


/*
**calculation of correlation-maximum (CA/CP-template)
*/
double
match2(ipoint ca, ipoint cp, double angle)
{
  int anzca,anzcp,i;
  VUByte *src_arr_ca,*src_arr_cp,*ca_arr,*cp_arr;
  float roll,pitch,yaw,cor,cop,coy,sir,sip,siy;
  double corr;
  double rca,rcp;
  double fixca[3],shiftca[3],fixcp[3],shiftcp[3],rot[3][3];

  anzca      = cabands * carows *cacols;
  ca_arr     = (VUByte *) VMalloc(anzca*sizeof(VUByte));
  src_arr_ca = (VUByte *) VMalloc(anzca*sizeof(VUByte));
  for (i=0;i<anzca;i++) ca_arr[i]=src_arr_ca[i]=0;

  anzcp      = cpbands * cprows *cpcols;
  cp_arr     = (VUByte *) VMalloc(anzcp*sizeof(VUByte));
  src_arr_cp = (VUByte *) VMalloc(anzcp*sizeof(VUByte));
  for (i=0;i<anzcp;i++) cp_arr[i]=src_arr_cp[i]=0;

  roll   = angle;
  pitch  = 0;
  yaw    = 0;

  cor = cos(roll);
  cop = cos(pitch);
  coy = cos(yaw);
  sir = sin(roll);
  sip = sin(pitch);
  siy = sin(yaw);
  rot[0][0] = cor * coy + sir * sip * siy;
  rot[0][1] = sir * cop;
  rot[0][2] = sir * sip * coy - siy * cor;
  rot[1][0] = cor * sip * siy - sir * coy;
  rot[1][1] = cor * cop;
  rot[1][2] = sir * siy + coy * cor * sip;
  rot[2][0] = cop * siy;
  rot[2][1] = - sip;
  rot[2][2] = cop * coy;

  fixca[0] = shiftca[0] = floor((double)cacols/2);
  fixca[1] = shiftca[1] = floor((double)carows/2);
  fixca[2] = shiftca[2] = floor((double)cabands/2);
  fixcp[0] = shiftcp[0] = floor((double)cpcols/2);
  fixcp[1] = shiftcp[1] = floor((double)cprows/2);
  fixcp[2] = shiftcp[2] = floor((double)cpbands/2);


  ca_rot = VReSample3d(ca_m,NULL,cabands,carows,cacols,rot,shiftca,fixca);
  CorrArray(src_arr_ca,ca.x,ca.y,ca.z,0,2);
  CorrArray(ca_arr,0,0,0,2,2);
  rca = pearson(src_arr_ca,ca_arr,anzca);

  cp_rot = VReSample3d(cp_m,NULL,cpbands,cprows,cpcols,rot,shiftcp,fixcp);
  CorrArray(src_arr_cp,cp.x,cp.y,cp.z,1,2);
  CorrArray(cp_arr,0,0,0,3,2);
  rcp = pearson(src_arr_cp,cp_arr,anzcp);

  if (rca>0 && rcp>0) corr = rca*rcp;
  else                corr = -1;

  return corr;
}


/*
** Single match CP
*/
double
matchCP(const gsl_vector *p, void *params)
 {
   int anzcp,i;
   VUByte *src_arr_cp,*cp_arr;
   double roll,pitch,yaw,cor,cop,coy,sir,sip,siy;
   double rcp;
   double fix[3],shift[3],rot[3][3];
   dpoint cp;

   anzcp      = cpbands * cprows * cpcols;
   cp_arr     = (VUByte *) VMalloc(anzcp*sizeof(VUByte));
   src_arr_cp = (VUByte *) VMalloc(anzcp*sizeof(VUByte));
   for (i=0;i<anzcp;i++) cp_arr[i]=src_arr_cp[i]=0;

   cp.x   = gsl_vector_get(p,0);
   cp.y   = gsl_vector_get(p,1);
   cp.z   = gsl_vector_get(p,2);
   roll   = gsl_vector_get(p,3);
   pitch  = gsl_vector_get(p,4);
   yaw    = gsl_vector_get(p,5);

   cor = cos(roll);
   cop = cos(pitch);
   coy = cos(yaw);
   sir = sin(roll);
   sip = sin(pitch);
   siy = sin(yaw);
   rot[0][0] = cor * coy + sir * sip * siy;
   rot[0][1] = sir * cop;
   rot[0][2] = sir * sip * coy - siy * cor;
   rot[1][0] = cor * sip * siy - sir * coy;
   rot[1][1] = cor * cop;
   rot[1][2] = sir * siy + coy * cor * sip;
   rot[2][0] = cop * siy;
   rot[2][1] = - sip;
   rot[2][2] = cop * coy;

   fix[0]   = fabs(cpcols/2);
   fix[1]   = fabs(cprows/2);
   fix[2]   = fabs(cpbands/2);
   shift[0] = cp.x - floor(cp.x) + fix[0];
   shift[1] = cp.y - floor(cp.y) + fix[1];
   shift[2] = cp.z - floor(cp.z) + fix[2];

   cp_rot = VReSample3d(cp_m,NULL,cpbands,cprows,cpcols,rot,shift,fix);
   CorrArray(src_arr_cp,floor(cp.x),floor(cp.y),floor(cp.z),1,2);
   CorrArray(cp_arr,0,0,0,3,2);
   rcp = (-1)*pearson(src_arr_cp,cp_arr,anzcp);

  return rcp;
}


/*
** Single match CA
*/
double
matchCA(const gsl_vector *p, void *params)
 {
   int anzca,i;
   VUByte *src_arr_ca,*ca_arr;
   double roll,pitch,yaw,cor,cop,coy,sir,sip,siy;
   double rca;
   double fix[3],shift[3],rot[3][3];
   dpoint ca;

   anzca      = cabands * carows *cacols;
   ca_arr     = (VUByte *) VMalloc(anzca*sizeof(VUByte));
   src_arr_ca = (VUByte *) VMalloc(anzca*sizeof(VUByte));
   for (i=0;i<anzca;i++) ca_arr[i]=src_arr_ca[i]=0;

   ca.x   = gsl_vector_get(p,0);
   ca.y   = gsl_vector_get(p,1);
   ca.z   = gsl_vector_get(p,2);
   roll   = gsl_vector_get(p,3);
   pitch  = gsl_vector_get(p,4);
   yaw    = gsl_vector_get(p,5);

   cor = cos(roll);
   cop = cos(pitch);
   coy = cos(yaw);
   sir = sin(roll);
   sip = sin(pitch);
   siy = sin(yaw);
   rot[0][0] = cor * coy + sir * sip * siy;
   rot[0][1] = sir * cop;
   rot[0][2] = sir * sip * coy - siy * cor;
   rot[1][0] = cor * sip * siy - sir * coy;
   rot[1][1] = cor * cop;
   rot[1][2] = sir * siy + coy * cor * sip;
   rot[2][0] = cop * siy;
   rot[2][1] = - sip;
   rot[2][2] = cop * coy;

   fix[0]   = fabs(cacols/2);
   fix[1]   = fabs(carows/2);
   fix[2]   = fabs(cabands/2);
   shift[0] = ca.x - floor(ca.x) + fix[0];
   shift[1] = ca.y - floor(ca.y) + fix[1];
   shift[2] = ca.z - floor(ca.z) + fix[2];

   ca_rot = VReSample3d(ca_m,NULL,cabands,carows,cacols,rot,shift,fix);
   CorrArray(src_arr_ca,floor(ca.x),floor(ca.y),floor(ca.z),0,2);
   CorrArray(ca_arr,0,0,0,2,2);
   rca = (-1)*pearson(src_arr_ca,ca_arr,anzca);

  return rca;
}


/*
** Search point exactly
*/
void SearchPoint(ipoint ca, ipoint cp, double roll, gsl_vector *p)
{
  int n,x,y,z,dx,dy,dz;
  int iter,maxiter=100;
  dpoint dca,dcp;
  double bestcorr,corr;

  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex;
  gsl_multimin_fminimizer *s = NULL;
  gsl_vector *stepsizes=NULL;
  gsl_multimin_function minex_func;
  int status;
  double size;

  /*  set up optimization */
  n = 6;
  minex_func.f = &matchCA;
  minex_func.n = n;
  minex_func.params = NULL;
  s = gsl_multimin_fminimizer_alloc (T, n);
  stepsizes = gsl_vector_calloc(n);


  /*
  ** CA
  */
  dx=1;
  dy=dz=10;
  bestcorr=corr=0;
  for (z=ca.z-dz;z<=ca.z+dz;z++){
    for (y=ca.y-dy;y<=ca.y+dy;y++){
      for (x=ca.x-dx;x<=ca.x+dx;x++){

	gsl_vector_set(p,0,x);
	gsl_vector_set(p,1,y);
	gsl_vector_set(p,2,z);
	gsl_vector_set(p,3,roll);
	gsl_vector_set(p,4,0);
	gsl_vector_set(p,5,0);

        corr = -matchCA(p,NULL);
        if(corr>bestcorr){
          bestcorr=corr;
          dca.x = gsl_vector_get(p,0);
          dca.y = gsl_vector_get(p,1);
          dca.z = gsl_vector_get(p,2);
        }
      }
    }
  }
  

  /*  do optimization */
  gsl_vector_set(p,0,dca.x);
  gsl_vector_set(p,1,dca.y);
  gsl_vector_set(p,2,dca.z);
  gsl_vector_set(p,3,roll);
  gsl_vector_set(p,4,0);
  gsl_vector_set(p,5,0);

  gsl_vector_set_all (stepsizes,1.0);
  gsl_multimin_fminimizer_set (s, &minex_func, p, stepsizes);

  for (iter=0; iter<maxiter; iter++) {
    status = gsl_multimin_fminimizer_iterate(s);
    /* fprintf(stderr," CA iter %3d:  %f\n",iter,s->fval); */
    if (status) break;
    size = gsl_multimin_fminimizer_size (s);
    status = gsl_multimin_test_size (size, 1e-6);
    if (status == GSL_SUCCESS) break;
  }
  dca.x = gsl_vector_get(p,0);
  dca.y = gsl_vector_get(p,1);
  dca.z = gsl_vector_get(p,2);


  /*  
  ** CP 
  */
  n = 6;
  minex_func.f = &matchCP;
  minex_func.n = n;
  minex_func.params = NULL;

  bestcorr=corr=0;
  for (z=cp.z-dz;z<=cp.z+dz;z++){
    for (y=cp.y-dy;y<=cp.y+dy;y++){
      for (x=cp.x-dx;x<=cp.x+dx;x++){

	gsl_vector_set(p,0,x);
	gsl_vector_set(p,1,y);
	gsl_vector_set(p,2,z);
	gsl_vector_set(p,3,roll);
	gsl_vector_set(p,4,0);
	gsl_vector_set(p,5,0);

        corr = -matchCP(p,NULL);
        if(corr>bestcorr){
          bestcorr=corr;
          dcp.x = gsl_vector_get(p,0);
          dcp.y = gsl_vector_get(p,1);
          dcp.z = gsl_vector_get(p,2);
        }
      }
    }
  }

  gsl_vector_set(p,0,dcp.x);
  gsl_vector_set(p,1,dcp.y);
  gsl_vector_set(p,2,dcp.z);
  gsl_vector_set(p,3,roll);
  gsl_vector_set(p,4,0);
  gsl_vector_set(p,5,0);

  /* do optimization */
  gsl_vector_set_all (stepsizes,1.0);
  gsl_multimin_fminimizer_set (s, &minex_func, p, stepsizes);

  for (iter=0; iter<maxiter; iter++) {
    status = gsl_multimin_fminimizer_iterate(s);

    /* fprintf(stderr," CP iter %3d:  %f\n",iter,s->fval); */

    if (status) break;
    size = gsl_multimin_fminimizer_size (s);
    status = gsl_multimin_test_size (size, 1e-6);
    if (status == GSL_SUCCESS) break;
  }

  dcp.x = gsl_vector_get(p,0);
  dcp.y = gsl_vector_get(p,1);
  dcp.z = gsl_vector_get(p,2);

  
  gsl_vector_set(p,0,dca.x);
  gsl_vector_set(p,1,dca.y);
  gsl_vector_set(p,2,dca.z);
  gsl_vector_set(p,3,dcp.x);
  gsl_vector_set(p,4,dcp.y);
  gsl_vector_set(p,5,dcp.z);

}
/*################################################################################################
**Search region of interest
*/
VImage
SearchROI(ipoint mf, int anz, VImage src, gsl_vector *p)
{
  int i,j,x,y,z,k,dx,dy,dz,anzca,anzcp,m,n,hi,bds=0;
  float bcorr,corr,*bestcorr,hcorr;
  double dmin,dmax,r,rca,rcp,edca,edcp,edoc;
  VImage ca_mark,cp_mark,ca_corr,cp_corr,vert;
  int zaehler;
  double a,maxa,mina,angle;
  ipoint ca,cp,oc,*bca,*bcp,*boc;
  VUByte *src_arr_ca,*src_arr_cp,*ca_arr,*cp_arr;

  n=anz-1;
  
  bestcorr = (float *) VCalloc(n+1,sizeof(float));

  bca   = (ipoint *) malloc(sizeof(int) * anz * 3);
  bcp   = (ipoint *) malloc(sizeof(int) * anz * 3);
  boc   = (ipoint *) malloc(sizeof(int) * anz * 3);

  ca_mark = VCreateImage(sbands,srows,scols,s_repn);
  cp_mark = VCreateImage(sbands,srows,scols,s_repn);
  vert    = VCreateImage(sbands,srows,scols,VFloatRepn);
  VImageAttrList(vert) = VCopyAttrList(VImageAttrList(src));
  ca_corr = VCreateImage(sbands,srows,scols,VFloatRepn);
  cp_corr = VCreateImage(sbands,srows,scols,VFloatRepn);

  anzca      = cabands * carows *cacols;
  ca_arr     = (VUByte *) VMalloc(anzca*sizeof(VUByte));
  src_arr_ca = (VUByte *) VMalloc(anzca*sizeof(VUByte));
  for (i=0;i<anzca;i++) ca_arr[i]=src_arr_ca[i]=0;

  anzcp      = cpbands * cprows *cpcols;
  cp_arr     = (VUByte *) VMalloc(anzcp*sizeof(VUByte));
  src_arr_cp = (VUByte *) VMalloc(anzcp*sizeof(VUByte));
  for (i=0;i<anzcp;i++) cp_arr[i]=src_arr_cp[i]=0;

  dmin  = 11;
  dmax  = 17;
  dx    = 10;
  dy    = 20;
  dz    = 20;
  maxa  = 60/GRD;
  mina  = -30/GRD;

  for (i=0;i<anz;i++)bestcorr[i]=0;
  corr=zaehler=0;
  for (z=mf.z-dz;z<=mf.z+dz;z++){
    for (y=mf.y-dy;y<=mf.y+dy;y++){
      for (x=mf.x-dx;x<=mf.x+dx;x++){
        for (i=0;i<=dmax;i++){
          for (j=-dmax;j<=dmax;j++){
          for (k=-1;k<1;k++){
            ca.x = x;
            ca.y = y-i;
            ca.z = z-j;
            cp.x = x;
            cp.y = y+i+k;
            cp.z = z+j+k;
            oc.x = x;
            oc.y = y;
            oc.z = z;
            r = sqrt(SQR(oc.y-ca.y)+SQR(oc.z-ca.z));
            a = atan(((double)cp.z-ca.z)/((double)cp.y-ca.y));
            if (a<mina || a>maxa)continue;
            if (r>=dmin && r<=dmax){
              zaehler++;
              if (VPixel(ca_mark,ca.z,ca.y,ca.x,VUByte)<255){
                VPixel(ca_mark,ca.z,ca.y,ca.x,VUByte)=255;
                CorrArray(src_arr_ca,ca.x,ca.y,ca.z,0,0);
                CorrArray(ca_arr,0,0,0,2,0);
                rca = pearson(src_arr_ca,ca_arr,anzca);
                VPixel(ca_corr,ca.z,ca.y,ca.x,VFloat)=(float)rca;
              }
              else  rca = (double)VPixel(ca_corr,ca.z,ca.y,ca.x,VFloat);
              if (VPixel(cp_mark,cp.z,cp.y,cp.x,VUByte)<255){
                VPixel(cp_mark,cp.z,cp.y,cp.x,VUByte)=255;
                CorrArray(src_arr_cp,cp.x,cp.y,cp.z,1,0);
                CorrArray(cp_arr,0,0,0,3,0);
                rcp = pearson(src_arr_cp,cp_arr,anzcp);
                VPixel(cp_corr,cp.z,cp.y,cp.x,VFloat)=(float)rcp;
              }
              else  rcp = (double)VPixel(cp_corr,cp.z,cp.y,cp.x,VFloat);
            }
            else continue;

            if (rca>0 && rcp>0) corr = rca*rcp;
            else                corr = -1;

            if(corr>bestcorr[0]){
              bestcorr[0]=corr;
              bca[0].x = ca.x;
              bca[0].y = ca.y;
              bca[0].z = ca.z;
              bcp[0].x = cp.x;
              bcp[0].y = cp.y;
              bcp[0].z = cp.z;
              boc[0].x = oc.x;
              boc[0].y = oc.y;
              boc[0].z = oc.z;              	
              for (m=0;m<anz-1;m++) {
                if (bestcorr[m]>bestcorr[m+1]){

                  hcorr        = bestcorr[m+1];
                  bestcorr[m+1]= bestcorr[m];
                  bestcorr[m]  = hcorr;

                  hi       = bca[m+1].x;
                  bca[m+1].x = bca[m].x;
                  bca[m].x   = hi;

                  hi       = bca[m+1].y;
                  bca[m+1].y = bca[m].y;
                  bca[m].y   = hi;

                  hi       = bca[m+1].z;
                  bca[m+1].z = bca[m].z;
                  bca[m].z   = hi;

                  hi       = bcp[m+1].x;
                  bcp[m+1].x = bcp[m].x;
                  bcp[m].x   = hi;

                  hi       = bcp[m+1].y;
                  bcp[m+1].y = bcp[m].y;
                  bcp[m].y   = hi;

                  hi       = bcp[m+1].z;
                  bcp[m+1].z = bcp[m].z;
                  bcp[m].z   = hi;

                  hi       = boc[m+1].x;
                  boc[m+1].x = boc[m].x;
                  boc[m].x   = hi;

                  hi       = boc[m+1].y;
                  boc[m+1].y = boc[m].y;
                  boc[m].y   = hi;

                  hi       = boc[m+1].z;
                  boc[m+1].z = boc[m].z;
                  boc[m].z   = hi;
                }
                else goto m1;
	            }
              m1: ;
            }
          }
          }
        }
      }
    }
  }

  for (i=0;i<anz;i++){
    VPixel(vert,bca[i].z,bca[i].y,bca[i].x,VFloat)=1;
    VPixel(vert,bcp[i].z,bcp[i].y,bcp[i].x,VFloat)=2;
    VPixel(vert,boc[i].z,boc[i].y,boc[i].x,VFloat)=3;
  }

  r=edca=edcp=edoc=0;
  for (i=0;i<anz;i++){
    for (j=i+1;j<anz;j++){
      edca += sqrt(SQR(bca[i].x-bca[j].x)+SQR(bca[i].y-bca[j].y)+SQR(bca[i].z-bca[j].z));
      edcp += sqrt(SQR(bcp[i].x-bcp[j].x)+SQR(bcp[i].y-bcp[j].y)+SQR(bcp[i].z-bcp[j].z));
      edoc += sqrt(SQR(boc[i].x-boc[j].x)+SQR(boc[i].y-boc[j].y)+SQR(boc[i].z-boc[j].z));
      r++;
    }
  }

  corr=bcorr=0;
  for (i=0;i<anz;i++){
    angle = atan(((double)bcp[i].z-(double)bca[i].z)/((double)bcp[i].y-(double)bca[i].y));
    corr = (double)match2(bca[i],bcp[i],angle);
    if (corr>=bcorr){
      bds=i;
      bcorr=corr;
    }
  }

  angle = (double)atan(((double)bcp[bds].z-(double)bca[bds].z)/((double)bcp[bds].y-(double)bca[bds].y));
  SearchPoint(bca[bds],bcp[bds],angle,p);
  return vert;
}


/*
** main focus
*/
void
MainFocus(ipoint *mf, VImage src)
{
  int x,y,z,mem,dz,flag,sl,counter;
  int bands,rows,cols;
  double mw;

  bands = VImageNBands(src);
  rows  = VImageNRows(src);
  cols  = VImageNColumns(src);
  dz=70;
  sl=10;

  flag=mw=mem=counter=z=0;
  while (flag==0 && z<bands)
  {
    mw=0;
    for (y=0;y<rows;y++){
      for (x=0;x<cols;x++){
        mw += (double)VPixel(src,z,y,x,VUByte)/(rows*cols);
      }
    }
    if (mw>10){
      if((z-mem)>1){
        counter=0;
        mem=z;
      }
      else{
        counter++;
        mem=z;
      }
      if (counter>sl){
        flag=1;
      }
    }
    z++;
  }
  (*mf).z=mem;

  flag=mw=mem=counter=y=0;
  while (flag==0 && y<rows)
  {
    mw=0;
    for (z=(*mf).z;z<(*mf).z+dz;z++){
      for (x=0;x<cols;x++){
        mw += (double)VPixel(src,z,y,x,VUByte)/(dz*cols);
      }
    }
    if (mw>10){
      if((y-mem)>1){
        counter=0;
        mem=y;
      }
      else{
        counter++;
        mem=y;
      }
      if (counter>sl){
        flag=1;
      }
    }
    y++;
  }
  (*mf).y = mem;

  flag=mw=mem=counter=0;
  y=rows;
  while (flag==0 && y>0)
  {
    mw=0;
    for (z=(*mf).z;z<(*mf).z+dz;z++){
      for (x=0;x<cols;x++){
        mw += (double)VPixel(src,z,y,x,VUByte)/(dz*cols);
      }
    }
    if (mw>10){
      if((y-mem)>1){
        counter=0;
        mem=y;
      }
      else{
        counter++;
        mem=y;
      }
      if (counter>sl){
        flag=1;
      }
    }
    y--;
  }
  (*mf).y = VRint(((mem-(*mf).y)/2)+(*mf).y);

  flag=mw=mem=counter=x=0;
  while (flag==0 && x<cols)
  {
    mw=0;
    for (z=(*mf).z;z<(*mf).z+dz;z++){
      for (y=0;y<rows;y++){
        mw += (double)VPixel(src,z,y,x,VUByte)/(dz*rows);
      }
    }
    if (mw>10){
      if((x-mem)>1){
        counter=0;
        mem=x;
      }
      else{
        counter++;
        mem=x;
      }
      if (counter>sl){
        flag=1;
      }
    }
    x++;
  }
  (*mf).x = mem;

  flag=mw=mem=counter=0;
  x=cols;
  while (flag==0 && x>0)
  {
    mw=0;
    for (z=(*mf).z;z<(*mf).z+dz;z++){
      for (y=0;y<rows;y++){
        mw += (double)VPixel(src,z,y,x,VUByte)/(dz*rows);
      }
    }
    if (mw>10){
      if((x-mem)>1){
        counter=0;
        mem=x;
      }
      else{
        counter++;
        mem=x;
      }
      if (counter>sl){
        flag=1;
      }
    }
    x--;
  }
  (*mf).x = VRint(((mem-(*mf).x)/2)+(*mf).x);

  (*mf).x+=0;
  (*mf).y+=0;
  (*mf).z+=70;


}
/*################################################################################################
**read in images in temp. memory
*/
void
ReadInMemory(VImage src, VImage ca, VImage cp)
{
  int x,y,z;

  sbands = VImageNBands(src);
  srows  = VImageNRows(src);
  scols  = VImageNColumns(src);
  s_repn = VPixelRepn(src);
  src_m   = VCreateImage(sbands,srows,scols,s_repn);

  for (z=0;z<sbands;z++){
    for (y=0;y<srows;y++){
      for (x=0;x<scols;x++){
        VPixel(src_m,z,y,x,VUByte) = VPixel(src,z,y,x,VUByte);
      }
    }
  }

  cabands = VImageNBands(ca);
  carows  = VImageNRows(ca);
  cacols  = VImageNColumns(ca);
  ca_repn = VPixelRepn(ca);
  ca_m    = VCreateImage(cabands,carows,cacols,ca_repn);

  for (z=0;z<cabands;z++){
    for (y=0;y<carows;y++){
      for (x=0;x<cacols;x++){
        VPixel(ca_m,z,y,x,VUByte) = VPixel(ca,z,y,x,VUByte);
      }
    }
  }

  cpbands = VImageNBands(cp);
  cprows  = VImageNRows(cp);
  cpcols  = VImageNColumns(cp);
  cp_repn = VPixelRepn(cp);
  cp_m    = VCreateImage(cpbands,cprows,cpcols,cp_repn);

  for (z=0;z<cpbands;z++){
    for (y=0;y<cprows;y++){
      for (x=0;x<cpcols;x++){
        VPixel(cp_m,z,y,x,VUByte) = VPixel(cp,z,y,x,VUByte);
      }
    }
  }

}


/*
** preproccessng on original image
** rotation yaw/pitch as analoque the results of estimation of MSP
*/
VImage preprocess(VImage image, double *angle, ipoint mf)
{
  VImage dest_image;
  float roll,pitch,yaw,cr,cp,cy,sr,sp,sy;
  double fix[3],shift[3],rot[3][3];
  int b,r,c;

  b  = VImageNBands(image);
  r  = VImageNRows(image);
  c  = VImageNColumns(image);

  fix[0] = shift[0] = (float)mf.z;
  fix[1] = shift[1] = (float)mf.y;
  fix[2] = shift[2] = (float)mf.x;


  roll     = (float)angle[0];
  pitch    = (float)angle[1];
  yaw      = (float)angle[2];

  cr = cos(roll);
  cp = cos(pitch);
  cy = cos(yaw);
  sr = sin(roll);
  sp = sin(pitch);
  sy = sin(yaw);
  rot[0][0] = cr * cy + sr * sp * sy;
  rot[0][1] = sr * cp;
  rot[0][2] = sr * sp * cy - sy * cr;
  rot[1][0] = cr * sp * sy - sr * cy;
  rot[1][1] = cr * cp;
  rot[1][2] = sr * sy + cy * cr * sp;
  rot[2][0] = cp * sy;
  rot[2][1] = - sp;
  rot[2][2] = cp * cy;

  dest_image = VReSample3d(image,NULL,b,r,c,rot,shift,fix);

  return dest_image;
}

/***************************************/

double MedianEstimation(int bands, double arr[])
{
  int h;
  double median;

  gsl_sort(arr,1,bands);

  median = 0;
  if (bands%2==0){
    h=bands/2;
    if (h > 1 && h < bands)
      median=(arr[h-1]+arr[h])/2;
  }
  else{
    h=VCeil(bands/2);
    if (h > 1) median=arr[h-1];
  }
  return median;
}
/***********************************************/
double angle(VImage corr_image,double rohmin,double droh, int flag)
{
  int x,y,z,i;
  int bands,rows,cols;
  double corr,corr_max,roh,pnt,*a,median,w,sum_w;
  mspPoint *phi;
  double phi_mean1=0,phi_mean=0;
  double std_dev=0,sum=0,sum2=0;

  bands = VImageNBands(corr_image);
  rows  = VImageNRows(corr_image);
  cols  = VImageNColumns(corr_image);


  phi = (mspPoint *) malloc(sizeof(double) * bands * 3);
  a   = (double *) VCalloc(bands,sizeof(double));


  for (z=0;z<bands;z++){
    corr=corr_max=roh=pnt=0;
    for (y=0;y<rows;y++){
      for (x=0;x<cols;x++){
        corr=VPixel(corr_image,z,y,x,VFloat);
        if (corr>corr_max){
          corr_max=corr;
          roh=(double)y;
          pnt=(double)x;
        }
      }
    }
    phi[z].corr=corr_max;
    phi[z].roh=(roh*(double)droh)+rohmin;
    phi[z].pnt=pnt;
  }

  for (z=0;z<bands;z++){
    phi_mean1+=phi[z].roh/bands;
    a[z]=phi[z].roh;
  }

  for (z=0;z<bands;z++){
    sum+=(double)(phi[z].roh-phi_mean1)*(phi[z].roh-phi_mean1);
  }
  std_dev=(double)sqrt(sum/(bands-1));
  median = MedianEstimation(bands,a);

  if (flag==1){ /* pitch */
    phi_mean=i=sum_w=sum2=0;
    for (z=0;z<bands;z++){
      if (phi[z].roh>=median-(3*std_dev) && phi[z].roh<=median+(3*std_dev)){
        sum2    += phi[z].roh;
        if (z<=VRint(bands/2)) w = SQR(z+1);
        else w = SQR(bands-(z-1));
        sum_w   += w;
        phi_mean+= w*phi[z].roh;
        i++;
      }
    }
    phi_mean=(double)phi_mean/sum_w;
  }

  if (flag==0){  /* yaw */
    phi_mean=i=sum_w=sum2=0;
    for (z=0;z<bands;z++){
      if (phi[z].roh>=median-(3*std_dev) && phi[z].roh<=median+(3*std_dev)){
        sum2    += phi[z].roh;
        if (z<=VRint(bands/2)) w = SQR(z+1);
        else w = SQR(bands-(z-1));
        sum_w   += w;
        phi_mean+= w*phi[z].roh;
        i++;
      }
    }
    phi_mean=(double)phi_mean/sum_w;
  }

  return phi_mean;
}
/*####################################################################
**clipping N*N-image of rotated image
*/
VImage normalize(VImage rot, int N,int dz)
{
  VImage norm;
  int rotrows,rotcols;
  int x,y,z,i,j;

  norm = VCreateImage(dz,N,N,VUByteRepn);

    rotcols = VImageNColumns(rot);
    rotrows  = VImageNRows(rot);
    for (z=0;z<dz;z++){
      for (y=0;y<N;y++){
        for (x=0;x<N;x++){
          i = (int)(rotrows-N)/2+y;
          j = (int)(rotcols-N)/2+x;
          if (VPixel(rot,z,i,j,VUByte) > 1) VPixel (norm,z,y,x,VUByte) = 255;
          else VPixel (norm,z,y,x,VUByte) = 1;
        }
      }
    }
  return norm;
}
/*#######################################################################
**Computing of correlation factor between soll and rot
*/
VImage correlation(VImage soll, VImage rot,int N, int bands,int rg)
{
  VUByte *x,*y;
  int i,j,n,rg2;
  int b,r,c,xx;
  VImage corr_image;

  rg2= (int)rg/2;

  corr_image = VCreateImage(bands,1,rg+1,VFloatRepn);
  for (b=0; b<bands; b++) {
    for (r=0; r<1; r++) {
      for (c=0; c<rg+1; c++) {
        VPixel(corr_image,b,r,c,VFloat)=0;
      }
    }
  }
  n   = N*N;
  x   = (VUByte *) VMalloc(sizeof(VUByte) * n);
  y   = (VUByte *) VMalloc(sizeof(VUByte) * n);

  for (b=0; b<bands; b++) {
    for (j=0; j<n; j++) x[j] = 0;
    i=0;
    for (r=0; r<N; r++) {
      for (c=rg2; c<N-rg2; c++) {
	      x[i] = VPixel(rot,b,r,c,VUByte);
	      i++;
      }
    }
    for (xx=-rg2;xx<rg2+1;xx++){
      for (j=0; j<n; j++) y[j] = 0;
      i=0;
      for (r=0; r<N; r++) {
        for (c=rg2+xx; c<N-rg2+xx; c++) {
	        y[i] = VPixel(soll,b,r,c,VUByte);
	        i++;
         }
      }
      VPixel(corr_image,b,0,rg2+xx,VFloat) = (float)pearson(x,y,n);
    }
  }

  return corr_image;
}
/*#######################################################################
**estimation of pitch-angle
*/
#define PI 3.141593
double MidSagittalPitch (VImage src,int N,int N2,int rg,int dz, ipoint mf)
{
  int x,y,z,i,j,b,r,c,hvi,dz2;
  VImage norm,slice,temp,rot,soll,s_bin,refl,corr_image,corr_temp,histogramm;
  const double sigma = 3;
  double mz,my,mx;
  VUByte grw;
  double rohmin,rohmax;
  double droh,roh;
  int filter_size=5;
  double pitch;
  double *teiler,hvd,u;
  int *threshold,*sum,*histomax;
  gsl_matrix *histo;

  sum       = (int *) VCalloc(dz,sizeof(int));
  threshold = (int *) VCalloc(dz,sizeof(int));
  histomax  = (int *) VCalloc(dz,sizeof(int));
  teiler    = (double *) VCalloc(dz,sizeof(double));
  histo     = gsl_matrix_calloc(dz,256);

  slice  = VCreateImage(1,N,N,VUByteRepn);
  temp   = VCreateImage(1,N,N,VUByteRepn);
  soll   = VCreateImage(dz,N,N,VUByteRepn);
  s_bin  = VCreateImage(dz,N,N,VUByteRepn);
  refl   = VCreateImage(dz,N,N,VUByteRepn);

  dz2= (int)dz/2;
  mx = mf.x;
  my = mf.y;
  mz = mf.z-dz2;

  /* clipping of interest region of brain and filtering */
  for (z=mz-dz2;z<mz+dz2;z++){
    for (y=my-N2;y<my+N2;y++){
      for (x=mx-N2;x<mx+N2;x++){
        i = (int)y-(my-N2);
        j = (int)x-(mx-N2);
        VPixel(slice,0,i,j,VUByte) = VPixel(src,z,y,x,VUByte);
      }
    }
    VGaussianConv (slice,temp,0,sigma,filter_size);
    VCopyBand (temp,0,soll,z-(mz-dz2));
  }

  /* histogramm */
  gsl_matrix_set_zero(histo);

  /* readout greyvalues */
  for (b=0; b<dz; b++) {
    for (r=0; r<N; r++) {
      for (c=0; c<N; c++) {
        hvi = (int)VPixel(soll,b,r,c,VUByte);
	u = gsl_matrix_get(histo,b,hvi);
	gsl_matrix_set(histo,b,hvi,u+1);
      }
    }
  }

  /* estimation of greyvalue with maximum appearance */
  for (b=0; b<dz; b++) {
    histomax[b]=sum[b]=0;
    for (z=0;z<256;z++) {
      if (z<20) gsl_matrix_set(histo,b,z,0);
      hvi = gsl_matrix_get(histo,b,z);
      sum[b] += hvi;
      if (hvi>histomax[b])histomax[b]=hvi;
    }
  }

  /* estimation of scalefactor for histogrammdrawing */
  for (b=0; b<dz; b++){
    teiler[b]=(double)histomax[b]/255.0;
  }

  /* draw histogramm */
  histogramm  = VCreateImage(dz,256,256,VUByteRepn);
  for (b=0; b<dz; b++) {
    for (c=0; c<256;c++) {
      for (r=255; r>=0; r--) {
        if ((255-r)<=(gsl_matrix_get(histo,b,c)/teiler[b])) VPixel(histogramm,b,r,c,VUByte)=0;
        else                                  VPixel(histogramm,b,r,c,VUByte)=255;
      }
    }
  }


  /* threshold from histogramm */
  for (b=0; b<dz; b++) {
    threshold[b]=hvi=0;
    for (z=0;z<256;z++) {
      hvi += gsl_matrix_get(histo,b,z);
      if (hvi>(sum[b]*0.5)){
        threshold[b]=z;
        goto m1;
      }
    }
    m1: ;
  }

  /* binarize soll */
  for (b=0; b<dz; b++) {
    for (r=0; r<N; r++) {
      for (c=0; c<N; c++) {
        grw = VPixel(soll,b,r,c,VUByte);
        if(grw>threshold[b]) VPixel(s_bin,b,r,c,VUByte) = 255;
        else      VPixel(s_bin,b,r,c,VUByte) = 1;
      }
    }
  }


  /* reflection at x=0 */
  for (b=0; b<dz; b++) {
    for (r=0; r<N; r++) {
      for (c=0; c<N; c++) {
        VPixel(refl,b,r,N-c,VUByte) = VPixel(s_bin,b,r,c,VUByte);
      }
    }
  }


  /* rotation */
  corr_image = VCreateImage(dz,3,rg+1,VFloatRepn);
  hvd =8;
  pitch=0;
  for (j=1;j<=hvd;j*=2){
    rohmin=-hvd/j+pitch;
    rohmax=hvd/j+pitch;
    droh=hvd/(j);
    i=0;
    for (roh=rohmin;roh<=rohmax+0.00001;roh+=droh){
      rot = VRotateImage (refl,NULL,VAllBands,2*roh*PI/180);
      norm = normalize(rot,N,dz);
      corr_temp = correlation(s_bin,norm,N,dz,rg);
      for (z=0;z<dz;z++){
        for (x=0;x<rg+1;x++){
          VPixel(corr_image,z,i,x,VFloat) = VPixel(corr_temp,z,0,x,VFloat);
        }
      }
      i++;
    }
    pitch = angle(corr_image,rohmin,droh,1);
  }
  return pitch/GRD;
}
#undef PI


/*
**estimation of yaw-angle
*/
#define PI 3.141593
double MidSagittalYaw (VImage src,int N,int N2,int rg,int dz, ipoint mf)
{
  int x,y,z,i,j,b,r,c,hvi,dz2;
  VImage norm,slice,temp,rot,soll,s_bin,refl,corr_image,corr_temp,histogramm;
  const double sigma = 3;
  double mz,my,mx;
  VUByte grw;
  double rohmin,rohmax;
  double droh,roh;
  int filter_size=5;
  double yaw;
  double *teiler,hvd,u;
  int *threshold,*sum,*histomax;
  gsl_matrix *histo;

  sum       = (int *) VCalloc(dz,sizeof(int));
  threshold = (int *) VCalloc(dz,sizeof(int));
  histomax  = (int *) VCalloc(dz,sizeof(int));
  teiler    = (double *) VCalloc(dz,sizeof(double));
  histo     = gsl_matrix_calloc(dz,256);

  slice  = VCreateImage(1,N,N,VUByteRepn);
  temp   = VCreateImage(1,N,N,VUByteRepn);
  soll   = VCreateImage(dz,N,N,VUByteRepn);
  s_bin  = VCreateImage(dz,N,N,VUByteRepn);
  refl   = VCreateImage(dz,N,N,VUByteRepn);

  dz2= (int)dz/2;
  mx = mf.x;
  my = mf.y;
  mz = mf.z;


  /* clipping of interest region of brain and filtering */
  for (y=my-dz2;y<my+dz2;y++){
    for (z=mz-N2;z<mz+N2;z++){
      for (x=mx-N2;x<mx+N2;x++){
        i = (int)z-(mz-N2);
        j = (int)x-(mx-N2);
        VPixel(slice,0,i,j,VUByte) = VPixel(src,z,y,x,VUByte);
      }
    }
    VGaussianConv (slice,temp,0,sigma,filter_size);
    VCopyBand (temp,0,soll,y-(my-dz2));
  }

  /* histogramm */
  gsl_matrix_set_zero(histo);

  /* readout greyvalues */
  for (b=0; b<dz; b++) {
    for (r=0; r<N; r++) {
      for (c=0; c<N; c++) {
        hvi = (int)VPixel(soll,b,r,c,VUByte);
	u = gsl_matrix_get(histo,b,hvi);
	gsl_matrix_set(histo,b,hvi,u+1);
      }
    }
  }

  /* estimation of greyvalue with maximum appearance */
  for (b=0; b<dz; b++) {
    histomax[b]=sum[b]=0;
    for (z=0;z<256;z++) {
      if (z<20) gsl_matrix_set(histo,b,z,0);
      hvi = gsl_matrix_get(histo,b,z);
      sum[b] += hvi;
      if (hvi>histomax[b])histomax[b]=hvi;
    }
  }

  /* estimation of scalefactor for histogrammdrawing */
  for (b=0; b<dz; b++){
    teiler[b]=(double)histomax[b]/255;
  }

  /* draw histogramm */
  histogramm = VCreateImage(dz,256,256,VUByteRepn);
  for (b=0; b<dz; b++) {
    for (c=0; c<256;c++) {
      for (r=255; r>=0; r--) {
        if ((255-r)<=(gsl_matrix_get(histo,b,c)/teiler[b])) VPixel(histogramm,b,r,c,VUByte)=0;
        else                                  VPixel(histogramm,b,r,c,VUByte)=255;
      }
    }
  }

  /* threshold from histogramm */
  for (b=0; b<dz; b++) {
    threshold[b]=hvi=0;
    for (z=0;z<256;z++) {
      hvi += gsl_matrix_get(histo,b,z);
      if (hvi>(sum[b]*0.5)){
        threshold[b]=z;
        goto m1;
      }
    }
    m1: ;
  }

  /* binarize soll */
  for (b=0; b<dz; b++) {
    for (r=0; r<N; r++) {
      for (c=0; c<N; c++) {
        grw = VPixel(soll,b,r,c,VUByte);
        if(grw>threshold[b]) VPixel(s_bin,b,r,c,VUByte) = 255;
        else      VPixel(s_bin,b,r,c,VUByte) = 1;
      }
    }
  }


  /* reflection at x=0 */
  for (b=0; b<dz; b++) {
    for (r=0; r<N; r++) {
      for (c=0; c<N; c++) {
        VPixel(refl,b,r,N-c,VUByte) = VPixel(s_bin,b,r,c,VUByte);
      }
    }
  }


  /* rotation */
  corr_image = VCreateImage(dz,3,rg+1,VFloatRepn);
  hvd =8;
  yaw=0;
  for (j=1;j<=hvd;j*=2){
    rohmin=-hvd/j+yaw;
    rohmax=hvd/j+yaw;
    droh=hvd/(j);
    i=0;
    for (roh=rohmin;roh<=rohmax+0.00001;roh+=droh){
      rot = VRotateImage (refl,NULL,VAllBands,2*roh*PI/180);
      norm = normalize(rot,N,dz);
      corr_temp = correlation(s_bin,norm,N,dz,rg);
      for (z=0;z<dz;z++){
        for (x=0;x<rg+1;x++){
          VPixel(corr_image,z,i,x,VFloat) = VPixel(corr_temp,z,0,x,VFloat);

        }
      }
      i++;
    }
    yaw = angle(corr_image,rohmin,droh,0);
  }
  return yaw/GRD;
}
#undef PI


/*
** start of msp estimation
*/
void msp(VImage src_image,double *angle, ipoint mf)
{
  int N,N2,rg,dz;
  double yaw,pitch;

  N=120;
  N2=(int)N/2;
  rg=30;
  dz=60;
  pitch = MidSagittalPitch(src_image,N,N2,rg,dz,mf);
  N=120;
  N2=(int)N/2;
  rg=30;
  dz=60;
  yaw = MidSagittalYaw(src_image,N,N2,rg,dz,mf);
  angle[0]=0;
  angle[1]=pitch;
  angle[2]=yaw;
}


/*
**  mainroutine
*/
int main (int argc, char *argv[])
{
  static VString in = NULL;
  static VString out = NULL;
  static VString template_file ="/usr/share/lipsia/vcacp.v"; /* default filename */
  static VString report_file = NULL;
  static VOptionDescRec options[] = {
    { "in", VStringRepn, 1, &in, VRequiredOpt, 0,"Image to be processed" },
    { "out", VStringRepn, 1, &out, VOptionalOpt, 0,"Latent image for internal use" },
    { "template", VStringRepn, 1, &template_file, VOptionalOpt, 0,"CA/CP template" },
    { "report", VStringRepn, 1, &report_file, VOptionalOpt, 0,"Report file" }
   };

  FILE *in_file,*fp,*out_file,*reportprt;
  VAttrList list, list1;
  VAttrListPosn posn;
  VImage src=NULL, CAtemplate=NULL, CPtemplate=NULL, VCtemplate=NULL, AVEtemplate=NULL,src_image,vert;
  VString str=NULL;
  double *angle;
  ipoint mf;
  dpoint ca,cp;
  clock_t tStart, tEnd;
  float v0=0, v1=1, v2=2, eps=0.005;
  float v00=0, v11=1, v22=0;
  gsl_vector *p=NULL;
  char prg_name[50];
  sprintf(prg_name,"vcacp V%s", getLipsiaVersion()); 
  fprintf (stderr, "%s\n", prg_name);

  
  tStart = clock();
  angle = (double *) VCalloc(3,sizeof(double));
  p = gsl_vector_calloc(6);

  VParseFilterCmd(VNumber(options), options, argc, argv, NULL, NULL);
  /* Path exists? */
  if (report_file!=NULL){
    if (!(reportprt = fopen(report_file,"a"))){
      VError("Unable to open report file %s",report_file);
    }
    fclose(reportprt);
  }
  in_file = VOpenInputFile (in, TRUE);
  out_file = VOpenOutputFile (out, TRUE);
  

  /*  Create attribute list  */
  dest_list = VCreateAttrList();
  if (str == NULL) str = VMalloc(100);

  /*  Read the template image: */
  if (strlen(template_file) > 2) {
    fp = VOpenInputFile (template_file, TRUE);
    list1 = VReadFile (fp, NULL);
    if (! list1) VError("Error reading image");
    fclose(fp);

    for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      if (CAtemplate == NULL) {
	        VGetAttrValue (& posn, NULL, VImageRepn, & CAtemplate);
	        if (VPixelRepn(CAtemplate) != VUByteRepn) continue;
      }
      else if (CPtemplate == NULL) {
	      VGetAttrValue (& posn, NULL, VImageRepn, & CPtemplate);
	      if (VPixelRepn(CPtemplate) != VUByteRepn) continue;
      }
      else if (VCtemplate == NULL) {
	      VGetAttrValue (& posn, NULL, VImageRepn, & VCtemplate);
	      if (VPixelRepn(VCtemplate) != VUByteRepn) continue;
      }
      else if (AVEtemplate == NULL) {
	      VGetAttrValue (& posn, NULL, VImageRepn, & AVEtemplate);
	      if (VPixelRepn(VCtemplate) != VUByteRepn) continue;
      }
    }
  }
  /* check and manipulation (if necessary) of incomming paramters */
  if (CAtemplate == NULL) VError("CA template not found");
  if (CPtemplate == NULL) VError("CP template not found");
  if (VCtemplate == NULL) VError("VC template not found");

  /*  check the voxel dimensions */
  if (VGetAttr (VImageAttrList (CAtemplate), "voxel", NULL,VStringRepn, (VPointer) & str) != VAttrFound) 
    VError(" attribute 'voxel' missing in CA template");
  sscanf(str,"%f %f %f",&v0, &v1, &v2);

  if (VGetAttr (VImageAttrList (CPtemplate), "voxel", NULL,VStringRepn, (VPointer) & str) != VAttrFound) 
    VError(" attribute 'voxel' missing in CP template");
  sscanf(str,"%f %f %f",&v00, &v11, &v22);  
  if ( ABS(v0-v00)>eps || ABS(v1-v11)>eps || ABS(v2-v22)>eps ) 
    VError(" voxel size of CA and CP template mismatch");

  if (VGetAttr (VImageAttrList (VCtemplate), "voxel", NULL,VStringRepn, (VPointer) & str) != VAttrFound) 
    VError(" attribute 'voxel' missing in VC template");
  sscanf(str,"%f %f %f",&v00, &v11, &v22);  
  if ( ABS(v0-v00)>eps || ABS(v1-v11)>eps || ABS(v2-v22)>eps ) 
    VError(" voxel size of CA and VC template mismatch");
  
  
  /*  Read source image(s): */
  if (! (list = VReadFile(in_file, NULL))) return 1;
  fclose(in_file);

  /*  Operate on each source image: */
  for (VFirstAttr(list, &posn); VAttrExists(&posn); VNextAttr(&posn)) {
    if (VGetAttrRepn(&posn) == VImageRepn)  {
      VGetAttrValue(&posn, NULL, VImageRepn, &src);
      if (VPixelRepn(src) != VUByteRepn) VError("Input file must be ubyte");
            
      /* voxel */

      if (VGetAttr (VImageAttrList (src), "voxel", NULL,VStringRepn, (VPointer) & str) != VAttrFound) 
	VError(" attribute 'voxel' missing in input file.");
      sscanf(str,"%f %f %f",&v00, &v11, &v22);  
      if ( ABS(v0-v00)>eps || ABS(v1-v11)>eps || ABS(v2-v22)>eps ) 
	VError(" voxel size of infile and template mismatch");
 

      fprintf(stderr,"CA                    CP                    Angles(°)\n");

      MainFocus(&mf,src);
      msp(src,angle,mf);      /* estimation of MSP */
      angle[0]= 0;            /* roll */
      angle[1]=angle[1];      /* pitch */
      angle[2]=(-1)*angle[2]; /* yaw */

      src_image=preprocess(src,angle,mf);
      VAppendAttr(dest_list, "image", NULL, VImageRepn, src_image);
      ReadInMemory(src_image,CAtemplate,CPtemplate);


      vert=SearchROI(mf,10,src,p);
      VAppendAttr(dest_list, "image", NULL, VImageRepn, vert);

      ca.x = gsl_vector_get(p,0);
      ca.y = gsl_vector_get(p,1);
      ca.z = gsl_vector_get(p,2);
      cp.x = gsl_vector_get(p,3);
      cp.y = gsl_vector_get(p,4);
      cp.z = gsl_vector_get(p,5);
      ca = computePoint(ca,mf,angle);
      cp = computePoint(cp,mf,angle);

      fprintf(stderr,"%5.1f %5.1f %5.1f     %5.1f %5.1f %5.1f    %4.1f %4.1f %4.1f\n",
	      ca.x,ca.y,ca.z,cp.x,cp.y,cp.z,angle[0]*GRD,(-1)*angle[2]*GRD,angle[1]*GRD);

      if (report_file!=NULL){
        reportprt = fopen(report_file,"a");
        fprintf(reportprt,"-ca %5.1f %5.1f %5.1f   -cp %5.1f %5.1f %5.1f  -angle %4.1f %4.1f %4.1f\n",
		ca.x,ca.y,ca.z,cp.x,cp.y,cp.z,angle[0]*GRD,(-1)*angle[2]*GRD,angle[1]*GRD);
        fclose(reportprt);
      }
    }
  }

  /* Make History */
  VHistory(VNumber(options),options,prg_name,&list,&dest_list); 

  if(out!=NULL){
    if (! VWriteFile(out_file, dest_list)) {
      VError("Error while writing new file");
      exit(1);
    }
  }

  tEnd = clock();
  fprintf (stderr,"vCACP done in %3.0f seconds.\n",(float)((tEnd-tStart)/CLOCKS_PER_SEC));

  return 0;
}
