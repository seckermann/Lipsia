/*
** Brain peeling
**
** G.Lohmann, Mar 2007
** 
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <via.h>


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SQR(x) ((x)*(x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

extern VImage VLocalContrast(VImage src,VImage dest,VShort wsize,VDouble sfactor,int background);



/* find bounding box (parameter 'extent') */
int
BoundingBox(VImage src,XPoint *or1, XPoint *or2, XPoint *extent)
{
  int nbands,nrows,ncols;
  int b0,b1,r0,r1,c0,c1,r2,c2;
  int db,dr,dc;
  int i,b,r,c;
  VShort t_low  =   5;   /* lower threshold for background pixels  */
  VShort t_high = 252;   /* upper threshold for grey matter pixels */
  XPoint fix;
  float x,y,z;
  VString str;


  /* bounding box only makes sense if CA/CP is known */
  if (VGetAttr (VImageAttrList (src), "ca", NULL,
		VStringRepn, (VPointer) & str) != VAttrFound) return 0;


  /* get fixed point */
  if (VGetAttr (VImageAttrList (src), "fixpoint", NULL,
		VStringRepn, (VPointer) & str) == VAttrFound) {
    sscanf(str,"%f %f %f",&x,&y,&z);
    fix.x = x;
    fix.y = y;
    fix.z = z;
  }
  else {
    fix.x = 80;
    fix.y = 95;
    fix.z = 90;
  }


  nbands = VImageNBands(src);
  nrows  = VImageNRows(src);
  ncols  = VImageNColumns(src);

  c0 = ncols;
  c1 = 0;
  r0 = nrows;
  r1 = 0;
  b0 = nbands;
  b1 = 0;

  c2 = fix.x;
  r2 = fix.y + 5;
  r2 = fix.y + 4; 
  
  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
        if (ABS(c - c2) < 10) continue; /* avoid brain stem and falx */

        i = VGetPixel(src,b,r,c);
        if (i < t_low) continue;
        if (i > t_high) continue;

        if (ABS(c - c2) > 5)
          if (b < b0) b0 = b;
        
        if (r < r2 && ABS(c - c2) > 15) {
          if (b > b1){
            b1 = b;
          }
        }

        if (r < r0) r0 = r;
        if (r > r1) r1 = r;

        if (c < c0) c0 = c;
        if (c > c1) c1 = c;
      }
    }
  }
  db = b1 - b0;
  dr = r1 - r0;
  dc = c1 - c0;

  (*or1).x = c0;
  (*or1).y = r0;
  (*or1).z = b0;

  (*or2).x = c1;
  (*or2).y = r1;
  (*or2).z = b1;

  (*extent).x = dc;
  (*extent).y = dr;
  (*extent).z = db;

  fprintf(stderr," bounding box:  ");
  fprintf(stderr,"  x= %d %d,   y= %d %d,   z= %d %d.\n",
          c0,c1,r0,r1,b0,b1);

  return 1;
}


