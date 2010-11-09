/*
** apply output of 'vmulticomp' using 3 features
** (cluster size + max z-value + hemispheric symmetry)
**
** G.Lohmann, MPI-CBS
*/

/* From the Vista library: */
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via/via.h>


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))

extern void VPixel2Tal(float [3],float [3],float [3],int,int,int,float *,float *,float *);
extern void VolumeZero(Volume,VImage);
extern float VolumeZMax(Volume,VImage,float,int *,int *,int *);



/*
** check if multicomp is okay
*/
VBoolean
CheckValueSymm(VImage multicomp,int size,VFloat zval,float sym)
{
  int j,k,csize,msize;
  VFloat u,tiny=0.00001;
  float s[3] = {0.1, 0.2, 0.3};  /* symm dist */

  csize = VImageNRows(multicomp);
  msize = VImageNColumns(multicomp);
  if (size >= csize)  return TRUE;

  u = sym;
  if (u < s[0]) k = 0;                    /* non-symmetric */
  else if (u >= s[0] && u < s[1]) k = 1;  /* somewhat symmetric */
  else if (u >= s[1] && u < s[2]) k = 2;  /* symmetric */
  else k = 3;                             /* strongly symmetric */

  for (j=1; j<msize; j++) {
    u = VPixel(multicomp,k,size,j,VFloat);
    if (zval > u && u > tiny) return TRUE;
  }
  return FALSE;
}




/*
** get symmetry index in a volume
*/
float
VolumeSym(Volume vol,VImage src,float sign,VFloat zthr)
{
  int i,j,k,b,r,c,c0,c1,c2,ncols;
  VTrack t;
  double u,v,sumc,cog;

  ncols = VImageNColumns(src);
  c2 = ncols/2;

  sumc = 0;
  j = k = 0;
  for (i=0; i<VolumeNBuckets(vol); i++) {
    for (t = VFirstTrack(vol,i); VTrackExists(t); t = VNextTrack(t)) {
      b  = t->band;
      r  = t->row;
      c0 = t->col;
      c1 = c0 + t->length;

      for (c=c0; c<c1; c++) {
	sumc += c;
        v = VPixel(src,b,r,ABS(ncols-c),VFloat);
	v *= sign;
	j++;
	if (v > zthr) k++;
      }
    }
  }
  cog = sumc/(float)i;
  if (ABS(cog - c2) < 3) return 0;

  u = 0;
  if (j > 10) u = (float)k/(float)j;
  return u;
}







