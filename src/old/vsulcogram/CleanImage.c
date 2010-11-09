/*
 *  Program: vthin3d
 *
 *  G.Lohmann <lohmann@cns.mpg.de>, Nov. 96
 */
#include <stdio.h>
#include <stdlib.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <via.h>

int
FinalPoint(VImage src,int b,int r, int c)
{
  int n,bb,rr,cc;

  n = 0;
  for (bb=b-1; bb<=b+1; bb++) {
    for (rr=r-1; rr<=r+1; rr++) {
      for (cc=c-1; cc<=c+1; cc++) {
	if (VPixel(src,bb,rr,cc,VBit) > 0) n++;
      }
    }
  }
  return n;
}


VImage
VCleanImage(VImage src,VImage dest,VImage depth_image,int nadj)
{
  int b,r,c,nbands,nrows,ncols,npixels;
  int i,j,n,ndel;
  VBit *src_pp;
  float x0,x1;
  VPoint *array;
  float depth,maxdepth;

  nrows  = VImageNRows (src);
  ncols  = VImageNColumns (src);
  nbands = VImageNBands (src);
  npixels = nbands * nrows * ncols;
  if (dest == NULL)
    dest = VCreateImage(nbands,nrows,ncols,VBitRepn);

  for (r=0; r<nrows; r++) {
    for (c=0; c<ncols; c++) {
      VPixel(src,0,r,c,VBit) = 0;
      VPixel(src,nbands-1,r,c,VBit) = 0;
    }
  }

  /*
  ** distance transform
  */
  src_pp  = (VBit *) VPixelPtr(src, 0, 0, 0);
  n = 0;
  for (i=0; i<npixels; i++) {
    if (*src_pp > 0) n++;
    src_pp++;
  }

  /*
  ** set up array of black points
  */
  array = (VPoint *) VMalloc(sizeof(VPoint) * (n+1));

  array[0].b = 0;
  array[0].r = 0;
  array[0].c = 0;
  array[0].val = 0;

  n = 1;
  x0 = 9999;
  x1 = 0;
  for (b=1; b<nbands-1; b++) {
    for (r=1; r<nrows-1; r++) {
      for (c=1; c<ncols-1; c++) {
	if (VPixel(src,b,r,c,VBit) > 0) {
	  array[n].b = b;
	  array[n].r = r;
	  array[n].c = c;
	  array[n].val = (float)VPixel(depth_image,b,r,c,VFloat);

  	  if (array[n].val < x0) x0 = array[n].val;
	  if (array[n].val > x1) x1 = array[n].val;

	  n++;
	}
      }
    }
  }
  VPoint_hpsort((n-1),array);
  maxdepth = x1;

  /* copy src to dest */
  dest = VCopyImage(src,dest,VAllBands);


  for (depth = maxdepth; depth >=0 ; depth -= 0.05) {

    ndel = 1;
    while (ndel > 0) {
      ndel = 0;

      for (j=1; j<n; j++) {

	if (array[j].val < depth) continue;
	b = array[j].b;
	r = array[j].r;
	c = array[j].c;
	if (b < 1 || b >= nbands-1) continue;
	if (r < 1 || r >= nrows-1) continue;
	if (c < 1 || c >= ncols-1) continue;

	/* check if already deleted */
	if (VPixel(dest,b,r,c,VBit) == 0) continue;

	/* check if border point */
	if (VPixel(dest,b+1,r,c,VBit) == 0) goto next;
	if (VPixel(dest,b-1,r,c,VBit) == 0) goto next;
	if (VPixel(dest,b,r+1,c,VBit) == 0) goto next;
	if (VPixel(dest,b,r-1,c,VBit) == 0) goto next;
	if (VPixel(dest,b,r,c+1,VBit) == 0) goto next;
	if (VPixel(dest,b,r,c-1,VBit) == 0) goto next;
	continue;

      next:
	/* final point condition */
	if (FinalPoint(dest,b,r,c) < 3) continue;

	/* topological correctness */
	if (VSimplePoint(dest,b,r,c,nadj) == 1) {
	  VPixel(dest,b,r,c,VBit) = 0;  
	  ndel++;
	}
      }
    }
  }

  /* output */
  VCopyImageAttrs (src, dest);
  return dest;
}

