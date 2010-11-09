/*
** convert from axial to sagittal or coronal orientation.
**
*/

#include <stdio.h>
#include <stdlib.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/VImage.h>


#define AXIAL    0 
#define CORONAL  1
#define SAGITTAL 2


VImage
VAxial2Coronal_short(VImage src,VImage dest)
{
  int nbands,nrows,ncols,b,r,c;
  int v=0;

  nbands  = VImageNBands(src);
  nrows   = VImageNRows(src);
  ncols   = VImageNColumns(src);

  dest = VSelectDestImage("VAxial2Coronal_short",dest,nrows,nbands,ncols,VShortRepn);
  if (dest == NULL) VError(" error converting axial to coronal");

  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	v = VPixel(src,b,r,c,VShort);
	VPixel(dest,r,nbands-b-1,c,VShort) = v;
      }
    }
  }
  return dest;
}


VImage
VAxial2Sagittal_short(VImage src,VImage dest)
{
  int nbands,nrows,ncols,b,r,c;
  int v=0;

  nbands  = VImageNBands(src);
  nrows   = VImageNRows(src);
  ncols   = VImageNColumns(src);

  dest = VSelectDestImage("VAxial2Sagittal",dest,nrows,ncols,nbands,VShortRepn);
  if (dest == NULL) VError(" error converting axial to sagittal");

  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	v = VPixel(src,b,r,c,VShort);
	VPixel(dest,r,c,nbands-b-1,VShort) = v;
      }
    }
  }
  return dest;
}



VImage
VAxial2Coronal(VImage src,VImage dest)
{
  int nbands,nrows,ncols,b,r,c;
  VDouble v=0;

  nbands  = VImageNBands(src);
  nrows   = VImageNRows(src);
  ncols   = VImageNColumns(src);

  dest = VSelectDestImage("VAxial2Coronal",dest,nrows,nbands,ncols,VPixelRepn(src));
  if (dest == NULL) VError(" error converting axial to coronal");

  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	v = VGetPixel(src,b,r,c);
	VSetPixel(dest,r,nbands-b-1,c,v);
      }
    }
  }
  return dest;
}


VImage
VAxial2Sagittal(VImage src,VImage dest)
{
  int nbands,nrows,ncols,b,r,c;
  VDouble v=0;

  nbands  = VImageNBands(src);
  nrows   = VImageNRows(src);
  ncols   = VImageNColumns(src);


  dest = VSelectDestImage("VAxial2Sagittal",dest,nrows,ncols,nbands,VPixelRepn(src));
  if (dest == NULL) VError(" error converting axial to sagittal");

  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	v = VGetPixel(src,b,r,c);
	VSetPixel(dest,r,c,nbands-b-1,v);
      }
    }
  }
  return dest;
}