VImage
VDoMulticomp3d(VImage src,VImage multicomp,VBoolean verbose)
{  
  VString str;
  Volumes volumes;
  Volume vol;
  VImage tmp=NULL,label_image=NULL,dest=NULL;
  VFloat *src_pp,*dst_pp,u,zthr=0,zmax=0,voxsize=1,sym=0;
  VBit   *bin_pp;
  int   i,nl,size,nbands,nrows,ncols,npixels;
  int   b,r,c,b0,r0,c0;
  int   nflag,pflag;
  float x0,x1,x2;
  float sign=1;
  float voxel[3],ca[3],extent[3];


  /* 
  ** read multicomp file header 
  */
  voxsize = 1;
  if (VGetAttr (VImageAttrList (multicomp), "voxel_size", NULL,
                VFloatRepn, (VPointer) & voxsize) != VAttrFound)
    VError(" attribute 'voxel_size' not found");


  if (VGetAttr(VImageAttrList(multicomp),"zthr",NULL,VFloatRepn,(VPointer) &zthr) != VAttrFound)
    VError(" attribute 'zthr' not found");


  /* 
  ** read src header 
  */
  if (verbose) {
    if (VGetAttr (VImageAttrList (src), "voxel", NULL,
		  VStringRepn, (VPointer) & str) != VAttrFound)
      VError(" attribute 'voxel' not found");
    sscanf(str,"%f %f %f",&x0,&x1,&x2);
    if (ABS(x0*x1*x2 - voxsize) > 0.01) 
      VError(" voxel sizes do not match %.3f %.3f %.3f",x0,x1,x2);
    voxel[0] = x0;
    voxel[1] = x1;
    voxel[2] = x2;

    if (VGetAttr (VImageAttrList (src), "ca", NULL,
		  VStringRepn, (VPointer) & str) != VAttrFound)
      VError(" attribute 'ca' not found");
    sscanf(str,"%f %f %f",&x0,&x1,&x2);    
    ca[0] = x0;
    ca[1] = x1;
    ca[2] = x2;

    if (VGetAttr (VImageAttrList (src), "extent", NULL,
		  VStringRepn, (VPointer) & str) != VAttrFound)
      VError(" attribute 'extent' not found");
    sscanf(str,"%f %f %f",&x0,&x1,&x2);    
    extent[0] = x0;
    extent[1] = x1;
    extent[2] = x2;
  }


  nbands  = VImageNBands(src);
  nrows   = VImageNRows(src);
  ncols   = VImageNColumns(src);
  npixels = VImageNPixels(src);

  tmp  = VCreateImage(nbands,nrows,ncols,VBitRepn);
  dest = VCopyImage(src,NULL,VAllBands);
  VFillImage(dest,VAllBands,0);


  /*
  ** positive threshold
  */
  nflag = pflag = 0;
  src_pp = VImageData(src);
  bin_pp = VImageData(tmp);      
  for (i=0; i<npixels; i++) {
    *bin_pp = 0;
    u = *src_pp;
    if (u > 0 && u >= zthr) *bin_pp = 1;
    src_pp++;
    bin_pp++;
  }

  label_image = VLabelImage3d(tmp,label_image,(int)26,VShortRepn,&nl);
  if (nl < 1 && pflag == 0) {
    VWarning(" no voxels above threshold %.3f",zthr);
    goto next;
  }
  else 
    pflag++;


  volumes = VImage2Volumes(label_image);

  for (vol = volumes->first; vol != NULL; vol = vol->next) {
    sign = 1;
    size = VolumeSize(vol);
    zmax = VolumeZMax(vol,src,(float)1,&b,&r,&c);
    sym  = VolumeSym(vol,src,sign,zthr);

    if (CheckValueSymm(multicomp,size,zmax,sym) == FALSE) {
      VolumeZero(vol,tmp);
    }
    else {
      if (verbose) {
	VPixel2Tal(ca,voxel,extent,b,r,c,&x0,&x1,&x2);
	c0 = VRint(x0);
	r0 = VRint(x1);
	b0 = VRint(x2);
	fprintf(stderr," nvoxels: %5d,   zmax: %7.3f,   sym: %.3f,  addr: %3d %3d %3d\n",
		size,zmax,sym,c0,r0,b0);
      }
    }
  }
  src_pp = VImageData(src);
  dst_pp = VImageData(dest);
  bin_pp = VImageData(tmp);      
  for (i=0; i<npixels; i++) {
    if (*bin_pp > 0) *dst_pp = *src_pp;
    src_pp++;
    dst_pp++;
    bin_pp++;
  }


  /*
  ** negative threshold
  */
 next:
  src_pp = VImageData(src);
  bin_pp = VImageData(tmp);      
  for (i=0; i<npixels; i++) {
    *bin_pp = 0;
    u = *src_pp;
    if (u < 0 && -u >= zthr) *bin_pp = 1;
    src_pp++;
    bin_pp++;
  }


  label_image = VLabelImage3d(tmp,label_image,(int)26,VShortRepn,&nl);
  if (nl < 1 && nflag == 0) {
    /* VWarning(" no voxels below negative threshold %.3f",-zthr); */
    goto ende;
  }
  else 
    nflag++;


  volumes = VImage2Volumes(label_image);

  for (vol = volumes->first; vol != NULL; vol = vol->next) {
    sign = -1;
    size = VolumeSize(vol);
    zmax = VolumeZMax(vol,src,sign,&b,&r,&c);
    sym  = VolumeSym(vol,src,sign,zthr);

    if (CheckValueSymm(multicomp,size,zmax,sym) == FALSE) {
      VolumeZero(vol,tmp);
    }
    else {
      if (verbose) {
	VPixel2Tal(ca,voxel,extent,b,r,c,&x0,&x1,&x2);
	c0 = VRint(x0);
	r0 = VRint(x1);
	b0 = VRint(x2);

	fprintf(stderr," nvoxels: %5d,   zmax: %7.3f,   sym: %.3f,  addr: %3d %3d %3d\n",
		size,zmax,sym,c0,r0,b0);

      }
    }
  }

  src_pp = VImageData(src);
  dst_pp = VImageData(dest);
  bin_pp = VImageData(tmp);      
  for (i=0; i<npixels; i++) {
    if (*bin_pp > 0) *dst_pp = *src_pp;
    src_pp++;
    dst_pp++;
    bin_pp++;
  }

 ende:
  if (nflag == 0 && pflag == 0) VError(" no voxels passed threshold");
  return dest;
}