/*
** brain peeling
*/
VImage 
VPeel(VImage src1,VImage dest,VDouble d1,VDouble d2,
      VFloat edge_strength,VShort threshold,VShort background)
{
  VImage src=NULL;
  VImage bin_image=NULL,bin1_image=NULL,label_image=NULL;
  VImage dorsal_image=NULL,tmp=NULL;
  VImage edge_image=NULL,gradc=NULL,gradr=NULL,gradb=NULL;
  VString str;
  int nbands,nrows,ncols,npixels;
  int b,r,c,i,nl;
  int b0=0,b1=0,step=5;
  VUByte *ubyte_pp;
  VFloat *float_pp;
  VBit   *bin_pp;
  XPoint or1,or2,extent;
  char info[120];
  int wsize;
  double sfactor;
  float cp_x,cp_y,cp_z;
  float voxelsize=1;
  VBoolean tal=FALSE;


  /*
  ** ini
  */
  nbands  = VImageNBands(src1);
  nrows   = VImageNRows(src1);
  ncols   = VImageNColumns(src1);
  npixels = nbands * nrows * ncols;

  cp_x = ncols/2;
  cp_y = nrows/2;
  cp_z = nbands/2;
  tal = FALSE;
  if (VGetAttr (VImageAttrList (src1), "cp", NULL,
		VStringRepn, (VPointer) & str) == VAttrFound) {
    sscanf(str,"%f %f %f",&cp_x,&cp_y,&cp_z);
    tal = TRUE;
  }

  voxelsize = 1;
  if (VGetAttr (VImageAttrList (src1), "voxel", NULL,
		VStringRepn, (VPointer) & str) == VAttrFound) {
    sscanf(str,"%f %f %f",&voxelsize,&voxelsize,&voxelsize);
  }


  /*
  ** get initial threshold
  */
  wsize = 20;
  sfactor = 3.0;
  src = VLocalContrast(src1,NULL,wsize,sfactor,1);



  /* binarize (crude white matter segmentation) */
  bin_image = VCreateImage(nbands,nrows,ncols,VBitRepn);
  VCopyImageAttrs (src,bin_image);

  ubyte_pp = (VUByte *) VImageData(src);
  bin_pp = (VBit *) VImageData(bin_image);
  for (i=0; i<npixels; i++) {
    *bin_pp = 1;
    if (*ubyte_pp < threshold) *bin_pp = 0;
    bin_pp++;
    ubyte_pp++;
  }

  ubyte_pp = (VUByte *) VImageData(src1);
  bin_pp = (VBit *) VImageData(bin_image);
  for (i=0; i<npixels; i++) {
    if (*ubyte_pp < background) *bin_pp = 0;
    bin_pp++;
    ubyte_pp++;
  }



  /* remove strong edges */
  VCanny3d (src,(int)3,&gradb,&gradr,&gradc);
  edge_image = VMagnitude3d(gradb,gradr,gradc,NULL);


  float_pp = (VFloat *) VImageData(edge_image);
  bin_pp   = (VBit   *) VImageData(bin_image);
  for (i=0; i<npixels; i++) {
    if ((*float_pp) > edge_strength) *bin_pp = 0;
    float_pp++;
    bin_pp++;
  }

  /* erode */
  if (d1 > 0)
    bin1_image = VDTErode(bin_image,bin1_image,d1);
  else
    bin1_image = VCopyImage(bin_image,bin1_image,VAllBands);


  /*
  ** no erosion around cerebellum and frontal pole
  */
  if (d1 > 0) {
    
    for (b=0; b<nbands; b++) {
      for (r=0; r<nrows; r++) {
	for (c=0; c<ncols; c++) {
	  if (VPixel(bin_image,b,r,c,VBit) == 0) continue;

	  if (r > cp_y+10.0/voxelsize && b > cp_z)
	    VPixel(bin1_image,b,r,c,VBit) = 1;
	}
      }
    }
  }

  /*
  ** dorsal
  */
  tmp = VCopyImage(bin1_image,tmp,VAllBands);
  for (b=cp_z; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	VPixel(tmp,b,r,c,VBit) = 0;
      }
    }
  }


  /* select largest component */
  label_image = VLabelImage3d(tmp,label_image,6,VShortRepn,&nl);
  if (nl < 1) VError(" no components found");
  dorsal_image = VSelectBig (label_image,dorsal_image);


  /*
  ** remove basal, if in Talairach space
  */
  if (tal) {
    for (b=cp_z+45.0/voxelsize; b<nbands; b++) {
      for (r=0; r<cp_y+5.0/voxelsize; r++) {
	for (c=0; c<ncols; c++) {
	  VPixel(bin1_image,b,r,c,VBit) = 0;
	}
      }
    }
    for (b=cp_z+50.0/voxelsize; b<nbands; b++) {
      for (r=0; r<nrows; r++) {
	for (c=0; c<ncols; c++) {
	  VPixel(bin1_image,b,r,c,VBit) = 0;
	}
      }
    }

    for (b=cp_z+30.0/voxelsize; b<nbands; b++) {
      for (r=0; r<nrows; r++) {
	for (c=0; c<ncols; c++) {
	  if (VPixel(src1,b,r,c,VUByte) > 253) 
	    VPixel(bin1_image,b,r,c,VBit) = 0;
	}
      }
    }
  }



  /*
  ** ventral 
  */
  b0 = cp_z;
  b1 = b0+step;
  while (b1 < nbands-1) {
    tmp = VCopyImage(bin1_image,tmp,VAllBands);
    if (b1 < nbands-step-1) {
      for (b=b1; b<nbands; b++) {
	for (r=0; r<nrows; r++) {
	  for (c=0; c<ncols; c++) {
	    VPixel(tmp,b,r,c,VBit) = 0;
	  }
	}
      }
    }

    for (b=0; b<=b0-1; b++) {
      for (r=0; r<nrows; r++) {
	for (c=0; c<ncols; c++) {
	  VPixel(tmp,b,r,c,VBit) = VPixel(dorsal_image,b,r,c,VBit);
	}
      }
    }

    /* select largest component */
    label_image = VLabelImage3d(tmp,label_image,6,VShortRepn,&nl);
    if (nl < 1) VError(" no components found");
    bin_image = VSelectBig (label_image,bin_image);
    dorsal_image = VCopyImage(bin_image,dorsal_image,VAllBands);
    b0 = b1;
    b1 += step;
  }
  bin_image = VCopyImage(dorsal_image,bin_image,VAllBands);


  /* dilate */
  bin1_image = VDTDilate(bin_image,bin1_image,d2);
  bin_image = VDTClose(bin1_image,bin_image,(VDouble)16.0);


  /* restore */  
  dest = VCopyImage(src1,NULL,VAllBands);
  ubyte_pp = VImageData(dest);
  bin_pp   = VImageData(bin_image);
  for (i=0; i<npixels; i++) {
    if (*bin_pp == 0) *ubyte_pp = 0;
    if (*ubyte_pp < background) *ubyte_pp = 0;
    bin_pp++;
    ubyte_pp++;
  }


  /* get extent */
  if (tal) {
    if (BoundingBox(dest,&or1,&or2,&extent) != 0) {
      sprintf(info, "%g %g %g",or1.x,or1.y,or1.z);
      VSetAttr(VImageAttrList(dest), "origin", NULL, VStringRepn, info);
      
      sprintf(info, "x: %g %g, y: %g %g, z: %g %g",
	      or1.x,or2.x, or1.y,or2.y, or1.z,or2.z);
      VSetAttr(VImageAttrList(dest), "boundingBox", NULL, VStringRepn, info);
      
      sprintf(info, "%g %g %g", extent.x, extent.y, extent.z);
      VSetAttr(VImageAttrList(dest), "extent", NULL, VStringRepn, info);
    }
  }
  return dest;
}
